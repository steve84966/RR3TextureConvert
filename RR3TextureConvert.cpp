#include <iostream>
#include "ListAllFiles.h"
#include "file_process_unpack.h"

int main()
{
	std::cout << "Hello World!\n";
	wchar_t path[] = LR"(R:\2016_mclaren_mp4x)";
	ListAllFiles(path, Pack::process_filter);
}
