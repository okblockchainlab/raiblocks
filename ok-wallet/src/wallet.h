#pragma once

#include <string>
#include <vector>

extern "C"
__attribute__ ((visibility("default")))
bool GetAddressFromPrivateKey(const std::string& prv, std::string& address);

extern "C"
__attribute__ ((visibility("default")))
bool
signTransaction(
  const std::string& utx,
  const std::string& prv,
  const std::string& net_type,
  std::string& stx);

//hot
//balance_s: Decimal String in Raw, not Nano.
//  This is the final balance for account after block creation,
//  so it's value is: balance_before_call_this_function - account_you_want_to_send.
extern "C"
__attribute__ ((visibility("default")))
bool
produceUnsignedTx(
  const std::string& link,
  const std::string& previous,
  const std::string& representative,
  const std::string& balance_s,
  std::string& utx);
