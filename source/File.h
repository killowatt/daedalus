#pragma once

#include <vector>
#include <string>

namespace File
{
	std::vector<uint8_t> ReadAllBytes(const std::string& fileName);
}