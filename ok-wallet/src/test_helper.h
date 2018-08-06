#pragma once

#include <string>

/*testInit initialize some data for test.
  This function just for test envirment. */
extern "C"
__attribute__((visibility("default")))
bool testInit(const std::string& test_prv1, const std::string& test_account2);

extern "C"
__attribute__ ((visibility("default")))
bool commitTransaction(const std::string& stx, const std::string& net_type, std::string& result_str);
