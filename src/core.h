#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>
namespace course {
std::string sha1(const std::string& text);
std::map<std::string,std::string> load_users(const std::string& path);
bool authenticate(const std::string& message, const std::map<std::string,std::string>& users);
uint16_t sum_squares(const std::vector<uint16_t>& values);
}
