import pathlib, subprocess, sys
out=pathlib.Path('/Users/mac/Projects/Scopa/rc-pwm-bridge/specs/01-foundation/verification/sfdp-2026-10-28')
base=['/private/tmp/scopa01-replay-clean/bin/python','-m','esptool','--chip','esp32c3','--port','/dev/cu.usbmodem1101','--baud','115200','--no-stub','--before','no_reset','--after','no_reset','read_flash_sfdp']
for addr in (0x10,0x14,0x18,0x1c,*range(0x30,0x70,4)):
 path=out/f'sfdp-{addr:02x}.txt'
 if path.exists(): print('Evidence already exists:',path,flush=True); sys.exit(2)
 cmd=base+[hex(addr),'4']; r=subprocess.run(cmd,capture_output=True,text=True)
 with path.open('x') as f: f.write('command: '+' '.join(cmd)+'\nexit: '+str(r.returncode)+'\nstdout:\n'+r.stdout+'\nstderr:\n'+r.stderr)
 print(f'{hex(addr)} exit={r.returncode}',flush=True)
 if r.returncode or 'MAC: 18:8b:0e:93:03:2c' not in r.stdout or f'SFDP[{addr}..{addr+3}]:' not in r.stdout:
  print(r.stdout+r.stderr,flush=True); sys.exit(r.returncode or 3)
