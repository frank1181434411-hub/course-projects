#pragma once
#include "net.hpp"
#include <chrono>
#include <cstdio>
#include <functional>

namespace ftp {
struct Request { std::string host, port, user, password; };
struct Result { std::string log, listing, status; bool success=false; };
inline void erase(std::string& secret) { if(!secret.empty()) SecureZeroMemory(secret.data(),secret.size()); secret.clear(); }
class Client {
    net::Socket control;
    std::string pending;
    Result& result;
    const std::string& password;
    const std::chrono::steady_clock::time_point deadline=std::chrono::steady_clock::now()+std::chrono::seconds(20);
    void check_time() { if(std::chrono::steady_clock::now()>deadline) throw std::runtime_error("FTP operation exceeded 20 seconds"); }
    void log(std::string text) {
        // Even a server that echoes the password must not disclose it in the UI.
        if(!password.empty()) {
            size_t at=0;
            while((at=text.find(password,at))!=std::string::npos) { text.replace(at,password.size(),"[REDACTED]"); at+=10; }
        }
        if(result.log.size()+text.size()>262144) throw std::runtime_error("FTP transcript too large");
        result.log+=text+"\r\n";
    }
    std::string line() {
        while(true) {
            check_time();
            auto at=pending.find("\r\n");
            if(at!=std::string::npos) { auto text=pending.substr(0,at); pending.erase(0,at+2); return text; }
            if(pending.size()>8192) throw std::runtime_error("FTP reply line too long");
            char buf[2048]; int n=recv(control.get(),buf,sizeof(buf),0);
            if(n==0) throw std::runtime_error("FTP control connection closed unexpectedly");
            if(n<0) throw net::error("FTP receive");
            pending.append(buf,n);
        }
    }
    int reply(std::string* full=nullptr) {
        std::string first=line(); log("S: "+first);
        if(first.size()<4 || first[0]<'1' || first[0]>'5' || first[1]<'0' || first[1]>'9' || first[2]<'0' || first[2]>'9' || (first[3]!=' ' && first[3]!='-')) throw std::runtime_error("Malformed FTP reply");
        std::string text=first; int code=std::stoi(first.substr(0,3));
        if(first[3]=='-') {
            for(size_t i=0;;++i) {
                if(i>128) throw std::runtime_error("Too many FTP reply lines");
                auto next=line(); log("S: "+next); text+='\n'+next;
                if(next.rfind(first.substr(0,3)+" ",0)==0) break;
            }
        }
        if(full) *full=std::move(text); return code;
    }
    int command(const std::string& cmd,std::string* full=nullptr,bool secret=false) {
        check_time(); log(secret?"C: PASS [REDACTED]":"C: "+cmd);
        net::send_all(control.get(),cmd+"\r\n"); return reply(full);
    }
    void expect(int actual,int required,const char* operation) { if(actual!=required) throw std::runtime_error(std::string(operation)+" rejected (FTP "+std::to_string(actual)+")"); }
public:
    Client(Result& r,const std::string& pass):result(r),password(pass) {}
    void run(const Request& q) {
        for(const auto* field:{&q.host,&q.port,&q.user,&q.password}) if(field->find_first_of("\r\n")!=std::string::npos) throw std::runtime_error("Fields must not contain newlines");
        control=net::connect(q.host,net::port(q.port)); net::timeout(control.get(),5000); expect(reply(),220,"Greeting");
        int login=command("USER "+q.user);
        if(login==331) {
            std::string passCommand="PASS "+q.password;
            // Erase the temporary wire command even if send/reply throws.
            try { login=command(passCommand,nullptr,true); } catch(...) { erase(passCommand); throw; }
            erase(passCommand);
        }
        expect(login,230,"Login");
        expect(command("CWD /"),250,"Root directory");
        expect(command("TYPE A"),200,"ASCII transfer");
        std::string response; int code=command("EPSV",&response); unsigned short dataPort=0;
        if(code==229) {
            auto open=response.find('('),close=response.find(')',open);
            if(open==std::string::npos || close==std::string::npos) throw std::runtime_error("Malformed EPSV response");
            auto body=response.substr(open+1,close-open-1);
            if(body.size()<5 || body[0]!=body[1] || body[0]!=body[2] || body.back()!=body[0]) throw std::runtime_error("Malformed EPSV port");
            dataPort=net::port(body.substr(3,body.size()-4));
        } else if(code==500 || code==502 || code==522) {
            expect(command("PASV",&response),227,"Passive mode");
            auto open=response.find('('); int a,b,c,d,hi,lo;
            if(open==std::string::npos || sscanf_s(response.c_str()+open+1,"%d,%d,%d,%d,%d,%d",&a,&b,&c,&d,&hi,&lo)!=6 || a<0 || a>255 || b<0 || b>255 || c<0 || c>255 || d<0 || d>255 || hi<0 || hi>255 || lo<0 || lo>255 || hi*256+lo==0) throw std::runtime_error("Malformed PASV response");
            dataPort=static_cast<unsigned short>(hi*256+lo);
        } else throw std::runtime_error("Passive mode rejected (FTP "+std::to_string(code)+")");
        auto data=net::connect(q.host,dataPort); // Never trust a foreign PASV address.
        code=command("LIST"); if(code!=125 && code!=150) throw std::runtime_error("LIST rejected (FTP "+std::to_string(code)+")");
        std::string listing; char buf[4096];
        while(true) {
            check_time(); int n=recv(data.get(),buf,sizeof(buf),0);
            if(n==0) break; if(n<0) throw net::error("FTP data receive");
            listing.append(buf,n); if(listing.size()>1024*1024) throw std::runtime_error("Directory listing too large");
        }
        expect(reply(),226,"Directory transfer");
        expect(command("QUIT"),221,"Quit");
        // Sanitize the listing too: it is untrusted server data displayed in UI.
        if(!password.empty()) { size_t at=0; while((at=listing.find(password,at))!=std::string::npos) { listing.replace(at,password.size(),"[REDACTED]"); at+=10; } }
        result.listing=std::move(listing); result.success=true;
        result.status="登录成功；根目录列表已刷新，FTP 连接已正常关闭。";
    }
};
inline Result fetch(Request& q) {
    Result result;
    try { net::Winsock wsa; Client client(result,q.password); client.run(q); }
    catch(const std::exception& e) { result.status=std::string("连接或 FTP 操作失败：")+e.what(); result.log+="ERROR: "+result.status+"\r\n"; }
    erase(q.password); return result;
}
}
