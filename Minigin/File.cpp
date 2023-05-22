#include "File.h"
#include <filesystem>
#include <fstream>

using namespace dae;
namespace fs = std::filesystem;

File dae::File::OpenFile(const std::string& path)
{
	auto p = fs::current_path().append(path);
	File file;
	file.stream = new std::fstream();
	file.stream->open(p, std::fstream::binary | std::fstream::in);
	if (file.stream->is_open()) {
		return file;
	}
	delete file.stream;
	file.stream = nullptr;
	return File();
}

File dae::File::CreateFile(const std::string& path)
{
	auto p = fs::current_path().append(path);
	File file;
	file.stream = new std::fstream();
	file.stream->open(p, std::fstream::binary | std::fstream::out | std::fstream::trunc);
	if (file.stream->is_open()) {
		return file;
	}
	delete file.stream;
	file.stream = nullptr;
	return File();
}

void dae::File::Write(const char* data, int len)
{
	if (stream) stream->write(data, len);
}

void dae::File::Read(char* out, int count) {
	if (!stream) return;
	stream->read(out, count);
}

std::string dae::File::ReadAll(int& count)
{
	if (!stream) return "";
	stream->clear();
	stream->seekg(0);
	std::stringstream buffer;
	buffer << stream->rdbuf();
	count = (int)buffer.str().length();
	return std::move(buffer.str());
}

dae::File::operator bool()
{
	return stream && stream->is_open();
}

dae::File::File()
{
	stream = nullptr;
}

dae::File::~File()
{
	if (stream) {
		stream->close();
		delete stream;
	}
}
