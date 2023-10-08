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
#define PIG_WASM_STD_USE_BUILTINS_FMIN_FMAX   0
#define PIG_WASM_STD_USE_BUILTINS_FABS        1
#define PIG_WASM_STD_USE_BUILTINS_FMOD        0
#define PIG_WASM_STD_USE_BUILTINS_ROUND       0
#define PIG_WASM_STD_USE_BUILTINS_FLOOR_CEIL  1
#define PIG_WASM_STD_USE_BUILTINS_SCALBN      0
#define PIG_WASM_STD_USE_BUILTINS_SIN_COS_TAN 0

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

#include "math_trig_helpers.c"

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