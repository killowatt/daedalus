#include "File.h"
#include <fstream>

std::vector<char> File::ReadAllBytes(const std::string& fileName)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("oops");
		// err lol
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}