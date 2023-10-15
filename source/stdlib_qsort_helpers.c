/*
File:   stdlib_qsort_helpers.c
Author: Taylor Robbins
Date:   10\15\2023
Description: 
	** Helper functions for qsort
*/

int a_ctz_64(uint64_t x)
{
	static const char debruijn64[64] =
	{
		0, 1, 2, 53, 3, 7, 54, 27, 4, 38, 41, 8, 34, 55, 48, 28,
		62, 5, 39, 46, 44, 42, 22, 9, 24, 35, 59, 56, 49, 18, 29, 11,
		63, 52, 6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10,
		51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12
	};
	return debruijn64[((x & -x) * 0x022FDD63CC95386Dull) >> 58];
}

int pntz(size_t array[2])
{
	int dValue = a_ctz_64(array[0] - 1);
	if (dValue != 0 || (dValue = 8 * sizeof(size_t) + a_ctz_64(array[1])) != (8 * sizeof(size_t)))
	{
		return dValue;
	}
	return 0;
}

void cycle(size_t width, unsigned char* pntrArray[], int arraySize)
{
	unsigned char tmp[256];
	size_t word;
	int index;

	if (arraySize < 2) { return; }

	pntrArray[arraySize] = tmp;
	while(width)
	{
		word = ((sizeof(tmp) < width) ? sizeof(tmp) : width);
		memcpy(pntrArray[arraySize], pntrArray[0], word);
		for(index = 0; index < arraySize; index++)
		{
			memcpy(pntrArray[index], pntrArray[index + 1], word);
			pntrArray[index] += word;
		}
		width -= word;
	}
}

// shiftLeft() and shiftRight() need numBits > 0
void shiftLeft(size_t array[2], int numBits)
{
	if (numBits >= 8 * sizeof(size_t))
	{
		numBits -= (8 * sizeof(size_t));
		array[1] = array[0];
		array[0] = 0;
	}
	array[1] <<= numBits;
	array[1] |= (array[0] >> ((sizeof(size_t) * 8) - numBits));
	array[0] <<= numBits;
}

void shiftRight(size_t array[2], int numBits)
{
	if (numBits >= 8 * sizeof(size_t))
	{
		numBits -= (8 * sizeof(size_t));
		array[0] = array[1];
		array[1] = 0;
	}
	array[0] >>= numBits;
	array[0] |= (array[1] << ((sizeof(size_t) * 8) - numBits));
	array[1] >>= numBits;
}

void sift(unsigned char* headPntr, size_t itemSize, CompareFuncEx_f* compareFunc, void* argPntr, int pshift, size_t wordArray[])
{
	unsigned char* rightPntr;
	unsigned char* leftPntr;
	unsigned char* array[(14 * sizeof(size_t)) + 1];
	int index = 1;

	array[0] = headPntr;
	while (pshift > 1)
	{
		rightPntr = headPntr - itemSize;
		leftPntr = headPntr - itemSize - wordArray[pshift - 2];

		if (compareFunc(array[0], leftPntr, argPntr) >= 0 && compareFunc(array[0], rightPntr, argPntr) >= 0)
		{
			break;
		}
		
		if (compareFunc(leftPntr, rightPntr, argPntr) >= 0)
		{
			array[index++] = leftPntr;
			headPntr = leftPntr;
			pshift -= 1;
		}
		else
		{
			array[index++] = rightPntr;
			headPntr = rightPntr;
			pshift -= 2;
		}
	}
	
	cycle(itemSize, array, index);
}

void trinkle(unsigned char* headPntr, size_t itemSize, CompareFuncEx_f* compareFunc, void* compareFuncRaw, size_t twoWords[2], int pshift, int trusty, size_t wordArray[])
{
	unsigned char* stepson;
	unsigned char* rightPntr;
	unsigned char* leftPntr;
	size_t twoWordsLocal[2];
	unsigned char* array[(14 * sizeof(size_t)) + 1];
	int index = 1;
	int trail;

	twoWordsLocal[0] = twoWords[0];
	twoWordsLocal[1] = twoWords[1];

	array[0] = headPntr;
	while(twoWordsLocal[0] != 1 || twoWordsLocal[1] != 0)
	{
		stepson = headPntr - wordArray[pshift];
		if (compareFunc(stepson, array[0], compareFuncRaw) <= 0)
		{
			break;
		}
		if (!trusty && pshift > 1)
		{
			rightPntr = headPntr - itemSize;
			leftPntr = headPntr - itemSize - wordArray[pshift - 2];
			if (compareFunc(rightPntr, stepson, compareFuncRaw) >= 0 || compareFunc(leftPntr, stepson, compareFuncRaw) >= 0)
			{
				break;
			}
		}

		array[index++] = stepson;
		headPntr = stepson;
		trail = pntz(twoWordsLocal);
		shiftRight(twoWordsLocal, trail);
		pshift += trail;
		trusty = 0;
	}
	
	if (!trusty)
	{
		cycle(itemSize, array, index);
		sift(headPntr, itemSize, compareFunc, compareFuncRaw, pshift, wordArray);
	}
}

void __qsort_r(void* basePntr, size_t numItems, size_t itemSize, CompareFuncEx_f* compareFunc, void* compareFuncRaw)
{
	size_t localStorage[12 * sizeof(size_t)];
	size_t index;
	size_t totalSize = (itemSize * numItems);
	unsigned char* headPntr;
	unsigned char* highPntr;
	size_t twoWords[2] = { 1, 0 };
	int pshift = 1;
	int trail;

	if (!totalSize) { return; }

	headPntr = basePntr;
	highPntr = headPntr + totalSize - itemSize;

	// Precompute Leonardo numbers, scaled by element itemSize
	localStorage[0] = itemSize;
	localStorage[1] = itemSize;
	for (index = 2; (localStorage[index] = localStorage[index - 2] + localStorage[index - 1] + itemSize) < totalSize; index++) { }

	while (headPntr < highPntr)
	{
		if ((twoWords[0] & 3) == 3)
		{
			sift(headPntr, itemSize, compareFunc, compareFuncRaw, pshift, localStorage);
			shiftRight(twoWords, 2);
			pshift += 2;
		}
		else
		{
			if (localStorage[pshift - 1] >= (highPntr - headPntr))
			{
				trinkle(headPntr, itemSize, compareFunc, compareFuncRaw, twoWords, pshift, 0, localStorage);
			}
			else
			{
				sift(headPntr, itemSize, compareFunc, compareFuncRaw, pshift, localStorage);
			}

			if (pshift == 1)
			{
				shiftLeft(twoWords, 1);
				pshift = 0;
			}
			else
			{
				shiftLeft(twoWords, pshift - 1);
				pshift = 1;
			}
		}

		twoWords[0] |= 1;
		headPntr += itemSize;
	}

	trinkle(headPntr, itemSize, compareFunc, compareFuncRaw, twoWords, pshift, 0, localStorage);

	while (pshift != 1 || twoWords[0] != 1 || twoWords[1] != 0)
	{
		if (pshift <= 1)
		{
			trail = pntz(twoWords);
			shiftRight(twoWords, trail);
			pshift += trail;
		}
		else
		{
			shiftLeft(twoWords, 2);
			pshift -= 2;
			twoWords[0] ^= 7;
			shiftRight(twoWords, 1);
			trinkle((headPntr - localStorage[pshift] - itemSize), itemSize, compareFunc, compareFuncRaw, twoWords, pshift + 1, 1, localStorage);
			shiftLeft(twoWords, 1);
			twoWords[0] |= 1;
			trinkle(headPntr - itemSize, itemSize, compareFunc, compareFuncRaw, twoWords, pshift, 1, localStorage);
		}
		headPntr -= itemSize;
	}
}

int WrapperCompareFunc(const void* left, const void* right, void* compareFunc)
{
	return ((CompareFunc_f*)compareFunc)(left, right);
}
