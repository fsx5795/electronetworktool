#ifdef WIN32
#include <WinSock2.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#endif
#include <iostream>
#include <napi.h>
Napi::String get_ips(const Napi::CallbackInfo &info)
{
    char buf[100] = { '\0' };
    std::string ips;
    if (gethostname(buf, sizeof(buf)) == 0) {
        struct hostent *he = gethostbyname(buf);
        if (he != nullptr) {
            for (int i{0}; he->h_addr_list[i] != nullptr; ++i) {
                char *localIp = inet_ntoa(*(struct in_addr*)(he->h_addr_list[i]));
                if (localIp != nullptr && strcmp("127.0.0.1", localIp) == 0)
                    continue;
                ips.append(localIp);
                ips.push_back(',');
            }
        }
    }
    if (ips.length() != 0)
        ips.erase(ips.length() - 1);
    return Napi::String::New(info.Env(), ips.c_str());
}
Napi::Object init(Napi::Env env, Napi::Object exports)
{
    exports.Set("getIps", Napi::Function::New(env, get_ips));
    return exports;
}
NODE_API_MODULE(addon, init)