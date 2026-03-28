#include "framework.h"
#include "ListAllFiles.h"
#include "file_process_unpack.h"

std::wstring inType;
std::vector<std::wstring> outTypes;
std::wstring inDir;
std::vector<std::wstring> outDirs;

int wmain(int c,wchar_t** v)
{

	std::cout << "Hello World!\n";
	inType = L"etc";
	wchar_t* path = new wchar_t[512];
	if (c > 1) {
		wcscpy(path, v[1]);
	}
	else {
		wcscpy(path, L"R:\\Temp");
	}
	ULONGLONG st = GetTickCount64(), et = 0;
	ListAllFiles(path, Pack::process_filter);
	et = GetTickCount64();
	auto diff = et - st;
	{
		SYSTEMTIME sys;
		sys.wMilliseconds = diff % 1000;
		diff /= 1000;
		sys.wSecond = diff % 60;
		diff /= 60;
		sys.wMinute = diff % 60;
		diff /= 60;
		sys.wHour = diff;
		Helper::printconsole(std::format(L"用时：{:02}:{:02}:{:02}.{:03}\n", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds));
	}
}
