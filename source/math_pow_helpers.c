/*
File:   math_pow_helpers.c
Author: Taylor Robbins
Date:   10\11\2023
Description: 
	** Holds some functions and constants that feed implementations for pow and powf
*/

//NOTE: This file is only #inluded if !PIG_WASM_STD_USE_BUILTINS_POW

// +--------------------------------------------------------------+
// |                  log2_innline and Constants                  |
// +--------------------------------------------------------------+

#define POWF_LOG2_TABLE_BITS 4
#define POWF_LOG2_POLY_ORDER 5
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

#define log2inline_N (1 << POWF_LOG2_TABLE_BITS)
#define log2inline_T __powf_log2_data.tab
#define log2inline_A __powf_log2_data.poly
#define log2inline_OFF 0x3f330000

// Subnormal input is normalized so ix has negative biased exponent.
// Output is multiplied by log2inline_N (POWF_SCALE) if TOINT_INTRINICS is set.
double_t log2_inline(uint32_t ix)
{
	double_t z, r, r2, r4, p, q, y, y0, invc, logc;
	uint32_t iz, top, tmp;
	int k, i;
	
	// x = 2^k z; where z is in range [log2inline_OFF,2*log2inline_OFF] and exact.
	// The range is split into log2inline_N subintervals.
	// The ith subinterval contains z and c is near its center.
	tmp = ix - log2inline_OFF;
	i = (tmp >> (23 - POWF_LOG2_TABLE_BITS)) % log2inline_N;
	top = tmp & 0xff800000;
	iz = ix - top;
	k = (int32_t)top >> (23 - POWF_SCALE_BITS); /* arithmetic shift */
	invc = log2inline_T[i].invc;
	logc = log2inline_T[i].logc;
	z = (double_t)asfloat(iz);
	
	// log2(x) = log1p(z/c-1)/ln2 + log2(c) + k
	r = z * invc - 1;
	y0 = logc + (double_t)k;
	
	// Pipelined polynomial evaluation to approximate log1p(r)/ln2.
	r2 = r * r;
	y = log2inline_A[0] * r + log2inline_A[1];
	p = log2inline_A[2] * r + log2inline_A[3];
	r4 = r2 * r2;
	q = log2inline_A[4] * r + y0;
	q = p * r2 + q;
	y = y * r4 + q;
	return y;
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
	tVarInt += ski << (52 - EXP2F_TABLE_BITS);
	tVar = asdouble(tVarInt);
	zVar = exp2inline_C[0] * rVar + exp2inline_C[1];
	rVarSquared = rVar * rVar;
	result = exp2inline_C[2] * rVar + 1;
	result = zVar * rVarSquared + result;
	result = result * tVar;
	return eval_as_float(result);
}

// +--------------------------------------------------------------+
// |                     powf Direct Helpers                      |
// +--------------------------------------------------------------+
// Returns 0 if not int, 1 if odd int, 2 if even int.  The argument is
// the bit representation of a non-zero finite floating-point value.
int checkint(uint32_t floatInt)
{
	int exponent = ((floatInt >> 23) & 0xFF);
	if (exponent < 0x7F)                                { return 0; }
	if (exponent > 0x7F + 23)                           { return 2; }
	if (floatInt & ((1 << (0x7F + 23 - exponent)) - 1)) { return 0; }
	if (floatInt & (1 << (0x7F + 23 - exponent)))       { return 1; }
	return 2;
}

int zeroinfnan(uint32_t floatInt)
{
	return ((2 * floatInt) - 1) >= ((2u * 0x7F800000) - 1);
}

float __math_xflowf(uint32_t sign, float value)
{
	return eval_as_float(fp_barrierf(sign ? -value : value) * value);
}
float __math_oflowf(uint32_t sign)
{
	return __math_xflowf(sign, 0x1p97F);
}
float __math_uflowf(uint32_t sign)
{
	return __math_xflowf(sign, 0x1p-95F);
}
