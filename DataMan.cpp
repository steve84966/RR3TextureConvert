#include "DataMan.h"


void DataManiger::DumpToFile(LPCWSTR path) const
{
#ifdef _DEBUG
	assert(this->GetPtr() != nullptr);
	DWORD err;
	HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	err = GetLastError();
	assert(hFile != INVALID_HANDLE_VALUE);
	assert(this->lenth < MAXDWORD);
	DWORD _;
	auto re = WriteFile(hFile, this->pData.get(), this->lenth, &_, NULL);
	err = GetLastError();
	assert(re);
	assert(_ == this->lenth);
	CloseHandle(hFile);
	return;
#endif
}

bool DataManiger::WriteToFile(LPCWSTR path) const
{
	if (!this->GetPtr()) {
		SetLastError(ERROR_NO_MORE_ITEMS);
		return false;
	}
	if (this->GetLen() > MAXDWORD) {
		SetLastError(ERROR_MORE_DATA);
		return false;
	}
	HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_PATH_NOT_FOUND) {
			if (Helper::CreatePathFromFileName(std::wstring(path))) {
				hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile == INVALID_HANDLE_VALUE) {
					return false;
				}
			}
		}
		else {
			return false;
		}
	}
	DWORD _;
	auto re = WriteFile(hFile, this->GetPtr(), (DWORD)this->GetLen(), &_, NULL);
	CloseHandle(hFile);
	if (!re || _ != this->GetLen()) {
		return false;
	}
	return true;
}