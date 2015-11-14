#pragma once

#include <vector>
#include <string>

class sprite_base;

void pack(const std::vector<std::unique_ptr<sprite_base>>& sprites,
		const std::string& sheet_name,
		int sheet_width, int sheet_height,
		int border,
		const std::string& texture_path_base);
