/*
File:   string_strstr_helpers.c
Author: Taylor Robbins
Date:   10\15\2023
Description: 
	** Contains helper functions for strstr implementation
*/

void* memchr(const void* pntr, int targetChar, size_t numBytes)
{
	const unsigned char* charPntr = pntr;
	targetChar = (unsigned char)targetChar;
	for (; ((uintptr_t)charPntr & (sizeof(size_t)-1)) && numBytes && *charPntr != targetChar; charPntr++, numBytes--) { }
	if (numBytes && *charPntr != targetChar)
	{
		const a_size_t* wordPntr;
		size_t targetCharMask = ((size_t)-1/UCHAR_MAX) * targetChar;
		for (wordPntr = (const void*)charPntr; numBytes >= sizeof(size_t) && !WORD_CONTAINS_ZERO(*wordPntr ^ targetCharMask); wordPntr++, numBytes -= sizeof(size_t)) { }
		charPntr = (const void*)wordPntr;
	}
	for (; numBytes && *charPntr != targetChar; charPntr++, numBytes--) { }
	return (numBytes ? (void*)charPntr : 0);
}

char* twobyte_strstr(const unsigned char* haystack, const unsigned char* needle)
{
	uint16_t needleWord = (needle[0] << 8) | needle[1];
	uint16_t haystackWord = (haystack[0] << 8) | haystack[1];
	for (haystack++; *haystack && haystackWord != needleWord; haystackWord = (haystackWord << 8) | *++haystack);
	return *haystack ? (char *)haystack-1 : 0;
}

char* threebyte_strstr(const unsigned char* haystack, const unsigned char* needle)
{
	uint32_t needleWord = (uint32_t)(needle[0] << 24) | (needle[1] << 16) | (needle[2] << 8);
	uint32_t haystackWord = (uint32_t)(haystack[0] << 24) | (haystack[1] << 16) | (haystack[2] << 8);
	for (haystack += 2; *haystack && haystackWord != needleWord; haystackWord = ((haystackWord | *++haystack) << 8));
	return *haystack ? (char *)haystack-2 : 0;
}

char* fourbyte_strstr(const unsigned char* haystack, const unsigned char* needle)
{
	uint32_t needleWord = (uint32_t)(needle[0] << 24) | (needle[1] << 16) | (needle[2] << 8) | needle[3];
	uint32_t haystackWord = (uint32_t)(haystack[0] << 24) | (haystack[1] << 16) | (haystack[2] << 8) | haystack[3];
	for (haystack += 3; *haystack && haystackWord != needleWord; haystackWord = (haystackWord << 8) | *++haystack);
	return *haystack ? ((char*)haystack - 3) : 0;
}

#define strstr_BITOP(a,b,op) ((a)[(size_t)(b)/(8*sizeof *(a))] op (size_t)1<<((size_t)(b)%(8*sizeof *(a))))

char* twoway_strstr(const unsigned char* haystack, const unsigned char* needle)
{
	const unsigned char* charPntr;
	size_t index, iVar, jVar, vVar, pVar, iVar0, pVar0, wVar, wVar0;
	size_t byteset[32 / sizeof(size_t)] = { 0 };
	size_t shiftBuffer[256];
	
	// Computing length of needle and fill shift table
	for (index=0; needle[index] && haystack[index]; index++)
	{
		strstr_BITOP(byteset, needle[index], |=);
		shiftBuffer[needle[index]] = index + 1;
	}
	if (needle[index]) { return 0; } // hit the end of haystack
	
	// Compute maximal suffix
	iVar = -1;
	jVar = 0;
	vVar = 1;
	pVar = 1;
	while (jVar + vVar < index)
	{
		if (needle[iVar + vVar] == needle[jVar + vVar])
		{
			if (vVar == pVar)
			{
				jVar += pVar;
				vVar = 1;
			}
			else { vVar++; }
		}
		else if (needle[iVar + vVar] > needle[jVar + vVar])
		{
			jVar += vVar;
			vVar = 1;
			pVar = jVar - iVar;
		}
		else
		{
			iVar = jVar++;
			vVar = pVar = 1;
		}
	}
	iVar0 = iVar;
	pVar0 = pVar;
	
	// And with the opposite comparison
	iVar = -1;
	jVar = 0;
	vVar = 1;
	pVar = 1;
	while (jVar + vVar < index)
	{
		if (needle[iVar + vVar] == needle[jVar + vVar])
		{
			if (vVar == pVar)
			{
				jVar += pVar;
				vVar = 1;
			}
			else { vVar++; }
		}
		else if (needle[iVar + vVar] < needle[jVar + vVar])
		{
			jVar += vVar;
			vVar = 1;
			pVar = jVar - iVar;
		}
		else
		{
			iVar = jVar++;
			vVar = pVar = 1;
		}
	}
	if (iVar + 1 > iVar0 + 1) { iVar0 = iVar; }
	else { pVar = pVar0; }
	
	// Periodic needle?
	if (memcmp(needle, needle + pVar, iVar0 + 1))
	{
		wVar0 = 0;
		pVar = MAX(iVar0, index - iVar0 - 1) + 1;
	}
	else { wVar0 = index - pVar; }
	wVar = 0;
	
	// Initialize incremental end-of-haystack pointer
	charPntr = haystack;
	
	// Search loop
	for (;;)
	{
		// Update incremental end-of-haystack pointer
		if ((charPntr - haystack) < index)
		{
			// Fast estimate for MAX(index,63)
			size_t grow = (index | 63);
			const unsigned char* newCharPntr = memchr(charPntr, 0, grow);
			if (newCharPntr)
			{
				charPntr = newCharPntr;
				if (charPntr - haystack < index) return 0;
			}
			else { charPntr += grow; }
		}
		
		// Check last byte first; advance by shift on mismatch
		if (strstr_BITOP(byteset, haystack[index - 1], &))
		{
			vVar = index - shiftBuffer[haystack[index - 1]];
			if (vVar)
			{
				if (vVar < wVar) { vVar = wVar; }
				haystack += vVar;
				wVar = 0;
				continue;
			}
		}
		else
		{
			haystack += index;
			wVar = 0;
			continue;
		}
		
		// Compare right half
		for (vVar = MAX(iVar0 + 1, wVar); needle[vVar] && needle[vVar] == haystack[vVar]; vVar++) { }
		if (needle[vVar])
		{
			haystack += vVar - iVar0;
			wVar = 0;
			continue;
		}
		// Compare left half
		for (vVar = iVar0 + 1; vVar > wVar && needle[vVar - 1] == haystack[vVar - 1]; vVar--) { }
		if (vVar <= wVar) { return (char*)haystack; }
		haystack += pVar;
		wVar = wVar0;
	}
}

char* __strchrnul(const char* str, int targetChar)
{
	targetChar = (unsigned char)targetChar;
	if (!targetChar) { return (char*)str + strlen(str); }
	
	const a_size_t* wordPntr;
	for (; (uintptr_t)str % sizeof(size_t); str++)
	{
		if (!*str || *(unsigned char*)str == targetChar) { return (char*)str; }
	}
	
	size_t targetCharMask = ((size_t)-1/UCHAR_MAX) * targetChar;
	for (wordPntr = (void*)str; !WORD_CONTAINS_ZERO(*wordPntr) && !WORD_CONTAINS_ZERO(*wordPntr ^ targetCharMask); wordPntr++) { }
	
	str = (void*)wordPntr;
	for (; *str && *(unsigned char*)str != targetChar; str++) { }
	
	return (char*)str;
}

char *strchr(const char* str, int targetChar)
{
	char* foundPntr = __strchrnul(str, targetChar);
	return ((*(unsigned char*)foundPntr == (unsigned char)targetChar) ? foundPntr : 0);
}
