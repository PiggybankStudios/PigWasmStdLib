/*
File:   math_log_helpers.c
Author: Taylor Robbins
Date:   10\08\2023
Description: 
	** Holds some larger functions that feed implementations for things like log, logf, log2, etc.
*/

// +--------------------------------------------------------------+
// |                    log and logf constants                    |
// +--------------------------------------------------------------+
#define LOGF_TABLE_BITS 4
#define LOGF_POLY_ORDER 4
const struct logf_data
{
	struct
	{
		double invc, logc;
	} tab[1 << LOGF_TABLE_BITS];
	double ln2;
	double poly[LOGF_POLY_ORDER - 1]; /* First order coefficient is 1.  */
} __logf_data =
{
	.tab =
	{
		{ 0x1.661EC79F8F3BEp+0, -0x1.57BF7808CAADEp-2 },
		{ 0x1.571ED4AAF883Dp+0, -0x1.2BEF0A7C06DDBp-2 },
		{ 0x1.49539F0F010Bp+0,  -0x1.01EAE7F513A67p-2 },
		{ 0x1.3C995B0B80385p+0, -0x1.B31D8A68224E9p-3 },
		{ 0x1.30D190C8864A5p+0, -0x1.6574F0AC07758p-3 },
		{ 0x1.25E227B0B8EAp+0,  -0x1.1AA2BC79C81p-3   },
		{ 0x1.1BB4A4A1A343Fp+0, -0x1.A4E76CE8C0E5Ep-4 },
		{ 0x1.12358F08AE5BAp+0, -0x1.1973C5A611CCCp-4 },
		{ 0x1.0953F419900A7p+0, -0x1.252F438E10C1Ep-5 },
		{ 0x1p+0,               0x0p+0                },
		{ 0x1.E608CFD9A47ACp-1, 0x1.AA5AA5DF25984p-5  },
		{ 0x1.CA4B31F026AAp-1,  0x1.C5E53AA362EB4p-4  },
		{ 0x1.B2036576AFCE6p-1, 0x1.526E57720DB08p-3  },
		{ 0x1.9C2D163A1AA2Dp-1, 0x1.BC2860D22477p-3   },
		{ 0x1.886E6037841EDp-1, 0x1.1058BC8A07EE1p-2  },
		{ 0x1.767DCF5534862p-1, 0x1.4043057B6EE09p-2  },
	},
	.ln2 = 0x1.62E42FEFA39EFp-1,
	.poly = { -0x1.00EA348B88334p-2, 0x1.5575B0BE00B6Ap-2, -0x1.FFFFEF20A4123p-2, }
};

#define logf_T __logf_data.tab
#define logf_A __logf_data.poly
#define logf_Ln2 __logf_data.ln2
#define logf_N (1 << LOGF_TABLE_BITS)
#define logf_OFF 0x3F330000

#define LOG_TABLE_BITS 7
#define LOG_POLY_ORDER 6
#define LOG_POLY1_ORDER 12
const struct log_data
{
	double ln2hi;
	double ln2lo;
	double poly[LOG_POLY_ORDER - 1]; /* First coefficient is 1.  */
	double poly1[LOG_POLY1_ORDER - 1];
	struct
	{
		double invc, logc;
	} tab[1 << LOG_TABLE_BITS];
	struct
	{
		double chi, clo;
	} tab2[1 << LOG_TABLE_BITS];
} __log_data =
{
	.ln2hi = 0x1.62E42FEFA3800p-1,
	.ln2lo = 0x1.EF35793C76730p-45,
	.poly1 =
	{
		// relative error: 0x1.c04d76cp-63
		// in -0x1p-4 0x1.09p-4 (|log(1+x)| > 0x1p-4 outside the interval)
		-0x1p-1,
		0x1.5555555555577p-2,
		-0x1.FFFFFFFFFFDCBp-3,
		0x1.999999995DD0Cp-3,
		-0x1.55555556745A7p-3,
		0x1.24924A344DE3p-3,
		-0x1.FFFFFA4423D65p-4,
		0x1.C7184282AD6CAp-4,
		-0x1.999EB43B068FFp-4,
		0x1.78182F7AFD085p-4,
		-0x1.5521375D145CDp-4,
	},
	.poly =
	{
		// relative error: 0x1.926199e8p-56
		// abs error: 0x1.882ff33p-65
		// in -0x1.fp-9 0x1.fp-9
		-0x1.0000000000001p-1,
		0x1.555555551305Bp-2,
		-0x1.FFFFFFFEB459p-3,
		0x1.999B324F10111p-3,
		-0x1.55575E506C89Fp-3,
	},
	/*
	Algorithm:
			x = 2^k z
			log(x) = k ln2 + log(c) + log(z/c)
			log(z/c) = poly(z/c - 1)
		where z is in [1.6p-1; 1.6p0] which is split into N subintervals and z falls
		into the ith one, then table entries are computed as
			tab[i].invc = 1/c
			tab[i].logc = (double)log(c)
			tab2[i].chi = (double)c
			tab2[i].clo = (double)(c - (double)c)
		where c is near the center of the subinterval and is chosen by trying +-2^29
		floating point invc candidates around 1/center and selecting one for which
			1) the rounding error in 0x1.8p9 + logc is 0,
			2) the rounding error in z - chi - clo is < 0x1p-66 and
			3) the rounding error in (double)log(c) is minimized (< 0x1p-66).
		Note: 1) ensures that k*ln2hi + logc can be computed without rounding error,
		2) ensures that z/c - 1 can be computed as (z - chi - clo)*invc with close to
		a single rounding error when there is no fast fma for z*invc - 1, 3) ensures
		that logc + poly(z/c - 1) has small error, however near x == 1 when
		|log(x)| < 0x1p-4, this is not enough so that is special cased.
	*/
	.tab =
	{
		{ 0x1.734F0C3E0DE9Fp+0, -0x1.7CC7F79E69000p-2 },
		{ 0x1.713786A2CE91Fp+0, -0x1.76FEEC20D0000p-2 },
		{ 0x1.6F26008FAB5A0p+0, -0x1.713E31351E000p-2 },
		{ 0x1.6D1A61F138C7Dp+0, -0x1.6B85B38287800p-2 },
		{ 0x1.6B1490BC5B4D1p+0, -0x1.65D5590807800p-2 },
		{ 0x1.69147332F0CBAp+0, -0x1.602D076180000p-2 },
		{ 0x1.6719F18224223p+0, -0x1.5A8CA86909000p-2 },
		{ 0x1.6524F99A51ED9p+0, -0x1.54F4356035000p-2 },
		{ 0x1.63356AA8F24C4p+0, -0x1.4F637C36B4000p-2 },
		{ 0x1.614B36B9DDC14p+0, -0x1.49DA7FDA85000p-2 },
		{ 0x1.5F66452C65C4Cp+0, -0x1.445923989A800p-2 },
		{ 0x1.5D867B5912C4Fp+0, -0x1.3EDF439B0B800p-2 },
		{ 0x1.5BABCCB5B90DEp+0, -0x1.396CE448F7000p-2 },
		{ 0x1.59D61F2D91A78p+0, -0x1.3401E17BDA000p-2 },
		{ 0x1.5805612465687p+0, -0x1.2E9E2EF468000p-2 },
		{ 0x1.56397CEE76BD3p+0, -0x1.2941B3830E000p-2 },
		{ 0x1.54725E2A77F93p+0, -0x1.23EC58CDA8800p-2 },
		{ 0x1.52AFF42064583p+0, -0x1.1E9E129279000p-2 },
		{ 0x1.50F22DBB2BDDFp+0, -0x1.1956D2B48F800p-2 },
		{ 0x1.4F38F4734DED7p+0, -0x1.141679AB9F800p-2 },
		{ 0x1.4D843CFDE2840p+0, -0x1.0EDD094EF9800p-2 },
		{ 0x1.4BD3EC078A3C8p+0, -0x1.09AA518DB1000p-2 },
		{ 0x1.4A27FC3E0258Ap+0, -0x1.047E65263B800p-2 },
		{ 0x1.4880524D48434p+0, -0x1.FEB224586F000p-3 },
		{ 0x1.46DCE1B192D0Bp+0, -0x1.F474A7517B000p-3 },
		{ 0x1.453D9D3391854p+0, -0x1.EA4443D103000p-3 },
		{ 0x1.43A2744B4845Ap+0, -0x1.E020D44E9B000p-3 },
		{ 0x1.420B54115F8FBp+0, -0x1.D60A22977F000p-3 },
		{ 0x1.40782DA3EF4B1p+0, -0x1.CC00104959000p-3 },
		{ 0x1.3EE8F5D57FE8Fp+0, -0x1.C202956891000p-3 },
		{ 0x1.3D5D9A00B4CE9p+0, -0x1.B81178D811000p-3 },
		{ 0x1.3BD60C010C12Bp+0, -0x1.AE2C9CCD3D000p-3 },
		{ 0x1.3A5242B75DAB8p+0, -0x1.A45402E129000p-3 },
		{ 0x1.38D22CD9FD002p+0, -0x1.9A877681DF000p-3 },
		{ 0x1.3755BC5847A1Cp+0, -0x1.90C6D69483000p-3 },
		{ 0x1.35DCE49AD36E2p+0, -0x1.87120A645C000p-3 },
		{ 0x1.34679984DD440p+0, -0x1.7D68FB4143000p-3 },
		{ 0x1.32F5CCEFFCB24p+0, -0x1.73CB83C627000p-3 },
		{ 0x1.3187775A10D49p+0, -0x1.6A39A9B376000p-3 },
		{ 0x1.301C8373E3990p+0, -0x1.60B3154B7A000p-3 },
		{ 0x1.2EB4EBB95F841p+0, -0x1.5737D76243000p-3 },
		{ 0x1.2D50A0219A9D1p+0, -0x1.4DC7B8FC23000p-3 },
		{ 0x1.2BEF9A8B7FD2Ap+0, -0x1.4462C51D20000p-3 },
		{ 0x1.2A91C7A0C1BABp+0, -0x1.3B08ABC830000p-3 },
		{ 0x1.293726014B530p+0, -0x1.31B996B490000p-3 },
		{ 0x1.27DFA5757A1F5p+0, -0x1.2875490A44000p-3 },
		{ 0x1.268B39B1D3BBFp+0, -0x1.1F3B9F879A000p-3 },
		{ 0x1.2539D838FF5BDp+0, -0x1.160C8252CA000p-3 },
		{ 0x1.23EB7AAC9083Bp+0, -0x1.0CE7F57F72000p-3 },
		{ 0x1.22A012BA940B6p+0, -0x1.03CDC49FEA000p-3 },
		{ 0x1.2157996CC4132p+0, -0x1.F57BDBC4B8000p-4 },
		{ 0x1.201201DD2FC9Bp+0, -0x1.E370896404000p-4 },
		{ 0x1.1ECF4494D480Bp+0, -0x1.D17983EF94000p-4 },
		{ 0x1.1D8F5528F6569p+0, -0x1.BF9674ED8A000p-4 },
		{ 0x1.1C52311577E7Cp+0, -0x1.ADC79202F6000p-4 },
		{ 0x1.1B17C74CB26E9p+0, -0x1.9C0C3E7288000p-4 },
		{ 0x1.19E010C2C1AB6p+0, -0x1.8A646B372C000p-4 },
		{ 0x1.18AB07BB670BDp+0, -0x1.78D01B3AC0000p-4 },
		{ 0x1.1778A25EFBCB6p+0, -0x1.674F145380000p-4 },
		{ 0x1.1648D354C31DAp+0, -0x1.55E0E6D878000p-4 },
		{ 0x1.151B990275FDDp+0, -0x1.4485CDEA1E000p-4 },
		{ 0x1.13F0EA432D24Cp+0, -0x1.333D94D6AA000p-4 },
		{ 0x1.12C8B7210F9DAp+0, -0x1.22079F8C56000p-4 },
		{ 0x1.11A3028ECB531p+0, -0x1.10E4698622000p-4 },
		{ 0x1.107FBDA8434AFp+0, -0x1.FFA6C6AD20000p-5 },
		{ 0x1.0F5EE0F4E6BB3p+0, -0x1.DDA8D4A774000p-5 },
		{ 0x1.0E4065D2A9FCEp+0, -0x1.BBCECE4850000p-5 },
		{ 0x1.0D244632CA521p+0, -0x1.9A1894012C000p-5 },
		{ 0x1.0C0A77CE2981Ap+0, -0x1.788583302C000p-5 },
		{ 0x1.0AF2F83C636D1p+0, -0x1.5715E67D68000p-5 },
		{ 0x1.09DDB98A01339p+0, -0x1.35C8A49658000p-5 },
		{ 0x1.08CABAF52E7DFp+0, -0x1.149E364154000p-5 },
		{ 0x1.07B9F2F4E28FBp+0, -0x1.E72C082EB8000p-6 },
		{ 0x1.06AB58C358F19p+0, -0x1.A55F152528000p-6 },
		{ 0x1.059EEA5ECF92Cp+0, -0x1.63D62CF818000p-6 },
		{ 0x1.04949CDD12C90p+0, -0x1.228FB8CAA0000p-6 },
		{ 0x1.038C6C6F0ADA9p+0, -0x1.C317B20F90000p-7 },
		{ 0x1.02865137932A9p+0, -0x1.419355DAA0000p-7 },
		{ 0x1.0182427EA7348p+0, -0x1.81203C2EC0000p-8 },
		{ 0x1.008040614B195p+0, -0x1.0040979240000p-9 },
		{ 0x1.FE01FF726FA1Ap-1, 0x1.FEFF384900000p-9 },
		{ 0x1.FA11CC261EA74p-1, 0x1.7DC41353D0000p-7 },
		{ 0x1.F6310B081992Ep-1, 0x1.3CEA3C4C28000p-6 },
		{ 0x1.F25F63CEEADCDp-1, 0x1.B9FC114890000p-6 },
		{ 0x1.EE9C8039113E7p-1, 0x1.1B0D8CE110000p-5 },
		{ 0x1.EAE8078CBB1ABp-1, 0x1.58A5BD001C000p-5 },
		{ 0x1.E741AA29D0C9Bp-1, 0x1.95C8340D88000p-5 },
		{ 0x1.E3A91830A99B5p-1, 0x1.D276AEF578000p-5 },
		{ 0x1.E01E009609A56p-1, 0x1.07598E598C000p-4 },
		{ 0x1.DCA01E577BB98p-1, 0x1.253F5E30D2000p-4 },
		{ 0x1.D92F20B7C9103p-1, 0x1.42EDD8B380000p-4 },
		{ 0x1.D5CAC66FB5CCEp-1, 0x1.606598757C000p-4 },
		{ 0x1.D272CAA5EDE9Dp-1, 0x1.7DA76356A0000p-4 },
		{ 0x1.CF26E3E6B2CCDp-1, 0x1.9AB434E1C6000p-4 },
		{ 0x1.CBE6DA2A77902p-1, 0x1.B78C7BB0D6000p-4 },
		{ 0x1.C8B266D37086Dp-1, 0x1.D431332E72000p-4 },
		{ 0x1.C5894BD5D5804p-1, 0x1.F0A3171DE6000p-4 },
		{ 0x1.C26B533BB9F8Cp-1, 0x1.067152B914000p-3 },
		{ 0x1.BF583EEECE73Fp-1, 0x1.147858292B000p-3 },
		{ 0x1.BC4FD75DB96C1p-1, 0x1.2266ECDCA3000p-3 },
		{ 0x1.B951E0C864A28p-1, 0x1.303D7A6C55000p-3 },
		{ 0x1.B65E2C5EF3E2Cp-1, 0x1.3DFC33C331000p-3 },
		{ 0x1.B374867C9888Bp-1, 0x1.4BA366B7A8000p-3 },
		{ 0x1.B094B211D304Ap-1, 0x1.5933928D1F000p-3 },
		{ 0x1.ADBE885F2EF7Ep-1, 0x1.66ACD2418F000p-3 },
		{ 0x1.AAF1D31603DA2p-1, 0x1.740F8EC669000p-3 },
		{ 0x1.A82E63FD358A7p-1, 0x1.815C0F51AF000p-3 },
		{ 0x1.A5740EF09738Bp-1, 0x1.8E92954F68000p-3 },
		{ 0x1.A2C2A90AB4B27p-1, 0x1.9BB3602F84000p-3 },
		{ 0x1.A01A01393F2D1p-1, 0x1.A8BED1C2C0000p-3 },
		{ 0x1.9D79F24DB3C1Bp-1, 0x1.B5B515C01D000p-3 },
		{ 0x1.9AE2505C7B190p-1, 0x1.C2967CCBCC000p-3 },
		{ 0x1.9852EF297CE2Fp-1, 0x1.CF635D5486000p-3 },
		{ 0x1.95CBAEEA44B75p-1, 0x1.DC1BD3446C000p-3 },
		{ 0x1.934C69DE74838p-1, 0x1.E8C01B8CFE000p-3 },
		{ 0x1.90D4F2F6752E6p-1, 0x1.F5509C0179000p-3 },
		{ 0x1.8E6528EFFD79Dp-1, 0x1.00E6C121FB800p-2 },
		{ 0x1.8BFCE9FCC007Cp-1, 0x1.071B80E93D000p-2 },
		{ 0x1.899C0DABEC30Ep-1, 0x1.0D46B9E867000p-2 },
		{ 0x1.87427AA2317FBp-1, 0x1.13687334BD000p-2 },
		{ 0x1.84F00ACB39A08p-1, 0x1.1980D67234800p-2 },
		{ 0x1.82A49E8653E55p-1, 0x1.1F8FFE0CC8000p-2 },
		{ 0x1.8060195F40260p-1, 0x1.2595FD7636800p-2 },
		{ 0x1.7E22563E0A329p-1, 0x1.2B9300914A800p-2 },
		{ 0x1.7BEB377DCB5ADp-1, 0x1.3187210436000p-2 },
		{ 0x1.79BAA679725C2p-1, 0x1.377266DEC1800p-2 },
		{ 0x1.77907F2170657p-1, 0x1.3D54FFBAF3000p-2 },
		{ 0x1.756CADBD6130Cp-1, 0x1.432EEE32FE000p-2 },
	},
	.tab2 =
	{
		{ 0x1.61000014FB66Bp-1,  0x1.E026C91425B3Cp-56 },
		{ 0x1.63000034DB495p-1,  0x1.DBFEA48005D41p-55 },
		{ 0x1.650000D94D478p-1,  0x1.E7FA786D6A5B7p-55 },
		{ 0x1.67000074E6FADp-1,  0x1.1FCEA6B54254Cp-57 },
		{ 0x1.68FFFFEDF0FAEp-1, -0x1.C7E274C590EFDp-56 },
		{ 0x1.6B0000763C5BCp-1, -0x1.AC16848DCDA01p-55 },
		{ 0x1.6D0001E5CC1F6p-1,  0x1.33F1C9D499311p-55 },
		{ 0x1.6EFFFEB05F63Ep-1, -0x1.E80041AE22D53p-56 },
		{ 0x1.710000E86978p-1,   0x1.BFF6671097952p-56 },
		{ 0x1.72FFFFC67E912p-1,  0x1.C00E226BD8724p-55 },
		{ 0x1.74FFFDF81116Ap-1, -0x1.E02916EF101D2p-57 },
		{ 0x1.770000F679C9p-1,  -0x1.7FC71CD549C74p-57 },
		{ 0x1.78FFFFA7EC835p-1,  0x1.1BEC19EF50483p-55 },
		{ 0x1.7AFFFFE20C2E6p-1, -0x1.07E1729CC6465p-56 },
		{ 0x1.7CFFFED3FC9p-1,   -0x1.08072087B8B1Cp-55 },
		{ 0x1.7EFFFE9261A76p-1,  0x1.DC0286D9DF9AEp-55 },
		{ 0x1.81000049CA3E8p-1,  0x1.97FD251E54C33p-55 },
		{ 0x1.8300017932C8Fp-1, -0x1.AFEE9B630F381p-55 },
		{ 0x1.850000633739Cp-1,  0x1.9BFBF6B6535BCp-55 },
		{ 0x1.87000204289C6p-1, -0x1.BBF65F3117B75p-55 },
		{ 0x1.88FFFEBF57904p-1, -0x1.9006EA23DCB57p-55 },
		{ 0x1.8B00022BC04DFp-1, -0x1.D00DF38E04B0Ap-56 },
		{ 0x1.8CFFFE50C1B8Ap-1, -0x1.8007146FF9F05p-55 },
		{ 0x1.8EFFFFC918E43p-1,  0x1.3817BD07A7038p-55 },
		{ 0x1.910001EFA5FC7p-1,  0x1.93E9176DFB403p-55 },
		{ 0x1.9300013467BB9p-1,  0x1.F804E4B980276p-56 },
		{ 0x1.94FFFE6EE076Fp-1, -0x1.F7EF0D9FF622Ep-55 },
		{ 0x1.96FFFDE3C12D1p-1, -0x1.082AA962638BAp-56 },
		{ 0x1.98FFFF4458A0Dp-1, -0x1.7801B9164A8EFp-55 },
		{ 0x1.9AFFFDD982E3Ep-1, -0x1.740E08A5A9337p-55 },
		{ 0x1.9CFFFED49FB66p-1,  0x1.FCE08C19BEp-60    },
		{ 0x1.9F00020F19C51p-1, -0x1.A3FAA27885B0Ap-55 },
		{ 0x1.A10001145B006p-1,  0x1.4FF489958DA56p-56 },
		{ 0x1.A300007BBF6FAp-1,  0x1.CBEAB8A2B6D18p-55 },
		{ 0x1.A500010971D79p-1,  0x1.8FECADD78793p-55  },
		{ 0x1.A70001DF52E48p-1, -0x1.F41763DD8ABDBp-55 },
		{ 0x1.A90001C593352p-1, -0x1.EBF0284C27612p-55 },
		{ 0x1.AB0002A4F3E4Bp-1, -0x1.9FD043CFF3F5Fp-57 },
		{ 0x1.ACFFFD7AE1ED1p-1, -0x1.23EE7129070B4p-55 },
		{ 0x1.AEFFFEE510478p-1,  0x1.A063EE00EDEA3p-57 },
		{ 0x1.B0FFFDB650D5Bp-1,  0x1.A06C8381F0AB9p-58 },
		{ 0x1.B2FFFFEAACA57p-1, -0x1.9011E74233C1Dp-56 },
		{ 0x1.B4FFFD995BADCp-1, -0x1.9FF1068862A9Fp-56 },
		{ 0x1.B7000249E659Cp-1,  0x1.AFF45D0864F3Ep-55 },
		{ 0x1.B8FFFF987164p-1,   0x1.CFE7796C2C3F9p-56 },
		{ 0x1.BAFFFD204CB4Fp-1, -0x1.3FF27EEF22BC4p-57 },
		{ 0x1.BCFFFD2415C45p-1, -0x1.CFFB7EE3BEA21p-57 },
		{ 0x1.BEFFFF86309DFp-1, -0x1.14103972E0B5Cp-55 },
		{ 0x1.C0FFFE1B57653p-1,  0x1.BC16494B76A19p-55 },
		{ 0x1.C2FFFF1FA57E3p-1, -0x1.4FEEF8D30C6EDp-57 },
		{ 0x1.C4FFFDCBFE424p-1, -0x1.43F68BCEC4775p-55 },
		{ 0x1.C6FFFED54B9F7p-1,  0x1.47EA3F053E0ECp-55 },
		{ 0x1.C8FFFEB998FD5p-1,  0x1.383068DF992F1p-56 },
		{ 0x1.CB0002125219Ap-1, -0x1.8FD8E64180E04p-57 },
		{ 0x1.CCFFFDD94469Cp-1,  0x1.E7EBE1CC7EA72p-55 },
		{ 0x1.CEFFFEAFDC476p-1,  0x1.EBE39AD9F88FEp-55 },
		{ 0x1.D1000169AF82Bp-1,  0x1.57D91A8B95A71p-56 },
		{ 0x1.D30000D0FF71Dp-1,  0x1.9C1906970C7DAp-55 },
		{ 0x1.D4FFFEA790FC4p-1, -0x1.80E37C558FE0Cp-58 },
		{ 0x1.D70002EDC87E5p-1, -0x1.F80D64DC10F44p-56 },
		{ 0x1.D900021DC82AAp-1, -0x1.47C8F94FD5C5Cp-56 },
		{ 0x1.DAFFFD86B0283p-1,  0x1.C7F1DC521617Ep-55 },
		{ 0x1.DD000296C4739p-1,  0x1.8019EB2FFB153p-55 },
		{ 0x1.DEFFFE54490F5p-1,  0x1.E00D2C652CC89p-57 },
		{ 0x1.E0FFFCDABF694p-1, -0x1.F8340202D69D2p-56 },
		{ 0x1.E2FFFDB52C8DDp-1,  0x1.B00C1CA1B0864p-56 },
		{ 0x1.E4FFFF24216EFp-1,  0x1.2FFA8B094AB51p-56 },
		{ 0x1.E6FFFE88A5E11p-1, -0x1.7F673B1EFBE59p-58 },
		{ 0x1.E9000119EFF0Dp-1, -0x1.4808D5E0BC801p-55 },
		{ 0x1.EAFFFDFA51744p-1,  0x1.80006D54320B5p-56 },
		{ 0x1.ED0001A127FA1p-1, -0x1.002F860565C92p-58 },
		{ 0x1.EF00007BABCC4p-1, -0x1.540445D35E611p-55 },
		{ 0x1.F0FFFF57A8D02p-1, -0x1.FFB3139EF9105p-59 },
		{ 0x1.F30001EE58AC7p-1,  0x1.A81ACF2731155p-55 },
		{ 0x1.F4FFFF5823494p-1,  0x1.A3F41D4D7C743p-55 },
		{ 0x1.F6FFFFCA94C6Bp-1, -0x1.202F41C987875p-57 },
		{ 0x1.F8FFFE1F9C441p-1,  0x1.77DD1F477E74Bp-56 },
		{ 0x1.FAFFFD2E0E37Ep-1, -0x1.F01199A7CA331p-57 },
		{ 0x1.FD0001C77E49Ep-1,  0x1.181EE4BCEACB1p-56 },
		{ 0x1.FEFFFF7E0C331p-1, -0x1.E05370170875Ap-57 },
		{ 0x1.00FFFF465606Ep+0, -0x1.A7EAD491C0ADAp-55 },
		{ 0x1.02FFFF3867A58p+0, -0x1.77F69C3FCB2Ep-54  },
		{ 0x1.04FFFFDFC0D17p+0,  0x1.7BFFE34CB945Bp-54 },
		{ 0x1.0700003CD4D82p+0,  0x1.20083C0E456CBp-55 },
		{ 0x1.08FFFF9F2CBE8p+0, -0x1.DFFDFBE37751Ap-57 },
		{ 0x1.0B000010CDA65p+0, -0x1.13F7FAEE626EBp-54 },
		{ 0x1.0D00001A4D338p+0,  0x1.07DFA79489FF7p-55 },
		{ 0x1.0EFFFFADAFDFDp+0, -0x1.7040570D66BCp-56  },
		{ 0x1.110000BBAFD96p+0,  0x1.E80D4846D0B62p-55 },
		{ 0x1.12FFFFAE5F45Dp+0,  0x1.DBFFA64FD36EFp-54 },
		{ 0x1.150000DD59AD9p+0,  0x1.A0077701250AEp-54 },
		{ 0x1.170000F21559Ap+0,  0x1.DFDF9E2E3DEEEp-55 },
		{ 0x1.18FFFFC275426p+0,  0x1.10030DC3B7273p-54 },
		{ 0x1.1B000123D3C59p+0,  0x1.97F7980030188p-54 },
		{ 0x1.1CFFFF8299EB7p+0, -0x1.5F932AB9F8C67p-57 },
		{ 0x1.1EFFFF48AD4p+0,    0x1.37FBF9DA75BEBp-54 },
		{ 0x1.210000C8B86A4p+0,  0x1.F806B91FD5B22p-54 },
		{ 0x1.2300003854303p+0,  0x1.3FFC2EB9FBF33p-54 },
		{ 0x1.24FFFFFBCF684p+0,  0x1.601E77E2E2E72p-56 },
		{ 0x1.26FFFF52921D9p+0,  0x1.FFCBB767F0C61p-56 },
		{ 0x1.2900014933A3Cp+0, -0x1.202CA3C02412Bp-56 },
		{ 0x1.2B00014556313p+0, -0x1.2808233F21F02p-54 },
		{ 0x1.2CFFFEBFE523Bp+0, -0x1.8FF7E384FDCF2p-55 },
		{ 0x1.2F0000BB8AD96p+0, -0x1.5FF51503041C5p-55 },
		{ 0x1.30FFFFB7AE2AFp+0, -0x1.10071885E289Dp-55 },
		{ 0x1.32FFFFEAC5F7Fp+0, -0x1.1FF5D3FB7B715p-54 },
		{ 0x1.350000CA66756p+0,  0x1.57F82228B82BDp-54 },
		{ 0x1.3700011FBF721p+0,  0x1.000BAC40DD5CCp-55 },
		{ 0x1.38FFFF9592FB9p+0, -0x1.43F9D2DB2A751p-54 },
		{ 0x1.3B00004DDD242p+0,  0x1.57F6B707638E1p-55 },
		{ 0x1.3CFFFF5B2C957p+0,  0x1.A023A10BF1231p-56 },
		{ 0x1.3EFFFEAB0B418p+0,  0x1.87F6D66B152Bp-54  },
		{ 0x1.410001532AFF4p+0,  0x1.7F8375F198524p-57 },
		{ 0x1.4300017478B29p+0,  0x1.301E672DC5143p-55 },
		{ 0x1.44FFFE795B463p+0,  0x1.9FF69B8B2895Ap-55 },
		{ 0x1.46FFFE80475Ep+0,  -0x1.5C0B19BC2F254p-54 },
		{ 0x1.48FFFEF6FC1E7p+0,  0x1.B4009F23A2A72p-54 },
		{ 0x1.4AFFFE5BEA704p+0, -0x1.4FFB7BF0D7D45p-54 },
		{ 0x1.4D000171027DEp+0, -0x1.9C06471DC6A3Dp-54 },
		{ 0x1.4F0000FF03EE2p+0,  0x1.77F890B85531Cp-54 },
		{ 0x1.5100012DC4BD1p+0,  0x1.004657166A436p-57 },
		{ 0x1.530001605277Ap+0, -0x1.6BFCECE233209p-54 },
		{ 0x1.54FFFECDB704Cp+0, -0x1.902720505A1D7p-55 },
		{ 0x1.56FFFEF5F54A9p+0,  0x1.BBFE60EC96412p-54 },
		{ 0x1.5900017E61012p+0,  0x1.87EC581AFEF9p-55  },
		{ 0x1.5B00003C93E92p+0, -0x1.F41080ABF0CCp-54  },
		{ 0x1.5D0001D4919BCp+0, -0x1.8812AFB254729p-54 },
		{ 0x1.5EFFFE7B87A89p+0, -0x1.47EB780ED6904p-54 },
	},
};

#define log_T __log_data.tab
#define log_T2 __log_data.tab2
#define log_B __log_data.poly1
#define log_A __log_data.poly
#define log_Ln2hi __log_data.ln2hi
#define log_Ln2lo __log_data.ln2lo
#define log_N (1 << LOG_TABLE_BITS)
#define log_OFF 0x3fe6000000000000

// +--------------------------------------------------------------+
// |                   log2 and log2f constants                   |
// +--------------------------------------------------------------+
#define LOG2F_TABLE_BITS 4
#define LOG2F_POLY_ORDER 4
const struct log2f_data
{
	struct
	{
		double invc, logc;
	} tab[1 << LOG2F_TABLE_BITS];
	double poly[LOG2F_POLY_ORDER];
} __log2f_data =
{
  .tab =
  {
	  { 0x1.661EC79F8F3BEp+0, -0x1.EFEC65B963019p-2 },
	  { 0x1.571ED4AAF883Dp+0, -0x1.B0B6832D4FCA4p-2 },
	  { 0x1.49539F0F010Bp+0,  -0x1.7418B0A1FB77Bp-2 },
	  { 0x1.3C995B0B80385p+0, -0x1.39DE91A6DCF7Bp-2 },
	  { 0x1.30D190C8864A5p+0, -0x1.01D9BF3F2B631p-2 },
	  { 0x1.25E227B0B8EAp+0,  -0x1.97C1D1B3B7AFp-3  },
	  { 0x1.1BB4A4A1A343Fp+0, -0x1.2F9E393AF3C9Fp-3 },
	  { 0x1.12358F08AE5BAp+0, -0x1.960CBBF788D5Cp-4 },
	  { 0x1.0953F419900A7p+0, -0x1.A6F9DB6475FCEp-5 },
	  { 0x1p+0,                0x0p+0 },
	  { 0x1.E608CFD9A47ACp-1,  0x1.338CA9F24F53Dp-4 },
	  { 0x1.CA4B31F026AAp-1,   0x1.476A9543891BAp-3 },
	  { 0x1.B2036576AFCE6p-1,  0x1.E840B4AC4E4D2p-3 },
	  { 0x1.9C2D163A1AA2Dp-1,  0x1.40645F0C6651Cp-2 },
	  { 0x1.886E6037841EDp-1,  0x1.88E9C2C1B9FF8p-2 },
	  { 0x1.767DCF5534862p-1,  0x1.CE0A44EB17BCCp-2 },
  },
  .poly =
  {
	  -0x1.712B6F70A7E4Dp-2, 0x1.ECABF496832Ep-2, -0x1.715479FFAE3DEp-1, 0x1.715475F35C8B8p0,
  }
};

#define log2f_N    (1 << LOG2F_TABLE_BITS)
#define log2f_T    __log2f_data.tab
#define log2f_A    __log2f_data.poly
#define log2f_OFF  0x3f330000

#define LOG2_TABLE_BITS 6
#define LOG2_POLY_ORDER 7
#define LOG2_POLY1_ORDER 11
const struct log2_data
{
	double invln2hi;
	double invln2lo;
	double poly[LOG2_POLY_ORDER - 1];
	double poly1[LOG2_POLY1_ORDER - 1];
	struct
	{
		double invc, logc;
	} tab[1 << LOG2_TABLE_BITS];
	struct
	{
		double chi, clo;
	} tab2[1 << LOG2_TABLE_BITS];
} __log2_data =
{
	// First coefficient: 0x1.71547652b82fe1777d0ffda0d24p0
	.invln2hi = 0x1.7154765200000p+0,
	.invln2lo = 0x1.705fc2eefa200p-33,
	.poly1 =
	{
		// relative error: 0x1.2fad8188p-63
		// in -0x1.5b51p-5 0x1.6ab2p-5
		-0x1.71547652b82fep-1,
		0x1.ec709dc3a03f7p-2,
		-0x1.71547652b7c3fp-2,
		0x1.2776c50f05be4p-2,
		-0x1.ec709dd768fe5p-3,
		0x1.a61761ec4e736p-3,
		-0x1.7153fbc64a79bp-3,
		0x1.484d154f01b4ap-3,
		-0x1.289e4a72c383cp-3,
		0x1.0b32f285aee66p-3,
	},
	.poly =
	{
		// relative error: 0x1.a72c2bf8p-58
		// abs error: 0x1.67a552c8p-66
		// in -0x1.f45p-8 0x1.f45p-8
		-0x1.71547652b8339p-1,
		0x1.ec709dc3a04bep-2,
		-0x1.7154764702ffbp-2,
		0x1.2776c50034c48p-2,
		-0x1.ec7b328ea92bcp-3,
		0x1.a6225e117f92ep-3,
	},
	/*
	Algorithm:
			x = 2^k z
			log2(x) = k + log2(c) + log2(z/c)
			log2(z/c) = poly(z/c - 1)
		where z is in [1.6p-1; 1.6p0] which is split into N subintervals and z falls
		into the ith one, then table entries are computed as
			tab[i].invc = 1/c
			tab[i].logc = (double)log2(c)
			tab2[i].chi = (double)c
			tab2[i].clo = (double)(c - (double)c)
		where c is near the center of the subinterval and is chosen by trying +-2^29
		floating point invc candidates around 1/center and selecting one for which
			1) the rounding error in 0x1.8p10 + logc is 0,
			2) the rounding error in z - chi - clo is < 0x1p-64 and
			3) the rounding error in (double)log2(c) is minimized (< 0x1p-68).
		Note: 1) ensures that k + logc can be computed without rounding error, 2)
		ensures that z/c - 1 can be computed as (z - chi - clo)*invc with close to a
		single rounding error when there is no fast fma for z*invc - 1, 3) ensures
		that logc + poly(z/c - 1) has small error, however near x == 1 when
		|log2(x)| < 0x1p-4, this is not enough so that is special cased.
	*/
	.tab =
	{
		{0x1.724286bb1acf8p+0, -0x1.1095feecdb000p-1},
		{0x1.6e1f766d2cca1p+0, -0x1.08494bd76d000p-1},
		{0x1.6a13d0e30d48ap+0, -0x1.00143aee8f800p-1},
		{0x1.661ec32d06c85p+0, -0x1.efec5360b4000p-2},
		{0x1.623fa951198f8p+0, -0x1.dfdd91ab7e000p-2},
		{0x1.5e75ba4cf026cp+0, -0x1.cffae0cc79000p-2},
		{0x1.5ac055a214fb8p+0, -0x1.c043811fda000p-2},
		{0x1.571ed0f166e1ep+0, -0x1.b0b67323ae000p-2},
		{0x1.53909590bf835p+0, -0x1.a152f5a2db000p-2},
		{0x1.5014fed61adddp+0, -0x1.9217f5af86000p-2},
		{0x1.4cab88e487bd0p+0, -0x1.8304db0719000p-2},
		{0x1.49539b4334feep+0, -0x1.74189f9a9e000p-2},
		{0x1.460cbdfafd569p+0, -0x1.6552bb5199000p-2},
		{0x1.42d664ee4b953p+0, -0x1.56b23a29b1000p-2},
		{0x1.3fb01111dd8a6p+0, -0x1.483650f5fa000p-2},
		{0x1.3c995b70c5836p+0, -0x1.39de937f6a000p-2},
		{0x1.3991c4ab6fd4ap+0, -0x1.2baa1538d6000p-2},
		{0x1.3698e0ce099b5p+0, -0x1.1d98340ca4000p-2},
		{0x1.33ae48213e7b2p+0, -0x1.0fa853a40e000p-2},
		{0x1.30d191985bdb1p+0, -0x1.01d9c32e73000p-2},
		{0x1.2e025cab271d7p+0, -0x1.e857da2fa6000p-3},
		{0x1.2b404cf13cd82p+0, -0x1.cd3c8633d8000p-3},
		{0x1.288b02c7ccb50p+0, -0x1.b26034c14a000p-3},
		{0x1.25e2263944de5p+0, -0x1.97c1c2f4fe000p-3},
		{0x1.234563d8615b1p+0, -0x1.7d6023f800000p-3},
		{0x1.20b46e33eaf38p+0, -0x1.633a71a05e000p-3},
		{0x1.1e2eefdcda3ddp+0, -0x1.494f5e9570000p-3},
		{0x1.1bb4a580b3930p+0, -0x1.2f9e424e0a000p-3},
		{0x1.19453847f2200p+0, -0x1.162595afdc000p-3},
		{0x1.16e06c0d5d73cp+0, -0x1.f9c9a75bd8000p-4},
		{0x1.1485f47b7e4c2p+0, -0x1.c7b575bf9c000p-4},
		{0x1.12358ad0085d1p+0, -0x1.960c60ff48000p-4},
		{0x1.0fef00f532227p+0, -0x1.64ce247b60000p-4},
		{0x1.0db2077d03a8fp+0, -0x1.33f78b2014000p-4},
		{0x1.0b7e6d65980d9p+0, -0x1.0387d1a42c000p-4},
		{0x1.0953efe7b408dp+0, -0x1.a6f9208b50000p-5},
		{0x1.07325cac53b83p+0, -0x1.47a954f770000p-5},
		{0x1.05197e40d1b5cp+0, -0x1.d23a8c50c0000p-6},
		{0x1.03091c1208ea2p+0, -0x1.16a2629780000p-6},
		{0x1.0101025b37e21p+0, -0x1.720f8d8e80000p-8},
		{0x1.fc07ef9caa76bp-1, 0x1.6fe53b1500000p-7},
		{0x1.f4465d3f6f184p-1, 0x1.11ccce10f8000p-5},
		{0x1.ecc079f84107fp-1, 0x1.c4dfc8c8b8000p-5},
		{0x1.e573a99975ae8p-1, 0x1.3aa321e574000p-4},
		{0x1.de5d6f0bd3de6p-1, 0x1.918a0d08b8000p-4},
		{0x1.d77b681ff38b3p-1, 0x1.e72e9da044000p-4},
		{0x1.d0cb5724de943p-1, 0x1.1dcd2507f6000p-3},
		{0x1.ca4b2dc0e7563p-1, 0x1.476ab03dea000p-3},
		{0x1.c3f8ee8d6cb51p-1, 0x1.7074377e22000p-3},
		{0x1.bdd2b4f020c4cp-1, 0x1.98ede8ba94000p-3},
		{0x1.b7d6c006015cap-1, 0x1.c0db86ad2e000p-3},
		{0x1.b20366e2e338fp-1, 0x1.e840aafcee000p-3},
		{0x1.ac57026295039p-1, 0x1.0790ab4678000p-2},
		{0x1.a6d01bc2731ddp-1, 0x1.1ac056801c000p-2},
		{0x1.a16d3bc3ff18bp-1, 0x1.2db11d4fee000p-2},
		{0x1.9c2d14967feadp-1, 0x1.406464ec58000p-2},
		{0x1.970e4f47c9902p-1, 0x1.52dbe093af000p-2},
		{0x1.920fb3982bcf2p-1, 0x1.651902050d000p-2},
		{0x1.8d30187f759f1p-1, 0x1.771d2cdeaf000p-2},
		{0x1.886e5ebb9f66dp-1, 0x1.88e9c857d9000p-2},
		{0x1.83c97b658b994p-1, 0x1.9a80155e16000p-2},
		{0x1.7f405ffc61022p-1, 0x1.abe186ed3d000p-2},
		{0x1.7ad22181415cap-1, 0x1.bd0f2aea0e000p-2},
		{0x1.767dcf99eff8cp-1, 0x1.ce0a43dbf4000p-2},
		},
	.tab2 =
	{
		{0x1.6200012b90a8ep-1, 0x1.904ab0644b605p-55},
		{0x1.66000045734a6p-1, 0x1.1ff9bea62f7a9p-57},
		{0x1.69fffc325f2c5p-1, 0x1.27ecfcb3c90bap-55},
		{0x1.6e00038b95a04p-1, 0x1.8ff8856739326p-55},
		{0x1.71fffe09994e3p-1, 0x1.afd40275f82b1p-55},
		{0x1.7600015590e1p-1, -0x1.2fd75b4238341p-56},
		{0x1.7a00012655bd5p-1, 0x1.808e67c242b76p-56},
		{0x1.7e0003259e9a6p-1, -0x1.208e426f622b7p-57},
		{0x1.81fffedb4b2d2p-1, -0x1.402461ea5c92fp-55},
		{0x1.860002dfafcc3p-1, 0x1.df7f4a2f29a1fp-57},
		{0x1.89ffff78c6b5p-1, -0x1.e0453094995fdp-55},
		{0x1.8e00039671566p-1, -0x1.a04f3bec77b45p-55},
		{0x1.91fffe2bf1745p-1, -0x1.7fa34400e203cp-56},
		{0x1.95fffcc5c9fd1p-1, -0x1.6ff8005a0695dp-56},
		{0x1.9a0003bba4767p-1, 0x1.0f8c4c4ec7e03p-56},
		{0x1.9dfffe7b92da5p-1, 0x1.e7fd9478c4602p-55},
		{0x1.a1fffd72efdafp-1, -0x1.a0c554dcdae7ep-57},
		{0x1.a5fffde04ff95p-1, 0x1.67da98ce9b26bp-55},
		{0x1.a9fffca5e8d2bp-1, -0x1.284c9b54c13dep-55},
		{0x1.adfffddad03eap-1, 0x1.812c8ea602e3cp-58},
		{0x1.b1ffff10d3d4dp-1, -0x1.efaddad27789cp-55},
		{0x1.b5fffce21165ap-1, 0x1.3cb1719c61237p-58},
		{0x1.b9fffd950e674p-1, 0x1.3f7d94194cep-56},
		{0x1.be000139ca8afp-1, 0x1.50ac4215d9bcp-56},
		{0x1.c20005b46df99p-1, 0x1.beea653e9c1c9p-57},
		{0x1.c600040b9f7aep-1, -0x1.c079f274a70d6p-56},
		{0x1.ca0006255fd8ap-1, -0x1.a0b4076e84c1fp-56},
		{0x1.cdfffd94c095dp-1, 0x1.8f933f99ab5d7p-55},
		{0x1.d1ffff975d6cfp-1, -0x1.82c08665fe1bep-58},
		{0x1.d5fffa2561c93p-1, -0x1.b04289bd295f3p-56},
		{0x1.d9fff9d228b0cp-1, 0x1.70251340fa236p-55},
		{0x1.de00065bc7e16p-1, -0x1.5011e16a4d80cp-56},
		{0x1.e200002f64791p-1, 0x1.9802f09ef62ep-55},
		{0x1.e600057d7a6d8p-1, -0x1.e0b75580cf7fap-56},
		{0x1.ea00027edc00cp-1, -0x1.c848309459811p-55},
		{0x1.ee0006cf5cb7cp-1, -0x1.f8027951576f4p-55},
		{0x1.f2000782b7dccp-1, -0x1.f81d97274538fp-55},
		{0x1.f6000260c450ap-1, -0x1.071002727ffdcp-59},
		{0x1.f9fffe88cd533p-1, -0x1.81bdce1fda8bp-58},
		{0x1.fdfffd50f8689p-1, 0x1.7f91acb918e6ep-55},
		{0x1.0200004292367p+0, 0x1.b7ff365324681p-54},
		{0x1.05fffe3e3d668p+0, 0x1.6fa08ddae957bp-55},
		{0x1.0a0000a85a757p+0, -0x1.7e2de80d3fb91p-58},
		{0x1.0e0001a5f3fccp+0, -0x1.1823305c5f014p-54},
		{0x1.11ffff8afbaf5p+0, -0x1.bfabb6680bac2p-55},
		{0x1.15fffe54d91adp+0, -0x1.d7f121737e7efp-54},
		{0x1.1a00011ac36e1p+0, 0x1.c000a0516f5ffp-54},
		{0x1.1e00019c84248p+0, -0x1.082fbe4da5dap-54},
		{0x1.220000ffe5e6ep+0, -0x1.8fdd04c9cfb43p-55},
		{0x1.26000269fd891p+0, 0x1.cfe2a7994d182p-55},
		{0x1.2a00029a6e6dap+0, -0x1.00273715e8bc5p-56},
		{0x1.2dfffe0293e39p+0, 0x1.b7c39dab2a6f9p-54},
		{0x1.31ffff7dcf082p+0, 0x1.df1336edc5254p-56},
		{0x1.35ffff05a8b6p+0, -0x1.e03564ccd31ebp-54},
		{0x1.3a0002e0eaeccp+0, 0x1.5f0e74bd3a477p-56},
		{0x1.3e000043bb236p+0, 0x1.c7dcb149d8833p-54},
		{0x1.4200002d187ffp+0, 0x1.e08afcf2d3d28p-56},
		{0x1.460000d387cb1p+0, 0x1.20837856599a6p-55},
		{0x1.4a00004569f89p+0, -0x1.9fa5c904fbcd2p-55},
		{0x1.4e000043543f3p+0, -0x1.81125ed175329p-56},
		{0x1.51fffcc027f0fp+0, 0x1.883d8847754dcp-54},
		{0x1.55ffffd87b36fp+0, -0x1.709e731d02807p-55},
		{0x1.59ffff21df7bap+0, 0x1.7f79f68727b02p-55},
		{0x1.5dfffebfc3481p+0, -0x1.180902e30e93ep-54},
	},
};

#define log2_T        __log2_data.tab
#define log2_T2       __log2_data.tab2
#define log2_B        __log2_data.poly1
#define log2_A        __log2_data.poly
#define log2_InvLn2hi __log2_data.invln2hi
#define log2_InvLn2lo __log2_data.invln2lo
#define log2_N        (1 << LOG2_TABLE_BITS)
#define log2_OFF      0x3fe6000000000000
