#include "ftp.hpp"
#include <memory>
#include <thread>

namespace {
constexpr UINT complete=WM_APP+1;
constexpr int hostId=101,portId=102,userId=103,passId=104,connectId=105,refreshId=106,logId=107,listId=108,statusId=109;
HWND window=nullptr;
HFONT font=nullptr;
std::thread worker;
bool busy=false,closing=false;
HWND control(int id) { return GetDlgItem(window,id); }
std::wstring text(int id) { auto h=control(id); int n=GetWindowTextLengthW(h); std::wstring out(static_cast<size_t>(n)+1,L'\0'); GetWindowTextW(h,out.data(),n+1); out.resize(n); return out; }
void label(const wchar_t* name,int x,int y,int width) { auto h=CreateWindowW(L"STATIC",name,WS_CHILD|WS_VISIBLE,x,y,width,24,window,nullptr,nullptr,nullptr); SendMessageW(h,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE); }
void child(const wchar_t* cls,const wchar_t* name,int id,int x,int y,int width,int height,DWORD style) {
    auto h=CreateWindowExW(cls==std::wstring(L"EDIT")?WS_EX_CLIENTEDGE:0,cls,name,WS_CHILD|WS_VISIBLE|style,x,y,width,height,window,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),nullptr,nullptr);
    SendMessageW(h,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);
}
void start() {
    if(busy) return;
    auto password=text(passId);
    ftp::Request q{net::utf8(text(hostId)),net::utf8(text(portId)),net::utf8(text(userId)),net::utf8(password)};
    if(!password.empty()) SecureZeroMemory(password.data(),password.size()*sizeof(wchar_t));
    if(q.host.empty() || q.port.empty() || q.user.empty()) { SetWindowTextW(control(statusId),L"请填写 IP、端口和用户名。"); ftp::erase(q.password); return; }
    if(worker.joinable()) worker.join();
    busy=true;
    for(int id:{hostId,portId,userId,passId,connectId,refreshId}) EnableWindow(control(id),FALSE);
    SetWindowTextW(control(logId),L""); SetWindowTextW(control(listId),L"");
    SetWindowTextW(control(statusId),L"正在连接并读取根目录……可继续移动窗口；网络操作在后台进行。");
    try {
        worker=std::thread([q=std::move(q)]() mutable {
            auto result=std::make_unique<ftp::Result>(ftp::fetch(q));
            if(PostMessageW(window,complete,0,reinterpret_cast<LPARAM>(result.get()))) result.release();
        });
    } catch(const std::exception&) {
        ftp::erase(q.password); busy=false;
        for(int id:{hostId,portId,userId,passId,connectId,refreshId}) EnableWindow(control(id),TRUE);
        SetWindowTextW(control(statusId),L"无法创建网络工作线程，请重试。");
    }
}
LRESULT CALLBACK procedure(HWND h,UINT message,WPARAM wp,LPARAM lp) {
    switch(message) {
    case WM_CREATE:
        window=h; font=static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        label(L"服务器 IPv4",20,18,105); label(L"端口",235,18,50); label(L"用户名",330,18,80); label(L"密码",525,18,80);
        child(L"EDIT",L"127.0.0.1",hostId,20,45,195,27,WS_TABSTOP|ES_AUTOHSCROLL);
        child(L"EDIT",L"2121",portId,235,45,75,27,WS_TABSTOP|ES_AUTOHSCROLL|ES_NUMBER);
        child(L"EDIT",L"",userId,330,45,175,27,WS_TABSTOP|ES_AUTOHSCROLL);
        child(L"EDIT",L"",passId,525,45,215,27,WS_TABSTOP|ES_AUTOHSCROLL|ES_PASSWORD);
        SendMessageW(control(passId),EM_SETPASSWORDCHAR,L'*',0);
        for(int id:{hostId,portId,userId,passId}) SendMessageW(control(id),EM_SETLIMITTEXT,id==passId?256:128,0);
        child(L"BUTTON",L"连接并列目录",connectId,20,90,150,32,WS_TABSTOP|BS_PUSHBUTTON);
        child(L"BUTTON",L"重新连接 / 刷新",refreshId,185,90,160,32,WS_TABSTOP|BS_PUSHBUTTON);
        label(L"普通 FTP：用于本机课程测试。密码输入框掩码显示，命令记录隐藏 PASS 参数。",365,95,570);
        label(L"FTP 命令和服务器响应",20,137,430); label(L"服务器根目录文件列表",510,137,430);
        child(L"EDIT",L"",logId,20,165,465,375,ES_MULTILINE|ES_READONLY|WS_VSCROLL|WS_HSCROLL|ES_AUTOHSCROLL);
        child(L"EDIT",L"",listId,510,165,450,375,ES_MULTILINE|ES_READONLY|WS_VSCROLL|WS_HSCROLL|ES_AUTOHSCROLL);
        child(L"STATIC",L"等待连接。",statusId,20,558,940,45,0);
        return 0;
    case WM_COMMAND:
        if(LOWORD(wp)==connectId || LOWORD(wp)==refreshId) start();
        return 0;
    case complete: {
        std::unique_ptr<ftp::Result> result(reinterpret_cast<ftp::Result*>(lp));
        if(worker.joinable()) worker.join(); busy=false;
        SetWindowTextW(control(logId),net::wide(result->log).c_str());
        SetWindowTextW(control(listId),net::wide(result->listing).c_str());
        SetWindowTextW(control(statusId),net::wide(result->status).c_str());
        for(int id:{hostId,portId,userId,passId,connectId,refreshId}) EnableWindow(control(id),TRUE);
        if(closing) DestroyWindow(h);
        return 0;
    }
    case WM_CLOSE:
        if(busy) { closing=true; SetWindowTextW(control(statusId),L"等待当前网络操作结束后关闭（操作有超时限制）……"); }
        else DestroyWindow(h);
        return 0;
    case WM_DESTROY:
        SetWindowTextW(control(passId),L""); PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h,message,wp,lp);
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int show) {
    WNDCLASSW wc{}; wc.lpfnWndProc=procedure; wc.hInstance=instance; wc.lpszClassName=L"NetworkLabsFTP";
    wc.hCursor=LoadCursorW(nullptr,IDC_ARROW); wc.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
    if(!RegisterClassW(&wc)) return 1;
    auto h=CreateWindowW(wc.lpszClassName,L"实验三 — Winsock FTP 客户端",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,1000,660,nullptr,nullptr,instance,nullptr);
    if(!h) return 1;
    ShowWindow(h,show); UpdateWindow(h);
    MSG message{}; int rc;
    while((rc=GetMessageW(&message,nullptr,0,0))>0) {
        if(!IsDialogMessageW(h,&message)) { TranslateMessage(&message); DispatchMessageW(&message); }
    }
    if(worker.joinable()) worker.join();
    return rc<0?1:static_cast<int>(message.wParam);
}
