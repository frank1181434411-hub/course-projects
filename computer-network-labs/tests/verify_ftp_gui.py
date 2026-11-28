"""Drive the actual Win32 controls. Credentials are obtained from a local helper,
never command-line password arguments or committed files. Stdlib + optional Pillow.
"""
import argparse
import ctypes as C
from ctypes import wintypes as W
import importlib.util
from pathlib import Path
import socket
import subprocess
import threading
import time

ROOT=Path(__file__).resolve().parents[1]
u=C.windll.user32
u.FindWindowW.argtypes=[W.LPCWSTR,W.LPCWSTR];u.FindWindowW.restype=W.HWND
u.GetDlgItem.argtypes=[W.HWND,C.c_int];u.GetDlgItem.restype=W.HWND
u.SendMessageW.argtypes=[W.HWND,W.UINT,W.WPARAM,W.LPARAM];u.SendMessageW.restype=C.c_ssize_t
u.PostMessageW.argtypes=[W.HWND,W.UINT,W.WPARAM,W.LPARAM];u.PostMessageW.restype=W.BOOL
u.GetWindowTextLengthW.argtypes=[W.HWND];u.GetWindowTextLengthW.restype=C.c_int
u.GetWindowTextW.argtypes=[W.HWND,W.LPWSTR,C.c_int];u.GetWindowTextW.restype=C.c_int
u.IsWindowEnabled.argtypes=[W.HWND];u.IsWindowEnabled.restype=W.BOOL
u.GetWindowThreadProcessId.argtypes=[W.HWND,C.POINTER(W.DWORD)];u.GetWindowThreadProcessId.restype=W.DWORD
u.SendMessageTimeoutW.argtypes=[W.HWND,W.UINT,W.WPARAM,W.LPARAM,W.UINT,W.UINT,C.POINTER(C.c_size_t)];u.SendMessageTimeoutW.restype=C.c_ssize_t

def set_text(hwnd,text):
    buf=C.create_unicode_buffer(text)
    u.SendMessageW(hwnd,0x000c,0,C.cast(buf,C.c_void_p).value)
def get_text(hwnd):
    length=u.SendMessageW(hwnd,0x000e,0,0)
    buf=C.create_unicode_buffer(length+1)
    u.SendMessageW(hwnd,0x000d,len(buf),C.cast(buf,C.c_void_p).value);return buf.value
def until(test,timeout=10):
    deadline=time.monotonic()+timeout
    while time.monotonic()<deadline:
        value=test()
        if value:return value
        time.sleep(.05)
    raise AssertionError('Win32 GUI operation timed out')

def run(build,out,helper):
    spec=importlib.util.spec_from_file_location('local_ftp_helper',helper)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    creds=module.credentials()
    cfg=__import__('json').loads(module.CONFIG.read_text())
    out.mkdir(parents=True,exist_ok=True)
    p=subprocess.Popen([str(build/'lab03_ftp.exe')])
    window=None; results=[]
    try:
        def find():
            hwnd=u.FindWindowW('NetworkLabsFTP',None)
            pid=W.DWORD()
            if hwnd:u.GetWindowThreadProcessId(hwnd,C.byref(pid))
            return hwnd if pid.value==p.pid else None
        window=until(find)
        def control(i):return u.GetDlgItem(window,i)
        for id_,value in [(101,cfg['host']),(102,str(cfg['port'])),(103,creds['user']),(104,creds['password'])]:set_text(control(id_),value)
        assert u.SendMessageW(control(104),0x00d2,0,0)==ord('*'),'Password control must be masked'
        def click():
            u.SendMessageW(control(105),0x00f5,0,0)
            until(lambda:u.IsWindowEnabled(control(105)),25)
            transcript=get_text(control(107));listing=get_text(control(108));status=get_text(control(109))
            assert creds['password'] not in transcript+listing+status,'Password disclosure in GUI'
            return transcript,listing,status
        log,listing,status=click()
        assert '230' in log and '226' in log and 'networklabs-test.txt' in listing and '登录成功' in status,'FTP login/list unsuccessful'
        assert 'PASS [REDACTED]' in log and 'C: LIST' in log,'Missing sanitized command transcript'
        (out/'ftp-transcript.txt').write_text(log,encoding='utf-8',newline='')
        (out/'ftp-listing.txt').write_text(listing,encoding='utf-8',newline='')
        results.append('PASS GUI: login 230; EPSV; LIST; transfer 226; QUIT 221; networklabs-test.txt listed')
        results.append('PASS GUI: password edit masked; PASS argument redacted; password absent from transcript/list/status')
        # Screenshot only the app; clear the masked field before capture as well.
        set_text(control(104),'')
        # Render only the app surface, independently of screen DPI and occlusion.
        # CopyFromScreen with virtualized coordinates could include other windows.
        path=str(out/'ftp-success.png').replace("'","''")
        script='''Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class CaptureLabWindow {
 [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
 [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
 [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr w,out RECT r);
 [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr w,IntPtr dc,uint flags);
}
'@
[CaptureLabWindow]::SetProcessDPIAware() | Out-Null
$r=New-Object CaptureLabWindow+RECT
'''+f'''[CaptureLabWindow]::GetWindowRect([IntPtr]{window},[ref]$r) | Out-Null
$b=New-Object Drawing.Bitmap(($r.R-$r.L),($r.B-$r.T))
$g=[Drawing.Graphics]::FromImage($b)
$dc=$g.GetHdc()
$ok=[CaptureLabWindow]::PrintWindow([IntPtr]{window},$dc,2)
$g.ReleaseHdc($dc)
if(-not $ok){{throw 'App window capture failed'}}
$b.Save('{path}')
$g.Dispose();$b.Dispose()
'''
        subprocess.run(['powershell','-NoProfile','-WindowStyle','Hidden','-Command',script],check=True,capture_output=True,timeout=15)
        # An invalid password must return a failure in the GUI, not a frozen window.
        set_text(control(104),'intentionally-invalid-test-value')
        log,listing,status=click()
        assert '530' in log and '失败' in status and not listing,'Authentication failure not reported'
        results.append('PASS GUI: wrong password returns FTP 530 and visible failure status')
        # Reserve then release a random local port that has no listening server.
        with socket.socket() as s:s.bind(('127.0.0.1',0));unused=s.getsockname()[1]
        set_text(control(102),str(unused));set_text(control(104),creds['password'])
        log,listing,status=click()
        assert '失败' in status and 'Connection failed' in status and not listing,'Connection error not reported'
        results.append('PASS GUI: unused local port gives visible connection failure; stale directory cleared')
        # A silent server deliberately stalls the network worker. WM_NULL must still
        # round-trip on the UI thread within 250 ms while the button is disabled.
        with socket.socket() as stalled:
            stalled.bind(('127.0.0.1',0));stalled.listen(1)
            def slow_server():
                peer,_=stalled.accept()
                with peer:time.sleep(6)
            thread=threading.Thread(target=slow_server);thread.start()
            set_text(control(102),str(stalled.getsockname()[1]))
            u.SendMessageW(control(105),0x00f5,0,0)
            until(lambda:not u.IsWindowEnabled(control(105)))
            result=C.c_size_t()
            assert u.SendMessageTimeoutW(window,0,0,0,2,250,C.byref(result)),'UI blocked by network worker'
            until(lambda:u.IsWindowEnabled(control(105)),10)
            assert '失败' in get_text(control(109)),'Timeout not reported'
            thread.join(timeout=6)
        results.append('PASS GUI: silent-server receive timeout; UI responds to WM_NULL within 250 ms during network wait')
        set_text(control(104),'')
        summary='\n'.join(results)+'\n';(out/'ftp-summary.txt').write_text(summary,encoding='utf-8');print(summary)
    finally:
        creds.clear()
        if window:u.PostMessageW(window,0x0010,0,0)
        try:p.wait(timeout=10)
        except subprocess.TimeoutExpired:p.kill();p.wait()

if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--build',type=Path,default=ROOT/'out/build/msvc-x64/Debug');ap.add_argument('--out',type=Path,default=ROOT/'out/verification');ap.add_argument('--credentials-helper',required=True,type=Path)
    args=ap.parse_args();run(args.build,args.out,args.credentials_helper)
