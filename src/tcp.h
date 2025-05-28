#pragma once
#include <napi.h>

class TcpServer
{
public:
    TcpServer();
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;
    TcpServer(const TcpServer&&) = delete;
    TcpServer& operator=(const TcpServer&&) = delete;

    void set_callbacks(Napi::ThreadSafeFunction&&, Napi::ThreadSafeFunction&&) const;
    void start(std::string_view, unsigned short, const Napi::Env&) const;
    void stop() noexcept;
    bool send_client(const std::string_view, const std::string_view) const;
    std::vector<std::string> get_clients() const;

private:
    void run(const Napi::Env&, std::string_view, unsigned short) const;
    void recv_msg(int, const Napi::Env&, char*, unsigned short) const;

#ifdef WIN32
    using SocketType = SOCKET;
#else
    using SocketType = int;
    using INVALID_SOCKET = -1;
#endif

    mutable std::atomic<bool> running{false};
    mutable std::unordered_map<SocketType, std::string> clients;
    mutable Napi::ThreadSafeFunction netLink, showInfo;
    mutable std::thread thr;
    mutable SocketType fd{INVALID_SOCKET};
};
/*
extern std::unordered_map<int, std::string> clients;
extern Napi::ThreadSafeFunction netLink, showInfo;
extern void start_tcp(std::string_view, unsigned short, const Napi::Env&);
extern void stop_tcp();
*/