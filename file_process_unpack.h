#pragma once
#include"framework.h"
#include"DataMan.h"

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


