#include <cstring>
#ifdef WIN32
	#define _WIN32_WINNT 0x0600
	#include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#endif
#include <thread>
#include "tcp.h"

static bool running{true};
std::unordered_map<int, std::string> clients;
Napi::ThreadSafeFunction netLink, showInfo;

#ifdef WIN32
static SOCKET fd = INVALID_SOCKET;
#else
static int fd = -1;
#endif

static void recv_msg(int csd, const Napi::Env &env, char *ipstr, unsigned short port)
{
    while (running) {
        char buf[BUFSIZ] = { '\0' };
        auto n = recv(csd, buf, sizeof buf, 0);
        if (n < 0) {
            throw Napi::Error::New(env, strerror(errno));
        } else if (n == 0) {
            clients.erase(csd);
            netLink.BlockingCall([&ipstr, port] (Napi::Env env, Napi::Function callback) {
                callback.Call({Napi::Boolean::New(env, true), Napi::String::New(env, ipstr), Napi::Number::New(env, port)});
            });
#ifdef WIN32
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
static void run_tcp(const Napi::Env &env, std::string_view ip, unsigned short port)
{
    sockaddr_in client;
    while (running) {
        socklen_t clientLen = sizeof client;
        int csd = accept(fd, reinterpret_cast<sockaddr*>(&client), &clientLen);
        if (csd >= 0) {
            char ipstr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client.sin_addr, ipstr, INET_ADDRSTRLEN);
            clients.emplace(csd, ipstr);
            unsigned short port = client.sin_port;
            netLink.BlockingCall([&ipstr, port] (Napi::Env env, Napi::Function callback) {
                callback.Call({Napi::Boolean::New(env, false), Napi::String::New(env, ipstr), Napi::Number::New(env, port)});
            });
            std::thread(recv_msg, csd, std::cref(env), ipstr, client.sin_port).detach();
        }
    }
}
void start_tcp(std::string_view ip, unsigned short port, const Napi::Env &env)
{
    if (fd < 0) {
        running = true;
        fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
            throw Napi::Error::New(env, strerror(errno));
        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        inet_pton(AF_INET, ip.data(), &server.sin_addr.s_addr);
        if (bind(fd, reinterpret_cast<sockaddr*>(&server), sizeof server) < 0) {
#ifdef WIN32
            closesocket(fd);
#else
            close(fd);
#endif
            throw Napi::Error::New(env, strerror(errno));
        }
        if (listen(fd, 8) < 0) {
#ifdef WIN32
            closesocket(fd);
#else
            close(fd);
#endif
            throw Napi::Error::New(env, strerror(errno));
        }
        std::thread(run_tcp, env, ip, port).detach();
    }
}
void stop_tcp()
{
    running = false;
    if (fd >= 0) {
#ifdef WIN32
        shutdown(fd, SD_BOTH);
        closesocket(fd);
#else
        shutdown(fd, SHUT_SHUT_RDWR);
        close(fd);
#endif
    }
    for (const auto &pair : clients) {
#ifdef WIN32
        closesocket(pair.first);
#else
        close(pair.first);
#endif
    }
    clients.clear();
}