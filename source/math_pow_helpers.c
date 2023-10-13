/*
File:   math_pow_helpers.c
Author: Taylor Robbins
Date:   10\11\2023
Description: 
	** Holds some functions and constants that feed implementations for pow and powf
*/

//NOTE: This file is only #inluded if !PIG_WASM_STD_USE_BUILTINS_POW

// Returns 0 if not int, 1 if odd int, 2 if even int.  The argument is
// the bit representation of a non-zero finite floating-point value.
int checkint32(uint32_t floatInt)
{
	int exponent = ((floatInt >> 23) & 0xFF);
	if (exponent < 0x7F)                                { return 0; }
	if (exponent > 0x7F + 23)                           { return 2; }
	if (floatInt & ((1 << (0x7F + 23 - exponent)) - 1)) { return 0; }
	if (floatInt & (1 << (0x7F + 23 - exponent)))       { return 1; }
	return 2;
}
int checkint64(uint64_t iy)
{
	int e = iy >> 52 & 0x7FF;
	if (e < 0x3FF) { return 0; }
	if (e > 0x3FF + 52) { return 2; }
	if (iy & ((1ULL << (0x3FF + 52 - e)) - 1)) { return 0; }
	if (iy & (1ULL << (0x3FF + 52 - e))) { return 1; }
	return 2;
}

// Returns 1 if input is the bit representation of 0, infinity or nan.
int zeroinfnan32(uint32_t floatInt)
{
	return ((2 * floatInt) - 1) >= ((2u * 0x7F800000) - 1);
}
int zeroinfnan64(uint64_t i)
{
	return ((2 * i) - 1) >= ((2 * asuint64(INFINITY)) - 1);
}

float __math_xflowf(uint32_t sign, float value)
{
	return eval_as_float(fp_barrierf(sign ? -value : value) * value);
}
double __math_xflow(uint32_t sign, double y)
{
	return eval_as_double(fp_barrier(sign ? -y : y) * y);
}
float __math_oflowf(uint32_t sign)
{
	return __math_xflowf(sign, 0x1p97F);
}
double __math_oflow(uint32_t sign)
{
	return __math_xflow(sign, 0x1p769);
}
float __math_uflowf(uint32_t sign)
{
	return __math_xflowf(sign, 0x1p-95F);
}
double __math_uflow(uint32_t sign)
{
	return __math_xflow(sign, 0x1p-767);
}

// +--------------------------------------------------------------+
// |                  log2_inline and Constants                   |
// +--------------------------------------------------------------+
#define POWF_LOG2_TABLE_BITS 4
#define POWF_LOG2_POLY_ORDER 5
#define log2inline_N (1 << POWF_LOG2_TABLE_BITS)
#define POWF_SCALE_BITS 0
#define POWF_SCALE ((double)(1 << POWF_SCALE_BITS))

const struct powf_log2_data
{
	struct
	{
		double invc, logc;
	} tab[1 << POWF_LOG2_TABLE_BITS];
	double poly[POWF_LOG2_POLY_ORDER];
} __powf_log2_data =
{
	.tab =
	{
		{ 0x1.661EC79F8F3BEp+0, -0x1.EFEC65B963019p-2 * POWF_SCALE },
		{ 0x1.571ED4AAF883Dp+0, -0x1.B0B6832D4FCA4p-2 * POWF_SCALE },
		{ 0x1.49539F0F010Bp+0,  -0x1.7418B0A1FB77Bp-2 * POWF_SCALE },
		{ 0x1.3C995B0B80385p+0, -0x1.39DE91A6DCF7Bp-2 * POWF_SCALE },
		{ 0x1.30D190C8864A5p+0, -0x1.01D9BF3F2B631p-2 * POWF_SCALE },
		{ 0x1.25E227B0B8EAp+0,  -0x1.97C1D1B3B7AFp-3  * POWF_SCALE },
		{ 0x1.1BB4A4A1A343Fp+0, -0x1.2F9E393AF3C9Fp-3 * POWF_SCALE },
		{ 0x1.12358F08AE5BAp+0, -0x1.960CBBF788D5Cp-4 * POWF_SCALE },
		{ 0x1.0953F419900A7p+0, -0x1.A6F9DB6475FCEp-5 * POWF_SCALE },
		{               0x1p+0,                0x0p+0 * POWF_SCALE },
		{ 0x1.E608CFD9A47ACp-1,  0x1.338CA9F24F53Dp-4 * POWF_SCALE },
		{ 0x1.CA4B31F026AAp-1,   0x1.476A9543891BAp-3 * POWF_SCALE },
		{ 0x1.B2036576AFCE6p-1,  0x1.E840B4AC4E4D2p-3 * POWF_SCALE },
		{ 0x1.9C2D163A1AA2Dp-1,  0x1.40645F0C6651Cp-2 * POWF_SCALE },
		{ 0x1.886E6037841EDp-1,  0x1.88E9C2C1B9FF8p-2 * POWF_SCALE },
		{ 0x1.767DCF5534862p-1,  0x1.CE0A44EB17BCCp-2 * POWF_SCALE },
	},
	.poly =
	{
		0x1.27616C9496E0Bp-2 * POWF_SCALE, -0x1.71969A075C67Ap-2 * POWF_SCALE,
		0x1.EC70A6CA7BADDp-2 * POWF_SCALE, -0x1.7154748BEF6C8p-1 * POWF_SCALE,
		0x1.71547652AB82Bp0 * POWF_SCALE,
	}
};

#define log2inline_T __powf_log2_data.tab
#define log2inline_A __powf_log2_data.poly
#define log2inline_OFF 0x3F330000

// Subnormal input is normalized so ix has negative biased exponent.
// Output is multiplied by log2inline_N (POWF_SCALE) if TOINT_INTRINICS is set.
double_t log2_inline(uint32_t floatInt)
{
	double_t zVar, rVar, rVarSquared, rVarQuad, polynomial, qVar, result, yVar, cInverse, logc;
	uint32_t zVarInt, topBits, temp;
	int vVar, index;
	
	// x = 2^vVar zVar; where zVar is in range [log2inline_OFF,2*log2inline_OFF] and exact.
	// The range is split into log2inline_N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	temp = floatInt - log2inline_OFF;
	index = ((temp >> (23 - POWF_LOG2_TABLE_BITS)) % log2inline_N);
	topBits = (temp & 0xFF800000);
	zVarInt = floatInt - topBits;
	vVar = ((int32_t)topBits >> (23 - POWF_SCALE_BITS)); /* arithmetic shift */
	cInverse = log2inline_T[index].invc;
	logc = log2inline_T[index].logc;
	zVar = (double_t)asfloat(zVarInt);
	
	// log2(x) = log1p(zVar/c-1)/ln2 + log2(c) + vVar
	rVar = (zVar * cInverse) - 1;
	yVar = logc + (double_t)vVar;
	
	// Pipelined polynomial evaluation to approximate log1p(rVar)/ln2.
	rVarSquared = rVar * rVar;
	result = (log2inline_A[0] * rVar) + log2inline_A[1];
	polynomial = (log2inline_A[2] * rVar) + log2inline_A[3];
	rVarQuad = rVarSquared * rVarSquared;
	qVar = (log2inline_A[4] * rVar) + yVar;
	qVar = (polynomial * rVarSquared) + qVar;
	result = (result * rVarQuad) + qVar;
	return result;
}

// +--------------------------------------------------------------+
// |                  exp2_inline and Constants                   |
// +--------------------------------------------------------------+
// Shared between expf, exp2f and powf.
#define EXP2F_TABLE_BITS 5
#define EXP2F_POLY_ORDER 3
#define EXP2F_N (1 << EXP2F_TABLE_BITS)

const struct exp2f_data
{
	uint64_t tab[1 << EXP2F_TABLE_BITS];
	double shift_scaled;
	double poly[EXP2F_POLY_ORDER];
	double shift;
	double invln2_scaled;
	double poly_scaled[EXP2F_POLY_ORDER];
} __exp2f_data =
{
	// tab[i] = uint(2^(i/N)) - (i << 52-BITS)
	// used for computing 2^(k/N) for an int |k| < 150 N as
	// double(tab[k%N] + (k << 52-BITS))
	.tab =
	{
		0x3FF0000000000000, 0x3FEFD9B0D3158574, 0x3FEFB5586CF9890F, 0x3FEF9301D0125B51,
		0x3FEF72B83C7D517B, 0x3FEF54873168B9AA, 0x3FEF387A6E756238, 0x3FEF1E9DF51FDEE1,
		0x3FEF06FE0A31B715, 0x3FEEF1A7373AA9CB, 0x3FEEDEA64C123422, 0x3FEECE086061892D,
		0x3FEEBFDAD5362A27, 0x3FEEB42B569D4F82, 0x3FEEAB07DD485429, 0x3FEEA47EB03A5585,
		0x3FEEA09E667F3BCD, 0x3FEE9F75E8EC5F74, 0x3FEEA11473EB0187, 0x3FEEA589994CCE13,
		0x3FEEACE5422AA0DB, 0x3FEEB737B0CDC5E5, 0x3FEEC49182A3F090, 0x3FEED503B23E255D,
		0x3FEEE89F995AD3AD, 0x3FEEFF76F2FB5E47, 0x3FEF199BDD85529C, 0x3FEF3720DCEF9069,
		0x3FEF5818DCFBA487, 0x3FEF7C97337B9B5F, 0x3FEFA4AFA2A490DA, 0x3FEFD0765B6E4540,
	},
	.shift_scaled = 0x1.8p+52 / EXP2F_N,
	.poly = { 0x1.C6AF84B912394p-5, 0x1.EBFCE50FAC4F3p-3, 0x1.62E42FF0C52D6p-1, },
	.shift = 0x1.8p+52,
	.invln2_scaled = 0x1.71547652B82FEp+0 * EXP2F_N,
	.poly_scaled = { 0x1.C6AF84B912394p-5/EXP2F_N/EXP2F_N/EXP2F_N, 0x1.EBFCE50FAC4F3p-3/EXP2F_N/EXP2F_N, 0x1.62E42FF0C52D6p-1/EXP2F_N, },
};

#define exp2inline_T __exp2f_data.tab
#define exp2inline_C __exp2f_data.poly
#define exp2inline_SHIFT __exp2f_data.shift_scaled
#define exp2inline_SIGN_BIAS (1 << (EXP2F_TABLE_BITS + 11))

// The output of log2 and thus the input of exp2 is either scaled by N
// (in case of fast toint intrinsics) or not.  The unscaled xd must be
// in [-1021,1023], signBias sets the sign of the result.
float exp2_inline(double_t value, uint32_t signBias)
{
	uint64_t ki, ski, tVarInt;
	double_t kd, zVar, rVar, rVarSquared, result, tVar;
	
	// x = k/N + rVar with rVar in [-1/(2N), 1/(2N)]
	kd = eval_as_double(value + exp2inline_SHIFT);
	ki = asuint64(kd);
	kd -= exp2inline_SHIFT; // k/N
	rVar = value - kd;
	
	// exp2(x) = 2^(k/N) * 2^rVar ~= s * (C0*rVar^3 + C1*rVar^2 + C2*rVar + 1)
	tVarInt = exp2inline_T[ki % EXP2F_N];
	ski = ki + signBias;
	tVarInt += (ski << (52 - EXP2F_TABLE_BITS));
	tVar = asdouble(tVarInt);
	zVar = (exp2inline_C[0] * rVar) + exp2inline_C[1];
	rVarSquared = rVar * rVar;
	result = (exp2inline_C[2] * rVar) + 1;
	result = (zVar * rVarSquared) + result;
	result = result * tVar;
	return eval_as_float(result);
}

// +--------------------------------------------------------------+
// |                   log_inline and Constants                   |
// +--------------------------------------------------------------+
#define POW_LOG_TABLE_BITS 7
#define POW_LOG_POLY_ORDER 8
#define powlog_N (1 << POW_LOG_TABLE_BITS)

const struct pow_log_data
{
	double ln2hi;
	double ln2lo;
	double poly[POW_LOG_POLY_ORDER - 1]; /* First coefficient is 1.  */
	/* Note: the pad field is unused, but allows slightly faster indexing.  */
	struct
	{
		double invc, pad, logc, logctail;
	} tab[1 << POW_LOG_TABLE_BITS];
} __pow_log_data =
{
	.ln2hi = 0x1.62E42FEFA3800p-1,
	.ln2lo = 0x1.EF35793C76730p-45,
	.poly =
	{
		// relative error: 0x1.11922ap-70
		// in -0x1.6bp-8 0x1.6bp-8
		// Coefficients are scaled to match the scaling during evaluation.
		-0x1p-1,
		 0x1.555555555556p-2  * -2,
		-0x1.0000000000006p-2 * -2,
		 0x1.999999959554Ep-3 *  4,
		-0x1.555555529A47Ap-3 *  4,
		 0x1.2495B9B4845E9p-3 * -8,
		-0x1.0002B8B263FC3p-3 * -8,
	},
	/*
	Algorithm:
			x = 2^k z
			log(x) = k ln2 + log(c) + log(z/c)
			log(z/c) = poly(z/c - 1)
		where z is in [0x1.69555p-1; 0x1.69555p0] which is split into N subintervals
		and z falls into the ith one, then table entries are computed as
			tab[i].invc = 1/c
			tab[i].logc = round(0x1p43*log(c))/0x1p43
			tab[i].logctail = (double)(log(c) - logc)
		where c is chosen near the center of the subinterval such that 1/c has only a
		few precision bits so z/c - 1 is exactly representible as double:
			1/c = center < 1 ? round(N/center)/N : round(2*N/center)/N/2
		Note: |z/c - 1| < 1/N for the chosen c, |log(c) - logc - logctail| < 0x1p-97,
		the last few bits of logc are rounded away so k*ln2hi + logc has no rounding
		error and the interval for z is selected such that near x == 1, where log(x)
		is tiny, large cancellation error is avoided in logc + poly(z/c - 1).
	*/
	.tab =
	{
		{ 0x1.6A00000000000p+0, 0, -0x1.62C82F2B9C800p-2,  0x1.AB42428375680p-48 },
		{ 0x1.6800000000000p+0, 0, -0x1.5D1BDBF580800p-2, -0x1.CA508D8E0F720p-46 },
		{ 0x1.6600000000000p+0, 0, -0x1.5767717455800p-2, -0x1.362A4D5B6506Dp-45 },
		{ 0x1.6400000000000p+0, 0, -0x1.51AAD872DF800p-2, -0x1.684E49EB067D5p-49 },
		{ 0x1.6200000000000p+0, 0, -0x1.4BE5F95777800p-2, -0x1.41B6993293EE0p-47 },
		{ 0x1.6000000000000p+0, 0, -0x1.4618BC21C6000p-2,  0x1.3D82F484C84CCp-46 },
		{ 0x1.5E00000000000p+0, 0, -0x1.404308686A800p-2,  0x1.C42F3ED820B3Ap-50 },
		{ 0x1.5C00000000000p+0, 0, -0x1.3A64C55694800p-2,  0x1.0B1C686519460p-45 },
		{ 0x1.5A00000000000p+0, 0, -0x1.347DD9A988000p-2,  0x1.5594DD4C58092p-45 },
		{ 0x1.5800000000000p+0, 0, -0x1.2E8E2BAE12000p-2,  0x1.67B1E99B72BD8p-45 },
		{ 0x1.5600000000000p+0, 0, -0x1.2895A13DE8800p-2,  0x1.5CA14B6CFB03Fp-46 },
		{ 0x1.5600000000000p+0, 0, -0x1.2895A13DE8800p-2,  0x1.5CA14B6CFB03Fp-46 },
		{ 0x1.5400000000000p+0, 0, -0x1.22941FBCF7800p-2, -0x1.65A242853DA76p-46 },
		{ 0x1.5200000000000p+0, 0, -0x1.1C898C1699800p-2, -0x1.FAFBC68E75404p-46 },
		{ 0x1.5000000000000p+0, 0, -0x1.1675CABABA800p-2,  0x1.F1FC63382A8F0p-46 },
		{ 0x1.4E00000000000p+0, 0, -0x1.1058BF9AE4800p-2, -0x1.6A8C4FD055A66p-45 },
		{ 0x1.4C00000000000p+0, 0, -0x1.0A324E2739000p-2, -0x1.C6BEE7EF4030Ep-47 },
		{ 0x1.4A00000000000p+0, 0, -0x1.0402594B4D000p-2, -0x1.036B89EF42D7Fp-48 },
		{ 0x1.4A00000000000p+0, 0, -0x1.0402594B4D000p-2, -0x1.036B89EF42D7Fp-48 },
		{ 0x1.4800000000000p+0, 0, -0x1.FB9186D5E4000p-3,  0x1.D572AAB993C87p-47 },
		{ 0x1.4600000000000p+0, 0, -0x1.EF0ADCBDC6000p-3,  0x1.B26B79C86AF24p-45 },
		{ 0x1.4400000000000p+0, 0, -0x1.E27076E2AF000p-3, -0x1.72F4F543FFF10p-46 },
		{ 0x1.4200000000000p+0, 0, -0x1.D5C216B4FC000p-3,  0x1.1BA91BBCA681Bp-45 },
		{ 0x1.4000000000000p+0, 0, -0x1.C8FF7C79AA000p-3,  0x1.7794F689F8434p-45 },
		{ 0x1.4000000000000p+0, 0, -0x1.C8FF7C79AA000p-3,  0x1.7794F689F8434p-45 },
		{ 0x1.3E00000000000p+0, 0, -0x1.BC286742D9000p-3,  0x1.94EB0318BB78Fp-46 },
		{ 0x1.3C00000000000p+0, 0, -0x1.AF3C94E80C000p-3,  0x1.A4E633FCD9066p-52 },
		{ 0x1.3A00000000000p+0, 0, -0x1.A23BC1FE2B000p-3, -0x1.58C64DC46C1EAp-45 },
		{ 0x1.3A00000000000p+0, 0, -0x1.A23BC1FE2B000p-3, -0x1.58C64DC46C1EAp-45 },
		{ 0x1.3800000000000p+0, 0, -0x1.9525A9CF45000p-3, -0x1.AD1D904C1D4E3p-45 },
		{ 0x1.3600000000000p+0, 0, -0x1.87FA06520D000p-3,  0x1.BBDBF7FDBFA09p-45 },
		{ 0x1.3400000000000p+0, 0, -0x1.7AB890210E000p-3,  0x1.BDB9072534A58p-45 },
		{ 0x1.3400000000000p+0, 0, -0x1.7AB890210E000p-3,  0x1.BDB9072534A58p-45 },
		{ 0x1.3200000000000p+0, 0, -0x1.6D60FE719D000p-3, -0x1.0E46AA3B2E266p-46 },
		{ 0x1.3000000000000p+0, 0, -0x1.5FF3070A79000p-3, -0x1.E9E439F105039p-46 },
		{ 0x1.3000000000000p+0, 0, -0x1.5FF3070A79000p-3, -0x1.E9E439F105039p-46 },
		{ 0x1.2E00000000000p+0, 0, -0x1.526E5E3A1B000p-3, -0x1.0DE8B90075B8Fp-45 },
		{ 0x1.2C00000000000p+0, 0, -0x1.44D2B6CCB8000p-3,  0x1.70CC16135783Cp-46 },
		{ 0x1.2C00000000000p+0, 0, -0x1.44D2B6CCB8000p-3,  0x1.70CC16135783Cp-46 },
		{ 0x1.2A00000000000p+0, 0, -0x1.371FC201E9000p-3,  0x1.178864D27543Ap-48 },
		{ 0x1.2800000000000p+0, 0, -0x1.29552F81FF000p-3, -0x1.48D301771C408p-45 },
		{ 0x1.2600000000000p+0, 0, -0x1.1B72AD52F6000p-3, -0x1.E80A41811A396p-45 },
		{ 0x1.2600000000000p+0, 0, -0x1.1B72AD52F6000p-3, -0x1.E80A41811A396p-45 },
		{ 0x1.2400000000000p+0, 0, -0x1.0D77E7CD09000p-3,  0x1.A699688E85BF4p-47 },
		{ 0x1.2400000000000p+0, 0, -0x1.0D77E7CD09000p-3,  0x1.A699688E85BF4p-47 },
		{ 0x1.2200000000000p+0, 0, -0x1.FEC9131DBE000p-4, -0x1.575545CA333F2p-45 },
		{ 0x1.2000000000000p+0, 0, -0x1.E27076E2B0000p-4,  0x1.A342C2AF0003Cp-45 },
		{ 0x1.2000000000000p+0, 0, -0x1.E27076E2B0000p-4,  0x1.A342C2AF0003Cp-45 },
		{ 0x1.1E00000000000p+0, 0, -0x1.C5E548F5BC000p-4, -0x1.D0C57585FBE06p-46 },
		{ 0x1.1C00000000000p+0, 0, -0x1.A926D3A4AE000p-4,  0x1.53935E85BAAC8p-45 },
		{ 0x1.1C00000000000p+0, 0, -0x1.A926D3A4AE000p-4,  0x1.53935E85BAAC8p-45 },
		{ 0x1.1A00000000000p+0, 0, -0x1.8C345D631A000p-4,  0x1.37C294D2F5668p-46 },
		{ 0x1.1A00000000000p+0, 0, -0x1.8C345D631A000p-4,  0x1.37C294D2F5668p-46 },
		{ 0x1.1800000000000p+0, 0, -0x1.6F0D28AE56000p-4, -0x1.69737C93373DAp-45 },
		{ 0x1.1600000000000p+0, 0, -0x1.51B073F062000p-4,  0x1.F025B61C65E57p-46 },
		{ 0x1.1600000000000p+0, 0, -0x1.51B073F062000p-4,  0x1.F025B61C65E57p-46 },
		{ 0x1.1400000000000p+0, 0, -0x1.341D7961BE000p-4,  0x1.C5EDACCF913DFp-45 },
		{ 0x1.1400000000000p+0, 0, -0x1.341D7961BE000p-4,  0x1.C5EDACCF913DFp-45 },
		{ 0x1.1200000000000p+0, 0, -0x1.16536EEA38000p-4,  0x1.47C5E768FA309p-46 },
		{ 0x1.1000000000000p+0, 0, -0x1.F0A30C0118000p-5,  0x1.D599E83368E91p-45 },
		{ 0x1.1000000000000p+0, 0, -0x1.F0A30C0118000p-5,  0x1.D599E83368E91p-45 },
		{ 0x1.0E00000000000p+0, 0, -0x1.B42DD71198000p-5,  0x1.C827AE5D6704Cp-46 },
		{ 0x1.0E00000000000p+0, 0, -0x1.B42DD71198000p-5,  0x1.C827AE5D6704Cp-46 },
		{ 0x1.0C00000000000p+0, 0, -0x1.77458F632C000p-5, -0x1.CFC4634F2A1EEp-45 },
		{ 0x1.0C00000000000p+0, 0, -0x1.77458F632C000p-5, -0x1.CFC4634F2A1EEp-45 },
		{ 0x1.0A00000000000p+0, 0, -0x1.39E87B9FEC000p-5,  0x1.502B7F526FEAAp-48 },
		{ 0x1.0A00000000000p+0, 0, -0x1.39E87B9FEC000p-5,  0x1.502B7F526FEAAp-48 },
		{ 0x1.0800000000000p+0, 0, -0x1.F829B0E780000p-6, -0x1.980267C7E09E4p-45 },
		{ 0x1.0800000000000p+0, 0, -0x1.F829B0E780000p-6, -0x1.980267C7E09E4p-45 },
		{ 0x1.0600000000000p+0, 0, -0x1.7B91B07D58000p-6, -0x1.88D5493FAA639p-45 },
		{ 0x1.0400000000000p+0, 0, -0x1.FC0A8B0FC0000p-7, -0x1.F1E7CF6D3A69Cp-50 },
		{ 0x1.0400000000000p+0, 0, -0x1.FC0A8B0FC0000p-7, -0x1.F1E7CF6D3A69Cp-50 },
		{ 0x1.0200000000000p+0, 0, -0x1.FE02A6B100000p-8, -0x1.9E23F0DDA40E4p-46 },
		{ 0x1.0200000000000p+0, 0, -0x1.FE02A6B100000p-8, -0x1.9E23F0DDA40E4p-46 },
		{ 0x1.0000000000000p+0, 0,  0x0.0000000000000p+0,  0x0.0000000000000p+0  },
		{ 0x1.0000000000000p+0, 0,  0x0.0000000000000p+0,  0x0.0000000000000p+0  },
		{ 0x1.FC00000000000p-1, 0,  0x1.0101575890000p-7, -0x1.0C76B999D2BE8p-46 },
		{ 0x1.F800000000000p-1, 0,  0x1.0205658938000p-6, -0x1.3DC5B06E2F7D2p-45 },
		{ 0x1.F400000000000p-1, 0,  0x1.8492528C90000p-6, -0x1.AA0BA325A0C34p-45 },
		{ 0x1.F000000000000p-1, 0,  0x1.0415D89E74000p-5,  0x1.111C05CF1D753p-47 },
		{ 0x1.EC00000000000p-1, 0,  0x1.466AED42E0000p-5, -0x1.C167375BDFD28p-45 },
		{ 0x1.E800000000000p-1, 0,  0x1.894AA149FC000p-5, -0x1.97995D05A267Dp-46 },
		{ 0x1.E400000000000p-1, 0,  0x1.CCB73CDDDC000p-5, -0x1.A68F247D82807p-46 },
		{ 0x1.E200000000000p-1, 0,  0x1.EEA31C006C000p-5, -0x1.E113E4FC93B7Bp-47 },
		{ 0x1.DE00000000000p-1, 0,  0x1.1973BD1466000p-4, -0x1.5325D560D9E9Bp-45 },
		{ 0x1.DA00000000000p-1, 0,  0x1.3BDF5A7D1E000p-4,  0x1.CC85EA5DB4ED7p-45 },
		{ 0x1.D600000000000p-1, 0,  0x1.5E95A4D97A000p-4, -0x1.C69063C5D1D1Ep-45 },
		{ 0x1.D400000000000p-1, 0,  0x1.700D30AEAC000p-4,  0x1.C1E8DA99DED32p-49 },
		{ 0x1.D000000000000p-1, 0,  0x1.9335E5D594000p-4,  0x1.3115C3ABD47DAp-45 },
		{ 0x1.CC00000000000p-1, 0,  0x1.B6AC88DAD6000p-4, -0x1.390802BF768E5p-46 },
		{ 0x1.CA00000000000p-1, 0,  0x1.C885801BC4000p-4,  0x1.646D1C65AACD3p-45 },
		{ 0x1.C600000000000p-1, 0,  0x1.EC739830A2000p-4, -0x1.DC068AFE645E0p-45 },
		{ 0x1.C400000000000p-1, 0,  0x1.FE89139DBE000p-4, -0x1.534D64FA10AFDp-45 },
		{ 0x1.C000000000000p-1, 0,  0x1.1178E8227E000p-3,  0x1.1EF78CE2D07F2p-45 },
		{ 0x1.BE00000000000p-1, 0,  0x1.1AA2B7E23F000p-3,  0x1.CA78E44389934p-45 },
		{ 0x1.BA00000000000p-1, 0,  0x1.2D1610C868000p-3,  0x1.39D6CCB81B4A1p-47 },
		{ 0x1.B800000000000p-1, 0,  0x1.365FCB0159000p-3,  0x1.62FA8234B7289p-51 },
		{ 0x1.B400000000000p-1, 0,  0x1.4913D8333B000p-3,  0x1.5837954FDB678p-45 },
		{ 0x1.B200000000000p-1, 0,  0x1.527E5E4A1B000p-3,  0x1.633E8E5697DC7p-45 },
		{ 0x1.AE00000000000p-1, 0,  0x1.6574EBE8C1000p-3,  0x1.9CF8B2C3C2E78p-46 },
		{ 0x1.AC00000000000p-1, 0,  0x1.6F0128B757000p-3, -0x1.5118DE59C21E1p-45 },
		{ 0x1.AA00000000000p-1, 0,  0x1.7898D85445000p-3, -0x1.C661070914305p-46 },
		{ 0x1.A600000000000p-1, 0,  0x1.8BEAFEB390000p-3, -0x1.73D54AAE92CD1p-47 },
		{ 0x1.A400000000000p-1, 0,  0x1.95A5ADCF70000p-3,  0x1.7F22858A0FF6Fp-47 },
		{ 0x1.A000000000000p-1, 0,  0x1.A93ED3C8AE000p-3, -0x1.8724350562169p-45 },
		{ 0x1.9E00000000000p-1, 0,  0x1.B31D8575BD000p-3, -0x1.C358D4EACE1AAp-47 },
		{ 0x1.9C00000000000p-1, 0,  0x1.BD087383BE000p-3, -0x1.D4BC4595412B6p-45 },
		{ 0x1.9A00000000000p-1, 0,  0x1.C6FFBC6F01000p-3, -0x1.1EC72C5962BD2p-48 },
		{ 0x1.9600000000000p-1, 0,  0x1.DB13DB0D49000p-3, -0x1.AFF2AF715B035p-45 },
		{ 0x1.9400000000000p-1, 0,  0x1.E530EFFE71000p-3,  0x1.212276041F430p-51 },
		{ 0x1.9200000000000p-1, 0,  0x1.EF5ADE4DD0000p-3, -0x1.A211565BB8E11p-51 },
		{ 0x1.9000000000000p-1, 0,  0x1.F991C6CB3B000p-3,  0x1.BCBECCA0CDF30p-46 },
		{ 0x1.8C00000000000p-1, 0,  0x1.07138604D5800p-2,  0x1.89CDB16ED4E91p-48 },
		{ 0x1.8A00000000000p-1, 0,  0x1.0C42D67616000p-2,  0x1.7188B163CEAE9p-45 },
		{ 0x1.8800000000000p-1, 0,  0x1.1178E8227E800p-2, -0x1.C210E63A5F01Cp-45 },
		{ 0x1.8600000000000p-1, 0,  0x1.16B5CCBACF800p-2,  0x1.B9ACDF7A51681p-45 },
		{ 0x1.8400000000000p-1, 0,  0x1.1BF99635A6800p-2,  0x1.CA6ED5147BDB7p-45 },
		{ 0x1.8200000000000p-1, 0,  0x1.214456D0EB800p-2,  0x1.A87DEBA46BAEAp-47 },
		{ 0x1.7E00000000000p-1, 0,  0x1.2BEF07CDC9000p-2,  0x1.A9CFA4A5004F4p-45 },
		{ 0x1.7C00000000000p-1, 0,  0x1.314F1E1D36000p-2, -0x1.8E27AD3213CB8p-45 },
		{ 0x1.7A00000000000p-1, 0,  0x1.36B6776BE1000p-2,  0x1.16ECDB0F177C8p-46 },
		{ 0x1.7800000000000p-1, 0,  0x1.3C25277333000p-2,  0x1.83B54B606BD5Cp-46 },
		{ 0x1.7600000000000p-1, 0,  0x1.419B423D5E800p-2,  0x1.8E436EC90E09Dp-47 },
		{ 0x1.7400000000000p-1, 0,  0x1.4718DC271C800p-2, -0x1.F27CE0967D675p-45 },
		{ 0x1.7200000000000p-1, 0,  0x1.4C9E09E173000p-2, -0x1.E20891B0AD8A4p-45 },
		{ 0x1.7000000000000p-1, 0,  0x1.522AE0738A000p-2,  0x1.EBE708164C759p-45 },
		{ 0x1.6E00000000000p-1, 0,  0x1.57BF753C8D000p-2,  0x1.FADEDEE5D40EFp-46 },
		{ 0x1.6C00000000000p-1, 0,  0x1.5D5BDDF596000p-2, -0x1.A0B2A08A465DCp-47 },
	},
};

#define powlog_T __pow_log_data.tab
#define powlog_A __pow_log_data.poly
#define powlog_Ln2hi __pow_log_data.ln2hi
#define powlog_Ln2lo __pow_log_data.ln2lo
#define powlog_OFF 0x3fe6955500000000

// Compute y+TAIL = log(x) where the rounded result is y and TAIL has about
// additional 15 bits precision.  IX is the bit representation of x, but
// normalized in the subnormal range using the sign bit for the exponent.
double_t log_inline(uint64_t doubleInt, double_t* tail)
{
	// double_t for better performance on targets with FLT_EVAL_METHOD==2.
	double_t zVar, result, invc, logc, logctail, vVar, resultHigh, tVar1, tVar2, resultLow, p;
	double_t lowPart1, lowPart2, lowPart3, lowPart4;
	double_t rVar, arVar, arVarSquared, arVarCubed;
	uint64_t zVarInt, temp;
	int vVarInt, index;
	
	// x = 2^vVar zVar; where zVar is in range [OFF,2*OFF) and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	temp = doubleInt - powlog_OFF;
	index = ((temp >> (52 - POW_LOG_TABLE_BITS)) % powlog_N);
	vVarInt = (int64_t)temp >> 52; // arithmetic shift
	zVarInt = doubleInt - (temp & 0xfffULL << 52);
	zVar = asdouble(zVarInt);
	vVar = (double_t)vVarInt;
	
	// log(x) = vVar*Ln2 + log(c) + log1p(zVar/c-1).
	invc = powlog_T[index].invc;
	logc = powlog_T[index].logc;
	logctail = powlog_T[index].logctail;
	
	// Note: 1/c is j/N or j/N/2 where j is an integer in [N,2N) and
    // |zVar/c - 1| < 1/N, so rVar = zVar/c - 1 is exactly representible.
	// Split zVar such that rhi, rlo and rhi*rhi are exact and |rlo| <= |rVar|.
	double_t zhi = asdouble((zVarInt + (1ULL << 31)) & (-1ULL << 32));
	double_t zlo = zVar - zhi;
	double_t rhi = zhi * invc - 1.0;
	double_t rlo = zlo * invc;
	rVar = rhi + rlo;
	
	// vVar*Ln2 + log(c) + rVar.
	tVar1 = (vVar * powlog_Ln2hi) + logc;
	tVar2 = tVar1 + rVar;
	lowPart1 = (vVar * powlog_Ln2lo) + logctail;
	lowPart2 = tVar1 - tVar2 + rVar;
	
	// Evaluation is optimized assuming superscalar pipelined execution.
	arVar = powlog_A[0] * rVar; // A[0] = -0.5.
	arVarSquared = rVar * arVar;
	arVarCubed = rVar * arVarSquared;
	// vVar*Ln2 + log(c) + rVar + A[0]*rVar*rVar.
	double_t arhi = powlog_A[0] * rhi;
	double_t arhi2 = rhi * arhi;
	resultHigh = tVar2 + arhi2;
	lowPart3 = rlo * (arVar + arhi);
	lowPart4 = tVar2 - resultHigh + arhi2;
	// p = log1p(rVar) - rVar - A[0]*rVar*rVar.
	p = (arVarCubed * (powlog_A[1] + rVar * powlog_A[2] + arVarSquared * (powlog_A[3] + rVar * powlog_A[4] + arVarSquared * (powlog_A[5] + rVar * powlog_A[6]))));
	resultLow = lowPart1 + lowPart2 + lowPart3 + lowPart4 + p;
	result = resultHigh + resultLow;
	*tail = resultHigh - result + resultLow;
	return result;
}


#define EXP_TABLE_BITS 7
#define EXP_POLY_ORDER 5
#define EXP_USE_TOINT_NARROW 0
#define EXP2_POLY_ORDER 5
#define expinline_N (1 << EXP_TABLE_BITS)

const struct exp_data
{
	double invln2N;
	double shift;
	double negln2hiN;
	double negln2loN;
	double poly[4]; /* Last four coefficients.  */
	double exp2_shift;
	double exp2_poly[EXP2_POLY_ORDER];
	uint64_t tab[2*(1 << EXP_TABLE_BITS)];
} __exp_data =
{
	// N/ln2
	.invln2N = 0x1.71547652B82FEp0 * expinline_N,
	// -ln2/N
	.negln2hiN = -0x1.62E42FEFA0000p-8,
	.negln2loN = -0x1.CF79ABC9E3B3Ap-47,
	// Used for rounding when !TOINT_INTRINSICS
	.shift = 0x1.8p52,
	// exp polynomial coefficients.
	.poly =
	{
		// abs error: 1.555*2^-66
		// ulp error: 0.509 (0.511 without fma)
		// if |x| < ln2/256+eps
		// abs error if |x| < ln2/256+0x1p-15: 1.09*2^-65
		// abs error if |x| < ln2/128: 1.7145*2^-56
		0x1.FFFFFFFFFFDBDp-2,
		0x1.555555555543Cp-3,
		0x1.55555CF172B91p-5,
		0x1.1111167A4D017p-7,
	},
	.exp2_shift = 0x1.8p52 / expinline_N,
	// exp2 polynomial coefficients.
	.exp2_poly =
	{
		// abs error: 1.2195*2^-65
		// ulp error: 0.507 (0.511 without fma)
		// if |x| < 1/256
		// abs error if |x| < 1/128: 1.9941*2^-56
		0x1.62E42FEFA39EFp-1,
		0x1.EBFBDFF82C424p-3,
		0x1.C6B08D70CF4B5p-5,
		0x1.3B2ABD24650CCp-7,
		0x1.5D7E09B4E3A84p-10,
	},
	// 2^(k/N) ~= H[k]*(1 + T[k]) for int k in [0,N)
	// tab[2*k] = asuint64(T[k])
	// tab[2*k+1] = asuint64(H[k]) - (k << 52)/N
	.tab =
	{
		0x0,                0x3FF0000000000000,
		0x3C9B3B4F1A88BF6E, 0x3FEFF63DA9FB3335,
		0xBC7160139CD8DC5D, 0x3FEFEC9A3E778061,
		0xBC905E7A108766D1, 0x3FEFE315E86E7F85,
		0x3C8CD2523567F613, 0x3FEFD9B0D3158574,
		0xBC8BCE8023F98EFA, 0x3FEFD06B29DDF6DE,
		0x3C60F74E61E6C861, 0x3FEFC74518759BC8,
		0x3C90A3E45B33D399, 0x3FEFBE3ECAC6F383,
		0x3C979AA65D837B6D, 0x3FEFB5586CF9890F,
		0x3C8EB51A92FDEFFC, 0x3FEFAC922B7247F7,
		0x3C3EBE3D702F9CD1, 0x3FEFA3EC32D3D1A2,
		0xBC6A033489906E0B, 0x3FEF9B66AFFED31B,
		0xBC9556522A2FBD0E, 0x3FEF9301D0125B51,
		0xBC5080EF8C4EEA55, 0x3FEF8ABDC06C31CC,
		0xBC91C923B9D5F416, 0x3FEF829AAEA92DE0,
		0x3C80D3E3E95C55AF, 0x3FEF7A98C8A58E51,
		0xBC801B15EAA59348, 0x3FEF72B83C7D517B,
		0xBC8F1FF055DE323D, 0x3FEF6AF9388C8DEA,
		0x3C8B898C3F1353BF, 0x3FEF635BEB6FCB75,
		0xBC96D99C7611EB26, 0x3FEF5BE084045CD4,
		0x3C9AECF73E3A2F60, 0x3FEF54873168B9AA,
		0xBC8FE782CB86389D, 0x3FEF4D5022FCD91D,
		0x3C8A6F4144A6C38D, 0x3FEF463B88628CD6,
		0x3C807A05B0E4047D, 0x3FEF3F49917DDC96,
		0x3C968EFDE3A8A894, 0x3FEF387A6E756238,
		0x3C875E18F274487D, 0x3FEF31CE4FB2A63F,
		0x3C80472B981FE7F2, 0x3FEF2B4565E27CDD,
		0xBC96B87B3F71085E, 0x3FEF24DFE1F56381,
		0x3C82F7E16D09AB31, 0x3FEF1E9DF51FDEE1,
		0xBC3D219B1A6FBFFA, 0x3FEF187FD0DAD990,
		0x3C8B3782720C0AB4, 0x3FEF1285A6E4030B,
		0x3C6E149289CECB8F, 0x3FEF0CAFA93E2F56,
		0x3C834D754DB0ABB6, 0x3FEF06FE0A31B715,
		0x3C864201E2AC744C, 0x3FEF0170FC4CD831,
		0x3C8FDD395DD3F84A, 0x3FEEFC08B26416FF,
		0xBC86A3803B8E5B04, 0x3FEEF6C55F929FF1,
		0xBC924AEDCC4B5068, 0x3FEEF1A7373AA9CB,
		0xBC9907F81B512D8E, 0x3FEEECAE6D05D866,
		0xBC71D1E83E9436D2, 0x3FEEE7DB34E59FF7,
		0xBC991919B3CE1B15, 0x3FEEE32DC313A8E5,
		0x3C859F48A72A4C6D, 0x3FEEDEA64C123422,
		0xBC9312607A28698A, 0x3FEEDA4504AC801C,
		0xBC58A78F4817895B, 0x3FEED60A21F72E2A,
		0xBC7C2C9B67499A1B, 0x3FEED1F5D950A897,
		0x3C4363ED60C2AC11, 0x3FEECE086061892D,
		0x3C9666093B0664EF, 0x3FEECA41ED1D0057,
		0x3C6ECCE1DAA10379, 0x3FEEC6A2B5C13CD0,
		0x3C93FF8E3F0F1230, 0x3FEEC32AF0D7D3DE,
		0x3C7690CEBB7AAFB0, 0x3FEEBFDAD5362A27,
		0x3C931DBDEB54E077, 0x3FEEBCB299FDDD0D,
		0xBC8F94340071A38E, 0x3FEEB9B2769D2CA7,
		0xBC87DECCDC93A349, 0x3FEEB6DAA2CF6642,
		0xBC78DEC6BD0F385F, 0x3FEEB42B569D4F82,
		0xBC861246EC7B5CF6, 0x3FEEB1A4CA5D920F,
		0x3C93350518FDD78E, 0x3FEEAF4736B527DA,
		0x3C7B98B72F8A9B05, 0x3FEEAD12D497C7FD,
		0x3C9063E1E21C5409, 0x3FEEAB07DD485429,
		0x3C34C7855019C6EA, 0x3FEEA9268A5946B7,
		0x3C9432E62B64C035, 0x3FEEA76F15AD2148,
		0xBC8CE44A6199769F, 0x3FEEA5E1B976DC09,
		0xBC8C33C53BEF4DA8, 0x3FEEA47EB03A5585,
		0xBC845378892BE9AE, 0x3FEEA34634CCC320,
		0xBC93CEDD78565858, 0x3FEEA23882552225,
		0x3C5710AA807E1964, 0x3FEEA155D44CA973,
		0xBC93B3EFBF5E2228, 0x3FEEA09E667F3BCD,
		0xBC6A12AD8734B982, 0x3FEEA012750BDABF,
		0xBC6367EFB86DA9EE, 0x3FEE9FB23C651A2F,
		0xBC80DC3D54E08851, 0x3FEE9F7DF9519484,
		0xBC781F647E5A3ECF, 0x3FEE9F75E8EC5F74,
		0xBC86EE4AC08B7DB0, 0x3FEE9F9A48A58174,
		0xBC8619321E55E68A, 0x3FEE9FEB564267C9,
		0x3C909CCB5E09D4D3, 0x3FEEA0694FDE5D3F,
		0xBC7B32DCB94DA51D, 0x3FEEA11473EB0187,
		0x3C94ECFD5467C06B, 0x3FEEA1ED0130C132,
		0x3C65EBE1ABD66C55, 0x3FEEA2F336CF4E62,
		0xBC88A1C52FB3CF42, 0x3FEEA427543E1A12,
		0xBC9369B6F13B3734, 0x3FEEA589994CCE13,
		0xBC805E843A19FF1E, 0x3FEEA71A4623C7AD,
		0xBC94D450D872576E, 0x3FEEA8D99B4492ED,
		0x3C90AD675B0E8A00, 0x3FEEAAC7D98A6699,
		0x3C8DB72FC1F0EAB4, 0x3FEEACE5422AA0DB,
		0xBC65B6609CC5E7FF, 0x3FEEAF3216B5448C,
		0x3C7BF68359F35F44, 0x3FEEB1AE99157736,
		0xBC93091FA71E3D83, 0x3FEEB45B0B91FFC6,
		0xBC5DA9B88B6C1E29, 0x3FEEB737B0CDC5E5,
		0xBC6C23F97C90B959, 0x3FEEBA44CBC8520F,
		0xBC92434322F4F9AA, 0x3FEEBD829FDE4E50,
		0xBC85CA6CD7668E4B, 0x3FEEC0F170CA07BA,
		0x3C71AFFC2B91CE27, 0x3FEEC49182A3F090,
		0x3C6DD235E10A73BB, 0x3FEEC86319E32323,
		0xBC87C50422622263, 0x3FEECC667B5DE565,
		0x3C8B1C86E3E231D5, 0x3FEED09BEC4A2D33,
		0xBC91BBD1D3BCBB15, 0x3FEED503B23E255D,
		0x3C90CC319CEE31D2, 0x3FEED99E1330B358,
		0x3C8469846E735AB3, 0x3FEEDE6B5579FDBF,
		0xBC82DFCD978E9DB4, 0x3FEEE36BBFD3F37A,
		0x3C8C1A7792CB3387, 0x3FEEE89F995AD3AD,
		0xBC907B8F4AD1D9FA, 0x3FEEEE07298DB666,
		0xBC55C3D956DCAEBA, 0x3FEEF3A2B84F15FB,
		0xBC90A40E3DA6F640, 0x3FEEF9728DE5593A,
		0xBC68D6F438AD9334, 0x3FEEFF76F2FB5E47,
		0xBC91EEE26B588A35, 0x3FEF05B030A1064A,
		0x3C74FFD70A5FDDCD, 0x3FEF0C1E904BC1D2,
		0xBC91BDFBFA9298AC, 0x3FEF12C25BD71E09,
		0x3C736EAE30AF0CB3, 0x3FEF199BDD85529C,
		0x3C8EE3325C9FFD94, 0x3FEF20AB5FFFD07A,
		0x3C84E08FD10959AC, 0x3FEF27F12E57D14B,
		0x3C63CDAF384E1A67, 0x3FEF2F6D9406E7B5,
		0x3C676B2C6C921968, 0x3FEF3720DCEF9069,
		0xBC808A1883CCB5D2, 0x3FEF3F0B555DC3FA,
		0xBC8FAD5D3FFFFA6F, 0x3FEF472D4A07897C,
		0xBC900DAE3875A949, 0x3FEF4F87080D89F2,
		0x3C74A385A63D07A7, 0x3FEF5818DCFBA487,
		0xBC82919E2040220F, 0x3FEF60E316C98398,
		0x3C8E5A50D5C192AC, 0x3FEF69E603DB3285,
		0x3C843A59AC016B4B, 0x3FEF7321F301B460,
		0xBC82D52107B43E1F, 0x3FEF7C97337B9B5F,
		0xBC892AB93B470DC9, 0x3FEF864614F5A129,
		0x3C74B604603A88D3, 0x3FEF902EE78B3FF6,
		0x3C83C5EC519D7271, 0x3FEF9A51FBC74C83,
		0xBC8FF7128FD391F0, 0x3FEFA4AFA2A490DA,
		0xBC8DAE98E223747D, 0x3FEFAF482D8E67F1,
		0x3C8EC3BC41AA2008, 0x3FEFBA1BEE615A27,
		0x3C842B94C3A9EB32, 0x3FEFC52B376BBA97,
		0x3C8A64A931D185EE, 0x3FEFD0765B6E4540,
		0xBC8E37BAE43BE3ED, 0x3FEFDBFDAD9CBE14,
		0x3C77893B4D91CD9D, 0x3FEFE7C1819E90D8,
		0x3C5305C14160CC89, 0x3FEFF3C22B8F71F1,
	},
};

#define expinline_InvLn2N __exp_data.invln2N
#define expinline_NegLn2hiN __exp_data.negln2hiN
#define expinline_NegLn2loN __exp_data.negln2loN
#define expinline_Shift __exp_data.shift
#define expinline_T __exp_data.tab
#define expinline_C2 __exp_data.poly[5 - EXP_POLY_ORDER]
#define expinline_C3 __exp_data.poly[6 - EXP_POLY_ORDER]
#define expinline_C4 __exp_data.poly[7 - EXP_POLY_ORDER]
#define expinline_C5 __exp_data.poly[8 - EXP_POLY_ORDER]
#define expinline_C6 __exp_data.poly[9 - EXP_POLY_ORDER] //TODO: Do we even use this?

// Handle cases that may overflow or underflow when computing the result that
// is scale*(1+TMP) without intermediate rounding.  The bit representation of
// scale is in SBITS, however it has a computed exponent that may have
// overflown into the sign bit so that needs to be adjusted before using it as
// a double.  (int32_t)KI is the k used in the argument reduction and exponent
// adjustment of scale, positive k here means the result may overflow and
// negative k means the result may underflow.
double specialcase(double_t tmp, uint64_t sbits, uint64_t ki)
{
	double_t scale, result;

	if ((ki & 0x80000000) == 0)
	{
		// k > 0, the exponent of scale might have overflowed by <= 460.
		sbits -= (1009ull << 52);
		scale = asdouble(sbits);
		result = 0x1p1009 * (scale + scale * tmp);
		return eval_as_double(result);
	}
	// k < 0, need special care in the subnormal range.
	sbits += (1022ull << 52);
	// Note: sbits is signed scale.
	scale = asdouble(sbits);
	result = scale + scale * tmp;
	if (fabs(result) < 1.0)
	{
		// Round result to the right precision before scaling it into the subnormal
		// range to avoid double rounding that can cause 0.5+E/2 ulp error where
		// E is the worst-case ulp error outside the subnormal range.  So this
		// is only useful if the goal is better than 1 ulp worst-case error.
		double_t highDouble, lowDouble, signedOne = 1.0;
		if (result < 0.0) { signedOne = -1.0; }
		lowDouble = scale - result + (scale * tmp);
		highDouble = signedOne + result;
		lowDouble = signedOne - highDouble + result + lowDouble;
		result = eval_as_double(highDouble + lowDouble) - signedOne;
		// Fix the sign of 0.
		if (result == 0.0) { result = asdouble(sbits & 0x8000000000000000); }
		// The underflow exception needs to be signaled explicitly.
		fp_force_eval(fp_barrier(0x1p-1022) * 0x1p-1022);
	}
	result = 0x1p-1022 * result;
	return eval_as_double(result);
}

#define expinline_SIGN_BIAS (0x800 << EXP_TABLE_BITS)

// Computes sign*exp(value+valueTail) where |valueTail| < 2^-8/N and |valueTail| <= |value|.
// The sign_bias argument is SIGN_BIAS or 0 and sets the sign to -1 or 1.
double exp_inline(double_t value, double_t valueTail, uint32_t sign_bias)
{
	uint32_t valueTopBitsAbs;
	uint64_t zVarShiftedInt, index, topBits, sbits;
	// double_t for better performance on targets with FLT_EVAL_METHOD==2.
	double_t zVarShifted, zVar, rVar, rVarSquared, scale, tail, temp;
	
	valueTopBitsAbs = (top12(value) & 0x7FF);
	if (predict_false(valueTopBitsAbs - top12(0x1p-54) >= top12(512.0) - top12(0x1p-54)))
	{
		if (valueTopBitsAbs - top12(0x1p-54) >= 0x80000000)
		{
			// Avoid spurious underflow for tiny value.
			// Note: 0 is common input.
			double_t one = 1.0;
			return sign_bias ? -one : one;
		}
		if (valueTopBitsAbs >= top12(1024.0))
		{
			// Note: inf and nan are already handled.
			if (asuint64(value) >> 63) { return __math_uflow(sign_bias); }
			else { return __math_oflow(sign_bias); }
		}
		// Large value is special cased below.
		valueTopBitsAbs = 0;
	}
	
	// exp(value) = 2^(k/N) * exp(rVar), with exp(rVar) in [2^(-1/2N),2^(1/2N)].
	// value = ln2/N*k + rVar, with int k and rVar in [-ln2/2N, ln2/2N].
	zVar = expinline_InvLn2N * value;
	// zVar - zVarShifted is in [-1, 1] in non-nearest rounding modes.
	zVarShifted = eval_as_double(zVar + expinline_Shift);
	zVarShiftedInt = asuint64(zVarShifted);
	zVarShifted -= expinline_Shift;
	rVar = value + (zVarShifted * expinline_NegLn2hiN) + (zVarShifted * expinline_NegLn2loN);
	// The code assumes 2^-200 < |valueTail| < 2^-8/N.
	rVar += valueTail;
	// 2^(k/N) ~= scale * (1 + tail).
	index = 2 * (zVarShiftedInt % expinline_N);
	topBits = ((zVarShiftedInt + sign_bias) << (52 - EXP_TABLE_BITS));
	tail = asdouble(expinline_T[index]);
	// This is only a valid scale when -1023*N < k < 1024*N.
	sbits = expinline_T[index + 1] + topBits;
	// exp(value) = 2^(k/N) * exp(rVar) ~= scale + scale * (tail + exp(rVar) - 1).
	// Evaluation is optimized assuming superscalar pipelined execution.
	rVarSquared = rVar * rVar;
	// Without fma the worst case error is 0.25/N ulp larger.
	// Worst case error is less than 0.5+1.11/N+(abs poly error * 2^53) ulp.
	temp = tail + rVar + rVarSquared * (expinline_C2 + rVar * expinline_C3) + rVarSquared * rVarSquared * (expinline_C4 + rVar * expinline_C5);
	if (predict_false(valueTopBitsAbs == 0)) { return specialcase(temp, sbits, zVarShiftedInt); }
	scale = asdouble(sbits);
	// Note: temp == 0 or |temp| > 2^-200 and scale > 2^-739, so there
	// is no spurious underflow here even without fma.
	return eval_as_double(scale + (scale * temp));
}
