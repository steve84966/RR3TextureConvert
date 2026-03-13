#include"file_process_unpack.h"

void Helper::printconsole(const wchar_t* s)
{
	auto len = wcslen(s);
	DWORD _;
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), s, len, &_, NULL);
	return;
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

void Pack::process_filter(wchar_t* base, wchar_t* relative)
{
	auto fullpath = std::wstring(base) + relative;
	auto pwsPath = fullpath.c_str();
	auto szpos = fullpath.find_last_of(L"\\/");
	std::wstring FileName;
	if (szpos == std::wstring::npos) {
		FileName = fullpath;
	}
	else
	{
		FileName = fullpath.substr(szpos + 1);
	}

	HANDLE hFile = CreateFileW(fullpath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		Helper::printconsole(L"File [");
		Helper::printconsole(fullpath.c_str());
		Helper::printconsole(L"] Open Failed with Code: ");
		wchar_t tmp[16];
		swprintf_s(tmp, 16, L"%X\n", GetLastError());
		Helper::printconsole((wchar_t*)tmp);
		return;
	}
	HandleWarpper closefile(hFile);

	if (FileName.ends_with(L".z.bin")) {
		auto pData = Helper::ReadAll(hFile);
		auto HeaderBuffer = pData.GetPtr();
		if (*(uint32_t*)HeaderBuffer != 0xFFFFFFFF) {
			Helper::printconsole(L"File [");
			Helper::printconsole(fullpath.c_str());
			Helper::printconsole(L"] May Not be z.bin Format.\n");
			return;
		}
		//
		auto Datas = Pack::zlib_z_bin(pData);
#if _DEBUG && 0
		//输出zlib_bin切割后的文件
		int index = 0;
		for (auto& i : Datas) {
			WCHAR tmp[16];
			swprintf_s(tmp, 16, L"%03X", index);
			index++;
			std::wstring path = L"R:\\" + FileName + tmp;
			i.DumpToFile(path.c_str());
		}
#endif
		std::vector<DataManiger> uncompressZlibDatas;
		for (auto cpos = Datas.begin() + 1; cpos < Datas.end(); ++cpos) {
			uncompressZlibDatas.push_back(zlib_z(*cpos));
		}
		int index = 1;
		for (auto& i : uncompressZlibDatas) {
			WCHAR tmp[16];
			swprintf_s(tmp, 16, L"%02X.etc.dds", index);
			index++;
			std::wstring path = L"R:\\" + FileName + tmp;
			i.DumpToFile(path.c_str());
		}
	}
	else if (FileName.ends_with(L"dds.z")) {
		//
		auto pData = Helper::ReadAll(hFile);
		auto HeaderBuffer = pData.GetPtr();
		if (HeaderBuffer[4] != 'x') {
			Helper::printconsole(L"File [");
			Helper::printconsole(fullpath.c_str());
			Helper::printconsole(L"] May Not be .z Format.\n");
			return;
		}
		auto uc_data = zlib_z(pData);
#if _DEBUG && 0
		auto str = FileName;
		auto pos = str.find_last_of(L".");
		str = str.substr(pos);
		auto str2 = std::wstring(L"R:\\") + str.c_str();
		uc_data.DumpToFile(str2.c_str());
#endif
		uint32_t w, h, mip;
		uint8_t a;
		Converter::dds2rgb_a(uc_data, w, h, mip, a);
	}
	else if (FileName.ends_with(L".dds")) {
		//
		auto pData = Helper::ReadAll(hFile);
		auto HeaderBuffer = pData.GetPtr();
		if (*(uint32_t*)HeaderBuffer != 0x20534444) {
			Helper::printconsole(L"File [");
			Helper::printconsole(fullpath.c_str());
			Helper::printconsole(L"] May Not be dds Format.\n");
			return;
		}
	}
	else
	{
		Helper::printconsole(L"File [");
		Helper::printconsole(fullpath);
		Helper::printconsole(L"] Make Hard Link to [");
		Helper::printconsole(L"]\n");
	}
}

std::vector<DataManiger> Pack::zlib_z_bin(const DataManiger& pData)
{
	auto raw = pData.GetPtr();
	size_t lenth = pData.GetLen();
	std::vector<DataManiger> re;
	auto pos = raw;
	auto end = pos + lenth;
	auto header = std::make_unique<uint8_t[]>(16);
	memcpy(header.get(), raw, 16);
	re.push_back(DataManiger{ std::move(header),16 });

	pos += 16;
	while (pos < end) {
		std::unique_ptr<uint8_t[]> zlibdata;
		size_t lenth = 0;
		auto dwlen = *(uint32_t*)pos;
		pos += sizeof(uint32_t);
		lenth = dwlen;
		zlibdata = std::make_unique<uint8_t[]>(lenth);
		memcpy(zlibdata.get(), pos, lenth);
		pos += lenth;
		re.emplace_back(std::move(zlibdata), lenth);
	}
#if _DEBUG
	for (auto pos = re.begin() + 1; pos < re.end(); ++pos) {
		assert((pos->GetPtr())[4] == 'x');
	}
#endif
	return re;
}

DataManiger Pack::zlib_z(const DataManiger& pData)
{
	DataManiger re;
	auto raw = pData.GetPtr();
	auto pcdata = raw + 4;
	auto len = pData.GetLen() - 4;//skip a uint32
	auto uncompresslen = *(uint32_t*)raw;
	DataManiger::ByteFlow f = std::make_unique<uint8_t[]>((size_t)uncompresslen + 4);
	auto pucdata = f.get();
	static_assert(sizeof(uLongf) == sizeof(uint32_t));
	auto ucre = uncompress(pucdata, (uLongf*)&uncompresslen, pcdata, len);
	if (ucre != Z_OK) {
		assert(ucre == Z_OK);//false
		return DataManiger();
	}
	re.pData = std::move(f);
	re.lenth = uncompresslen;
	return re;
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

DataManiger Converter::dds2rgb_a(const DataManiger& etcfile, uint32_t& w, uint32_t& h, uint32_t& mipLevel, uint8_t& bHasAlpha)
{
	DataManiger re;
	const DirectX::DDS_HEADER* const pHeader = (DirectX::DDS_HEADER*)(etcfile.GetPtr() + 4);
	auto rawData = etcfile.GetPtr() + 4 + sizeof(DirectX::DDS_HEADER);
	auto len = etcfile.GetLen() - 4 - sizeof(DirectX::DDS_HEADER);
	if (*(uint32_t*)etcfile.GetPtr() != 0x20534444)
	{
		return re;
	}
	w = pHeader->width;
	h = pHeader->height;
	mipLevel = pHeader->mipMapCount;
	CMP_FORMAT format{};
	bool bAlpha = false;
	switch (pHeader->ddspf.fourCC) {
	case 0x20435441u: {
		format = CMP_FORMAT_ATC_RGB;
		break;
	}
	case 0x49435441u: {
		format = CMP_FORMAT_ATC_RGBA_Interpolated;
		bAlpha = true;
		break;
	}
	case 0x41435441u: {
		format = CMP_FORMAT_ATC_RGBA_Explicit;
		bAlpha = true;
		break;
	}
	case 0x20435445u: {
		format = CMP_FORMAT_ETC_RGB;
		break;
	}
	case 0: {
		format = CMP_FORMAT_RG_8;//RGBA 4444
		bAlpha = true;
		break;
	}
	default:
		format = CMP_FORMAT_RG_8;//RGBA 4444
		bAlpha = true;
		break;
	}
	bHasAlpha = bAlpha;
	if (format == CMP_FORMAT_RG_8) {
		bHasAlpha = true;
		re = Converter::rgba4_to_rgba8(rawData, len);

#if _DEBUG
		//show image
		auto ptrdst = re.GetPtr();
		auto type = bHasAlpha ? CV_8UC4 : CV_8UC3;
		auto channal = bHasAlpha ? 4 : 3;
		for (int i = 0; i != mipLevel; ++i) {
			auto _w = w / uint32_t(std::pow(2, i));
			auto _h = h / uint32_t(std::pow(2, i));
			cv::Mat mat = cv::Mat(_h, _w, type, ptrdst, 0);
			ptrdst += _w * _h * channal;
			cv::imshow("rgba4 etc", mat);
			cv::waitKey(0);
		}
#endif
		return re;
	}
	else //call CMP SDK
	{

		CMP_Texture Tex1 = { 0 };
		Tex1.dwSize = sizeof(Tex1);
		Tex1.dwWidth = w;
		Tex1.dwHeight = h;
		Tex1.dwPitch = 0;
		Tex1.format = format;
		CMP_Texture Tex2 = { 0 };
		Tex2.dwSize = sizeof(Tex2);
		//Choose BGR for OpenCV
		Tex2.format = bHasAlpha ? CMP_FORMAT_RGBA_8888 : CMP_FORMAT_RGB_888;
		size_t totalSize = 0;
		size_t totalSize2 = 0;
		for (unsigned int i = 0; i != mipLevel; ++i) {
			Tex1.dwWidth = w / uint32_t(std::pow(2, i));
			Tex1.dwHeight = h / uint32_t(std::pow(2, i));
			totalSize += CMP_CalculateBufferSize(&Tex1);
			Tex2.dwPitch = 0;
			Tex2.dwWidth = Tex1.dwWidth;
			Tex2.dwHeight = Tex1.dwHeight;
			totalSize2 += CMP_CalculateBufferSize(&Tex2);
		}
		if (len < totalSize) {
			mipLevel = 1;
		}
		auto ptrsrc = rawData;
		DataManiger::ByteFlow pDest = std::make_unique<uint8_t[]>(totalSize2);
		auto ptrdst = pDest.get();
		CMP_CompressOptions options = {};
		options.dwSize = sizeof(options);
		options.bDisableMultiThreading = true;
		//options.bUseGPUDecompress = true;
		for (uint32_t i = 0; i != mipLevel; ++i) {
			Tex1.dwWidth = w / uint32_t(std::pow(2, i));
			Tex1.dwHeight = h / uint32_t(std::pow(2, i));
			Tex1.dwPitch = Tex1.dwWidth;
			Tex1.dwDataSize = CMP_CalculateBufferSize(&Tex1);
			Tex1.pData = ptrsrc;
			ptrsrc += Tex1.dwDataSize;

			Tex2.dwWidth = Tex1.dwWidth;
			Tex2.dwPitch = 0;
			Tex2.dwHeight = Tex1.dwHeight;
			Tex2.dwDataSize = CMP_CalculateBufferSize(&Tex2);
			Tex2.pData = ptrdst;
			ptrdst += Tex2.dwDataSize;
			CMP_ConvertTexture(&Tex1, &Tex2, &options, 0);
		}
#if _DEBUG
		//show image
		ptrdst = pDest.get();
		auto type = bHasAlpha ? CV_8UC4 : CV_8UC3;
		auto channal = bHasAlpha ? 4 : 3;
		for (int i = 0; i != mipLevel; ++i) {
			auto _w = w / uint32_t(std::pow(2, i));
			auto _h = h / uint32_t(std::pow(2, i));
			cv::Mat mat = cv::Mat(_h, _w, type, ptrdst, 0);
			ptrdst += _w * _h * channal;
			cv::imshow("cmp_decode", mat);
			cv::waitKey(1000);
		}
#endif

	}
	return re;
}
DataManiger Converter::rgba4_to_rgba8(const uint8_t* data, size_t size)
{
	auto src = data;
	auto pixnum = (size + 1) / 2;
	DataManiger::ByteFlow f = std::make_unique<uint8_t[]>(pixnum * 4);
	auto dst = f.get();
	for (int i = 0; i != pixnum; ++i)
	{
		uint16_t tmpdata;
		tmpdata = *(uint16_t*)src;

		uint16_t b, g, r, a;
		b = tmpdata & 0xF000u;
		g = tmpdata & 0x0F00u;
		r = tmpdata & 0x00f0u;
		a = tmpdata & 0x000fu;
		b >>= 12;
		g >>= 8;
		r >>= 4;
		a >>= 0;
		unsigned char cb = 0, cg = 0, cr = 0, ca = 0;
		cb |= b;
		cg |= g;
		cr |= r;
		ca |= a;
		cb = cb | (cb << 4);
		cg = (cg << 4) | cg;
		cr = (cr << 4) | cr;
		ca = (ca << 4) | ca;
		uint8_t* rgba = dst;
		//修改顺序使得其与CMP解码一致
		rgba[0] = cb;
		rgba[1] = cg;
		rgba[2] = cr;
		rgba[3] = ca;

		src += 2;
		dst += 4;
	}
	return DataManiger(std::move(f), pixnum * 4);
}

DataManiger Converter::dds_to_rgba(const DataManiger& File, uint8_t& bHasAlpha)
{

	return DataManiger();
}

