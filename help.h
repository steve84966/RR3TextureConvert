#pragma once
#include"framework.h"
#include"DataMan.h"

class Helper {
public:

	//读取所有内容到内存，太大的(4gb+)不干
	static DataManiger ReadAll(HANDLE hFile);

	//RGB RB互换
	static void RBChannelExchange(DataManiger::ByteFlow& rgb_a_flow, size_t sz, uint8_t bHasAlpha);

	//封装WriteConsole
	static void printconsole(const std::wstring&);
	static void printconsole(const std::wstring_view&);
	static void printconsole(const wchar_t*);

	//修改文件名(类型)
	static bool ChangeFileExtention(std::wstring& io, const std::wstring& src, const std::wstring& dst);

	//WINAPI: GetLastError->String
	static std::wstring ErrorMessageToWstring(DWORD nCode);

	//WINAPI: ERROR_PATH_NOT_FOUND (3)
	static bool CreatePathFromFileName(const std::wstring& p);

	static void ExtendTexture(const cv::Mat& i, cv::Mat& o);

	static constexpr std::wstring_view CreateView(const wchar_t* p) {
		return std::wstring_view(p);
	}

	class HandleWarpper {
	public:
		HandleWarpper() = default;
		HandleWarpper(HANDLE _h) { this->h = _h; };
		~HandleWarpper() {
			if (h != NULL && h != INVALID_HANDLE_VALUE) {
				CloseHandle(h);
				h = NULL;
			};
		};
	public:
		HANDLE h = NULL;
	};
};
