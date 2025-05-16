#include <cstring>
#include <sys/socket.h>
#ifdef _WIN32
    #include <WinSock2.h>
#else
    #include <netdb.h>
    #include <arpa/inet.h>
#endif
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
static std::optional<std::string> start_tcp()
{
    static int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
        return strerror(errno);
    sockaddr_in laddr, raddr;
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(9527);
    inet_pton(AF_INET, INADDR_ANY, &laddr.sin_addr.s_addr);
    if (bind(sd, reinterpret_cast<sockaddr*>(&laddr), sizeof laddr) < 0)
        return strerror(errno);
    return std::nullopt;
}
Napi::String start_network(const Napi::CallbackInfo &info)
{
    std::string ip = info[0].ToString().Utf8Value();
    uint16_t port = info[1].As<Napi::Number>().Int32Value();
    std::string type = info[2].ToString().Utf8Value();
    if (type.compare("tcp"))
        start_tcp();
    return Napi::String::New(info.Env(), ip.c_str());
}
Napi::Object init(Napi::Env env, Napi::Object exports)
{
    exports.Set("getIps", Napi::Function::New(env, get_ips));
    exports.Set("startNetwork", Napi::Function::New(env, start_network));
    return exports;
}
NODE_API_MODULE(addon, init)
