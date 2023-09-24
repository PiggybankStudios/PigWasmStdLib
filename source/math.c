/*
File:   math.c
Author: Taylor Robbins
Date:   09\23\2023
Description: 
	** Holds all the implementations for math.h declared functions
*/

float fminf(float value1, float value2)
{
	if (isnan(x)) { return y; }
	if (isnan(y)) { return x; }
	// handle signed zeros, see C99 Annex F.9.9.2
	if (signbit(x) != signbit(y)) { return (signbit(x) ? x : y); }
	return (x < y ? x : y);
}
double fmin(double value1, double value2)
{
	if (isnan(x)) { return y; }
	if (isnan(y)) { return x; }
	// handle signed zeros, see C99 Annex F.9.9.2
	if (signbit(x) != signbit(y)) { return (signbit(x) ? x : y); }
	return (x < y ? x : y);
}
