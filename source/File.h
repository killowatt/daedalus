#pragma once

#include <vector>
#include <string>

namespace File
{
	std::vector<char> ReadAllBytes(const std::string& fileName);
}