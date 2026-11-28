"""Integration tests of the actual Windows executables; Python stdlib only."""
import argparse
from pathlib import Path
import queue
import socket
import subprocess
import sys
import threading
import time

ROOT=Path(__file__).resolve().parents[1]

class Process:
    def __init__(self, args):
        self.proc=subprocess.Popen([str(x) for x in args],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,encoding='utf-8',bufsize=1)
        self.lines=[]
        self.changed=threading.Condition()
        self.reader=threading.Thread(target=self.read,daemon=True)
        self.reader.start()
    def read(self):
        for line in self.proc.stdout:
            with self.changed:
                self.lines.append(line.rstrip('\n'))
                self.changed.notify_all()
    def wait_for(self, text, timeout=8):
        deadline=time.monotonic()+timeout
        with self.changed:
            while not any(text in line for line in self.lines):
                remaining=deadline-time.monotonic()
                if remaining<=0: raise AssertionError(f'Missing output: {text!r}; got {self.lines[-8:]}')
                self.changed.wait(remaining)
    def send(self, text):
        self.proc.stdin.write(text+'\n'); self.proc.stdin.flush()
    def finish(self):
        code=self.proc.wait(timeout=8); self.reader.join(timeout=2)
        assert code==0, (code,self.lines[-8:])
        assert any('SESSION CLOSED' in x for x in self.lines)
    def close(self):
        if self.proc.poll() is None: self.proc.kill(); self.proc.wait()

def run(build,out):
    out.mkdir(parents=True,exist_ok=True)
    results=[]
    for lab,port in [('lab02',19325),('lab03',19326)]:
        server=Process([build/f'{lab}_server.exe',port]); client=None
        try:
            server.wait_for('LISTENING')
            client=Process([build/f'{lab}_client.exe','127.0.0.1',port])
            server.wait_for('CONNECTED'); client.wait_for('CONNECTED')
            # Server initiates: this proves the client is not restricted to request/reply.
            server.send('server-first'); client.wait_for('RECV: server-first')
            client.send('client-to-server'); server.wait_for('RECV: client-to-server')
            server.send('independent-server-message'); client.send('independent-client-message')
            client.wait_for('RECV: independent-server-message'); server.wait_for('RECV: independent-client-message')
            long_message='跨缓冲区-'+'X'*9000
            client.send(long_message); server.wait_for('RECV: '+long_message)
            client.send('/quit'); client.finish(); server.finish()
            for role,p in [('server',server),('client',client)]:
                (out/f'{lab}-{role}.txt').write_text('\n'.join(p.lines)+'\n',encoding='utf-8')
            results.append(f'PASS {lab}: server-first; bidirectional independent messages; UTF-8/9000-byte framing; client /quit; both exit 0')
        finally:
            server.close()
            if client: client.close()
        # Reverse close and EOF paths, with a fresh listening port.
        server=Process([build/f'{lab}_server.exe',port+10]); client=None
        try:
            server.wait_for('LISTENING'); client=Process([build/f'{lab}_client.exe','127.0.0.1',port+10])
            client.wait_for('CONNECTED'); server.send('/quit'); server.finish(); client.finish()
            results.append(f'PASS {lab}: server /quit; peer exits without keyboard input')
        finally:
            server.close()
            if client: client.close()
        # Deliberately split and coalesce TCP writes; recv boundaries are not messages.
        server=Process([build/f'{lab}_server.exe',port+20])
        try:
            server.wait_for('LISTENING')
            with socket.create_connection(('127.0.0.1',port+20),timeout=3) as peer:
                peer.sendall(b'split-'); time.sleep(.1); peer.sendall(b'message\ncoalesced-A\ncoalesced-B\n')
                for msg in ['split-message','coalesced-A','coalesced-B']: server.wait_for('RECV: '+msg)
                peer.shutdown(socket.SHUT_WR)
            server.finish()
            results.append(f'PASS {lab}: fragmented/coalesced frames; remote EOF')
        finally: server.close()
    failed=subprocess.run([str(build/'lab03_client.exe'),'127.0.0.1','19399'],capture_output=True,text=True,encoding='utf-8',timeout=6)
    assert failed.returncode==1 and 'Connection failed' in failed.stdout
    results.append('PASS lab03: unused local port produces a connection error and exit 1')
    text='\n'.join(results)+'\n'
    (out/'tcp-summary.txt').write_text(text,encoding='utf-8')
    print(text)

if __name__=='__main__':
    ap=argparse.ArgumentParser(); ap.add_argument('--build',type=Path,default=ROOT/'out/build/msvc-x64/Debug'); ap.add_argument('--out',type=Path,default=ROOT/'out/verification')
    args=ap.parse_args(); run(args.build,args.out)
