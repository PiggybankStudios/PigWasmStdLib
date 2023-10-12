/*
File:   math.c
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds all the implementations for math.h declared functions
*/

// +--------------------------------------------------------------+
// |                    Builtin Usage Controls                    |
// +--------------------------------------------------------------+
//TODO: Many of these have valid __builtin_ functions but they throw some kind of out of bounds error when executed. Maybe we are using the builtins wrong?
#define PIG_WASM_STD_USE_BUILTINS_FMIN_FMAX      0
#define PIG_WASM_STD_USE_BUILTINS_FABS           1
#define PIG_WASM_STD_USE_BUILTINS_FMOD           0
#define PIG_WASM_STD_USE_BUILTINS_ROUND          0
#define PIG_WASM_STD_USE_BUILTINS_FLOOR_CEIL     1
#define PIG_WASM_STD_USE_BUILTINS_SCALBN         0
#define PIG_WASM_STD_USE_BUILTINS_SQRT           0
#define PIG_WASM_STD_USE_BUILTINS_CBRT           0
#define PIG_WASM_STD_USE_BUILTINS_SIN_COS_TAN    0
#define PIG_WASM_STD_USE_BUILTINS_ASIN_ACOS_ATAN 0
#define PIG_WASM_STD_USE_BUILTINS_POW            0
#define PIG_WASM_STD_USE_BUILTINS_LOG            0
#define PIG_WASM_STD_USE_BUILTINS_LOG2           0
#define PIG_WASM_STD_USE_BUILTINS_LOG10          0
#define PIG_WASM_STD_USE_BUILTINS_LDEXP          0
#define PIG_WASM_STD_USE_BUILTINS_COPYSIGN       1

// +--------------------------------------------------------------+
// |                        Float Helpers                         |
// +--------------------------------------------------------------+
static const float_t tointf = 1/((FLT_EVAL_METHOD == 0) ? FLT_EPSILON : DBL_EPSILON);
static const float_t tointd = 1/DBL_EPSILON;

int __fpclassifyf(float value)
{
	union { float value; uint32_t integer; } valueUnion = { value };
	int exponent = (valueUnion.integer >> 23) & 0xFF;
	if (exponent == 0) { return (valueUnion.integer << 1) ? FP_SUBNORMAL : FP_ZERO; }
	if (exponent == 0xFF) { return (valueUnion.integer << 9) ? FP_NAN : FP_INFINITE; }
	return FP_NORMAL;
}
int __fpclassify(double value)
{
	union { double value; uint64_t integer; } valueUnion = { value };
	int e = (valueUnion.integer >> 52) & 0x7FF;
	if (!e) { return (valueUnion.integer << 1) ? FP_SUBNORMAL : FP_ZERO; }
	if (e==0x7FF) { return (valueUnion.integer << 12) ? FP_NAN : FP_INFINITE; }
	return FP_NORMAL;
}

unsigned __FLOAT_BITS(float value)
{
	union { float value; unsigned integer; } valueUnion = { value };
	return valueUnion.integer;
}
unsigned long long __DOUBLE_BITS(double value)
{
	union { double value; unsigned long long integer; } valueUnion = { value };
	return valueUnion.integer;
}

void fp_force_evalf(float value)
{
	volatile float volatileValue;
	volatileValue = value;
}
void fp_force_eval(double value)
{
	volatile double volatileValue;
	volatileValue = value;
}

float __math_invalidf(float value)
{
	return (value - value) / (value - value);
}
double __math_invalid(double value)
{
	return (value - value) / (value - value);
}

float eval_as_float(float x)
{
	float  y = x;
	return y;
}
double eval_as_double(double x)
{
	double y = x;
	return y;
}

#if !PIG_WASM_STD_USE_BUILTINS_SIN_COS_TAN || !PIG_WASM_STD_USE_BUILTINS_ASIN_ACOS_ATAN
#include "math_trig_helpers.c"
#endif

#if !PIG_WASM_STD_USE_BUILTINS_LOG || !PIG_WASM_STD_USE_BUILTINS_LOG2 || !PIG_WASM_STD_USE_BUILTINS_LOG10
#include "math_log_helpers.c"
#endif

#if !PIG_WASM_STD_USE_BUILTINS_SQRT || !PIG_WASM_STD_USE_BUILTINS_CBRT

static inline uint32_t MultiplyU32Overflow(uint32_t left, uint32_t right)
{
	return ((uint64_t)left * right) >> 32;
}
// returns a*b*2^-64 - e, with error 0 <= e < 3.
static inline uint64_t MultiplyU64Overflow(uint64_t a, uint64_t b)
{
	uint64_t ahi = (a >> 32);
	uint64_t alo = (a & 0xFFFFFFFF);
	uint64_t bhi = (b >> 32);
	uint64_t blo = (b & 0xFFFFFFFF);
	return (ahi * bhi) + ((ahi * blo) >> 32) + ((alo * bhi) >> 32);
}

const uint16_t __rsqrt_table[128] = {
	0xb451,0xb2f0,0xb196,0xb044,0xaef9,0xadb6,0xac79,0xab43,
	0xaa14,0xa8eb,0xa7c8,0xa6aa,0xa592,0xa480,0xa373,0xa26b,
	0xa168,0xa06a,0x9f70,0x9e7b,0x9d8a,0x9c9d,0x9bb5,0x9ad1,
	0x99f0,0x9913,0x983a,0x9765,0x9693,0x95c4,0x94f8,0x9430,
	0x936b,0x92a9,0x91ea,0x912e,0x9075,0x8fbe,0x8f0a,0x8e59,
	0x8daa,0x8cfe,0x8c54,0x8bac,0x8b07,0x8a64,0x89c4,0x8925,
	0x8889,0x87ee,0x8756,0x86c0,0x862b,0x8599,0x8508,0x8479,
	0x83ec,0x8361,0x82d8,0x8250,0x81c9,0x8145,0x80c2,0x8040,
	0xff02,0xfd0e,0xfb25,0xf947,0xf773,0xf5aa,0xf3ea,0xf234,
	0xf087,0xeee3,0xed47,0xebb3,0xea27,0xe8a3,0xe727,0xe5b2,
	0xe443,0xe2dc,0xe17a,0xe020,0xdecb,0xdd7d,0xdc34,0xdaf1,
	0xd9b3,0xd87b,0xd748,0xd61a,0xd4f1,0xd3cd,0xd2ad,0xd192,
	0xd07b,0xcf69,0xce5b,0xcd51,0xcc4a,0xcb48,0xca4a,0xc94f,
	0xc858,0xc764,0xc674,0xc587,0xc49d,0xc3b7,0xc2d4,0xc1f4,
	0xc116,0xc03c,0xbf65,0xbe90,0xbdbe,0xbcef,0xbc23,0xbb59,
	0xba91,0xb9cc,0xb90a,0xb84a,0xb78c,0xb6d0,0xb617,0xb560,
};

#endif

float __math_divzerof(uint32_t sign)
{
	return fp_barrierf(sign ? -1.0f : 1.0f) / 0.0f;
}
double __math_divzero(uint32_t sign)
{
	//TODO: This used to have an fp_barrier call around the numerator. Do we need that in WASM?
	return fp_barrierf(sign ? -1.0 : 1.0) / 0.0;
}

uint32_t top16(double x)
{
	return asuint64(x) >> 48;
}

float fp_barrierf(float x)
{
	volatile float y = x;
	return y;
}

// +--------------------------------------------------------------+
// |                        fmin and fmax                         |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_FMIN_FMAX_BUILTINS
inline float fminf(float value1, float value2)   { return __builtin_fminf(value1, value2); }
inline double fmin(double value1, double value2) { return __builtin_fmin(value1,  value2); }
inline float fmaxf(float value1, float value2)   { return __builtin_fmaxf(value1, value2); }
inline double fmax(double value1, double value2) { return __builtin_fmax(value1,  value2); }
#else
float fminf(float value1, float value2)
{
	if (isnan(value1)) { return value2; }
	if (isnan(value2)) { return value1; }
	// handle signed zeros, see C99 Annex F.9.9.2
	if (signbit(value1) != signbit(value2)) { return (signbit(value1) ? value1 : value2); }
	return (value1 < value2 ? value1 : value2);
}
double fmin(double value1, double value2)
{
	if (isnan(value1)) { return value2; }
	if (isnan(value2)) { return value1; }
	// handle signed zeros, see C99 Annex F.9.9.2
	if (signbit(value1) != signbit(value2)) { return (signbit(value1) ? value1 : value2); }
	return (value1 < value2 ? value1 : value2);
}

float fmaxf(float value1, float value2)
{
	if (isnan(value1)) { return value2; }
	if (isnan(value2)) { return value1; }
	// handle signed zeroes, see C99 Annex F.9.9.2
	if (signbit(value1) != signbit(value2)) { return (signbit(value1) ? value2 : value1); }
	return (value1 < value2 ? value2 : value1);
}
double fmax(double value1, double value2)
{
	if (isnan(value1)) { return value2; }
	if (isnan(value2)) { return value1; }
	// handle signed zeroes, see C99 Annex F.9.9.2
	if (signbit(value1) != signbit(value2)) { return (signbit(value1) ? value2 : value1); }
	return (value1 < value2 ? value2 : value1);
}
#endif

// +--------------------------------------------------------------+
// |                        fabs and fabsf                        |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_FABS
inline float fabsf(float value) { return __builtin_fabsf(value); }
inline double fabs(double value) { return __builtin_fabs(value); }
#else
float fabsf(float value)
{
	union { float value; uint32_t integer; } valueUnion = { value };
	valueUnion.integer &= 0x7FFFFFFF;
	return valueUnion.value;
}
double fabs(double value)
{
	union { double value; uint64_t integer; } valueUnion = { value };
	valueUnion.integer &= -1ULL/2;
	return valueUnion.value;
}
#endif

// +--------------------------------------------------------------+
// |                        fmod and fmodf                        |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_FMOD
inline float fmodf(float numer, float denom) { return __builtin_fmodf(numer, denom); }
inline double fmod(double numer, double denom) { return __builtin_fmod(numer, denom); }
#else
float fmodf(float numer, float denom)
{
	union { float value; uint32_t integer; } numerUnion = { numer }, denomUnion = { denom };
	int numerExponent = ((numerUnion.integer >> 23) & 0xFF);
	int denomExponent = ((denomUnion.integer >> 23) & 0xFF);
	uint32_t numerSignBit = (numerUnion.integer & 0x80000000);
	uint32_t integer;
	uint32_t numerInt = numerUnion.integer;
	
	if ((denomUnion.integer << 1) == 0 || isnan(denom) || numerExponent == 0xFF) { return (numer*denom)/(numer*denom); }
	if ((numerInt << 1) <= (denomUnion.integer << 1))
	{
		if ((numerInt << 1) == (denomUnion.integer << 1)) { return 0 * numer; }
		return numer;
	}
	
	// normalize numer and denom
	if (!numerExponent)
	{
		for (integer = (numerInt << 9); (integer >> 31) == 0; numerExponent--, integer <<= 1);
		numerInt <<= -numerExponent + 1;
	}
	else
	{
		numerInt &= -1U >> 9;
		numerInt |= 1U << 23;
	}
	if (!denomExponent)
	{
		for (integer = (denomUnion.integer << 9); (integer >> 31) == 0; denomExponent--, integer <<= 1);
		denomUnion.integer <<= -denomExponent + 1;
	}
	else
	{
		denomUnion.integer &= -1U >> 9;
		denomUnion.integer |= 1U << 23;
	}
	
	// numer mod denom
	for (; numerExponent > denomExponent; numerExponent--)
	{
		integer = numerInt - denomUnion.integer;
		if ((integer >> 31) == 0)
		{
			if (integer == 0) { return 0 * numer; }
			numerInt = integer;
		}
		numerInt <<= 1;
	}
	integer = numerInt - denomUnion.integer;
	if ((integer >> 31) == 0)
	{
		if (integer == 0) { return 0 * numer; }
		numerInt = integer;
	}
	for (; (numerInt >> 23) == 0; numerInt <<= 1, numerExponent--);
	
	// scale result up 
	if (numerExponent > 0)
	{
		numerInt -= 1U << 23;
		numerInt |= (uint32_t)numerExponent << 23;
	}
	else
	{
		numerInt >>= -numerExponent + 1;
	}
	numerInt |= numerSignBit;
	numerUnion.integer = numerInt;
	return numerUnion.value;
}
double fmod(double numer, double denom)
{
	union { double value; uint64_t integer; } numerUnion = { numer }, denomUnion = { denom };
	int numerExponent = (numerUnion.integer >> 52) & 0x7FF;
	int denomExponent = (denomUnion.integer >> 52) & 0x7FF;
	int numerSignBit = (numerUnion.integer >> 63);
	uint64_t integer;
	
	//in the followings numerInt should be numerUnion.integer, but then gcc wrongly adds 
	//float load/store to inner loops ruining performance and code size
	uint64_t numerInt = numerUnion.integer;
	
	if ((denomUnion.integer << 1) == 0 || isnan(denom) || numerExponent == 0x7FF) { return (numer*denom) / (numer*denom); }
	if ((numerInt << 1) <= (denomUnion.integer << 1))
	{
		if ((numerInt << 1) == (denomUnion.integer << 1)) { return 0 * numer; }
		return numer;
	}
	
	//normalize x and y
	if (!numerExponent)
	{
		for (integer = (numerInt << 12); (integer >> 63) == 0; numerExponent--, integer <<= 1);
		numerInt <<= -numerExponent + 1;
	}
	else
	{
		numerInt &= -1ULL >> 12;
		numerInt |= 1ULL << 52;
	}
	if (!denomExponent)
	{
		for (integer = (denomUnion.integer << 12); (integer >> 63) == 0; denomExponent--, integer <<= 1);
		denomUnion.integer <<= -denomExponent + 1;
	}
	else
	{
		denomUnion.integer &= -1ULL >> 12;
		denomUnion.integer |= 1ULL << 52;
	}
	
	//x mod y
	for (; numerExponent > denomExponent; numerExponent--)
	{
		integer = numerInt - denomUnion.integer;
		if ((integer >> 63) == 0)
		{
			if (integer == 0) { return 0 * numer; }
			numerInt = integer;
		}
		numerInt <<= 1;
	}
	integer = numerInt - denomUnion.integer;
	if ((integer >> 63) == 0)
	{
		if (integer == 0) { return 0 * numer; }
		numerInt = integer;
	}
	for (; (numerInt >> 52) == 0; numerInt <<= 1, numerExponent--);
	
	//scale result
	if (numerExponent > 0)
	{
		numerInt -= 1ULL << 52;
		numerInt |= (uint64_t)numerExponent << 52;
	}
	else
	{
		numerInt >>= -numerExponent + 1;
	}
	numerInt |= (uint64_t)numerSignBit << 63;
	numerUnion.integer = numerInt;
	return numerUnion.value;
}
#endif

// +--------------------------------------------------------------+
// |                       round and roundf                       |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_ROUND
inline float roundf(float value) { return __builtin_roundf(value); }
inline double round(double value) { return __builtin_round(value); }
#else
float roundf(float value)
{
	union { float value; uint32_t integer; } valueUnion = { value };
	int valueExponent = ((valueUnion.integer >> 23) & 0xFF);
	float_t result;
	
	if (valueExponent >= 0x7F+23) { return value; }
	if (valueUnion.integer >> 31) { value = -value; }
	if (valueExponent < 0x7F-1)
	{
		//raise inexact if value!=0 
		FORCE_EVAL(value + tointf);
		return 0*valueUnion.value;
	}
	result = value + tointf - tointf - value;
	if (result > 0.5f) { result = result + value - 1; }
	else if (result <= -0.5f) { result = result + value + 1; }
	else { result = result + value; }
	if (valueUnion.integer >> 31) { result = -result; }
	return result;
}
double round(double value)
{
	union { double value; uint64_t integer; } valueUnion = { value };
	int valueExponent = ((valueUnion.integer >> 52) & 0x7FF);
	double_t result;
	
	if (valueExponent >= 0x3FF+52) { return value; }
	if (valueUnion.integer >> 63) { value = -value; }
	if (valueExponent < 0x3FF-1)
	{
		//raise inexact if value!=0 
		FORCE_EVAL(value + tointd);
		return 0 * valueUnion.value;
	}
	result = value + tointd - tointd - value;
	if (result > 0.5) { result = result + value - 1; }
	else if (result <= -0.5) { result = result + value + 1; }
	else { result = result + value; }
	if (valueUnion.integer >> 63) { result = -result; }
	return result;
}
#endif

// +--------------------------------------------------------------+
// |                        floor and ceil                        |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_FLOOR_CEIL
inline float _floorf(float value) { return __builtin_floorf(value); }
inline double _floor(double value) { return __builtin_floor(value); }
inline float _ceilf(float value) { return __builtin_ceilf(value); }
inline double _ceil(double value) { return __builtin_ceil(value); }
#else
float _floorf(float value)
{
	union { float value; uint32_t integer; } valueUnion = { value };
	int valueExponent = (int)((valueUnion.integer >> 23) & 0xFF) - 0x7F;
	uint32_t temp;
	
	if (valueExponent >= 23) { return value; }
	if (valueExponent >= 0)
	{
		temp = (0x007FFFFF >> valueExponent);
		if ((valueUnion.integer & temp) == 0) { return value; }
		FORCE_EVAL(value + 0x1p120F);
		if (valueUnion.integer >> 31) { valueUnion.integer += temp; }
		valueUnion.integer &= ~temp;
	}
	else
	{
		FORCE_EVAL(value + 0x1p120F);
		if (valueUnion.integer >> 31 == 0) { valueUnion.integer = 0; }
		else if (valueUnion.integer << 1) { valueUnion.value = -1.0; }
	}
	return valueUnion.value;
}
double _floor(double value)
{
	union { double value; uint64_t integer; } valueUnion = { value };
	int valueExponent = ((valueUnion.integer >> 52) & 0x7FF);
	double_t result;
	
	if (valueExponent >= 0x3FF+52 || value == 0) { return value; }
	// result = int(value) - value, where int(value) is an integer neighbor of value
	if (valueUnion.integer >> 63) { result = value - tointd + tointd - value; }
	else { result = value + tointd - tointd - value; }
	// special case because of non-nearest rounding modes
	if (valueExponent <= 0x3FF-1)
	{
		FORCE_EVAL(result);
		return ((valueUnion.integer >> 63) ? -1 : 0);
	}
	if (result > 0) { return value + result - 1; }
	return value + result;
}

float _ceilf(float value)
{
	union { float value; uint32_t integer; } valueUnion = { value };
	int valueExponent = (int)((valueUnion.integer >> 23) & 0xff) - 0x7f;
	uint32_t temp;
	
	if (valueExponent >= 23) { return value; }
	if (valueExponent >= 0)
	{
		temp = (0x007FFFFF >> valueExponent);
		if ((valueUnion.integer & temp) == 0) { return value; }
		FORCE_EVAL(value + 0x1p120F);
		if (valueUnion.integer >> 31 == 0) { valueUnion.integer += temp; }
		valueUnion.integer &= ~temp;
	}
	else
	{
		FORCE_EVAL(value + 0x1p120F);
		if (valueUnion.integer >> 31) { valueUnion.value = -0.0; }
		else if (valueUnion.integer << 1) { valueUnion.value = 1.0; }
	}
	return valueUnion.value;
}
double _ceil(double value)
{
	union { double value; uint64_t integer; } valueUnion = { value };
	int valueExponent = ((valueUnion.integer >> 52) & 0x7FF);
	double_t temp;
	
	if (valueExponent >= 0x3ff+52 || value == 0) { return value; }
	// temp = int(value) - value, where int(value) is an integer neighbor of value
	if (valueUnion.integer >> 63) { temp = value - tointd + tointd - value; }
	else { temp = value + tointd - tointd - value; }
	// special case because of non-nearest rounding modes
	if (valueExponent <= 0x3FF-1)
	{
		FORCE_EVAL(temp);
		return ((valueUnion.integer >> 63) ? -0.0 : 1);
	}
	if (temp < 0) { return value + temp + 1; }
	return value + temp;
}
#endif

// +--------------------------------------------------------------+
// |                      scalbn and scalbnf                      |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_SCALBN
inline float _scalbnf(float value, int power)  { jsPrintString("Called scalbnf"); return __builtin_scalbnf(value, power); }
inline double _scalbn(double value, int power) { jsPrintString("Called scalbn"); return __builtin_scalbn(value,  power); }
#else
float _scalbnf(float value, int power)
{
	union { float value; uint32_t integer; } valueUnion;
	float_t result = value;
	
	if (power > 127)
	{
		result *= 0x1p127F;
		power -= 127;
		if (power > 127)
		{
			result *= 0x1p127F;
			power -= 127;
			if (power > 127) { power = 127; }
		}
	}
	else if (power < -126)
	{
		result *= 0x1p-126F * 0x1p24F;
		power += 126 - 24;
		if (power < -126)
		{
			result *= 0x1p-126F * 0x1p24F;
			power += 126 - 24;
			if (power < -126) { power = -126; }
		}
	}
	valueUnion.integer = ((uint32_t)(0x7F + power) << 23);
	result = result * valueUnion.value;
	return result;
}
double _scalbn(double value, int power)
{
	union { double value; uint64_t integer; } valueUnion;
	double_t result = value;
	
	if (power > 1023)
	{
		result *= 0x1p1023;
		power -= 1023;
		if (power > 1023)
		{
			result *= 0x1p1023;
			power -= 1023;
			if (power > 1023) { power = 1023; }
		}
	}
	else if (power < -1022)
	{
		// make sure final power < -53 to avoid double
		// rounding in the subnormal range
		result *= 0x1p-1022 * 0x1p53;
		power += 1022 - 53;
		if (power < -1022)
		{
			result *= 0x1p-1022 * 0x1p53;
			power += 1022 - 53;
			if (power < -1022) { power = -1022; }
		}
	}
	valueUnion.integer = ((uint64_t)(0x3FF + power) << 52);
	result = result * valueUnion.value;
	return result;
}
#endif

// +--------------------------------------------------------------+
// |                        sqrt and sqrtf                        |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_SQRT
inline float sqrtf(float value)  { return __builtin_sqrtf(value); }
inline double sqrt(double value) { return __builtin_sqrt(value);  }
#else
float sqrtf(float value)
{
	uint32_t valueInt, mVar, mVar1, mVar0, even, ey;
	
	jsPrintString("Called sqrtf!");
	valueInt = asuint(value);
	if (predict_false(valueInt - 0x00800000 >= 0x7F800000 - 0x00800000))
	{
		// value < 0x1p-126 or inf or nan.
		if (valueInt * 2 == 0) { return value; }
		if (valueInt == 0x7F800000) { return value; }
		if (valueInt > 0x7F800000) { return __math_invalidf(value); }
		// value is subnormal, normalize it.
		valueInt = asuint(value * 0x1p23f);
		valueInt -= (23 << 23);
	}
	
	// value = 4^e mVar; with int e and mVar in [1, 4).
	even = (valueInt & 0x00800000);
	mVar1 = ((valueInt << 8) | 0x80000000);
	mVar0 = ((valueInt << 7) & 0x7FFFFFFF);
	mVar = (even ? mVar0 : mVar1);
	
	// 2^e is the exponent part of the return value.
	ey = (valueInt >> 1);
	ey += (0x3F800000 >> 1);
	ey &= 0x7F800000;
	
	// compute rVar ~ 1/sqrt(mVar), sVar ~ sqrt(mVar) with 2 goldschmidt iterations.
	static const uint32_t three = 0xC0000000;
	uint32_t rVar, sVar, dVar, uVar, iVar;
	iVar = (valueInt >> 17) % 128;
	rVar = (uint32_t)__rsqrt_table[iVar] << 16;
	// |rVar*sqrt(mVar) - 1| < 0x1p-8
	sVar = MultiplyU32Overflow(mVar, rVar);
	// |sVar/sqrt(mVar) - 1| < 0x1p-8
	dVar = MultiplyU32Overflow(sVar, rVar);
	uVar = three - dVar;
	rVar = MultiplyU32Overflow(rVar, uVar) << 1;
	// |rVar*sqrt(mVar) - 1| < 0x1.7bp-16
	sVar = MultiplyU32Overflow(sVar, uVar) << 1;
	// |sVar/sqrt(mVar) - 1| < 0x1.7bp-16
	dVar = MultiplyU32Overflow(sVar, rVar);
	uVar = three - dVar;
	sVar = MultiplyU32Overflow(sVar, uVar);
	// -0x1.03p-28 < sVar/sqrt(mVar) - 1 < 0x1.fp-31
	sVar = (sVar - 1)>>6;
	// sVar < sqrt(mVar) < sVar + 0x1.08p-23
	
	// compute nearest rounded result. 
	uint32_t dVar0, dVar1, dVar2;
	float result, tVar;
	dVar0 = (mVar << 16) - (sVar * sVar);
	dVar1 = sVar - dVar0;
	dVar2 = dVar1 + sVar + 1;
	sVar += (dVar1 >> 31);
	sVar &= 0x007FFFFF;
	sVar |= ey;
	result = asfloat(sVar);
	// handle rounding and inexact exception.
	uint32_t tiny = (predict_false(dVar2 == 0) ? 0 : 0x01000000);
	tiny |= ((dVar1 ^ dVar2) & 0x80000000);
	tVar = asfloat(tiny);
	result = eval_as_float(result + tVar);
	return result;
}
double sqrt(double value)
{
	uint64_t valueInt, top, mVar;
	
	// special case handling.
	valueInt = asuint64(value);
	top = (valueInt >> 52);
	if (predict_false((top - 0x001) >= (0x7FF - 0x001)))
	{
		// value < 0x1p-1022 or inf or nan.
		if (valueInt * 2 == 0) { return value; }
		if (valueInt == 0x7FF0000000000000) { return value; }
		if (valueInt > 0x7FF0000000000000) { return __math_invalid(value); }
		// value is subnormal, normalize it.
		valueInt = asuint64(value * 0x1p52);
		top = valueInt >> 52;
		top -= 52;
	}
	
	// argument reduction:
	// value = 4^e mVar; with integer e, and mVar in [1, 4)
	// mVar: fixed point representation [2.62]
	// 2^e is the exponent part of the result.
	int even = (top & 1);
	mVar = ((valueInt << 11) | 0x8000000000000000);
	if (even) { mVar >>= 1; }
	top = ((top + 0x3FF) >> 1);
	
	/* approximate rVar ~ 1/sqrt(mVar) and sVar ~ sqrt(mVar) when mVar in [1,4)

	   initial estimate:
	   7bit table lookup (1bit exponent and 6bit significand).

	   iterative approximation:
	   using 2 goldschmidt iterations with 32bit int arithmetics
	   and a final iteration with 64bit int arithmetics.

	   details:

	   the relative error (e = r0 sqrt(mVar)-1) of a linear estimate
	   (r0 = a mVar + b) is |e| < 0.085955 ~ 0x1.6p-4 at best,
	   a table lookup is faster and needs one less iteration
	   6 bit lookup table (128b) gives |e| < 0x1.f9p-8
	   7 bit lookup table (256b) gives |e| < 0x1.fdp-9
	   for single and double prec 6bit is enough but for quad
	   prec 7bit is needed (or modified iterations). to avoid
	   one more iteration >=13bit table would be needed (16k).

	   a newton-raphson iteration for rVar is
	     w = rVar*rVar
	     uVar = 3 - mVar*w
	     rVar = rVar*uVar/2
	   can use a goldschmidt iteration for sVar at the end or
	     sVar = mVar*rVar

	   first goldschmidt iteration is
	     sVar = mVar*rVar
	     uVar = 3 - sVar*rVar
	     rVar = rVar*uVar/2
	     sVar = sVar*uVar/2
	   next goldschmidt iteration is
	     uVar = 3 - sVar*rVar
	     rVar = rVar*uVar/2
	     sVar = sVar*uVar/2
	   and at the end rVar is not computed only sVar.

	   they use the same amount of operations and converge at the
	   same quadratic rate, i.e. if
	     r1 sqrt(mVar) - 1 = e, then
	     r2 sqrt(mVar) - 1 = -3/2 e^2 - 1/2 e^3
	   the advantage of goldschmidt is that the mul for sVar and rVar
	   are independent (computed in parallel), however it is not
	   "self synchronizing": it only uses the input mVar in the
	   first iteration so rounding errors accumulate. at the end
	   or when switching to larger precision arithmetics rounding
	   errors dominate so the first iteration should be used.

	   the fixed point representations are
	     mVar: 2.30 rVar: 0.32, sVar: 2.30, dVar: 2.30, uVar: 2.30, three: 2.30
	   and after switching to 64 bit
	     mVar: 2.62 rVar: 0.64, sVar: 2.62, dVar: 2.62, uVar: 2.62, three: 2.62
	*/
	
	static const uint64_t three = 0xC0000000;
	uint64_t rVar, sVar, dVar, uVar, tableIndex;
	
	tableIndex = ((valueInt >> 46) % 128);
	rVar = ((uint32_t)__rsqrt_table[tableIndex] << 16);
	// |rVar sqrt(mVar) - 1| < 0x1.fdp-9
	sVar = MultiplyU32Overflow((mVar >> 32), rVar);
	// |sVar/sqrt(mVar) - 1| < 0x1.fdp-9
	dVar = MultiplyU32Overflow(sVar, rVar);
	uVar = three - dVar;
	rVar = (MultiplyU32Overflow(rVar, uVar) << 1);
	// |rVar sqrt(mVar) - 1| < 0x1.7bp-16
	sVar = (MultiplyU32Overflow(sVar, uVar) << 1);
	// |sVar/sqrt(mVar) - 1| < 0x1.7bp-16
	dVar = MultiplyU32Overflow(sVar, rVar);
	uVar = three - dVar;
	rVar = (MultiplyU32Overflow(rVar, uVar) << 1);
	// |rVar sqrt(mVar) - 1| < 0x1.3704p-29 (measured worst-case)
	rVar = (rVar << 32);
	sVar = MultiplyU64Overflow(mVar, rVar);
	dVar = MultiplyU64Overflow(sVar, rVar);
	uVar = (three << 32) - dVar;
	sVar = MultiplyU64Overflow(sVar, uVar);  // repr: 3.61
	// -0x1p-57 < sVar - sqrt(mVar) < 0x1.8001p-61
	sVar = ((sVar - 2) >> 9); // repr: 12.52
	// -0x1.09p-52 < sVar - sqrt(mVar) < -0x1.fffcp-63
	
	// sVar < sqrt(mVar) < sVar + 0x1.09p-52,
	// compute nearest rounded result:
	// the nearest result to 52 bits is either sVar or sVar+0x1p-52,
	// we can decide by comparing (2^52 sVar + 0.5)^2 to 2^104 mVar.
	uint64_t dVar0, dVar1, dVar2;
	double result, tVar;
	dVar0 = (mVar << 42) - (sVar * sVar);
	dVar1 = sVar - dVar0;
	dVar2 = dVar1 + sVar + 1;
	sVar += (dVar1 >> 63);
	sVar &= 0x000FFFFFFFFFFFFF;
	sVar |= (top << 52);
	result = asdouble(sVar);
	// handle rounding modes and inexact exception:
	// only (sVar+1)^2 == 2^42 mVar case is exact otherwise
	// add a tiny value to cause the fenv effects.
	uint64_t tiny = (predict_false(dVar2==0) ? 0 : 0x0010000000000000);
	tiny |= ((dVar1 ^ dVar2) & 0x8000000000000000);
	tVar = asdouble(tiny);
	result = eval_as_double(result + tVar);
	return result;
}
#endif

// +--------------------------------------------------------------+
// |                        cbrt and cbrtf                        |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_CBRT
inline float _cbrtf(float value)  { return __builtin_cbrtf(value); }
inline double _cbrt(double value) { return __builtin_cbrt(value);  }
#else
static const unsigned
	B1 = 709958130, // B1 = (127-127.0/3-0.03306235651)*2**23
	B2 = 642849266; // B2 = (127-127.0/3-24/3-0.03306235651)*2**23
static const uint32_t
	Bd1 = 715094163, // B1 = (1023-1023/3-0.03306235651)*2**20
	Bd2 = 696219795; // B2 = (1023-1023/3-54/3-0.03306235651)*2**20
// |1/cbrt(x) - p(x)| < 2**-23.5 (~[-7.93e-8, 7.929e-8]).
static const double
	P0 =  1.87595182427177009643,  // 0x3ffe03e6, 0x0f61e692
	P1 = -1.88497979543377169875,  // 0xbffe28e0, 0x92f02420
	P2 =  1.621429720105354466140, // 0x3ff9f160, 0x4a49d6c2
	P3 = -0.758397934778766047437, // 0xbfe844cb, 0xbee751d9
	P4 =  0.145996192886612446982; // 0x3fc2b000, 0xd4e4edd7

float _cbrtf(float value)
{
	double_t tVarCubed, tVar;
	union { float value; uint32_t integer; } valueUnion = { value };
	uint32_t valueUnsigned = (valueUnion.integer & 0x7FFFFFFF);
	
	if (valueUnsigned >= 0x7F800000) { return (value + value); } // cbrt(NaN,INF) is itself
	
	// rough cbrt to 5 bits
	if (valueUnsigned < 0x00800000) // zero or subnormal?
	{
		if (valueUnsigned == 0) { return value; } // cbrt(+-0) is itself
		valueUnion.value = (value * 0x1p24F);
		valueUnsigned = (valueUnion.integer & 0x7FFFFFFF);
		valueUnsigned = (valueUnsigned / 3) + B2;
	}
	else { valueUnsigned = (valueUnsigned / 3) + B1; }
	valueUnion.integer &= 0x80000000;
	valueUnion.integer |= valueUnsigned;
	
	// First step Newton iteration (solving t*t-value/t == 0) to 16 bits.  In
	// double precision so that its terms can be arranged for efficiency
	// without causing overflow or underflow.
	tVar = valueUnion.value;
	tVarCubed = (tVar * tVar * tVar);
	tVar = tVar * ((double_t)value + value + tVarCubed) / (value + tVarCubed + tVarCubed);
	
	// Second step Newton iteration to 47 bits.  In double precision for
	// efficiency and accuracy.
	tVarCubed = (tVar * tVar * tVar);
	tVar = tVar * ((double_t)value + value + tVarCubed) / (value + tVarCubed + tVarCubed);
	
	// rounding to 24 bits is perfect in round-to-nearest mode
	return tVar;
}
double _cbrt(double value)
{
	union { double value; uint64_t integer; } valueUnion = { value };
	double_t rVar, sVar, tVar, wVar;
	uint32_t upperUnsigned = ((valueUnion.integer >> 32) & 0x7FFFFFFF);
	
	if (upperUnsigned >= 0x7FF00000) { return (value + value); } // cbrt(NaN,INF) is itself
	
	// Rough cbrt to 5 bits:
	//    cbrt(2**e*(1+m) ~= 2**(e/3)*(1+(e%3+m)/3)
	// where e is integral and >= 0, m is real and in [0, 1), and "/" and
	// "%" are integer division and modulus with rounding towards minus
	// infinity.  The RHS is always >= the LHS and has a maximum relative
	// error of about 1 in 16.  Adding a bias of -0.03306235651 to the
	// (e%3+m)/3 term reduces the error to about 1 in 32. With the IEEE
	// floating point representation, for finite positive normal values,
	// ordinary integer divison of the value in bits magically gives
	// almost exactly the RHS of the above provided we first subtract the
	// exponent bias (1023 for doubles) and later add it back.  We do the
	// subtraction virtually to keep e >= 0 so that ordinary integer
	// division rounds towards minus infinity; this is also efficient.
	if (upperUnsigned < 0x00100000) // zero or subnormal?
	{
		valueUnion.value = value * 0x1p54;
		upperUnsigned = ((valueUnion.integer >> 32) & 0x7FFFFFFF);
		if (upperUnsigned == 0) { return value; } // cbrt(0) is itself
		upperUnsigned = upperUnsigned/3 + Bd2;
	}
	else { upperUnsigned = upperUnsigned/3 + Bd1; }
	valueUnion.integer &= (1ULL << 63);
	valueUnion.integer |= (uint64_t)upperUnsigned << 32;
	tVar = valueUnion.value;
	
	// New cbrt to 23 bits:
	//    cbrt(value) = tVar*cbrt(value/tVar**3) ~= tVar*P(tVar**3/value)
	// where P(rVar) is a polynomial of degree 4 that approximates 1/cbrt(rVar)
	// to within 2**-23.5 when |rVar - 1| < 1/10.  The rough approximation
	// has produced tVar such than |tVar/cbrt(value) - 1| ~< 1/32, and cubing this
	// gives us bounds for rVar = tVar**3/value.
	//
	// Try to optimize for parallel evaluation as in __tanf.c.
	rVar = (tVar * tVar) * (tVar / value);
	tVar = tVar * ((P0 + (rVar * (P1 + (rVar * P2)))) + ((rVar * rVar) * rVar) * (P3 + (rVar * P4)));
	
	// Round tVar away from zero to 23 bits (sloppily except for ensuring that
	// the result is larger in magnitude than cbrt(value) but not much more than
	// 2 23-bit ulps larger).  With rounding towards zero, the error bound
	// would be ~5/6 instead of ~4/6.  With a maximum error of 2 23-bit ulps
	// in the rounded tVar, the infinite-precision error in the Newton
	// approximation barely affects third digit in the final error
	// 0.667; the error in the rounded tVar can be up to about 3 23-bit ulps
	// before the final error is larger than 0.667 ulps.
	valueUnion.value = tVar;
	valueUnion.integer = ((valueUnion.integer + 0x80000000) & 0xFFFFFFFFC0000000ULL);
	tVar = valueUnion.value;
	
	// one step Newton iteration to 53 bits with error < 0.667 ulps
	sVar = tVar * tVar;         // tVar*tVar is exact
	rVar = value / sVar;         // error <= 0.5 ulps; |rVar| < |tVar|
	wVar = tVar + tVar;         // tVar+tVar is exact
	rVar = (rVar - tVar) / (wVar + rVar); // rVar-tVar is exact; wVar+rVar ~= 3*tVar
	tVar = tVar + (tVar * rVar);       // error <= 0.5 + 0.5/3 + epsilon
	return tVar;
}
#endif

// +--------------------------------------------------------------+
// |                       sin cos and tan                        |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_SIN_COS_TAN
inline float sinf(float value)  { return __builtin_sinf(value); }
inline double sin(double value) { return __builtin_sin(value);  }
inline float cosf(float value)  { return __builtin_cosf(value); }
inline double cos(double value) { return __builtin_cos(value);  }
inline float tanf(float value)  { return __builtin_tanf(value); }
inline double tan(double value) { return __builtin_tan(value);  }
#else

// Small multiples of pi/2 rounded to double precision.
static const double
	pio2_1x = 1*M_PI_2, // 0x3FF921FB, 0x54442D18
	pio2_2x = 2*M_PI_2, // 0x400921FB, 0x54442D18
	pio2_3x = 3*M_PI_2, // 0x4012D97C, 0x7F3321D2
	pio2_4x = 4*M_PI_2; // 0x401921FB, 0x54442D18

float sinf(float value)
{
	double result;
	uint32_t invValue;
	int temp, sign;
	
	GET_FLOAT_WORD(invValue, value);
	sign = (invValue >> 31);
	invValue &= 0x7FFFFFFF;
	
	if (invValue <= 0x3F490FDA) // |value| ~<= pi/4
	{
		if (invValue < 0x39800000) // |value| < 2**-12
		{
			// raise inexact if value!=0 and underflow if subnormal
			FORCE_EVAL((invValue < 0x00800000) ? value / 0x1p120F : value + 0x1p120F);
			return value;
		}
		return __sindf(value);
	}
	if (invValue <= 0x407B53D1) // |value| ~<= 5*pi/4
	{
		if (invValue <= 0x4016CBE3) // |value| ~<= 3pi/4
		{
			if (sign) { return -__cosdf(value + pio2_1x); }
			else { return __cosdf(value - pio2_1x); }
		}
		return __sindf(sign ? -(value + pio2_2x) : -(value - pio2_2x));
	}
	if (invValue <= 0x40E231D5) // |value| ~<= 9*pi/4
	{
		if (invValue <= 0x40AFEDDF) // |value| ~<= 7*pi/4
		{
			if (sign) { return __cosdf(value + pio2_3x); }
			else { return -__cosdf(value - pio2_3x); }
		}
		return __sindf(sign ? value + pio2_4x : value - pio2_4x);
	}
	
	// sin(Inf or NaN) is NaN
	if (invValue >= 0x7F800000) { return value - value; }
	
	// general argument reduction needed
	temp = __rem_pio2f(value, &result);
	switch (temp & 3)
	{
		case 0:  return  __sindf(result);
		case 1:  return  __cosdf(result);
		case 2:  return  __sindf(-result);
		default: return -__cosdf(result);
	}
}
double sin(double value)
{
	double result[2];
	uint32_t highWord;
	unsigned n; //TODO: give this a better name
	
	// High word of value.
	GET_HIGH_WORD(highWord, value);
	highWord &= 0x7FFFFFFF;
	
	// |value| ~< pi/4
	if (highWord <= 0x3FE921FB)
	{
		if (highWord < 0x3e500000) // |value| < 2**-26
		{
			// raise inexact if value != 0 and underflow if subnorma
			FORCE_EVAL((highWord < 0x00100000) ? value / 0x1p120f : value + 0x1p120f);
			return value;
		}
		return __sin(value, 0.0, 0);
	}
	
	// sin(Inf or NaN) is NaN
	if (highWord >= 0x7ff00000) { return value - value; }
	
	// argument reduction needed
	n = __rem_pio2(value, result);
	switch (n&3)
	{
		case 0:  return  __sin(result[0], result[1], 1);
		case 1:  return  __cos(result[0], result[1]);
		case 2:  return -__sin(result[0], result[1], 1);
		default: return -__cos(result[0], result[1]);
	}
}

float cosf(float value)
{
	double result;
	uint32_t valueWord;
	unsigned n, sign; //TODO: give this a better name

	GET_FLOAT_WORD(valueWord, value);
	sign = valueWord >> 31;
	valueWord &= 0x7FFFFFFF;

	if (valueWord <= 0x3F490FDA) // |value| ~<= pi/4
	{
		if (valueWord < 0x39800000) // |value| < 2**-12
		{
			// raise inexact if value != 0
			FORCE_EVAL(value + 0x1p120F);
			return 1.0f;
		}
		return __cosdf(value);
	}
	if (valueWord <= 0x407B53D1) // |value| ~<= 5*pi/4
	{
		if (valueWord > 0x4016CBE3) // |value|  ~> 3*pi/4
		{
			return -__cosdf(sign ? value+pio2_2x : value-pio2_2x);
		}
		else
		{
			if (sign) { return __sindf(value + pio2_1x); }
			else { return __sindf(pio2_1x - value); }
		}
	}
	if (valueWord <= 0x40E231D5) // |value| ~<= 9*pi/4
	{
		if (valueWord > 0x40AFEDDF) // |value| ~> 7*pi/4
		{
			return __cosdf(sign ? value+pio2_4x : value-pio2_4x);
		}
		else
		{
			if (sign) { return __sindf(-value - pio2_3x); }
			else { return __sindf(value - pio2_3x); }
		}
	}

	// cos(Inf or NaN) is NaN
	if (valueWord >= 0x7F800000) { return value-value; }

	// general argument reduction needed
	n = __rem_pio2f(value, &result);
	switch (n & 3)
	{
		case 0:  return  __cosdf(result);
		case 1:  return  __sindf(-result);
		case 2:  return -__cosdf(result);
		default: return  __sindf(result);
	}
}
double cos(double value)
{
	double result[2];
	uint32_t highWord;
	unsigned n; //TODO: give this a better name

	GET_HIGH_WORD(highWord, value);
	highWord &= 0x7FFFFFFF;

	// |value| ~< pi/4
	if (highWord <= 0x3FE921FB)
	{
		if (highWord < 0x3E46A09E) // |value| < 2**-27 * sqrt(2)
		{
			// raise inexact if value!=0
			FORCE_EVAL(value + 0x1p120F);
			return 1.0;
		}
		return __cos(value, 0);
	}

	// cos(Inf or NaN) is NaN
	if (highWord >= 0x7FF00000) { return (value - value); }

	// argument reduction
	n = __rem_pio2(value, result);
	switch (n & 3)
	{
		case 0: return  __cos(result[0], result[1]);
		case 1: return -__sin(result[0], result[1], 1);
		case 2: return -__cos(result[0], result[1]);
		default: return  __sin(result[0], result[1], 1);
	}
}

float tanf(float value)
{
	double result;
	uint32_t valueWord;
	unsigned n, sign; //TODO: give this a better name
	
	GET_FLOAT_WORD(valueWord, value);
	sign = (valueWord >> 31);
	valueWord &= 0x7FFFFFFF;
	
	if (valueWord <= 0x3F490FDA) // |value| ~<= pi/4
	{
		if (valueWord < 0x39800000) // |value| < 2**-12
		{
			// raise inexact if value!=0 and underflow if subnormal
			FORCE_EVAL((valueWord < 0x00800000) ? value / 0x1p120F : value + 0x1p120F);
			return value;
		}
		return __tandf(value, 0);
	}
	if (valueWord <= 0x407B53D1) // |value| ~<= 5*pi/4
	{
		if (valueWord <= 0x4016CBE3) // |value| ~<= 3pi/4
		{
			return __tandf((sign ? value + pio2_1x : value - pio2_1x), 1);
		}
		else
		{
			return __tandf((sign ? value + pio2_2x : value - pio2_2x), 0);
		}
	}
	if (valueWord <= 0x40E231D5) // |value| ~<= 9*pi/4
	{
		if (valueWord <= 0x40AFEDDF) // |value| ~<= 7*pi/4
		{
			return __tandf((sign ? value + pio2_3x : value - pio2_3x), 1);
		}
		else
		{
			return __tandf((sign ? value + pio2_4x : value - pio2_4x), 0);
		}
	}
	
	// tan(Inf or NaN) is NaN
	if (valueWord >= 0x7F800000) { return (value - value); }
	
	// argument reduction
	n = __rem_pio2f(value, &result);
	return __tandf(result, (n & 1));
}
double tan(double value)
{
	double result[2];
	uint32_t highWord;
	unsigned n; //TODO: give this a better name

	GET_HIGH_WORD(highWord, value);
	highWord &= 0x7FFFFFFF;

	// |value| ~< pi/4
	if (highWord <= 0x3FE921FB)
	{
		if (highWord < 0x3E400000) // |value| < 2**-27
		{
			// raise inexact if value!=0 and underflow if subnormal
			FORCE_EVAL((highWord < 0x00100000) ? value / 0x1p120F : value + 0x1p120F);
			return value;
		}
		return __tan(value, 0.0, 0);
	}

	// tan(Inf or NaN) is NaN
	if (highWord >= 0x7FF00000) { return (value - value); }

	// argument reduction
	n = __rem_pio2(value, result);
	return __tan(result[0], result[1], (n & 1));
}
#endif

// +--------------------------------------------------------------+
// |                   asin acos atan and atan2                   |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_ASIN_ACOS_ATAN
inline float asinf(float value)                 { return __builtin_asinf(value);         }
inline double asin(double value)                { return __builtin_asin(value);          }
inline float acosf(float value)                 { return __builtin_acosf(value);         }
inline double acos(double value)                { return __builtin_acos(value);          }
inline float atanf(float value)                 { return __builtin_atanf(value);         }
inline double atan(double value)                { return __builtin_atan(value);          }
inline float atan2f(float numer,  float denom)  { return __builtin_atan2f(numer, denom); }
inline double atan2(double numer, double denom) { return __builtin_atan2(numer,  denom); }
#else
float asinf(float value)
{
	double sqrtZ;
	float zVar;
	uint32_t wordValue, unsignedValue;
	
	GET_FLOAT_WORD(wordValue, value);
	unsignedValue = wordValue & 0x7FFFFFFF;
	if (unsignedValue >= 0x3F800000) // |value| >= 1
	{
		if (unsignedValue == 0x3F800000) // |value| == 1
		{
			return (value * pio2) + 0x1p-120f;  // asin(+-1) = +-pi/2 with inexact
		}
		return 0 / (value - value);  // asin(|value|>1) is NaN
	}
	if (unsignedValue < 0x3f000000) // |value| < 0.5
	{
		// if 0x1p-126 <= |value| < 0x1p-12, avoid raising underflow
		if (unsignedValue < 0x39800000 && unsignedValue >= 0x00800000) { return value; }
		return value + value * asinf_helper(value * value);
	}
	// 1 > |value| >= 0.5
	zVar = (1 - fabsf(value)) * 0.5f;
	sqrtZ = sqrt(zVar);
	value = pio2 - 2 * (sqrtZ + (sqrtZ * asinf_helper(zVar)));
	if (wordValue >> 31) { return -value; }
	return value;
}
double asin(double value)
{
	double zVar, rVar, sVar;
	uint32_t highWord, valueUnsigned;
	
	GET_HIGH_WORD(highWord, value);
	valueUnsigned = (highWord & 0x7FFFFFFF);
	// |value| >= 1 or nan
	if (valueUnsigned >= 0x3FF00000)
	{
		uint32_t lowWord;
		GET_LOW_WORD(lowWord, value);
		if (((valueUnsigned - 0x3FF00000) | lowWord) == 0)
		{
			// asin(1) = +-pi/2 with inexact
			return (value * pio2d_hi) + 0x1p-120F;
		}
		return 0 / (value - value);
	}
	// |value| < 0.5
	if (valueUnsigned < 0x3FE00000)
	{
		// if 0x1p-1022 <= |value| < 0x1p-26, avoid raising underflow
		if (valueUnsigned < 0x3E500000 && valueUnsigned >= 0x00100000) { return value; }
		return value + (value * asin_helper(value * value));
	}
	// 1 > |value| >= 0.5
	zVar = (1 - fabs(value)) * 0.5;
	sVar = sqrt(zVar);
	rVar = asin_helper(zVar);
	if (valueUnsigned >= 0x3FEF3333) // if |value| > 0.975
	{
		value = pio2d_hi - (2 * (sVar + (sVar * rVar)) - pio2d_lo);
	}
	else
	{
		double fVar, cVar;
		// fVar+cVar = sqrt(zVar)
		fVar = sVar;
		SET_LOW_WORD(fVar, 0);
		cVar = (zVar - (fVar * fVar)) / (sVar + fVar);
		value = (0.5 * pio2d_hi) - ((2 * sVar * rVar) - (pio2d_lo - (2 * cVar)) - ((0.5 * pio2d_hi) - (2 * fVar)));
	}
	if (highWord >> 31) { return -value; }
	return value;
}

float acosf(float value)
{
	float zVar, wVar, sVar, cVar, df;
	uint32_t valueWord, ix;
	
	GET_FLOAT_WORD(valueWord, value);
	ix = valueWord & 0x7FFFFFFF;
	// |value| >= 1 or nan
	if (ix >= 0x3F800000)
	{
		if (ix == 0x3F800000)
		{
			if (valueWord >> 31) { return (2 * pio2_hi) + 0x1p-120F; }
			return 0;
		}
		return 0 / (value - value);
	}
	// |value| < 0.5
	if (ix < 0x3F000000)
	{
		if (ix <= 0x32800000) // |value| < 2**-26
		{
			return pio2_hi + 0x1p-120F;
		}
		return pio2_hi - (value - (pio2_lo - value * acosf_helper(value * value)));
	}
	// value < -0.5
	if (valueWord >> 31)
	{
		zVar = (1 + value) * 0.5f;
		sVar = sqrtf(zVar);
		wVar = (acosf_helper(zVar) * sVar) - pio2_lo;
		return 2 * (pio2_hi - (sVar + wVar));
	}
	// value > 0.5
	zVar = (1 - value) * 0.5f;
	sVar = sqrtf(zVar);
	GET_FLOAT_WORD(valueWord, sVar);
	SET_FLOAT_WORD(df, (valueWord & 0xFFFFF000));
	cVar = (zVar - (df * df)) / (sVar + df);
	wVar = (acosf_helper(zVar) * sVar) + cVar;
	return 2 * (df + wVar);
}
double acos(double value)
{
	double zVar, wVar, sVar, cVar, df;
	uint32_t highWord, valueUnsigned;
	
	GET_HIGH_WORD(highWord, value);
	valueUnsigned = (highWord & 0x7FFFFFFF);
	// |value| >= 1 or nan
	if (valueUnsigned >= 0x3FF00000)
	{
		uint32_t lowWord;

		GET_LOW_WORD(lowWord, value);
		if (((valueUnsigned - 0x3FF00000) | lowWord) == 0)
		{
			// acos(1)=0, acos(-1)=pi
			if (highWord >> 31) { return (2 * pio2d_hi) + 0x1p-120f; }
			return 0;
		}
		return 0 / (value - value);
	}
	// |value| < 0.5
	if (valueUnsigned < 0x3FE00000)
	{
		if (valueUnsigned <= 0x3C600000)  // |value| < 2**-57
		{
			return pio2d_hi + 0x1p-120f;
		}
		return pio2d_hi - (value - (pio2d_lo - (value * acos_helper(value * value))));
	}
	// value < -0.5
	if (highWord >> 31)
	{
		zVar = (1.0 + value) * 0.5;
		sVar = sqrt(zVar);
		wVar = (acos_helper(zVar) * sVar) - pio2d_lo;
		return 2 * (pio2d_hi - (sVar + wVar));
	}
	// value > 0.5
	zVar = (1.0 - value) * 0.5;
	sVar = sqrt(zVar);
	df = sVar;
	SET_LOW_WORD(df, 0);
	cVar = (zVar - (df * df)) / (sVar + df);
	wVar = (acos_helper(zVar) * sVar) + cVar;
	return 2 * (df + wVar);
}

float atanf(float value)
{
	float_t wVar, sVar1, sVar2, zVar;
	uint32_t valueUnsigned, sign;
	int index;
	
	GET_FLOAT_WORD(valueUnsigned, value);
	sign = valueUnsigned>>31;
	valueUnsigned &= 0x7FFFFFFF;
	if (valueUnsigned >= 0x4C800000) // if |value| >= 2**26
	{
		if (isnan(value)) { return value; }
		zVar = atanhi[3] + 0x1p-120f;
		return (sign ? -zVar : zVar);
	}
	if (valueUnsigned < 0x3EE00000) // |value| < 0.4375
	{
		if (valueUnsigned < 0x39800000) // |value| < 2**-12
		{
			if (valueUnsigned < 0x00800000)
			{
				// raise underflow for subnormal value
				FORCE_EVAL(value*value);
			}
			return value;
		}
		index = -1;
	}
	else
	{
		value = fabsf(value);
		if (valueUnsigned < 0x3f980000) // |value| < 1.1875
		{
			if (valueUnsigned < 0x3f300000) //  7/16 <= |value| < 11/16
			{
				index = 0;
				value = (2.0f*value - 1.0f)/(2.0f + value);
			}
			else // 11/16 <= |value| < 19/16
			{
				index = 1;
				value = (value - 1.0f)/(value + 1.0f);
			}
		}
		else
		{
			if (valueUnsigned < 0x401c0000) // |value| < 2.4375
			{
				index = 2;
				value = (value - 1.5f)/(1.0f + 1.5f*value);
			}
			else // 2.4375 <= |value| < 2**26
			{
				index = 3;
				value = -1.0f/value;
			}
		}
	}
	// end of argument reduction
	zVar = value * value;
	wVar = zVar * zVar;
	// break sum from i=0 to 10 aT[i]zVar**(i+1) into odd and even poly
	sVar1 = zVar * (aT[0] + wVar * (aT[2] + wVar * aT[4]));
	sVar2 = wVar * (aT[1] + wVar * aT[3]);
	if (index < 0) { return value - value * (sVar1 + sVar2); }
	zVar = atanhi[index] - ((value * (sVar1 + sVar2) - atanlo[index]) - value);
	return (sign ? -zVar : zVar);
}
double atan(double value)
{
	double_t quad, sVar1, sVar2, square;
	uint32_t valueUnsigned, sign;
	int index;
	
	GET_HIGH_WORD(valueUnsigned, value);
	sign = (valueUnsigned >> 31);
	valueUnsigned &= 0x7FFFFFFF;
	if (valueUnsigned >= 0x44100000) // if |value| >= 2^66
	{
		if (isnan(value)) { return value; }
		square = atanhid[3] + 0x1p-120F;
		return (sign ? -square : square);
	}
	if (valueUnsigned < 0x3FDC0000) // |value| < 0.4375
	{
		if (valueUnsigned < 0x3E400000) // |value| < 2^-27
		{
			if (valueUnsigned < 0x00100000)
			{
				// raise underflow for subnormal value
				FORCE_EVAL((float)value);
			}
			return value;
		}
		index = -1;
	}
	else
	{
		value = fabs(value);
		if (valueUnsigned < 0x3FF30000) // |value| < 1.1875
		{
			if (valueUnsigned < 0x3FE60000) //  7/16 <= |value| < 11/16
			{
				index = 0;
				value = ((2.0 * value) - 1.0) / (2.0 + value);
			}
			else // 11/16 <= |value| < 19/16
			{
				index = 1;
				value = (value - 1.0) / (value + 1.0);
			}
		}
		else
		{
			if (valueUnsigned < 0x40038000) // |value| < 2.4375
			{
				index = 2;
				value = (value - 1.5) / (1.0 + (1.5 * value));
			}
			else // 2.4375 <= |value| < 2^66
			{
				index = 3;
				value = -1.0 / value;
			}
		}
	}
	// end of argument reduction
	square = (value * value);
	quad = (square * square);
	// break sum from i=0 to 10 aTd[i]square**(i+1) into odd and even poly
	sVar1 = square * (aTd[0] + quad * (aTd[2] + quad * (aTd[4] + quad * (aTd[6] + quad * (aTd[8] + quad * aTd[10])))));
	sVar2 = quad * (aTd[1] + quad * (aTd[3] + quad * (aTd[5] + quad * (aTd[7] + quad * aTd[9]))));
	if (index < 0) { return value - value * (sVar1 + sVar2); }
	square = atanhid[index] - (value * (sVar1 + sVar2) - atanlod[index] - value);
	return (sign ? -square : square);
}

float atan2f(float numer, float denom)
{
	float zVar;
	uint32_t index, denomWord, numerWord;

	if (isnan(denom) || isnan(numer)) { return (denom + numer); }
	GET_FLOAT_WORD(denomWord, denom);
	GET_FLOAT_WORD(numerWord, numer);
	if (denomWord == 0x3F800000) { return atanf(numer); } // denom=1.0
	index = ((numerWord >> 31) & 1) | ((denomWord >> 30) & 2);  // 2*sign(denom)+sign(numer)
	denomWord &= 0x7FFFFFFF;
	numerWord &= 0x7FFFFFFF;

	// when numer = 0
	if (numerWord == 0)
	{
		switch (index)
		{
			case 0:
			case 1: return numer; // atan(+-0,+anything)=+-0
			case 2: return  pi;   // atan(+0,-anything) = pi
			case 3: return -pi;   // atan(-0,-anything) =-pi
		}
	}
	// when denom = 0
	if (denomWord == 0) { return (index & 1) ? -pi/2 : pi/2; }
	// when denom is INF
	if (denomWord == 0x7F800000)
	{
		if (numerWord == 0x7F800000)
		{
			switch (index)
			{
				case 0: return  pi/4;   // atan(+INF,+INF)
				case 1: return -pi/4;   // atan(-INF,+INF)
				case 2: return 3*pi/4;  // atan(+INF,-INF)
				case 3: return -3*pi/4; // atan(-INF,-INF)
			}
		}
		else
		{
			switch (index)
			{
				case 0: return  0.0f;    // atan(+...,+INF)
				case 1: return -0.0f;    // atan(-...,+INF)
				case 2: return  pi; // atan(+...,-INF)
				case 3: return -pi; // atan(-...,-INF)
			}
		}
	}
	// |numer/denom| > 0x1p26
	if (denomWord + (26 << 23) < numerWord || numerWord == 0x7F800000)
	{
		return (index &1) ? -pi/2 : pi/2;
	}

	// zVar = atan(|numer/denom|) with correct underflow
	if ((index & 2) && numerWord + (26 << 23) < denomWord)  //|numer/denom| < 0x1p-26, denom < 0
	{
		zVar = 0.0;
	}
	else
	{
		zVar = atanf(fabsf(numer / denom));
	}
	switch (index)
	{
		case 0:  return zVar;              // atan(+,+)
		case 1:  return -zVar;             // atan(-,+)
		case 2:  return pi - (zVar-pi_lo); // atan(+,-)
		default: return (zVar-pi_lo) - pi; // atan(-,-)
	}
}
double atan2(double numer, double denom)
{
	double zVar;
	uint32_t index, denomLow, numerLow, denomHigh, numerHigh;
	
	if (isnan(denom) || isnan(numer)) { return denom+numer; }
	EXTRACT_WORDS(denomHigh, denomLow, denom);
	EXTRACT_WORDS(numerHigh, numerLow, numer);
	if ((denomHigh-0x3FF00000 | denomLow) == 0) { return atan(numer); } // denom = 1.0
	index = ((numerHigh >> 31) & 1) | ((denomHigh >> 30) & 2); // 2*sign(denom)+sign(numer)
	denomHigh = (denomHigh & 0x7FFFFFFF);
	numerHigh = (numerHigh & 0x7FFFFFFF);
	
	// when numer = 0
	if ((numerHigh|numerLow) == 0)
	{
		switch(index)
		{
			case 0:
			case 1: return numer;   // atan(+-0,+anything)=+-0
			case 2: return  pi; // atan(+0,-anything) = pi
			case 3: return -pi; // atan(-0,-anything) =-pi
		}
	}
	// when denom = 0
	if ((denomHigh|denomLow) == 0) { return (index & 1) ? -pi/2 : pi/2; }
	// when denom is INF
	if (denomHigh == 0x7FF00000)
	{
		if (numerHigh == 0x7FF00000)
		{
			switch(index)
			{
				case 0: return  pi/4;   // atan(+INF,+INF)
				case 1: return -pi/4;   // atan(-INF,+INF)
				case 2: return  3*pi/4; // atan(+INF,-INF)
				case 3: return -3*pi/4; // atan(-INF,-INF)
			}
		}
		else
		{
			switch(index)
			{
				case 0: return  0.0; // atan(+...,+INF)
				case 1: return -0.0; // atan(-...,+INF)
				case 2: return  pi;  // atan(+...,-INF)
				case 3: return -pi;  // atan(-...,-INF)
			}
		}
	}
	// |numer/denom| > 0x1p64
	if (denomHigh+(64<<20) < numerHigh || numerHigh == 0x7FF00000) { return (index & 1) ? -pi/2 : pi/2; }
	
	// zVar = atan(|numer/denom|) without spurious underflow
	if ((index & 2) && numerHigh + (64 << 20) < denomHigh)  // |numer/denom| < 0x1p-64, denom<0
	{
		zVar = 0;
	}
	else
	{
		zVar = atan(fabs(numer / denom));
	}
	switch (index)
	{
		case 0:  return zVar;                // atan(+,+)
		case 1:  return -zVar;               // atan(-,+)
		case 2:  return pi - (zVar - pi_lo); // atan(+,-)
		default: return (zVar - pi_lo) - pi; // atan(-,-)
	}
}
#endif

// +--------------------------------------------------------------+
// |                         pow and powf                         |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_POW
inline float powf(float value,  float exponent)  { return __builtin_powf(value, exponent); }
inline double pow(double value, double exponent) { return __builtin_pow(value,  exponent); }
#else

#include "math_pow_helpers.c"

float powf(float base, float exponent)
{
	uint32_t signBias = 0;
	uint32_t baseInt, exponentInt;
	
	baseInt = asuint(base);
	exponentInt = asuint(exponent);
	if (predict_false(baseInt - 0x00800000 >= 0x7F800000 - 0x00800000 || zeroinfnan(exponentInt)))
	{
		// Either (base < 0x1p-126 or inf or nan) or (exponent is 0 or inf or nan).
		if (predict_false(zeroinfnan(exponentInt)))
		{
			if (2 * exponentInt == 0) { return 1.0f; }
			if (baseInt == 0x3F800000) { return 1.0f; }
			if (2 * baseInt > 2u * 0x7F800000 || 2 * exponentInt > 2u * 0x7F800000) { return base + exponent; }
			if (2 * baseInt == 2 * 0x3F800000) { return 1.0f; }
			if ((2 * baseInt < 2 * 0x3F800000) == !(exponentInt & 0x80000000)) { return 0.0f; } // |base|<1 && exponent==inf or |base|>1 && exponent==-inf.
			return exponent * exponent;
		}
		if (predict_false(zeroinfnan(baseInt)))
		{
			float_t baseSquared = base * base;
			if (baseInt & 0x80000000 && checkint(exponentInt) == 1) { baseSquared = -baseSquared; }
			// Without the barrier some versions of clang hoist the 1/baseSquared and
			// thus division by zero exception can be signaled spuriously.
			return exponentInt & 0x80000000 ? fp_barrierf(1 / baseSquared) : baseSquared;
		}
		// base and exponent are non-zero finite.
		if (baseInt & 0x80000000)
		{
			// Finite base < 0.
			int exponentType = checkint(exponentInt);
			if (exponentType == 0) { return __math_invalidf(base); }
			if (exponentType == 1) { signBias = exp2inline_SIGN_BIAS; }
			baseInt &= 0x7FFFFFFF;
		}
		if (baseInt < 0x00800000)
		{
			// Normalize subnormal base so exponent becomes negative.
			baseInt = asuint(base * 0x1p23F);
			baseInt &= 0x7FFFFFFF;
			baseInt -= 23 << 23;
		}
	}
	double_t logBase = log2_inline(baseInt);
	double_t exponentLogBase = exponent * logBase; // cannot overflow, exponent is single prec.
	if (predict_false((asuint64(exponentLogBase) >> 47 & 0xFFFF) >= asuint64(126.0 * POWF_SCALE) >> 47))
	{
		// |exponent*log(base)| >= 126.
		if (exponentLogBase > 0x1.FFFFFFFD1D571p+6 * POWF_SCALE) { return __math_oflowf(signBias); }
		if (exponentLogBase <= -150.0 * POWF_SCALE) { return __math_uflowf(signBias); }
	}
	return exp2_inline(exponentLogBase, signBias);
}
double pow(double value, double exponent)
{
	return value; //TODO: Implement me!
}
#endif

// +--------------------------------------------------------------+
// |                         log and logf                         |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_LOG
inline float logf(float value)  { return __builtin_logf(value); }
inline double log(double value) { return __builtin_log(value);  }
#else
float logf(float value)
{
	double_t zVar, rVar, rVarSquared, result, yVar, cInverse, logc;
	uint32_t valueInt, zVarInt, temp;
	int vVar, index;

	valueInt = asuint(value);
	// Fix sign of zero with downward rounding when value==1.
	if (predict_false(valueInt == 0x3F800000)) { return 0; }
	if (predict_false(valueInt - 0x00800000 >= 0x7F800000 - 0x00800000))
	{
		// value < 0x1p-126 or inf or nan.
		if (valueInt * 2 == 0) { return __math_divzerof(1); }
		if (valueInt == 0x7F800000) { return value; } // log(inf) == inf.
		if ((valueInt & 0x80000000) || valueInt * 2 >= 0xFF000000) { return __math_invalidf(value); }
		// value is subnormal, normalize it.
		valueInt = asuint(value * 0x1p23F);
		valueInt -= 23 << 23;
	}

	// value = 2^vVar zVar; where zVar is in range [OFF,2*OFF] and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	temp = valueInt - logf_OFF;
	index = (temp >> (23 - LOGF_TABLE_BITS)) % logf_N;
	vVar = ((int32_t)temp >> 23); // arithmetic shift
	zVarInt = valueInt - (temp & 0x1FF << 23);
	cInverse = logf_T[index].invc;
	logc = logf_T[index].logc;
	zVar = (double_t)asfloat(zVarInt);

	// log(value) = log1p(zVar/c-1) + log(c) + vVar*Ln2
	rVar = (zVar * cInverse) - 1;
	yVar = logc + ((double_t)vVar * logf_Ln2);

	// Pipelined polynomial evaluation to approximate log1p(rVar).
	rVarSquared = (rVar * rVar);
	result = (logf_A[1] * rVar) + logf_A[2];
	result = (logf_A[0] * rVarSquared) + result;
	result = (result * rVarSquared) + (yVar + rVar);
	return eval_as_float(result);
}

double log(double value)
{
	double_t wVar, zVar, rVar, rVarSquared, rVarCubed, result, cInverse, logc, vVarDelta, resultHigh, resultLow;
	uint64_t valueInt, zVarInt, tmp;
	uint32_t top;
	int vVar, index;
	
	valueInt = asuint64(value);
	top = top16(value);
	if (predict_false(valueInt - asuint64(1.0 - 0x1p-4) < asuint64(1.0 + 0x1.09p-4) - asuint64(1.0 - 0x1p-4)))
	{
		// Handle close to 1.0 inputs separately.
		// Fix sign of zero with downward rounding when value==1.
		if (predict_false(valueInt == asuint64(1.0))) { return 0; }
		rVar = value - 1.0;
		rVarSquared = rVar * rVar;
		rVarCubed = rVar * rVarSquared;
		result = rVarCubed *
		    (log_B[1] + rVar * log_B[2] + rVarSquared * log_B[3] +
		     rVarCubed * (log_B[4] + rVar * log_B[5] + rVarSquared * log_B[6] +
			   rVarCubed * (log_B[7] + rVar * log_B[8] + rVarSquared * log_B[9] + rVarCubed * log_B[10])));
		// Worst-case error is around 0.507 ULP.
		wVar = rVar * 0x1p27;
		double_t rhi = rVar + wVar - wVar;
		double_t rlo = rVar - rhi;
		wVar = rhi * rhi * log_B[0]; // log_B[0] == -0.5.
		resultHigh = rVar + wVar;
		resultLow = rVar - resultHigh + wVar;
		resultLow += log_B[0] * rlo * (rhi + rVar);
		result += resultLow;
		result += resultHigh;
		return eval_as_double(result);
	}
	if (predict_false(top - 0x0010 >= 0x7FF0 - 0x0010))
	{
		// value < 0x1p-1022 or inf or nan.
		if (valueInt * 2 == 0) { return __math_divzero(1); }
		if (valueInt == asuint64(INFINITY)) { return value; } // log(inf) == inf.
		if ((top & 0x8000) || (top & 0x7FF0) == 0x7FF0) { return __math_invalid(value); }
		// value is subnormal, normalize it.
		valueInt = asuint64(value * 0x1p52);
		valueInt -= 52ULL << 52;
	}
	
	// value = 2^vVar zVar; where zVar is in range [OFF,2*OFF) and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	tmp = valueInt - log_OFF;
	index = (tmp >> (52 - LOG_TABLE_BITS)) % log_N;
	vVar = (int64_t)tmp >> 52; /* arithmetic shift */
	zVarInt = valueInt - (tmp & 0xfffULL << 52);
	cInverse = log_T[index].invc;
	logc = log_T[index].logc;
	zVar = asdouble(zVarInt);
	
	// log(value) = log1p(zVar/c-1) + log(c) + vVar*Ln2.
	// rVar ~= zVar/c - 1, |rVar| < 1/(2*N).
	// rounding error: 0x1p-55/N + 0x1p-66.
	rVar = (zVar - log_T2[index].chi - log_T2[index].clo) * cInverse;
	vVarDelta = (double_t)vVar;
	
	// resultHigh + resultLow = rVar + log(c) + vVar*Ln2.
	wVar = vVarDelta * log_Ln2hi + logc;
	resultHigh = wVar + rVar;
	resultLow = wVar - resultHigh + rVar + vVarDelta * log_Ln2lo;
	
	// log(value) = resultLow + (log1p(rVar) - rVar) + resultHigh.
	rVarSquared = rVar * rVar; // rounding error: 0x1p-54/N^2.
	// Worst case error if |result| > 0x1p-5:
	// 0.5 + 4.13/N + abs-poly-error*2^57 ULP (+ 0.002 ULP without fma)
	// Worst case error if |result| > 0x1p-4:
	// 0.5 + 2.06/N + abs-poly-error*2^56 ULP (+ 0.001 ULP without fma).
	result = resultLow + (rVarSquared * log_A[0]) +
	    rVar * rVarSquared * (log_A[1] + rVar * log_A[2] + rVarSquared * (log_A[3] + rVar * log_A[4])) + resultHigh;
	return eval_as_double(result);
}
#endif

// +--------------------------------------------------------------+
// |                        log2 and log2f                        |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_LOG2
inline float log2f(float value)  { return __builtin_log2f(value); }
inline double log2(double value) { return __builtin_log2(value);  }
#else

float log2f(float value)
{
	double_t zVar, rVar, rVarSquared, pVar, result, yVar, vInverse, logc;
	uint32_t valueInt, zVarInt, top, tmp;
	int k, i;
	
	valueInt = asuint(value);
	// Fix sign of zero with downward rounding when value==1.
	if (predict_false(valueInt == 0x3F800000)) { return 0; }
	if (predict_false(valueInt - 0x00800000 >= 0x7F800000 - 0x00800000))
	{
		// value < 0x1p-126 or inf or nan.
		if (valueInt * 2 == 0) { return __math_divzerof(1); }
		if (valueInt == 0x7F800000) { return value; } // log2(inf) == inf.
		if ((valueInt & 0x80000000) || valueInt * 2 >= 0xFF000000) { return __math_invalidf(value); }
		// value is subnormal, normalize it.
		valueInt = asuint(value * 0x1p23f);
		valueInt -= (23 << 23);
	}
	
	// value = 2^k zVar; where zVar is in range [OFF,2*OFF] and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	tmp = valueInt - log2f_OFF;
	i = (tmp >> (23 - LOG2F_TABLE_BITS)) % log2f_N;
	top = (tmp & 0xFF800000);
	zVarInt = valueInt - top;
	k = (int32_t)tmp >> 23; // arithmetic shift
	vInverse = log2f_T[i].invc;
	logc = log2f_T[i].logc;
	zVar = (double_t)asfloat(zVarInt);
	
	// log2(value) = log1p(z/c-1)/ln2 + log2(c) + k
	rVar = zVar * vInverse - 1;
	yVar = logc + (double_t)k;
	
	// Pipelined polynomial evaluation to approximate log1p(rVar)/ln2.
	rVarSquared = rVar * rVar;
	result = log2f_A[1] * rVar + log2f_A[2];
	result = log2f_A[0] * rVarSquared + result;
	pVar = log2f_A[3] * rVar + yVar;
	result = result * rVarSquared + pVar;
	return eval_as_float(result);
}
double log2(double value)
{
	double_t zVar, rVar, rVarSquared, rVarQuad, result, cInverse, logc, tempDouble, resultHigh, resultLow, tVar1, tVar2, tVar3, polyValue;
	uint64_t valueInt, iz, tmp;
	uint32_t top;
	int temp, index;
	
	valueInt = asuint64(value);
	top = top16(value);
	if (predict_false(valueInt - asuint64(1.0 - 0x1.5B51p-5) < asuint64(1.0 + 0x1.6AB2p-5) - asuint64(1.0 - 0x1.5B51p-5)))
	{
		// Handle close to 1.0 inputs separately.
		// Fix sign of zero with downward rounding when value==1.
		if (predict_false(valueInt == asuint64(1.0))) { return 0; }
		rVar = value - 1.0;
		double_t rVarHigh, rVarLow;
		rVarHigh = asdouble(asuint64(rVar) & -1ULL << 32);
		rVarLow = rVar - rVarHigh;
		resultHigh = (rVarHigh * log2_InvLn2hi);
		resultLow = (rVarLow * log2_InvLn2hi) + (rVar * log2_InvLn2lo);
		rVarSquared = rVar * rVar; // rounding error: 0x1p-62.
		rVarQuad = rVarSquared * rVarSquared;
		// Worst-case error is less than 0.54 ULP (0.55 ULP without fma).
		polyValue = rVarSquared * (log2_B[0] + (rVar * log2_B[1]));
		result = resultHigh + polyValue;
		resultLow += resultHigh - result + polyValue;
		resultLow += rVarQuad * (log2_B[2] + rVar * log2_B[3] + rVarSquared * (log2_B[4] + rVar * log2_B[5]) +
			    rVarQuad * (log2_B[6] + rVar * log2_B[7] + rVarSquared * (log2_B[8] + rVar * log2_B[9])));
		result += resultLow;
		return eval_as_double(result);
	}
	
	if (predict_false(top - 0x0010 >= 0x7FF0 - 0x0010))
	{
		// value < 0x1p-1022 or inf or nan.
		if (valueInt * 2 == 0) { return __math_divzero(1); }
		if (valueInt == asuint64(INFINITY)) { return value; } // log(inf) == inf.
		if ((top & 0x8000) || (top & 0x7FF0) == 0x7FF0) { return __math_invalid(value); }
		// value is subnormal, normalize it.
		valueInt = asuint64(value * 0x1p52);
		valueInt -= 52ULL << 52;
	}
	
	// value = 2^temp zVar; where zVar is in range [OFF,2*OFF) and exact.
	// The range is split into N subintervals.
	// The ith subinterval contains zVar and c is near its center.
	tmp = valueInt - log2_OFF;
	index = (tmp >> (52 - LOG2_TABLE_BITS)) % log2_N;
	temp = (int64_t)tmp >> 52; // arithmetic shift
	iz = valueInt - (tmp & 0xfffULL << 52);
	cInverse = log2_T[index].invc;
	logc = log2_T[index].logc;
	zVar = asdouble(iz);
	tempDouble = (double_t)temp;
	
	// log2(value) = log2(zVar/c) + log2(c) + temp.
	// rVar ~= zVar/c - 1, |rVar| < 1/(2*N).
	double_t rVarHigh, rVarLow;
	// rounding error: 0x1p-55/N + 0x1p-65.
	rVar = (zVar - log2_T2[index].chi - log2_T2[index].clo) * cInverse;
	rVarHigh = asdouble(asuint64(rVar) & -1ULL << 32);
	rVarLow = rVar - rVarHigh;
	tVar1 = rVarHigh * log2_InvLn2hi;
	tVar2 = rVarLow * log2_InvLn2hi + rVar * log2_InvLn2lo;
	
	// resultHigh + resultLow = rVar/ln2 + log2(c) + temp.
	tVar3 = tempDouble + logc;
	resultHigh = tVar3 + tVar1;
	resultLow = tVar3 - resultHigh + tVar1 + tVar2;
	
	// log2(rVar+1) = rVar/ln2 + rVar^2*poly(rVar).
	// Evaluation is optimized assuming superscalar pipelined execution.
	rVarSquared = rVar * rVar; // rounding error: 0x1p-54/N^2.
	rVarQuad = rVarSquared * rVarSquared;
	// Worst-case error if |result| > 0x1p-4: 0.547 ULP (0.550 ULP without fma).
	// ~ 0.5 + 2/N/ln2 + abs-poly-error*0x1p56 ULP (+ 0.003 ULP without fma).
	polyValue = log2_A[0] + rVar * log2_A[1] + rVarSquared * (log2_A[2] + rVar * log2_A[3]) + rVarQuad * (log2_A[4] + rVar * log2_A[5]);
	result = resultLow + (rVarSquared * polyValue) + resultHigh;
	return eval_as_double(result);
}
#endif

// +--------------------------------------------------------------+
// |                       log10 and log10f                       |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_LOG10
inline float log10f(float value)  { return __builtin_log10f(value); }
inline double log10(double value) { return __builtin_log10(value);  }
#else
float log10f(float value)
{
	union { float value; uint32_t integer; } valueUnion = { value };
	float_t hfsq, valueSubOne, sVar, sVarSquared, sVarQuad, tVar1, tVar2, tSum, vVarOriginal, highFloat, lowFloat;
	uint32_t valueInt;
	int vVar;
	
	valueInt = valueUnion.integer;
	vVar = 0;
	if (valueInt < 0x00800000 || valueInt>>31) // value < 2**-126
	{
		if (valueInt<<1 == 0) { return -1 / (value * value); } // log(+-0)=-inf
		if (valueInt>>31) { return (value - value) / 0.0f; } // log(-#) = NaN
		// subnormal number, scale up value
		vVar -= 25;
		value *= 0x1p25F;
		valueUnion.value = value;
		valueInt = valueUnion.integer;
	}
	else if (valueInt >= 0x7F800000) { return value; }
	else if (valueInt == 0x3F800000) { return 0; }
	
	// reduce value into [sqrt(2)/2, sqrt(2)]
	valueInt += 0x3F800000 - 0x3F3504F3;
	vVar += (int)(valueInt>>23) - 0x7F;
	valueInt = (valueInt & 0x007FFFFF) + 0x3F3504F3;
	valueUnion.integer = valueInt;
	value = valueUnion.value;
	
	valueSubOne = value - 1.0f;
	sVar = valueSubOne / (2.0f + valueSubOne);
	sVarSquared = sVar * sVar;
	sVarQuad = sVarSquared * sVarSquared;
	tVar1= sVarQuad * (Lg2 + (sVarQuad * Lg4));
	tVar2= sVarSquared * (Lg1 + (sVarQuad * Lg3));
	tSum = tVar2 + tVar1;
	hfsq = 0.5f * valueSubOne * valueSubOne;
	
	highFloat = valueSubOne - hfsq;
	valueUnion.value = highFloat;
	valueUnion.integer &= 0xFFFFF000;
	highFloat = valueUnion.value;
	lowFloat = valueSubOne - highFloat - hfsq + (sVar * (hfsq + tSum));
	vVarOriginal = vVar;
	return (vVarOriginal * log10_2lo) + ((lowFloat + highFloat) * ivln10lo) + (lowFloat * ivln10hi) + (highFloat * ivln10hi) + (vVarOriginal * log10_2hi);
}
double log10(double value)
{
	union { double value; uint64_t integer; } valueUnion = { value };
	double_t hfsq, valueSubOne, sVar, sVarSquared, sVarQuad, tVar1, tVar2, tSum, vVarOriginal, result, highDouble, lowDouble, resultHigh, resultLow;
	uint32_t valueUpperWord;
	int vVar;
	
	valueUpperWord = (valueUnion.integer >> 32);
	vVar = 0;
	if (valueUpperWord < 0x00100000 || (valueUpperWord >> 31))
	{
		if ((valueUnion.integer << 1) == 0) { return -1 / (value * value); } // log(+-0)=-inf
		if (valueUpperWord >> 31) { return (value - value) / 0.0; } // log(-#) = NaN
		// subnormal number, scale value up
		vVar -= 54;
		value *= 0x1p54;
		valueUnion.value = value;
		valueUpperWord = (valueUnion.integer >> 32);
	}
	else if (valueUpperWord >= 0x7FF00000) { return value; }
	else if (valueUpperWord == 0x3FF00000 && (valueUnion.integer << 32) == 0) { return 0; }
	
	// reduce value into [sqrt(2)/2, sqrt(2)]
	valueUpperWord += 0x3FF00000 - 0x3FE6A09E;
	vVar += (int)(valueUpperWord >> 20) - 0x3FF;
	valueUpperWord = (valueUpperWord & 0x000FFFFF) + 0x3FE6A09E;
	valueUnion.integer = (uint64_t)valueUpperWord << 32 | (valueUnion.integer & 0xFFFFFFFF);
	value = valueUnion.value;
	
	valueSubOne = value - 1.0;
	hfsq = 0.5 * valueSubOne * valueSubOne;
	sVar = valueSubOne / (2.0 + valueSubOne);
	sVarSquared = sVar * sVar;
	sVarQuad = sVarSquared * sVarSquared;
	tVar1 = sVarQuad * (Lg2d + sVarQuad * (Lg4d + sVarQuad * Lg6d));
	tVar2 = sVarSquared * (Lg1d + sVarQuad * (Lg3d + sVarQuad * (Lg5d + sVarQuad * Lg7d)));
	tSum = tVar2 + tVar1;
	
	// See log2.c for details.
	// highDouble+lowDouble = valueSubOne - hfsq + sVar*(hfsq+tSum) ~ log(1+valueSubOne)
	highDouble = valueSubOne - hfsq;
	valueUnion.value = highDouble;
	valueUnion.integer &= (uint64_t)-1 << 32;
	highDouble = valueUnion.value;
	lowDouble = valueSubOne - highDouble - hfsq + (sVar * (hfsq + tSum));
	
	// resultHigh+resultLow ~ log10(1+valueSubOne) + vVar*log10(2)
	resultHigh = highDouble * ivln10hid;
	vVarOriginal = vVar;
	result = vVarOriginal * log10_2hid;
	resultLow = vVarOriginal * log10_2lod + (lowDouble + highDouble) * ivln10lod + lowDouble * ivln10hid;
	
	// Extra precision in for adding result is not strictly needed
	// since there is no very large cancellation near value = sqrt(2) or
	// value = 1/sqrt(2), but we do it anyway since it costs little on CPUs
	// with some parallelism and it reduces the error for many args.
	sVarQuad = result + resultHigh;
	resultLow += (result - sVarQuad) + resultHigh;
	resultHigh = sVarQuad;
	
	return resultLow + resultHigh;
}
#endif

// +--------------------------------------------------------------+
// |                       ldexp and ldexpf                       |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_LDEXP
inline float ldexpf(float  value, int exponent) { return __builtin_ldexpf(value, exponent); }
inline double ldexp(double value, int exponent) { return __builtin_ldexp(value,  exponent); }
#else
float ldexpf(float value, int exponent)
{
	return scalbnf(value, exponent);
}
double ldexp(double value, int exponent)
{
	return scalbn(value, exponent);
}
#endif

// +--------------------------------------------------------------+
// |                    copysign and copysignf                    |
// +--------------------------------------------------------------+
#if PIG_WASM_STD_USE_BUILTINS_COPYSIGN
inline float copysignf(float  magnitude, float  sign) { return __builtin_copysignf(magnitude, sign); }
inline double copysign(double magnitude, double sign) { return __builtin_copysign(magnitude,  sign); }
#else
float copysignf(float magnitude, float sign)
{
	union { float value; uint32_t integer; } magnitudeUnion = { magnitude }, signUnion = { sign };
	magnitudeUnion.integer &= 0x7FFFFFFF;
	magnitudeUnion.integer |= (signUnion.integer & 0x80000000);
	return magnitudeUnion.value;
}
double copysign(double magnitude, double sign)
{
	union { double value; uint64_t integer; } magnitudeUnion = { magnitude }, signUnion = { sign };
	magnitudeUnion.integer &= (-1ULL / 2);
	magnitudeUnion.integer |= (signUnion.integer & (1ULL << 63));
	return magnitudeUnion.value;
}
#endif
