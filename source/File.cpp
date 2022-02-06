#include "File.h"
#include <fstream>

#include "Common.h"

std::vector<uint8_t> File::ReadAllBytes(const std::string& fileName)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	// TODO: maybe softer/soft err
	CRITICAL_ASSERT(file.is_open(), "Failed to open file %s", fileName.c_str());

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<uint8_t> buffer(fileSize);
	
	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();

	return buffer;
}