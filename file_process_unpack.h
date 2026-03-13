#pragma once
#include"framework.h"

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

class DataManiger {
public:
	using ByteFlow = std::unique_ptr<uint8_t[]>;

	DataManiger() = default;
	DataManiger(DataManiger&&r)  noexcept {
		this->lenth = r.lenth;
		this->pData = std::move(r.pData);
		r.lenth = 0;
	}
	DataManiger(std::unique_ptr<std::uint8_t[]>&& pdata,size_t len)  noexcept {
		this->lenth = len;
		this->pData = std::move(pdata);
	}
	~DataManiger() = default;
	DataManiger& operator=(DataManiger&& r) noexcept{
		this->lenth = r.lenth;
		this->pData = std::move(r.pData);
		r.lenth = 0;
		return *this;
	}
	void Set(std::unique_ptr<std::uint8_t[]>&& r, size_t len) {
		this->pData = std::move(r);
		this->lenth = len;
	}
	uint8_t* GetPtr()const noexcept{
		return this->pData.get();
	}
	size_t GetLen()const noexcept {
		return this->lenth;
	}
	//Debug only
	void DumpToFile(LPCWSTR path) const;

	//IsValid
	operator bool()const { return (bool)(pData); }
public:
	std::unique_ptr<std::uint8_t[]> pData;
	size_t lenth = 0;
};

class Pack {
public:
	enum class opstack {
		nop = 0,
		z_bin,
		z,
		etc,
		atc,
		ptc,
		dxt
	};

	static void process_filter(wchar_t* base, wchar_t* relative);

	// re:header,and many normal .z in order (eg. 2016_mclaren_mp4x_ext_shadows_high.etc.dds.z.bin)
	static std::vector<DataManiger> zlib_z_bin(const DataManiger& pData);

	// normal zlib
	// in:compressed data
	// re:uncompressed data
	static DataManiger zlib_z(const DataManiger& pData);
};

class Converter {
public:
	static DataManiger dds2rgb_a(const DataManiger& etcfile, uint32_t& w, uint32_t& h, uint32_t& mipLevel, uint8_t& bHasAlpha);
private:
	static DataManiger rgba4_to_rgba8(const uint8_t* data, size_t size);
	static DataManiger dds_to_rgba(const DataManiger& File, uint8_t& bHasAlpha);
};
class Helper {
public :

	//读取所有内容到内存，太大的(4gb+)不干
	static DataManiger ReadAll(HANDLE hFile);

	//封装WriteConsole
	static void printconsole(const std::wstring&);
	static void printconsole(const std::wstring_view&);
	static void printconsole(const wchar_t*);
};


