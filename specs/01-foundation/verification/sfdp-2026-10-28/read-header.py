import pathlib, subprocess, sys
out=pathlib.Path('/Users/mac/Projects/Scopa/rc-pwm-bridge/specs/01-foundation/verification/sfdp-2026-10-28')
base=['/private/tmp/scopa01-replay-clean/bin/python','-m','esptool','--chip','esp32c3','--port','/dev/cu.usbmodem1101','--baud','115200','--no-stub','--before','no_reset','--after','no_reset','read_flash_sfdp']
for addr in (0,4,8,12):
 path=out / f'header-{addr:02x}.txt'
 if path.exists(): print('Evidence already exists:',path,flush=True); sys.exit(2)
 cmd=base+[hex(addr),'4']
 result=subprocess.run(cmd,capture_output=True,text=True)
 with path.open('x') as f: f.write('command: '+' '.join(cmd)+'\nexit: '+str(result.returncode)+'\nstdout:\n'+result.stdout+'\nstderr:\n'+result.stderr)
 print(f'{hex(addr)} exit={result.returncode}\n{result.stdout}{result.stderr}',flush=True)
 if result.returncode: sys.exit(result.returncode)
