/*
File:   test_cases.cpp
Author: Taylor Robbins
Date:   10\07\2023
Description: 
	** Holds a bunch of test cases for the PigWasm StdLib
*/

void TestCaseFloat_(const char* testCaseStr, float result, float expectedValue)
{
	if (result != expectedValue && !(isnan(result) && isnan(expectedValue)))
	{
		jsPrintString("Test case failed!");
		jsPrintString(testCaseStr);
		jsPrintFloat("Expected", expectedValue);
		jsPrintFloat("Got", result);
	}
}
#define TestCaseFloat(testCase, expectedValue) do { numCases++; TestCaseFloat_(#testCase, (testCase), (expectedValue)); } while(0)

void TestCaseDouble_(const char* testCaseStr, double result, double expectedValue)
{
	if (result != expectedValue && !(isnan(result) && isnan(expectedValue)))
	{
		jsPrintString("Test case failed!");
		jsPrintString(testCaseStr);
		jsPrintFloat("Expected", expectedValue);
		jsPrintFloat("Got", result);
	}
}
#define TestCaseDouble(testCase, expectedValue) do { numCases++; TestCaseDouble_(#testCase, (testCase), (expectedValue)); } while(0)

void RunMathTestCases()
{
	int numCases = 0;
	
	TestCaseFloat(fminf(-13.1234f, 1.2f), -13.1234f);
	TestCaseFloat(fminf(1.2f, -13.1234f), -13.1234f);
	TestCaseFloat(fminf(1.2f, 0.0f), 0.0f);
	TestCaseFloat(fminf(1.2f, INFINITY), 1.2f);
	TestCaseFloat(fminf(1.2f, -INFINITY), -INFINITY);
	TestCaseFloat(fminf(1.2f, NAN), 1.2f);
	TestCaseFloat(fminf(1.20000001f, 1.2f), 1.2f);
	
	TestCaseDouble(fmin(-13.1234, 1.2), -13.1234);
	TestCaseDouble(fmin(1.2, -13.1234), -13.1234);
	TestCaseDouble(fmin(1.2, 0.0), 0.0);
	TestCaseDouble(fmin(1.2, INFINITY), 1.2);
	TestCaseDouble(fmin(1.2, -INFINITY), -INFINITY);
	TestCaseDouble(fmin(1.2, NAN), 1.2);
	TestCaseDouble(fmin(1.20000001, 1.2), 1.2);
	
	TestCaseFloat(fmaxf(-13.1234f, 1.2f), 1.2f);
	TestCaseFloat(fmaxf(1.2f, -13.1234f), 1.2f);
	TestCaseFloat(fmaxf(1.2f, 0.0f), 1.2f);
	TestCaseFloat(fmaxf(1.2f, INFINITY), INFINITY);
	TestCaseFloat(fmaxf(1.2f, -INFINITY), 1.2f);
	TestCaseFloat(fmaxf(1.2f, NAN), 1.2f);
	TestCaseFloat(fmaxf(1.20000001f, 1.2f), 1.20000001f);
	
	TestCaseDouble(fmax(-13.1234, 1.2), 1.2);
	TestCaseDouble(fmax(1.2, -13.1234), 1.2);
	TestCaseDouble(fmax(1.2, 0.0), 1.2);
	TestCaseDouble(fmax(1.2, INFINITY), INFINITY);
	TestCaseDouble(fmax(1.2, -INFINITY), 1.2);
	TestCaseDouble(fmax(1.2, NAN), 1.2);
	TestCaseDouble(fmax(1.20000001, 1.2), 1.20000001);
	
	TestCaseFloat(fmodf(5.0f, 3.0f), 2.0f);
	TestCaseFloat(fmodf(-13.1234f, 1.0f), -0.12339973449707031f); //very close to -0.1234
	TestCaseFloat(fmodf(5.24f, 1.33f), 1.2499996423721313f); //very close to 1.25
	TestCaseFloat(fmodf(INFINITY, 1.0f), NAN);
	TestCaseFloat(fmodf(1.2f, 0.0f), NAN);
	
	TestCaseDouble(fmod(5.0, 3.0), 2.0);
	TestCaseDouble(fmod(-13.1234, 1.0), -0.12340000000000018); //very close to -0.1234
	TestCaseDouble(fmod(5.24, 1.33), 1.25);
	TestCaseDouble(fmod(INFINITY, 1.0), NAN);
	TestCaseDouble(fmod(1.2, 0.0), NAN);
	
	TestCaseFloat(fabs(-13.1234f), 13.1234f);
	TestCaseFloat(fabs(13.1234f), 13.1234f);
	TestCaseFloat(fabs(-INFINITY), INFINITY);
	TestCaseFloat(fabs(INFINITY), INFINITY);
	TestCaseFloat(fabs(NAN), NAN);
	TestCaseFloat(fabs(0.0f), 0.0f);
	TestCaseFloat(fabs(-1000000.0001f), 1000000.0001f);
	
	TestCaseDouble(fabs(-13.1234), 13.1234);
	TestCaseDouble(fabs(13.1234), 13.1234);
	TestCaseDouble(fabs(-INFINITY), INFINITY);
	TestCaseDouble(fabs(INFINITY), INFINITY);
	TestCaseDouble(fabs(NAN), NAN);
	TestCaseDouble(fabs(0.0), 0.0);
	TestCaseDouble(fabs(-1000000.0001), 1000000.0001);
	
	TestCaseFloat(roundf(1.24f), 1.0f);
	TestCaseFloat(roundf(1.0f), 1.0f);
	TestCaseFloat(roundf(0.7f), 1.0f);
	TestCaseFloat(roundf(-1.24f), -1.0f);
	TestCaseFloat(roundf(-1.0f), -1.0f);
	TestCaseFloat(roundf(-0.7f), -1.0f);
	TestCaseFloat(roundf(0.5f), 1.0f);
	TestCaseFloat(roundf(0.5f - FLT_EPSILON), 0.0f);
	TestCaseFloat(roundf(-0.5f), -1.0f);
	TestCaseFloat(roundf(-0.5f + FLT_EPSILON), 0.0f);
	TestCaseFloat(roundf(INFINITY), INFINITY);
	TestCaseFloat(roundf(-INFINITY), -INFINITY);
	TestCaseFloat(roundf(NAN), NAN);
	
	TestCaseDouble(round(1.24), 1.0);
	TestCaseDouble(round(1.0), 1.0);
	TestCaseDouble(round(0.7), 1.0);
	TestCaseDouble(round(-1.24), -1.0);
	TestCaseDouble(round(-1.0), -1.0);
	TestCaseDouble(round(-0.7), -1.0);
	TestCaseDouble(round(0.5), 1.0);
	TestCaseDouble(round(0.5 - DBL_EPSILON), 0.0);
	TestCaseDouble(round(-0.5), -1.0);
	TestCaseDouble(round(-0.5 + DBL_EPSILON), 0.0);
	TestCaseDouble(round(INFINITY), INFINITY);
	TestCaseDouble(round(-INFINITY), -INFINITY);
	TestCaseDouble(round(NAN), NAN);
	
	TestCaseFloat(floorf(1.0f), 1.0f);
	TestCaseFloat(floorf(2.0f), 2.0f);
	TestCaseFloat(floorf(1.001f), 1.0f);
	TestCaseFloat(floorf(1.24f), 1.0f);
	TestCaseFloat(floorf(1.99f), 1.0f);
	TestCaseFloat(floorf(-1.0f), -1.0f);
	TestCaseFloat(floorf(-2.0f), -2.0f);
	TestCaseFloat(floorf(-1.001f), -2.0f);
	TestCaseFloat(floorf(-1.24f), -2.0f);
	TestCaseFloat(floorf(-1.99f), -2.0f);
	TestCaseFloat(floorf(1024.5f), 1024.0f);
	TestCaseFloat(floorf(INFINITY), INFINITY);
	TestCaseFloat(floorf(-INFINITY), -INFINITY);
	TestCaseFloat(floorf(NAN), NAN);
	
	TestCaseDouble(floor(1.0), 1.0);
	TestCaseDouble(floor(2.0), 2.0);
	TestCaseDouble(floor(1.001), 1.0);
	TestCaseDouble(floor(1.24), 1.0);
	TestCaseDouble(floor(1.99), 1.0);
	TestCaseDouble(floor(-1.0), -1.0);
	TestCaseDouble(floor(-2.0), -2.0);
	TestCaseDouble(floor(-1.001), -2.0);
	TestCaseDouble(floor(-1.24), -2.0);
	TestCaseDouble(floor(-1.99), -2.0);
	TestCaseDouble(floor(1024.5), 1024.0);
	TestCaseDouble(floor(INFINITY), INFINITY);
	TestCaseDouble(floor(-INFINITY), -INFINITY);
	TestCaseDouble(floor(NAN), NAN);
	
	TestCaseFloat(ceilf(1.0f), 1.0f);
	TestCaseFloat(ceilf(2.0f), 2.0f);
	TestCaseFloat(ceilf(1.001f), 2.0f);
	TestCaseFloat(ceilf(1.24f), 2.0f);
	TestCaseFloat(ceilf(1.99f), 2.0f);
	TestCaseFloat(ceilf(-1.0f), -1.0f);
	TestCaseFloat(ceilf(-2.0f), -2.0f);
	TestCaseFloat(ceilf(-1.001f), -1.0f);
	TestCaseFloat(ceilf(-1.24f), -1.0f);
	TestCaseFloat(ceilf(-1.99f), -1.0f);
	TestCaseFloat(ceilf(1024.5f), 1025.0f);
	TestCaseFloat(ceilf(INFINITY), INFINITY);
	TestCaseFloat(ceilf(-INFINITY), -INFINITY);
	TestCaseFloat(ceilf(NAN), NAN);
	
	TestCaseDouble(ceil(1.0), 1.0);
	TestCaseDouble(ceil(2.0), 2.0);
	TestCaseDouble(ceil(1.001), 2.0);
	TestCaseDouble(ceil(1.24), 2.0);
	TestCaseDouble(ceil(1.99), 2.0);
	TestCaseDouble(ceil(-1.0), -1.0);
	TestCaseDouble(ceil(-2.0), -2.0);
	TestCaseDouble(ceil(-1.001), -1.0);
	TestCaseDouble(ceil(-1.24), -1.0);
	TestCaseDouble(ceil(-1.99), -1.0);
	TestCaseDouble(ceil(1024.5), 1025.0);
	TestCaseDouble(ceil(INFINITY), INFINITY);
	TestCaseDouble(ceil(-INFINITY), -INFINITY);
	TestCaseDouble(ceil(NAN), NAN);
	
	TestCaseFloat(sinf(0.0f), 0.0f);
	TestCaseFloat(sinf(0.5f), 0.4794255495071411f);
	TestCaseFloat(sinf(1.0f), 0.8414709568023682f);
	TestCaseFloat(sinf(M_PI), -8.742277657347586e-8f); //very close to 0
	TestCaseFloat(sinf(2*M_PI), 1.7484555314695172e-7f); //very close to 0
	TestCaseFloat(sinf(M_PI_2), 1.0f);
	TestCaseFloat(sinf(3*M_PI_2), -1.0f);
	
	TestCaseDouble(sin(0.0), 0.0);
	TestCaseDouble(sin(0.5), 0.479425538604203);
	TestCaseDouble(sin(1.0), 0.8414709848078965);
	TestCaseDouble(sin(M_PI), 1.2246467991473532e-16); //very close to 0
	TestCaseDouble(sin(2*M_PI), -2.4492935982947064e-16); //very close to 0
	TestCaseDouble(sin(M_PI_2), 1.0);
	TestCaseDouble(sin(3*M_PI_2), -1.0);
	
	TestCaseFloat(cosf(0.0f), 1.0f);
	TestCaseFloat(cosf(0.5f), 0.8775825500488281f);
	TestCaseFloat(cosf(1.0f), 0.5403022766113281f);
	TestCaseFloat(cosf(M_PI), -1.0f);
	TestCaseFloat(cosf(2*M_PI), 1.0f);
	TestCaseFloat(cosf(M_PI_2), -4.371138828673793e-8f); //very close to 0
	TestCaseFloat(cosf(3*M_PI_2), 1.1924880638503055e-8f); //very close to 0
	
	TestCaseDouble(cos(0.0), 1.0);
	TestCaseDouble(cos(0.5), 0.8775825618903728);
	TestCaseDouble(cos(1.0), 0.5403023058681398);
	TestCaseDouble(cos(M_PI), -1.0);
	TestCaseDouble(cos(2*M_PI), 1.0);
	TestCaseDouble(cos(M_PI_2), 6.123233995736766e-17); //very close to 0
	TestCaseDouble(cos(3*M_PI_2), -1.8369701987210297e-16); //very close to 0
	
	TestCaseFloat(tanf(0.0f), 0.0f);
	TestCaseFloat(tanf(0.5f), 0.5463024973869324f);
	TestCaseFloat(tanf(1.0f), 1.5574077367782593f);
	TestCaseFloat(tanf(M_PI), 8.742278367890322e-8f); //very close to 0
	TestCaseFloat(tanf(2*M_PI), 1.7484556735780643e-7f); //very close to 0
	TestCaseFloat(tanf(M_PI_2 - FLT_EPSILON), 13245402.0f); //very large
	TestCaseFloat(tanf(M_PI_2 + FLT_EPSILON), -6137956.0f); //very large (negative)
	
	TestCaseDouble(tan(0.0), 0.0);
	TestCaseDouble(tan(0.5f), 0.5463024898437905);
	TestCaseDouble(tan(1.0), 1.5574077246549023);
	TestCaseDouble(tan(M_PI), -1.2246467991473532e-16); //very close to 0
	TestCaseDouble(tan(2*M_PI), -2.4492935982947064e-16); //very close to 0
	TestCaseDouble(tan(M_PI_2 - DBL_EPSILON), 3530114321217157.5); //very large
	TestCaseDouble(tan(M_PI_2 + DBL_EPSILON), -6218431163823738.0); //very large (negative)
	
	jsPrintNumber("Ran Math Tests", numCases);
}
