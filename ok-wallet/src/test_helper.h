#pragma once

#include <string>

extern "C"
__attribute__((visibility("default")))
bool testInit(const std::string& test_prv1, const std::string& test_prv2, std::string& data_dir);

extern "C"
__attribute__ ((visibility("default")))
bool commitTransaction(const std::vector<uint8_t>& stx, const std::string& net_type, const char* data_dir, std::string& result_str);
