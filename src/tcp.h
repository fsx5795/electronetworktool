#pragma once
#include <napi.h>

extern std::unordered_map<int, std::string> clients;
extern Napi::ThreadSafeFunction connect, showInfo;
extern void start_tcp(std::string_view, unsigned short, const Napi::Env&);
