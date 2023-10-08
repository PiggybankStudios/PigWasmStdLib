/*
File:   math.c
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds all the implementations for math.h declared functions
*/

// +--------------------------------------------------------------+
// |                        Float Helpers                         |
// +--------------------------------------------------------------+
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

// +--------------------------------------------------------------+
// |                        fmin and fmax                         |
// +--------------------------------------------------------------+
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

// +--------------------------------------------------------------+
// |                        fabs and fabsf                        |
// +--------------------------------------------------------------+
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

// +--------------------------------------------------------------+
// |                        fmod and fmodf                        |
// +--------------------------------------------------------------+
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
