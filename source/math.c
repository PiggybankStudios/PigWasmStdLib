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
#define PIG_WASM_STD_USE_BUILTINS_FMIN_FMAX  0
#define PIG_WASM_STD_USE_BUILTINS_FABS       1
#define PIG_WASM_STD_USE_BUILTINS_FMOD       0
#define PIG_WASM_STD_USE_BUILTINS_ROUND      0
#define PIG_WASM_STD_USE_BUILTINS_FLOOR_CEIL 1

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
