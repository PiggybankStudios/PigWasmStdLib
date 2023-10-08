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
	TestCaseFloat(fmodf(-13.1234f, 1.0f), -0.12339973449707031f);
	TestCaseFloat(fmodf(5.24f, 1.33f), 1.2499996423721313f);
	TestCaseFloat(fmodf(INFINITY, 1.0f), NAN);
	TestCaseFloat(fmodf(1.2f, 0.0f), NAN);
	
	TestCaseDouble(fmod(5.0, 3.0), 2.0);
	TestCaseDouble(fmod(-13.1234, 1.0), -0.12340000000000018);
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
	
	jsPrintNumber("Ran Math Tests", numCases);
}
