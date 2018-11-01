#include "com_okcoin_vault_jni_xrb_Xrbj.h"
#include "wallet.h"
#include "test_helper.h"
#include <regex>
#include <assert.h>
#include <mutex>

std::mutex mutex;

class AutoUnlock
{
public:
  ~AutoUnlock()
  {
    mutex.unlock();
  }
};

static jstring char2jstring(JNIEnv* env, const char* pat)
{
  jclass strClass = env->FindClass("Ljava/lang/String;");
  jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");

  jbyteArray bytes = env->NewByteArray(strlen(pat));
  env->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
  jstring encoding = env->NewStringUTF("utf-8");
  return (jstring)env->NewObject(strClass, ctorID, bytes, encoding);
}

static std::string jstring2stdstring(JNIEnv* env, const jstring& jstr)
{
  jclass clsstring = env->FindClass("java/lang/String");
  jstring strencode = env->NewStringUTF("utf-8");
  jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
  jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
  jsize alen = env->GetArrayLength(barr);
  jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);

  std::string res((const char*)ba, alen);
  env->ReleaseByteArrayElements(barr, ba, 0);
  return res;
}

static jobjectArray strings2jobjectArray(JNIEnv* env, const std::vector<std::string>& s)
{
  jobjectArray result = env->NewObjectArray(s.size(), env->FindClass("java/lang/String"), 0);

  for (int i = 0; i < s.size(); i++)
  {
    env->SetObjectArrayElement(result, i, char2jstring(env, s[i].c_str()));
  }

  return result;
}

static
std::vector<std::string> split_by_regex(const std::string& s, const char* pattern)
{
  std::regex pat(pattern);
  auto s_begin = std::sregex_iterator(s.begin(), s.end(), pat);
  auto s_end = std::sregex_iterator();

  std::vector<std::string> result;
  result.reserve(std::distance(s_begin, s_end));

  for (auto i = s_begin; i != s_end; i++) {
    std::smatch match = *i;
    result.emplace_back(match.str());
  }

  return result;
}

JNIEXPORT jobjectArray JNICALL
Java_com_okcoin_vault_jni_xrb_Xrbj_execute(JNIEnv *env, jclass, jstring networkType, jstring _command)
{
  if (mutex.try_lock() == false) {
    return strings2jobjectArray(env, {"Error", "JNI_LOCKED"});
  }
  AutoUnlock au;

  static struct {
    const char* cmd_name;
    std::function<jobjectArray(const std::vector<std::string>& args)> handler;
  } command_handlers[] = {
    {
      "getaddressbyprivatekey", [env](const std::vector<std::string>& args)->jobjectArray{
        assert(1 == args.size());
        std::string result("SUCCESS");
        std::string address;
        if (true != GetAddressFromPrivateKey(args[0], address)) {
          address.clear();
          result = "Error";
        }

        return strings2jobjectArray(env, {result, address});
      }
    },
    {
      "createrawtransaction", [env, &networkType](const std::vector<std::string>& args)->jobjectArray {
        assert(4 == args.size());

        std::string result("SUCCESS");
        std::string utx;
        const std::string& net_type = jstring2stdstring(env, networkType);
        if (true != produceUnsignedTx(args[0], args[1], args[2], args[3], utx)) {
          utx.clear();
          result = "Error";
        }

        return strings2jobjectArray(env, {result, utx});
      }
    },
    {
      "signrawtransaction", [env, &networkType](const std::vector<std::string>& args)->jobjectArray {
        assert(2 == args.size());

        std::string result("SUCCESS");
        std::string stx;
        const std::string& net_type = jstring2stdstring(env, networkType);
        if (true != signTransaction(args[0], args[1], net_type, stx)) {
          stx.clear();
          result = "Error";
        }

        return strings2jobjectArray(env, {result, stx});
      }
    }
  };

  try
  {

  }
  catch (const std::exception& e)
  {
      printf("execute except: %s\n", e.what());
      return 0;
  }
  const auto& command = jstring2stdstring(env, _command);
  const auto& cmd_vec = split_by_regex(command, "(\\S+)");
  if (cmd_vec.empty()) {
    return strings2jobjectArray(env, {"Error", "Invalid command"});
  }

  for (auto& h : command_handlers) {
    if (h.cmd_name != cmd_vec[0]) {
      continue;
    }

    return h.handler(std::vector<std::string>(cmd_vec.begin() + 1, cmd_vec.end()));
  }

  return strings2jobjectArray(env, {"Error", "Unknown command"});
}
