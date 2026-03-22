#include"file_process_unpack.h"

//setting modle make sure it's value
extern std::wstring inType;
extern std::vector<std::wstring> outTypes;

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

void Pack::process_filter(const wchar_t* full, const wchar_t* base, const wchar_t* relative)
{
#if _DEBUG
	Helper::printconsole(std::wstring_view(L"\t["));
	Helper::printconsole(full);
	Helper::printconsole(std::wstring_view(L"]\n"));
#endif

	auto fullpath = (std::wstring)full;
	auto basepath = (std::wstring)base;
	auto relativepath = (std::wstring)relative;

	//return "basepath" + "_dxt"
	auto GetDxtPath = [](const std::wstring& basepath)->std::wstring {
		auto lpwstr = DataManiger((basepath.size() + 32) * sizeof(wchar_t));
		auto strbuffer = (wchar_t*)lpwstr.GetPtr();
		auto strbufferlen = (basepath.size() + 32);
		wcscpy(strbuffer, basepath.c_str());
		PathCchRemoveBackslash(strbuffer, strbufferlen);
		wcscat(strbuffer, L"_dxt");
		PathCchAddBackslash(strbuffer, strbufferlen);
		return std::wstring(strbuffer);
		};

	auto LinkFile = [&GetDxtPath](const std::wstring& rawfullpath, const std::wstring& basepath, const std::wstring& relativepath) -> void {
		auto outpath = GetDxtPath(basepath) + relativepath;
		auto re = CreateHardLinkW(outpath.c_str(), rawfullpath.c_str(), NULL);
		if (!re) {
			if (GetLastError() == ERROR_PATH_NOT_FOUND) {
				auto re2 = Helper::CreatePathFromFileName(outpath);
				if (re2) {
					re = CreateHardLinkW(outpath.c_str(), rawfullpath.c_str(), NULL);
				}
			}
		}
		if (re) {
			Helper::printconsole(std::format(L"File [{}] Make Hard Link to [{}]\n", rawfullpath, outpath));
		}
		else {
			auto er = GetLastError();
			if (er != ERROR_ALREADY_EXISTS) {
				Helper::printconsole(std::format(L"Failed to Make Hard Link Form [{}] to [{}], Code {}\n", rawfullpath, outpath, GetLastError()));
			}
			else
			{
				Helper::printconsole(std::format(L"File [{}] Already Exists.\n", outpath));
			}

		}
		};
	if (relativepath.rfind(inType) == std::wstring::npos) {
		LinkFile(fullpath, basepath, relativepath);
		return;
	}


	auto GetNewRelativePath =
		[](const std::wstring& outType, const std::wstring& relativepath)->std::wstring {
		auto filetypepos = relativepath.rfind(inType);
		if (filetypepos == std::wstring::npos) {
			return std::wstring();
		}
		std::wstring path = relativepath;
		path.replace(filetypepos, outType.size(), outType);
		return path;
		};
	auto ErrorExit = []() ->void {
		auto Error = GetLastError();
		Helper::printconsole(Helper::ErrorMessageToWstring(Error));
		Helper::printconsole(std::wstring_view(L"\n"));
		return;
		};

	auto SaveFileAndoutput = [&GetNewRelativePath, &GetDxtPath](const DataManiger& data, const std::wstring& basepath, const std::wstring& relativepath, const std::wstring& oType)->void {
		auto outpath = GetDxtPath(basepath) + GetNewRelativePath(oType, relativepath);
		if (data.WriteToFile(outpath.c_str())) {
			Helper::printconsole(std::format(L"Convert and save file [{}]\n", outpath));
			return;
		}
		else
		{
			Helper::printconsole(std::format(L"Failed to save file [{}]. Code{}\n", outpath, GetLastError()));
			return;
		}
		};


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

	if (relativepath.ends_with(L".z.bin")) {
		auto pData = Helper::ReadAll(hFile);
		auto HeaderBuffer = pData.GetPtr();
		if (*(uint32_t*)HeaderBuffer != 0xFFFFFFFF) {
			Helper::printconsole(L"File [");
			Helper::printconsole(fullpath.c_str());
			Helper::printconsole(L"] May Not be z.bin Format.\n");
			return;
		}
		//
		auto Datas = Pack::zlib_z_bin_uc(pData);
#if _DEBUG && 0
		//输出zlib_bin切割后的文件
		int index = 0;
		for (auto& i : Datas) {
			WCHAR tmp[16];
			swprintf_s(tmp, 16, L"%03X", index);
			index++;
			std::wstring path = L"R:\\" + relativepath + tmp;
			i.DumpToFile(path.c_str());
		}
#endif
		std::vector<DataManiger> uncompressZlibDatas;
		for (auto cpos = Datas.begin() + 1; cpos < Datas.end(); ++cpos) {
			uncompressZlibDatas.push_back(zlib_z_uc(*cpos));
		}
#if _DEBUG && 0	
		int index = 1;
		for (auto& i : uncompressZlibDatas) {
			WCHAR tmp[16];
			swprintf_s(tmp, 16, L"%02X.etc.dds", index);
			index++;
			std::wstring path = L"R:\\" + relativepath + tmp;
			i.DumpToFile(path.c_str());
		}
#endif
		std::vector<std::pair<DataManiger, uint8_t>> rgbaDatas;
		for (const auto& i : uncompressZlibDatas) {
			uint32_t h, w, pix;
			uint8_t a;
			auto rgbdata = Converter::dds2rgb_a(i, w, h, pix, a);
			rgbaDatas.push_back({ std::move(rgbdata),a });
		}
		//now compress.
		//first dxt
		std::vector<DataManiger> dxtDatas;
		assert(rgbaDatas.size() == uncompressZlibDatas.size());
		for (size_t i = 0; i != rgbaDatas.size(); ++i) {
			const auto& dds = uncompressZlibDatas[i];
			const auto& rgb = rgbaDatas[i];
			auto dxtData = Converter::rgb_a2dxt(rgb.first, reinterpret_cast<const DirectX::DDS_HEADER*>(dds.GetPtr() + 4), 0, 0, 0, rgb.second);
			dxtDatas.push_back(std::move(dxtData));
		}
		//then zlib
		std::vector<DataManiger> zDatas;
		zDatas.push_back(std::move(Datas[0]));
		for (const auto& i : dxtDatas) {
			zDatas.push_back(zlib_z_c(i));
		}
		//then bin
		auto dxtddszbindata = zlib_z_bin_c(zDatas);
#if _DEBUG && 0
		{
			std::wstring path = L"R:\\" + relativepath;
			auto pos = path.find(L"etc");
			assert(pos != std::wstring::npos);
			path.replace(pos, 3, L"dxt");
			dxtddszbindata.DumpToFile(path.c_str());
		}
#endif
		//writefile
		SaveFileAndoutput(dxtddszbindata, basepath, relativepath, L"dxt");
		//z bin end
	}
	else if (relativepath.ends_with(L"dds.z")) {
		//
		auto pData = Helper::ReadAll(hFile);
		auto HeaderBuffer = pData.GetPtr();
		if (HeaderBuffer[4] != 'x') {
			Helper::printconsole(L"File [");
			Helper::printconsole(fullpath.c_str());
			Helper::printconsole(L"] May Not be .z Format.\n");
			return;
		}
		auto uc_data = zlib_z_uc(pData);
#if _DEBUG && 0
		{
			auto str = relativepath;
			auto pos = str.find_last_of(L".");
			str = str.substr(pos);
			auto str2 = std::wstring(L"R:\\") + str.c_str();
			uc_data.DumpToFile(str2.c_str());
		}
#endif
		uint32_t w, h, mip;
		uint8_t a;//alpha
		auto rgbflow = Converter::dds2rgb_a(uc_data, w, h, mip, a);
		auto dxtfile = Converter::rgb_a2dxt(rgbflow, (DirectX::DDS_HEADER*)(uc_data.GetPtr() + 4), w, h, mip, a);
#if _DEBUG && 0
		{
			auto strdxt = L"R:\\" + relativepath + L"_dmp.dxt.dds";
			dxtfile.DumpToFile(strdxt.c_str());
		}
#endif
		auto dxtzfile = Pack::zlib_z_c(dxtfile);
#if _DEBUG && 0
		{
			std::wstring path = L"R:\\" + relativepath;
			auto pos = path.find(L"etc");
			assert(pos != std::wstring::npos);
			path.replace(pos, 3, L"dxt");
			dxtzfile.DumpToFile(path.c_str());
		}
#endif
		SaveFileAndoutput(dxtzfile, basepath, relativepath, L"dxt");
	}
	else if (relativepath.ends_with(L".dds")) {
		//
		auto pData = Helper::ReadAll(hFile);
		auto HeaderBuffer = pData.GetPtr();
		if (*(uint32_t*)HeaderBuffer != 0x20534444) {
			Helper::printconsole(L"File [");
			Helper::printconsole(fullpath.c_str());
			Helper::printconsole(L"] May Not be dds Format.\n");
			return;
		}
		uint32_t a, b, c;
		uint8_t bAlpha;
		auto rgb = Converter::dds2rgb_a(pData, a, b, c, bAlpha);
		auto dxt = Converter::rgb_a2dxt(rgb, reinterpret_cast<DirectX::DDS_HEADER*>(pData.GetPtr() + 4), 0, 0, 0, bAlpha);
#if _DEBUG && 0
		std::wstring path = L"R:\\" + relativepath;
		auto pos = path.find(L"etc");
		assert(pos != std::wstring::npos);
		path.replace(pos, 3, L"dxt");
		dxt.DumpToFile(path.c_str());
#endif
		SaveFileAndoutput(dxt, basepath, relativepath, L"dxt");
	}
	else
	{
		LinkFile(fullpath, basepath, relativepath);

	}
}

std::vector<DataManiger> Pack::zlib_z_bin_uc(const DataManiger& pData)
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

DataManiger Pack::zlib_z_bin_c(const std::vector<DataManiger>& pDatas)
{
	size_t sz = 0;
	for (const auto& i : pDatas) {
		sz += i.GetLen();
	}
	sz += 4 * (pDatas.size() - 1);
	DataManiger re(sz);
	auto pdst = re.GetPtr();
	//Header
	memcpy(pdst, pDatas[0].GetPtr(), pDatas[0].GetLen());
	pdst += pDatas[0].GetLen();

	for (auto pos = pDatas.cbegin() + 1; pos != pDatas.cend(); ++pos) {
		auto& i = *pos;
		*(uint32_t*)pdst = (uint32_t)i.GetLen();
		pdst += 4;
		memcpy(pdst, i.GetPtr(), i.GetLen());
		pdst += i.GetLen();
	}
	return re;
}

DataManiger Pack::zlib_z_uc(const DataManiger& pData)
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

DataManiger Pack::zlib_z_c(const DataManiger& pData)
{
	auto sz = compressBound(pData.GetLen()) + 4;
	DataManiger::ByteFlow f = DataManiger::CreateBuffer(sz);
	auto fptr = f.get();
	*(uint32_t*)(fptr) = (uint32_t)pData.GetLen();
	fptr += 4;
	auto re = compress2(fptr, &sz, pData.GetPtr(), pData.GetLen(), 9);
	if (re != Z_OK) {
		assert(false);
		wchar_t buffer[16];
		Helper::printconsole(std::wstring_view(L"Zlib compress failed. Code:"));
		swprintf_s(buffer, 16, L"%d\n", re);
		Helper::printconsole(buffer);
		return DataManiger();
	}

	return DataManiger(std::move(f), sz + 4);
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
		auto re = Converter::rgba4_to_rgba8(rawData, len);

#if _DEBUG && 0
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
#if _DEBUG && 0
		{
			auto type = bHasAlpha ? CV_8UC4 : CV_8UC3;
			auto ptrdst = re.GetPtr();
			auto _w = w;
			auto _h = h;
			cv::Mat mat = cv::Mat(_h, _w, type, ptrdst, 0);
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
		// ohfuck. RGB/BGR use the same RGB order...
		// so why the SDK tell me nothing?
		// detail:
		// compressonator src file: 
		// compressonator-master\cmp_compressonatorlib\buffer\codecbuffer.cpp,
		// func:
		// CodecBufferType GetCodecBufferType(CMP_FORMAT format);
		// line 195~200:
		// case CMP_FORMAT_ARGB_8888:	case CMP_FORMAT_BGRA_8888:	case CMP_FORMAT_RGBA_8888:	CBT_type = CBT_RGBA8888;	break;
		// case CMP_FORMAT_BGR_888:		case CMP_FORMAT_RGB_888:								CBT_type = CBT_RGB888;		break;
		//
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
#if _DEBUG && 0
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

#if _DEBUG && 0
		{
			auto type = bHasAlpha ? CV_8UC4 : CV_8UC3;
			auto ptrdst = pDest.get();
			auto _w = w;
			auto _h = h;
			cv::Mat mat = cv::Mat(_h, _w, type, ptrdst, 0);
			cv::imshow("cmp_decode", mat);
			cv::waitKey(0);
		}
#endif
		return DataManiger(std::move(pDest), totalSize2);
	}
}
DataManiger Converter::rgb_a2dxt(const DataManiger& rgb_a_buffer, const DirectX::DDS_HEADER* pRawHeader, uint32_t w, uint32_t h, uint32_t mipLevel, uint8_t bHasAlpha)
{
	uint32_t magicNum = 0x20534444;
	DirectX::DDS_HEADER header = { 0 };
	if (pRawHeader) {
		memcpy(&header, pRawHeader, sizeof(header));
		header.size = sizeof(header);
		header.flags |= DDS_HEADER_FLAGS_LINEARSIZE;
		auto& headerpf = header.ddspf;
		headerpf = ((bHasAlpha ? DirectX::DDSPF_DXT5 : DirectX::DDSPF_DXT1));
		//overwrite
		if (w == 0) {
			w = header.width;
		}
		if (h == 0)
		{
			h = header.height;
		}
		if (mipLevel == 0)
		{
			mipLevel = header.mipMapCount;
		}
	}
	else {
		constexpr uint32_t ddsflag = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP | DDS_HEADER_FLAGS_LINEARSIZE;
		static_assert(ddsflag == 0xA1007);
		header.flags = ddsflag;
		header.height = h;
		header.width = w;
		header.pitchOrLinearSize = -1;//TODO: do it when CalcBufferSize.
		header.depth = 0;
		header.mipMapCount = mipLevel;
		header.reserved1;
		auto& headerpf = header.ddspf;
		headerpf = ((bHasAlpha ? DirectX::DDSPF_DXT5 : DirectX::DDSPF_DXT1));
		header.caps = DDS_SURFACE_FLAGS_MIPMAP | DDS_SURFACE_FLAGS_TEXTURE;
		header.caps2 = 0;//wait! mabe copy from src is a good choice.
	}
	size_t elemetsize = bHasAlpha ? 4 : 3;
	CMP_Texture Dst = { 0 };
	Dst.dwSize = sizeof(Dst);
	Dst.format = bHasAlpha ? CMP_FORMAT_DXT5 : CMP_FORMAT_DXT1;
	Dst.dwWidth = w;
	Dst.dwHeight = h;
	Dst.dwDataSize = CMP_CalculateBufferSize(&Dst);
	header.pitchOrLinearSize = Dst.dwDataSize;

	size_t totalsize = 128;
	for (uint32_t i = 0; i != mipLevel; ++i) {
		auto _w = w / uint32_t(std::pow(2, i));
		auto _h = h / uint32_t(std::pow(2, i));
		Dst.dwWidth = _w;
		Dst.dwHeight = _h;
		Dst.dwDataSize = CMP_CalculateBufferSize(&Dst);
		totalsize += Dst.dwDataSize;
	}

	CMP_Texture Src = { 0 };
	Src.dwSize = sizeof(Src);
	Src.format = bHasAlpha ? CMP_FORMAT_RGBA_8888 : CMP_FORMAT_BGR_888;
	DataManiger re(totalsize);

	CMP_CompressOptions opt = { 0 };
	opt.dwSize = sizeof(opt);
	opt.bDisableMultiThreading = true;
	opt.fquality = 0.01;

	uint8_t* srcptr;
	DataManiger dataexc;
	if (!bHasAlpha) {
		dataexc = DataManiger(rgb_a_buffer.GetLen());
		memcpy(dataexc.GetPtr(), rgb_a_buffer.GetPtr(), rgb_a_buffer.GetLen());
		// dont ask me why... I also want to konw why cmp sdk works like this...
		// I do make sure dds2rgba is all same with rgb/rgba, witch is r/b order reversed with cv.
		Helper::RBChannelExchange(dataexc.pData, dataexc.GetLen(), bHasAlpha);
		srcptr = dataexc.GetPtr();
	}
	else {
		srcptr = rgb_a_buffer.GetPtr();
	}

	auto dstptr = re.GetPtr() + 128;

	auto _w = w, _h = h;

	for (uint32_t i = 0; i != mipLevel; ++i) {
		Src.dwWidth = _w;
		Src.dwHeight = _h;
		Src.dwDataSize = (size_t)_w * _h * elemetsize;
		Src.pData = srcptr;


		Dst.dwWidth = _w;
		Dst.dwHeight = _h;
		Dst.dwDataSize = CMP_CalculateBufferSize(&Dst);
		Dst.pData = dstptr;

		CMP_ConvertTexture(&Src, &Dst, &opt, 0);
		_w /= 2;
		_h /= 2;
		srcptr += Src.dwDataSize;
		dstptr += Dst.dwDataSize;
		if ((_w | _h) == 0) {
			break;
		}
	}
	memcpy(re.GetPtr(), &magicNum, 4);
	memcpy(re.GetPtr() + 4, &header, 124);
	return re;
}
DataManiger Converter::rgb_a2atc(const DataManiger& rgb_a_buffer, const DirectX::DDS_HEADER* pRawHeader, uint32_t w, uint32_t h, uint32_t mipLevel, uint8_t bHasAlpha)
{
	//TODO:ChangeCode.
	uint32_t magicNum = 0x20534444;
	DirectX::DDS_HEADER header = { 0 };
	if (pRawHeader) {
		memcpy(&header, pRawHeader, sizeof(header));
		header.size = sizeof(header);
		header.flags |= DDS_HEADER_FLAGS_LINEARSIZE;
		auto& headerpf = header.ddspf;
		headerpf = ((bHasAlpha ? DirectX::DDSPF_DXT5 : DirectX::DDSPF_DXT1));
		//overwrite
		if (w == 0) {
			w = header.width;
		}
		if (h == 0)
		{
			h = header.height;
		}
		if (mipLevel == 0)
		{
			mipLevel = header.mipMapCount;
		}
	}
	else {
		constexpr uint32_t ddsflag = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP | DDS_HEADER_FLAGS_LINEARSIZE;
		static_assert(ddsflag == 0xA1007);
		header.flags = ddsflag;
		header.height = h;
		header.width = w;
		header.pitchOrLinearSize = -1;//TODO: do it when CalcBufferSize.
		header.depth = 0;
		header.mipMapCount = mipLevel;
		header.reserved1;
		auto& headerpf = header.ddspf;
		headerpf = ((bHasAlpha ? DirectX::DDSPF_DXT5 : DirectX::DDSPF_DXT1));
		header.caps = DDS_SURFACE_FLAGS_MIPMAP | DDS_SURFACE_FLAGS_TEXTURE;
		header.caps2 = 0;//wait! mabe copy from src is a good choice.
	}
	size_t elemetsize = bHasAlpha ? 4 : 3;
	CMP_Texture Dst = { 0 };
	Dst.dwSize = sizeof(Dst);
	Dst.format = bHasAlpha ? CMP_FORMAT_DXT5 : CMP_FORMAT_DXT1;
	Dst.dwWidth = w;
	Dst.dwHeight = h;
	Dst.dwDataSize = CMP_CalculateBufferSize(&Dst);
	header.pitchOrLinearSize = Dst.dwDataSize;

	size_t totalsize = 128;
	for (uint32_t i = 0; i != mipLevel; ++i) {
		auto _w = w / uint32_t(std::pow(2, i));
		auto _h = h / uint32_t(std::pow(2, i));
		Dst.dwWidth = _w;
		Dst.dwHeight = _h;
		Dst.dwDataSize = CMP_CalculateBufferSize(&Dst);
		totalsize += Dst.dwDataSize;
	}

	CMP_Texture Src = { 0 };
	Src.dwSize = sizeof(Src);
	Src.format = bHasAlpha ? CMP_FORMAT_RGBA_8888 : CMP_FORMAT_BGR_888;
	DataManiger re(totalsize);

	CMP_CompressOptions opt = { 0 };
	opt.dwSize = sizeof(opt);
	opt.bDisableMultiThreading = true;
	opt.fquality = 0.1;

	uint8_t* srcptr;
	DataManiger dataexc;
	if (!bHasAlpha) {
		dataexc = DataManiger(rgb_a_buffer.GetLen());
		memcpy(dataexc.GetPtr(), rgb_a_buffer.GetPtr(), rgb_a_buffer.GetLen());
		// dont ask me why... I also want to konw why cmp sdk works like this...
		// I do make sure dds2rgba is all same with rgb/rgba, witch is r/b order reversed with cv.
		Helper::RBChannelExchange(dataexc.pData, dataexc.GetLen(), bHasAlpha);
		srcptr = dataexc.GetPtr();
	}
	else {
		srcptr = rgb_a_buffer.GetPtr();
	}

	auto dstptr = re.GetPtr() + 128;

	auto _w = w, _h = h;

	for (uint32_t i = 0; i != mipLevel; ++i) {
		Src.dwWidth = _w;
		Src.dwHeight = _h;
		Src.dwDataSize = (size_t)_w * _h * elemetsize;
		Src.pData = srcptr;


		Dst.dwWidth = _w;
		Dst.dwHeight = _h;
		Dst.dwDataSize = CMP_CalculateBufferSize(&Dst);
		Dst.pData = dstptr;

		CMP_ConvertTexture(&Src, &Dst, &opt, 0);
		_w /= 2;
		_h /= 2;
		srcptr += Src.dwDataSize;
		dstptr += Dst.dwDataSize;
		if ((_w | _h) == 0) {
			break;
		}
	}
	memcpy(re.GetPtr(), &magicNum, 4);
	memcpy(re.GetPtr() + 4, &header, 124);
	return re;
	return DataManiger();
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

DataManiger Converter::rgba8_to_rgba4(const uint8_t* data, size_t size)
{
	auto src = data;
	auto pixnum = (size) / 4;
	DataManiger re = { DataManiger::CreateBuffer(pixnum * 2) ,pixnum * 2 };
	auto dst = re.GetPtr();
	//也许有更好的实现？不确定。总之留作lambda
	auto b82b4 = [](uint8_t i)->uint8_t {
		return i >> 4;
		};
	for (size_t i = 0; i != pixnum; ++i) {
		uint8_t cr, cg, cb, ca;
		auto rgba = src;
		cb = rgba[0];
		cg = rgba[1];
		cr = rgba[2];
		ca = rgba[3];
		b82b4(cb);
		b82b4(cg);
		b82b4(cr);
		b82b4(ca);
		uint16_t dest = 0;
		dest |= uint16_t(cb) << 12;
		dest |= uint16_t(cg) << 8;
		dest |= uint16_t(cr) << 4;
		dest |= uint16_t(ca) << 0;
		*(uint16_t*)dst = dest;
		dst += 2;
		src += 4;
	}
	return re;
}

