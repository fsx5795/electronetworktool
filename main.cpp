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
    static int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return strerror(errno);
    sockaddr_in server, client;
    server.sin_family = AF_INET;
    server.sin_port = htons(9527);
    inet_pton(AF_INET, INADDR_ANY, &server.sin_addr.s_addr);
    if (bind(fd, reinterpret_cast<sockaddr*>(&server), sizeof server) < 0)
        return strerror(errno);
    if (listen(fd, 8) < 0)
        return strerror(errno);
    while (true) {
        socklen_t clientLen = sizeof client;
        int csd = accept(fd, reinterpret_cast<sockaddr*>(&client), &clientLen);
        if (csd >= 0) {
            char ipstr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client.sin_addr, ipstr, INET_ADDRSTRLEN);
            while (true) {
                char buf[BUFSIZ] = { '\0' };
                ssize_t n = recv(csd, buf, sizeof buf, 0);
                if (n <= 0) {
                    close(csd);
                } else {
                    close(csd);
                }
            }
        } else {
            continue;
        }
    }
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
