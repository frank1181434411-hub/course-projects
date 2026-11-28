#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace net {
inline std::runtime_error error(const char* op) {
    return std::runtime_error(std::string(op) + " failed (Winsock " + std::to_string(WSAGetLastError()) + ")");
}
struct Winsock {
    Winsock() { WSADATA data{}; int rc = WSAStartup(MAKEWORD(2,2), &data); if(rc) throw std::runtime_error("WSAStartup: " + std::to_string(rc)); }
    ~Winsock() { WSACleanup(); }
    Winsock(const Winsock&) = delete;
    Winsock& operator=(const Winsock&) = delete;
};
class Socket {
    SOCKET value_ = INVALID_SOCKET;
public:
    explicit Socket(SOCKET value = INVALID_SOCKET) : value_(value) {}
    ~Socket() { if(value_ != INVALID_SOCKET) closesocket(value_); }
    Socket(Socket&& other) noexcept : value_(std::exchange(other.value_, INVALID_SOCKET)) {}
    Socket& operator=(Socket&& other) noexcept { if(this != &other) { if(value_ != INVALID_SOCKET) closesocket(value_); value_ = std::exchange(other.value_, INVALID_SOCKET); } return *this; }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    SOCKET get() const { return value_; }
};
inline unsigned short port(const std::string& text) {
    size_t end = 0; unsigned long n = std::stoul(text, &end);
    if(end != text.size() || n == 0 || n > 65535) throw std::runtime_error("Port must be 1..65535");
    return static_cast<unsigned short>(n);
}
inline sockaddr_in address(const std::string& ip, unsigned short p) {
    sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_port=htons(p);
    if(inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) throw std::runtime_error("Enter a numeric IPv4 address");
    return addr;
}
inline Socket create() { Socket s(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)); if(s.get()==INVALID_SOCKET) throw error("socket"); return s; }
inline void timeout(SOCKET s, DWORD ms) {
    if(setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&ms),sizeof(ms)) || setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<const char*>(&ms),sizeof(ms))) throw error("setsockopt");
}
inline Socket connect(const std::string& ip, unsigned short p, long seconds = 3) {
    auto s=create(); auto addr=address(ip,p); u_long mode=1;
    if(ioctlsocket(s.get(),FIONBIO,&mode)) throw error("nonblocking");
    if(::connect(s.get(),reinterpret_cast<sockaddr*>(&addr),sizeof(addr))==SOCKET_ERROR) {
        if(WSAGetLastError()!=WSAEWOULDBLOCK) throw error("connect");
        fd_set write,except; FD_ZERO(&write); FD_ZERO(&except); FD_SET(s.get(),&write); FD_SET(s.get(),&except);
        timeval wait{seconds,0}; int rc=select(0,nullptr,&write,&except,&wait);
        if(rc==0) throw std::runtime_error("Connection timed out");
        if(rc==SOCKET_ERROR) throw error("select");
        int e=0; int len=sizeof(e);
        if(getsockopt(s.get(),SOL_SOCKET,SO_ERROR,reinterpret_cast<char*>(&e),&len)) throw error("getsockopt");
        if(e) throw std::runtime_error("Connection failed (Winsock " + std::to_string(e) + ")");
    }
    mode=0; if(ioctlsocket(s.get(),FIONBIO,&mode)) throw error("blocking");
    timeout(s.get(),3000); return s;
}
inline void send_all(SOCKET s,const std::string& text) {
    size_t offset=0;
    while(offset<text.size()) {
        int n=send(s,text.data()+offset,static_cast<int>(std::min<size_t>(text.size()-offset,16384)),0);
        if(n<=0) throw error("send"); offset+=n;
    }
}
inline std::string utf8(const std::wstring& text) {
    if(text.empty()) return {};
    int n=WideCharToMultiByte(CP_UTF8,0,text.data(),static_cast<int>(text.size()),nullptr,0,nullptr,nullptr);
    std::string out(n,'\0'); WideCharToMultiByte(CP_UTF8,0,text.data(),static_cast<int>(text.size()),out.data(),n,nullptr,nullptr); return out;
}
inline std::wstring wide(const std::string& text) {
    if(text.empty()) return {};
    int n=MultiByteToWideChar(CP_UTF8,0,text.data(),static_cast<int>(text.size()),nullptr,0);
    std::wstring out(n,L'\0'); MultiByteToWideChar(CP_UTF8,0,text.data(),static_cast<int>(text.size()),out.data(),n); return out;
}
}
