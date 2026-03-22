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

	static ByteFlow CreateBuffer(size_t sz) { assert(sz != 0); return std::make_unique<uint8_t[]>(sz); }

	DataManiger() :pData(nullptr), lenth(0) {};
	DataManiger(size_t sz) {
		this->pData = std::make_unique<uint8_t[]>(sz);
		this->lenth = sz;
	}
	DataManiger(DataManiger&& r)  noexcept {
		this->lenth = r.lenth;
		this->pData = std::move(r.pData);
		r.lenth = 0;
	}
	DataManiger(std::unique_ptr<std::uint8_t[]>&& pdata, size_t len)  noexcept {
		this->lenth = len;
		this->pData = std::move(pdata);
	}
	~DataManiger() = default;
	DataManiger& operator=(DataManiger&& r) noexcept {
		this->lenth = r.lenth;
		this->pData = std::move(r.pData);
		r.lenth = 0;
		return *this;
	}
	void Set(std::unique_ptr<std::uint8_t[]>&& r, size_t len) {
		this->pData = std::move(r);
		this->lenth = len;
	}
	uint8_t* GetPtr()const noexcept {
		return this->pData.get();
	}
	size_t GetLen()const noexcept {
		return this->lenth;
	}
	//Debug only
	void DumpToFile(LPCWSTR path) const;

	bool WriteToFile(LPCWSTR path)const ;

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

	static void process_filter(const wchar_t* full, const wchar_t* base, const wchar_t* relative);
	class ConverterFuncs {
		using pDestFormatConverter = DataManiger(*)(const DataManiger& rgb_a_buffer, const DirectX::DDS_HEADER* prawHeader, uint32_t w, uint32_t h, uint32_t mipLevel, uint8_t bHasAlpha);
		pDestFormatConverter pFunc;
		std::wstring extention;
	};
	

	// re:header,and many normal .z in order (eg. 2016_mclaren_mp4x_ext_shadows_high.etc.dds.z.bin)
	static std::vector<DataManiger> zlib_z_bin_uc(const DataManiger& pData);

	// in:header, .z in order
	// re:full file.
	static DataManiger zlib_z_bin_c(const std::vector<DataManiger>& pDatas);

	// normal zlib
	// in:compressed data
	// re:uncompressed data
	static DataManiger zlib_z_uc(const DataManiger& pData);

	// in:uncompressed
	// re:compressed (lv 9)
	static DataManiger zlib_z_c(const DataManiger& pData);
};

class Converter {
public:
	//atc, etc suppot. dxt not sure. return rgb(a) byte flow only.
	static DataManiger dds2rgb_a(const DataManiger& ddsfile, uint32_t& w, uint32_t& h, uint32_t& mipLevel, uint8_t& bHasAlpha);
	//return DXT1 without alpha, DXT5 with alpha.
	//with full DDS_HEADER(128 bytes)
	static DataManiger rgb_a2dxt(const DataManiger& rgb_a_buffer, const DirectX::DDS_HEADER* pRawHeader, uint32_t w, uint32_t h, uint32_t mipLevel, uint8_t bHasAlpha);
	//retur atc/atci
	//with full DDS_HEADER
	static DataManiger rgb_a2atc(const DataManiger& rgb_a_buffer, const DirectX::DDS_HEADER* pHeader, uint32_t w, uint32_t h, uint32_t miplevel, uint8_t bHasAlpha);
private:
	static DataManiger rgba4_to_rgba8(const uint8_t* data, size_t size);
	static DataManiger rgba8_to_rgba4(const uint8_t* data, size_t size);
};
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

	static constexpr std::wstring_view CreateView(const wchar_t*p) {
		return std::wstring_view(p);
	}
};


