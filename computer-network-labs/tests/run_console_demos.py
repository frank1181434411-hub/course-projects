"""Open actual pairs of Windows consoles for each lab and validate transcripts."""
from pathlib import Path
import subprocess
import sys
import time

root=Path(__file__).resolve().parents[1]
out=root/'out/verification'
results=[]
for lab in ['lab02','lab03']:
    processes=[]
    try:
        for role in ['server','client']:
            processes.append(subprocess.Popen([sys.executable,str(root/'tests/console_demo.py'),lab,role],creationflags=subprocess.CREATE_NEW_CONSOLE))
            if role=='server':time.sleep(1)
        for p in processes:assert p.wait(timeout=50)==0,'Console driver failed'
        server=(out/f'console-{lab}-server.txt').read_text(encoding='utf-8')
        client=(out/f'console-{lab}-client.txt').read_text(encoding='utf-8')
        for index in ['first','second']:
            assert f'RECV: server-{index}-in-console' in client
            assert f'RECV: client-{index}-in-console' in server
        assert 'SESSION CLOSED' in server and 'SESSION CLOSED' in client
        results.append(f'PASS {lab}: two visible Windows consoles; independent server/client processes; two messages each direction; /quit; both exit 0')
    finally:
        for p in processes:
            if p.poll() is None:p.kill();p.wait()
text='\n'.join(results)+'\n';(out/'console-summary.txt').write_text(text,encoding='utf-8');print(text)
