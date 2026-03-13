#pragma once
#include"framework.h"

//readonly
class FileMapping
{
public:
	FileMapping() {
		bSuccess = false;
		hFile = INVALID_HANDLE_VALUE;
		hMap = NULL;
		pData = nullptr;
	}

	~FileMapping() { this->Close(); };

	bool Open(std::wstring s) {
		hFile = CreateFileW(s.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return false;
		}
		hMap = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hMap == NULL) {
			return false;
		}
		pData = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
		if (pData == NULL) {
			return false;
		}
		bSuccess = true;
		return true;
	}
	void Close() {
		if (pData) {
			UnmapViewOfFile(pData);
			pData = NULL;
		}
		bSuccess = false;
		if (hMap) {
			CloseHandle(hMap);
			hMap = NULL;
		}
		if (hFile) {
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
	}
	uint8_t* GetFilePtr() {
		return (uint8_t*)this->pData;
	}
	operator bool() { return (bool)this->bSuccess; };
private:
	uint8_t bSuccess;
	HANDLE hFile;
	HANDLE hMap;
	void* pData;
};

