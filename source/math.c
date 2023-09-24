/*
File:   math.c
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds all the implementations for math.h declared functions
*/

union ldshape
{
	long double value;
	struct
	{
		uint64_t lo;
		uint32_t mid;
		uint16_t top;
		uint16_t signExtension;
	} integer;
	struct
	{
		uint64_t lo;
		uint64_t hi;
	} integer2;
};

int __fpclassify(double value)
{
	union { double f; uint64_t i; } valueUnion = {value};
	int e = valueUnion.i>>52 & 0x7ff;
	if (!e) return valueUnion.i<<1 ? FP_SUBNORMAL : FP_ZERO;
	if (e==0x7ff) return valueUnion.i<<12 ? FP_NAN : FP_INFINITE;
	return FP_NORMAL;
}
int __fpclassifyl(long double value)
{
	return __fpclassify((double)value);
}

int __signbitl(long double value)
{
	union ldshape valueUnion = { value };
	return (valueUnion.integer.signExtension >> 15);
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
