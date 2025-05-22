#include <cstring>
#ifdef _WIN32
	#define _WIN32_WINNT 0x0600
	#include <ws2tcpip.h>
    #include <WinSock2.h>
#else
    #include <unistd.h>
    #include <netdb.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif
#include <napi.h>
static Napi::ThreadSafeFunction showInfo;
static std::unordered_map<int, std::string> clients;
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
static void recv_msg(int csd, const Napi::Env &env, char *ipstr, USHORT port)
{
    while (true) {
        char buf[BUFSIZ] = { '\0' };
        auto n = recv(csd, buf, sizeof buf, 0);
        if (n < 0) {
            throw Napi::Error::New(env, strerror(errno));
        } else if (n == 0) {
        #ifdef _WIN32
            closesocket(csd);
        #else
            close(csd);
        #endif
            break;
        } else {
            showInfo.BlockingCall([&ipstr, &port, buf] (Napi::Env env, Napi::Function callback) {
                callback.Call({Napi::String::New(env, ipstr), Napi::Number::New(env, port), Napi::String::New(env, buf)});
            });
        }
    }
}
static void run_tcp(const Napi::Env &env, int fd, std::string_view ip, unsigned short port)
{
    sockaddr_in client;
    while (true) {
        socklen_t clientLen = sizeof client;
        int csd = accept(fd, reinterpret_cast<sockaddr*>(&client), &clientLen);
        if (csd >= 0) {
            char ipstr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client.sin_addr, ipstr, INET_ADDRSTRLEN);
            clients.emplace(csd, ipstr);
            //send(csd, "abc", 4, 0);
            std::thread(recv_msg, csd, std::cref(env), ipstr, client.sin_port).detach();
           //}
        } else {
            //continue;
        }
    }
}
static void start_tcp(std::string_view ip, unsigned short port, const Napi::Env &env)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw Napi::Error::New(env, strerror(errno));
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip.data(), &server.sin_addr.s_addr);
    if (bind(fd, reinterpret_cast<sockaddr*>(&server), sizeof server) < 0)
        throw Napi::Error::New(env, strerror(errno));
    if (listen(fd, 8) < 0)
        throw Napi::Error::New(env, strerror(errno));
    std::thread(run_tcp, env, fd, ip, port).detach();
}
Napi::Value send_client(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 3 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsString())
        throw Napi::TypeError::New(env, "arg type is error!");
    for (auto &[csd, ipstr] : clients) {
        if (ipstr.compare(info[0].ToString().Utf8Value()) == 0) {
            std::string msg = info[2].ToString().Utf8Value();
            send(csd, msg.c_str(), msg.length(), 0);
        }
    }
    return env.Null();
}
Napi::Value start_network(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() < 4 || !info[0].IsString() || !info[1].IsNumber() || !info[2].IsString() || !info[3].IsFunction())
        throw Napi::TypeError::New(env, "arg type is error!");
    std::string ip = info[0].ToString().Utf8Value();
    uint16_t port = info[1].As<Napi::Number>().Int32Value();
    std::string type = info[2].ToString().Utf8Value();
    showInfo = Napi::ThreadSafeFunction::New(env, info[3].As<Napi::Function>(), "showInfo", 0, 1);
    if (type.compare("tcp") == 0)
        start_tcp(ip, port, env);
    return env.Null();
}
Napi::Object init(Napi::Env env, Napi::Object exports)
{
    exports.Set("getIps", Napi::Function::New(env, get_ips));
    exports.Set("startNetwork", Napi::Function::New(env, start_network));
    exports.Set("sendClient", Napi::Function::New(env, send_client));
    return exports;
}
NODE_API_MODULE(addon, init)
