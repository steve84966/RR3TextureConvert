#pragma once
#include"framework.h"
#include"help.h"

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

	bool WriteToFile(LPCWSTR path)const;

	//IsValid
	operator bool()const { return (bool)(pData); }
public:
	std::unique_ptr<std::uint8_t[]> pData;
	size_t lenth = 0;
};