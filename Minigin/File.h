#pragma once
#include "Singleton.h"
#include <string>

namespace dae 
{

class File final
{
public:
	static File OpenFile(const std::string& path);
	static bool CreateFile(const std::string& path);

	void Write(const char* data, int len, int pos = -1);

	// Doesn't handle pointers, use only pod types
	template<class T>
	void WriteObject(T& obj, int pos = -1) {
		if (std::is_standard_layout<T>()) {
			Write((const char*)&obj, sizeof(T), pos); 
		}
	}

	void Read(char* out, int count);
	std::string ReadAll(int& count);
	
	template <class T>
	T ReadObject() {
		T obj{};
		Read((char*)&obj, sizeof(T));
		return std::move(obj);
	}

	operator bool();

	File();
	~File();

	File(File& other) {
		stream = other.stream;
		other.stream = nullptr;
	}
	File(File&& other) noexcept {
		stream = other.stream;
		other.stream = nullptr;
	}
	File& operator=(const File& other) = delete;
	File& operator=(File&& other) noexcept {
		stream = other.stream;
		other.stream = nullptr;
		return *this;
	};
private:
	std::fstream* stream;
};

}