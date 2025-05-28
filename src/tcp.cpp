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

/*
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
            if (running)
                throw Napi::Error::New(env, strerror(errno));
            else
                break;
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
        shutdown(fd, SHUT_RDWR);
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
    netLink.Release();
    showInfo.Release();
}
*/
TcpServer::TcpServer(){}
void TcpServer::set_callbacks(Napi::ThreadSafeFunction &&netLink, Napi::ThreadSafeFunction &&showInfo) const
{
    this->netLink = std::move(netLink);
    this->showInfo = std::move(showInfo);
}
void TcpServer::start(std::string_view ip, unsigned short port, const Napi::Env &env) const
{
    if (this->fd == INVALID_SOCKET) {
        this->running = true;
        this->fd = socket(AF_INET, SOCK_STREAM, 0);
        if (this->fd == INVALID_SOCKET)
            throw Napi::Error::New(env, strerror(errno));

        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        inet_pton(AF_INET, ip.data(), &server.sin_addr.s_addr);
        if (bind(this->fd, reinterpret_cast<sockaddr*>(&server), sizeof server) < 0) {
#ifdef WIN32
            closesocket(this->fd);
#else
            close(this->fd);
#endif
            this->fd = INVALID_SOCKET;
            throw Napi::Error::New(env, strerror(errno));
        }
        if (listen(this->fd, 8) < 0) {
#ifdef WIN32
            closesocket(this->fd);
#else
            close(this->fd);
#endif
            this->fd = INVALID_SOCKET;
            throw Napi::Error::New(env, strerror(errno));
        }
        this->thr = std::thread(&TcpServer::run, this, std::cref(env), ip, port);
    } else {
        throw Napi::Error::New(env, "Server is already running");
    }
}
void TcpServer::stop() noexcept
{
    if (!this->running) return;
    this->running = false;
    if (this->fd == INVALID_SOCKET) {
#ifdef WIN32
        shutdown(this->fd, SD_BOTH);
        closesocket(this->fd);
#else
        shutdown(this->fd, SHUT_RDWR);
        close(this->fd);
#endif
    }
    for (const auto &client : this->clients) {
#ifdef WIN32
        shutdown(client.first, SD_BOTH);
        closesocket(client.first);
#else
        shutdown(client.first, SHUT_RDWR);
        close(client.first);
#endif
    }
    this->clients.clear();
    if (this->thr.joinable())
        this->thr.join();
}
bool TcpServer::send_client(const std::string_view ip, const std::string_view msg) const
{
    for (auto &[csd, ipstr] : this->clients) {
        if (ipstr.compare(ip) == 0)
            return send(csd, msg.data(), msg.length(), 0) != -1;
    }
    return false;
}
std::vector<std::string> TcpServer::get_clients() const
{
    std::vector<std::string> res;
    for (const auto &client : this->clients)
        res.push_back(client.second);
    return res;
}

void TcpServer::run(const Napi::Env &env, std::string_view ip, unsigned short port) const
{
    sockaddr_in client;
    while (this->running) {
        socklen_t clientLen = sizeof client;
        int csd = accept(this->fd, reinterpret_cast<sockaddr*>(&client), &clientLen);
        if (csd != INVALID_SOCKET) {
            char ipstr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client.sin_addr, ipstr, INET_ADDRSTRLEN);
            this->clients.emplace(csd, ipstr);
            unsigned short port = client.sin_port;
            if (this->netLink) {
                this->netLink.BlockingCall([&ipstr, port] (Napi::Env env, Napi::Function callback) {
                    callback.Call({Napi::Boolean::New(env, false), Napi::String::New(env, ipstr), Napi::Number::New(env, port)});
                });
            }
            std::thread(&TcpServer::recv_msg, this, csd, std::cref(env), ipstr, client.sin_port).detach();
        } else {
            if (this->running)
                throw Napi::Error::New(env, strerror(errno));
        }
    }
}
void TcpServer::recv_msg(int csd, const Napi::Env &env, char *ipstr, unsigned short port) const
{
    while (this->running) {
        char buf[BUFSIZ] = { '\0' };
        auto n = recv(csd, buf, sizeof buf, 0);
        if (n < 0) {
            if (this->running)
                throw Napi::Error::New(env, strerror(errno));
            else
                break;
        } else if (n == 0) {
            if (this->running) {
                this->clients.erase(csd);
                if (this->netLink) {
                    netLink.BlockingCall([&ipstr, port] (Napi::Env env, Napi::Function callback) {
                        callback.Call({Napi::Boolean::New(env, true), Napi::String::New(env, ipstr), Napi::Number::New(env, port)});
                    });
                }
            }
#ifdef WIN32
            closesocket(csd);
#else
            close(csd);
#endif
            break;
        } else {
            if (this->showInfo) {
                showInfo.BlockingCall([&ipstr, &port, buf] (Napi::Env env, Napi::Function callback) {
                    callback.Call({Napi::String::New(env, ipstr), Napi::Number::New(env, port), Napi::String::New(env, buf)});
                });
            }
        }
    }
}