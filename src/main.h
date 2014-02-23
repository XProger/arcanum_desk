#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "ctrl.h"
#include "mtp.h"

#ifdef _DEBUG
	#include <crtdbg.h>
	#define _CRTDBG_MAP_ALLOC
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

	#define LOG(...) printf(__VA_ARGS__)
	extern void LOG_dump(const char *data, int count);
#else
	#define LOG(...) ((void)0)	
	#define LOG_dump(...) ((void)0)
#endif

#define TL_BUF_SIZE (1024*512+64)	// 512kb + 64 bytes

struct Main {
	static Window	*window;
	static MTP		*mtp;
	static void init();
	static void deinit();
};

extern __int64 getUnixTime();

#endif MAIN_H
