#pragma once

#include <ctype.h>
#include <string>
#include <string_view>
#include <vector>

namespace pushos {
namespace strextens {

inline void trim(std::string& str) {
	std::string::size_type start = 0;
	auto end = str.size();
	while (start < end && isspace(static_cast<unsigned char>(str[start]))) start++;
	while (end > start && isspace(static_cast<unsigned char>(str[end - 1]))) end--;
	str.erase(end);
	str.erase(0, start);
}

inline bool stringHas(std::string_view str, char ch) {
	return str.find_first_of(ch) != std::string_view::npos;
}

inline std::vector<std::string> split(std::string_view input, std::string_view delimiter) {
	if (delimiter.empty()) return {std::string(input)};
	std::vector<std::string> result;
	auto start = std::string_view::size_type{0};
	auto end = input.find(delimiter, start);
	while (end != std::string_view::npos) {
		result.emplace_back(input.substr(start, end - start));
		start = end + delimiter.size();
		end = input.find(delimiter, start);
	}
	result.emplace_back(input.substr(start));
	return result;
}

inline std::vector<std::string> splitPath(std::string_view path) {
	std::string str{path};
	trim(str);
	std::vector<std::string> path_args;
	if (!str.empty() && str.front() == '/') {
		path_args.emplace_back("/");
	}
	auto start = str.find_first_not_of('/');
	while (start != std::string::npos) {
		const auto end = str.find('/', start);
		if (end == std::string::npos) {
			path_args.emplace_back(str, start);
			break;
		}
		path_args.emplace_back(str, start, end - start);
		start = str.find_first_not_of('/', end);
	}
	return path_args;
}

inline std::string makePath(const std::vector<std::string>& path_args) {
	std::string path;
	for (const auto& component : path_args) {
		if (!path.empty() && path.back() != '/') {
			path += '/';
		}
		path += component;
	}
	return path;
}

} // namespace strextens
} // namespace pushos
