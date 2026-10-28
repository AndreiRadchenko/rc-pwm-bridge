from pathlib import Path
from datetime import datetime, timezone
import json, subprocess, time
import serial
from serial.tools import list_ports

root = Path('/Users/mac/Projects/Scopa/rc-pwm-bridge')
out = root / 'specs/01-foundation/verification/board-usbmodem1101-2026-10-28'
command = ['/Users/mac/.platformio/penv/bin/python','-m','esptool','--chip','esp32c3',
           '--port','/dev/cu.usbmodem1101','--baud','115200','--no-stub',
           '--connect-attempts','3','--before','no_reset','--after','hard_reset','read_mac']
started = datetime.now(timezone.utc).isoformat()
result = subprocess.run(command,cwd=root,capture_output=True,text=True,timeout=30)
(out/'restore-boot.stdout.txt').write_text(result.stdout)
(out/'restore-boot.stderr.txt').write_text(result.stderr)
print(result.stdout+result.stderr,flush=True)
record = {'started_utc':started,'command':command,'returncode':result.returncode,
          'capture_settings':{'baudrate':115200,'dtr':False,'rts':False,'timeout':0.1,'duration_seconds':5},
          'confirmation':'User confirmed USB only; MD20A/pump/other loads disconnected.'}
if result.returncode:
    (out/'boot-capture.json').write_text(json.dumps(record,indent=2)+'\n')
    raise SystemExit(result.returncode)
port = serial.Serial(port=None,baudrate=115200,timeout=0.1,write_timeout=0)
port.dtr=False
port.rts=False
port.port='/dev/cu.usbmodem1101'
port.open()
chunks=[]
end=time.monotonic()+5
while time.monotonic()<end:
    chunk=port.read(min(max(port.in_waiting,1),4096))
    if chunk: chunks.append(chunk)
port.close()
raw=b''.join(chunks)
(out/'boot-console.bin').write_bytes(raw)
(out/'boot-console.txt').write_text(raw.decode('utf-8',errors='replace'))
record['captured_bytes']=len(raw)
record['finished_utc']=datetime.now(timezone.utc).isoformat()
record['ports_after']=[{'device':p.device,'description':p.description,'serial_number':p.serial_number,'vid':p.vid,'pid':p.pid} for p in list_ports.comports() if p.device=='/dev/cu.usbmodem1101']
(out/'boot-capture.json').write_text(json.dumps(record,indent=2)+'\n')
print('BOOT CONSOLE ('+str(len(raw))+' bytes):\n'+raw[:8000].decode('utf-8',errors='replace'),flush=True)
print('No serial payload transmitted during capture; no flash/eFuse writes or stub upload.',flush=True)
