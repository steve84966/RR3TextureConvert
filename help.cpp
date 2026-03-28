#include "help.h"

void Helper::printconsole(const wchar_t* s)
{
	auto len = wcslen(s);
	DWORD _;
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), s, len, &_, NULL);
	return;
}

bool Helper::ChangeFileExtention(std::wstring& io, const std::wstring& src, const std::wstring& dst)
{
	std::wstring path = io;
	auto pos = path.find(src);
	if (pos == std::wstring::npos) {
		return false;
	}
	path.replace(pos, dst.size(), dst);
	return true;
}

std::wstring Helper::ErrorMessageToWstring(DWORD nCode)
{
	std::unique_ptr<wchar_t[]> buffer = std::make_unique<wchar_t[]>(32768);
	auto re = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, nCode, 0, buffer.get(), 32768, 0);
	if (re == 0) {
		auto err = GetLastError();
		assert(printf("%d", err) && false);
		return std::wstring();
	}
	std::wstring str(buffer.get(), re);
	return str;
}

bool Helper::CreatePathFromFileName(const std::wstring& p)
{
	auto re = p.find_last_of(L'\\');
	if (re != std::wstring::npos) {
		auto directory = p.substr(0, re);

		// 逐级创建目录
		size_t currentPos = 0;
		BOOL re2 = false;
		while (true)
		{
			size_t nextPos = directory.find_first_of(L"\\/", currentPos);
			if (nextPos == std::wstring::npos)
			{
				// 最后一级目录
				if (!CreateDirectoryW(directory.c_str(), NULL) &&
					GetLastError() != ERROR_ALREADY_EXISTS)
				{
					return false;
				}
				else {
					return true;
				}
			}
			else
			{
				// 中间级目录
				std::wstring subDir = directory.substr(0, nextPos);
				auto re1 = CreateDirectoryW(subDir.c_str(), NULL);

				if (!re1) {
					auto re2 = GetLastError();
					switch (re2) {
					case ERROR_ALREADY_EXISTS:
					{
						;//nothing happen...
					}
					break;
					case ERROR_ACCESS_DENIED:
					{

						size_t Pos = directory.find_first_of(L"\\/", 0);
						if (Pos == nextPos) {
							//first path like "R:\\", so nothing happen and continue...
						}
						else {
							return false;
						}
					}
					break;
					default:
						return false;
						break;
					}
				}
				currentPos = nextPos + 1;
			}
		}
	}
	else
	{
		return false;
	}
}

void Helper::ExtendTexture(const cv::Mat& i, cv::Mat& o)
{
	assert(i.data);
	assert(o.data);
	assert(i.cols <= o.cols);
	assert(i.rows <= o.rows);
	auto& src = i;
	int src_w = src.cols;
	int src_h = src.rows;
	if (src.channels() == 3) {
		for (int y = 0; y < o.rows; ++y) {
			for (int x = 0; x < o.cols; ++x) {
				int src_x = (x < src_w) ? x : src_w - 1;
				int src_y = (y < src_h) ? y : src_h - 1;
				o.at<cv::Vec3b>(y, x) = src.at<cv::Vec3b>(src_y, src_x);
			}
		}
		return;
	}
	else if (src.channels() == 4) {
		for (int y = 0; y < o.rows; ++y) {
			for (int x = 0; x < o.cols; ++x) {
				int src_x = (x < src_w) ? x : src_w - 1;
				int src_y = (y < src_h) ? y : src_h - 1;
				o.at<cv::Vec4b>(y, x) = src.at<cv::Vec4b>(src_y, src_x);
			}
		}
		return;
	}
	else if (src.channels() == 1) {
		for (int y = 0; y < o.rows; ++y) {
			for (int x = 0; x < o.cols; ++x) {
				int src_x = (x < src_w) ? x : src_w - 1;
				int src_y = (y < src_h) ? y : src_h - 1;
				o.at<uchar>(y, x) = src.at<uchar>(src_y, src_x);
			}
		}
		return;
	}
	else {
		assert(false && "Unknown channel num");
	}
}

void Helper::printconsole(const std::wstring& s)
{
	auto len = s.length();
	DWORD _;
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), s.data(), len, &_, NULL);
	return;
}
void Helper::printconsole(const std::wstring_view& s)
{
	auto len = s.length();
	DWORD _;
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), s.data(), len, &_, NULL);
	return;
}


DataManiger Helper::ReadAll(HANDLE hFile)
{
	LARGE_INTEGER FileSize;
	if (hFile == INVALID_HANDLE_VALUE) {
		return DataManiger();
	}
	GetFileSizeEx(hFile, &FileSize);
	size_t readsize = FileSize.QuadPart;

	DWORD _;
	LARGE_INTEGER FileSize2;
	FileSize2.QuadPart = 0;
	SetFilePointerEx(hFile, FileSize2, NULL, FILE_BEGIN);
	if (readsize > MAXDWORD) {
		SetLastError(ERROR_FILE_TOO_LARGE);
		return DataManiger();
	}
	auto f = std::make_unique<uint8_t[]>(readsize + 4);
	auto re = ReadFile(hFile, f.get(), (DWORD)readsize, &_, NULL);
	if (re != 0) {
		return DataManiger(std::move(f), _);
	}
	else
	{
		return DataManiger();
	}

}

void Helper::RBChannelExchange(DataManiger::ByteFlow& rgb_a_flow, size_t sz, uint8_t bHasAlpha)
{
	auto ptr = rgb_a_flow.get();
	size_t elemsize = bHasAlpha ? 4 : 3;
	auto pixnum = sz / elemsize;
	for (size_t i = 0; i != pixnum; ++i) {
		std::swap(ptr[0], ptr[2]);
		ptr += elemsize;
	}
	return;
}
