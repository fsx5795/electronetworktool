#include <string>
#ifdef _WIN32
    #include <WinSock2.h>
#else
    #include <unistd.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#endif
#include "tcp.h"
static Napi::Value set_callback(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsFunction() || !info[1].IsFunction())
        throw Napi::TypeError::New(env, "arg type is error!");
    netLink = Napi::ThreadSafeFunction::New(env, info[0].As<Napi::Function>(), "netLinked", 0, 1);
    showInfo = Napi::ThreadSafeFunction::New(env, info[1].As<Napi::Function>(), "showInfo", 0, 1);
    return env.Null();
}
static Napi::Array get_ips(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    char buf[20] = { '\0' };
    std::vector<std::string> ips;
    if (gethostname(buf, sizeof(buf)) == 0) {
        struct hostent *he = gethostbyname(buf);
        if (he != nullptr) {
            for (int i{0}; he->h_addr_list[i] != nullptr; ++i) {
                char *localIp = inet_ntoa(*(struct in_addr*)(he->h_addr_list[i]));
                if (localIp != nullptr && strcmp("127.0.0.1", localIp) == 0)
                    continue;
                ips.push_back(localIp);
            }
        }
    }
    Napi::Array result = Napi::Array::New(env, ips.size());
    for (auto i{0}; i < ips.size(); ++i)
        result[i] = Napi::String::New(env, ips.at(i));
    return result;
}
static Napi::Boolean send_client(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsString())
        throw Napi::TypeError::New(env, "arg type is error!");
    for (auto &[csd, ipstr] : clients) {
        if (ipstr.compare(info[0].ToString().Utf8Value()) == 0) {
            std::string msg = info[2].ToString().Utf8Value();
            return Napi::Boolean::New(env, send(csd, msg.c_str(), msg.length(), 0) != -1);
        }
    }
    return Napi::Boolean(env, false);
}
static Napi::Value start_network(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsString())
        throw Napi::TypeError::New(env, "arg type is error!");
    std::string ip = info[0].ToString().Utf8Value();
    uint16_t port = info[1].As<Napi::Number>().Int32Value();
    std::string type = info[2].ToString().Utf8Value();
    if (type.compare("tcp") == 0)
        start_tcp(ip, port, env);
    return env.Null();
}
static Napi::Object init(Napi::Env env, Napi::Object exports)
{
    exports.Set("setCallback", Napi::Function::New(env, set_callback));
    exports.Set("getIps", Napi::Function::New(env, get_ips));
    exports.Set("startNetwork", Napi::Function::New(env, start_network));
    exports.Set("sendClient", Napi::Function::New(env, send_client));
    return exports;
}
NODE_API_MODULE(addon, init)
