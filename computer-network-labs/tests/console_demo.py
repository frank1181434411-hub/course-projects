"""Run one real TCP executable in a console with a timed input demonstration.
Use two consoles: python tests/console_demo.py lab02 server
                  python tests/console_demo.py lab02 client
The stdin/stdout relay shows and records the real executable's output.
"""
import argparse
import ctypes
from pathlib import Path
import subprocess
import threading
import time

root=Path(__file__).resolve().parents[1]
ap=argparse.ArgumentParser(); ap.add_argument('lab',choices=['lab02','lab03']); ap.add_argument('role',choices=['server','client']); args=ap.parse_args()
ctypes.windll.kernel32.SetConsoleTitleW(f'NetworkLabs {args.lab} {args.role} demonstration')
ctypes.windll.kernel32.SetConsoleOutputCP(65001)
port='19425' if args.lab=='lab02' else '19426'
command=[str(root/f'out/build/msvc-x64/Debug/{args.lab}_{args.role}.exe')]
command += [port] if args.role=='server' else ['127.0.0.1',port]
out=root/'out/verification';out.mkdir(parents=True,exist_ok=True)
p=subprocess.Popen(command,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,encoding='utf-8',bufsize=1)
connected=threading.Event()
def read():
    with (out/f'console-{args.lab}-{args.role}.txt').open('w',encoding='utf-8') as log:
        for line in p.stdout:
            print(line,end='',flush=True);log.write(line);log.flush()
            if line.startswith('CONNECTED'): connected.set()
thread=threading.Thread(target=read);thread.start()
assert connected.wait(30),'Connection timeout'
schedule=[(0.5,'server-first-in-console'),(1.5,'server-second-in-console')] if args.role=='server' else [(1.0,'client-first-in-console'),(2.0,'client-second-in-console'),(3.0,'/quit')]
start=time.monotonic()
for at,text in schedule:
    time.sleep(max(0,start+at-time.monotonic()));p.stdin.write(text+'\n');p.stdin.flush()
assert p.wait(timeout=10)==0
thread.join();print('Console demonstration complete. Window closes in 12 seconds.',flush=True);time.sleep(12)
