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

static std::string jstring2stdstring(JNIEnv* env, const jstring& javaString)
{
    const char *nativeString = env->GetStringUTFChars(javaString, JNI_FALSE);
    std::string res(nativeString);
    env->ReleaseStringUTFChars(javaString, nativeString);
    return res;
}

//
//static std::string jstring2stdstring2(JNIEnv* env, const jstring& jstr)
//{
//  jclass clsstring = env->FindClass("java/lang/String");
//  jstring strencode = env->NewStringUTF("utf-8");
//  jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
//  jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
//  jsize alen = env->GetArrayLength(barr);
//  jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
//
//  std::string res((const char*)ba, alen);
//  env->ReleaseByteArrayElements(barr, ba, 0);
//  return res;
//}

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

extern bool enable_port_mapping;

JNIEXPORT jobjectArray JNICALL
Java_com_okcoin_vault_jni_xrb_Xrbj_execute(JNIEnv *env, jclass, jstring networkType, jstring _command)
{
    if (mutex.try_lock() == false) {
        return strings2jobjectArray(env, {"Error", "JNI_LOCKED"});
    }
    AutoUnlock au;

    const auto& command = jstring2stdstring(env, _command);
    const auto& cmd_vec = split_by_regex(command, "(\\S+)");

    std::string result("Error");
    std::string context("Unknown command");

    if (cmd_vec.empty()) {

        context = "Invalid command";

    } else if (cmd_vec[0] == "getaddressbyprivatekey") {

        if (2 != cmd_vec.size()) {
            context = "Invalid command";
        } else {
            if (GetAddressFromPrivateKey(cmd_vec[1], context)) {
                result = "SUCCESS";
            }
        }
    } else if (cmd_vec[0] == "createrawtransaction") {

        if(5 != cmd_vec.size()) {
            context = "Invalid command";
        } else {

            if (produceUnsignedTx(cmd_vec[1], cmd_vec[2], cmd_vec[3], cmd_vec[4], context)) {
                result = "SUCCESS";
            }
        }
    } else if (cmd_vec[0] == "signrawtransaction") {

        if (3 != cmd_vec.size() && 4 != cmd_vec.size()) {
            context = "Invalid command";
        }
        else
        {
            const std::string& net_type = jstring2stdstring(env, networkType);

            enable_port_mapping = false;
            if (cmd_vec.size() == 4 && cmd_vec[3] == "nospeedup") {
                enable_port_mapping = true;
            }
            if (signTransaction(cmd_vec[1], cmd_vec[2], net_type, context)) {
                result = "SUCCESS";
            }
        }
    }

    return strings2jobjectArray(env, {result, context});
}
