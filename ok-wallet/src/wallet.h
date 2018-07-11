#pragma once

#include <string>
#include <vector>

extern "C"
__attribute__ ((visibility("default")))
bool GetAddressFromPrivateKey(const std::string& prv, std::string& address);

extern "C"
__attribute__ ((visibility("default")))
bool
signTransaction(const std::string& utx, const std::string& prv, const std::string& net_type, const char* data_dir, std::string& stx);

//hot
//amount: Decimal String in Raw, not Nano.
extern "C"
__attribute__ ((visibility("default")))
bool
produceUnsignedTx(const std::string& from, const std::string& to, const std::string& amount, const std::string& net_type, const char* data_dir, std::string& utx);
