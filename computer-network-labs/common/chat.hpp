#pragma once
#include "net.hpp"
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

namespace chat {
inline std::mutex output;
inline void log(const std::string& s) { std::lock_guard<std::mutex> lock(output); std::cout << s << std::endl; }
// Both terminal input and redirected pipes are polled, so peer disconnect never
// leaves the main thread blocked in getline. The receiver thread is always joined.
inline void session(SOCKET socket) {
    std::atomic<bool> done=false;
    net::timeout(socket,200);
    std::thread receiver([&] {
        try {
            std::string pending; char buf[4096];
            while(!done) {
                int n=recv(socket,buf,sizeof(buf),0);
                if(n==0) { if(!pending.empty()) log("RECV: "+pending); log("PEER CLOSED"); break; }
                if(n<0) { if(WSAGetLastError()==WSAETIMEDOUT) continue; throw net::error("recv"); }
                pending.append(buf,n);
                size_t pos;
                while((pos=pending.find('\n'))!=std::string::npos) { log("RECV: "+pending.substr(0,pos)); pending.erase(0,pos+1); }
                if(pending.size()>1024*1024) throw std::runtime_error("Incoming message too large");
            }
        } catch(const std::exception& e) { log(std::string("ERROR: ")+e.what()); }
        done=true;
    });
    HANDLE input=GetStdHandle(STD_INPUT_HANDLE); DWORD consoleMode=0;
    bool console=GetConsoleMode(input,&consoleMode)!=0;
    std::wstring consoleLine; std::string line;
    log("CONNECTED; type messages or /quit");
    try {
        bool quitting=false;
        auto deliver=[&](const std::string& message) {
            if(message=="/quit") { log("LOCAL QUIT"); quitting=true; return; }
            net::send_all(socket,message+"\n"); log("SENT: "+message);
        };
        while(!done && !quitting) {
            if(console) {
                if(WaitForSingleObject(input,50)!=WAIT_OBJECT_0) continue;
                INPUT_RECORD record{}; DWORD read=0;
                if(!ReadConsoleInputW(input,&record,1,&read)) throw std::runtime_error("Console read failed");
                if(record.EventType!=KEY_EVENT || !record.Event.KeyEvent.bKeyDown) continue;
                auto key=record.Event.KeyEvent; wchar_t ch=key.uChar.UnicodeChar;
                if(key.wVirtualKeyCode==VK_RETURN) { log(""); deliver(net::utf8(consoleLine)); consoleLine.clear(); }
                else if(key.wVirtualKeyCode==VK_BACK) {
                    if(!consoleLine.empty()) { consoleLine.pop_back(); std::lock_guard<std::mutex> lock(output); std::cout << "\b \b" << std::flush; }
                }
                else if(ch>=32) {
                    if(consoleLine.size()+key.wRepeatCount>65536) throw std::runtime_error("Message too long");
                    consoleLine.append(key.wRepeatCount,ch);
                    std::lock_guard<std::mutex> lock(output); std::cout << net::utf8(std::wstring(key.wRepeatCount,ch)) << std::flush;
                }
            } else {
                DWORD available=0;
                if(GetFileType(input)==FILE_TYPE_PIPE && !PeekNamedPipe(input,nullptr,0,nullptr,&available,nullptr)) { quitting=true; break; }
                if(GetFileType(input)==FILE_TYPE_PIPE && !available) { Sleep(20); continue; }
                char ch; DWORD read=0;
                if(!ReadFile(input,&ch,1,&read,nullptr) || !read) { if(!line.empty()) deliver(line); quitting=true; break; }
                if(ch=='\n') { deliver(line); line.clear(); }
                else if(ch!='\r') { if(line.size()>=1024*1024) throw std::runtime_error("Message too long"); line+=ch; }
            }
        }
        if(quitting && !done) {
            shutdown(socket,SD_SEND);
            for(int i=0;i<100 && !done;++i) Sleep(20);
        }
    } catch(const std::exception& e) { log(std::string("ERROR: ")+e.what()); }
    done=true; shutdown(socket,SD_BOTH); receiver.join(); log("SESSION CLOSED");
}
inline int run(bool server,int argc,char** argv,unsigned short defaultPort,const char* lab) {
    try {
        SetConsoleOutputCP(CP_UTF8); net::Winsock wsa;
        log(std::string(lab)+(server?" SERVER":" CLIENT"));
        if(server) {
            if(argc>2) throw std::runtime_error("Usage: server [port]");
            auto p=argc>1?net::port(argv[1]):defaultPort;
            auto listener=net::create(); auto addr=net::address("127.0.0.1",p);
            BOOL exclusive=TRUE;
            if(setsockopt(listener.get(),SOL_SOCKET,SO_EXCLUSIVEADDRUSE,reinterpret_cast<const char*>(&exclusive),sizeof(exclusive))) throw net::error("exclusive address");
            if(bind(listener.get(),reinterpret_cast<sockaddr*>(&addr),sizeof(addr))) throw net::error("bind");
            if(listen(listener.get(),1)) throw net::error("listen");
            log("LISTENING 127.0.0.1:"+std::to_string(p));
            net::Socket peer(accept(listener.get(),nullptr,nullptr));
            if(peer.get()==INVALID_SOCKET) throw net::error("accept");
            session(peer.get());
        } else {
            if(argc>3) throw std::runtime_error("Usage: client [IPv4] [port]");
            auto peer=net::connect(argc>1?argv[1]:"127.0.0.1",argc>2?net::port(argv[2]):defaultPort);
            session(peer.get());
        }
        return 0;
    } catch(const std::exception& e) { log(std::string("ERROR: ")+e.what()); return 1; }
}
}
