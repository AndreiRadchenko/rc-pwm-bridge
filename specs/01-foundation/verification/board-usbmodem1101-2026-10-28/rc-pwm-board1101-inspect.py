from pathlib import Path
from datetime import datetime, timezone
import hashlib, importlib.metadata, json, subprocess, sys

root = Path('/Users/mac/Projects/Scopa/rc-pwm-bridge')
out = root / 'specs/01-foundation/verification/board-usbmodem1101-2026-10-28'
out.mkdir(parents=True, exist_ok=True)
port = '/dev/cu.usbmodem1101'
python = '/Users/mac/.platformio/penv/bin/python'
queries = [
    ('chip', ['chip_id']),
    ('flash', ['flash_id']),
    ('security', ['get_security_info']),
    ('flash-status', ['read_flash_status']),
    ('boot-header', ['read_flash', '0x0', '0x1000', str(out / 'boot-header.bin')]),
    ('partition-table', ['read_flash', '0x8000', '0x1000', str(out / 'partition-table.bin')]),
]
results = []
for label, query in queries:
    command = [python, '-m', 'esptool', '--chip', 'esp32c3', '--port', port,
               '--baud', '115200', '--no-stub', '--connect-attempts', '3',
               '--before', 'no_reset', '--after', 'no_reset', *query]
    started = datetime.now(timezone.utc).isoformat()
    try:
        result = subprocess.run(command, cwd=root, capture_output=True, text=True, timeout=40)
        stdout, stderr, code = result.stdout, result.stderr, result.returncode
    except subprocess.TimeoutExpired as exc:
        stdout = (exc.stdout or b'').decode(errors='replace') if isinstance(exc.stdout, bytes) else (exc.stdout or '')
        stderr = 'Read-only query timeout; subsequent operations stopped.\n'
        code = 124
    (out / f'{label}.stdout.txt').write_text(stdout)
    (out / f'{label}.stderr.txt').write_text(stderr)
    results.append({'label': label, 'started_utc': started, 'command': command,
                    'returncode': code, 'stdout_file': f'{label}.stdout.txt',
                    'stderr_file': f'{label}.stderr.txt'})
    print(f'QUERY {label}, exit {code}\n{stdout}{stderr}', flush=True)
    if code:
        break
(out / 'rom-queries.json').write_text(json.dumps(results, indent=2) + '\n')
print('Read-only ROM query run complete; device remains in ROM bootloader. No flash/efuse writes or stub upload.', flush=True)
sys.exit(0 if all(r['returncode'] == 0 for r in results) else 1)
