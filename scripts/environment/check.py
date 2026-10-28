#!/usr/bin/env python3
"""Read-only verification of the recorded host and target environment."""
import argparse
import hashlib
import importlib.util
import json
import os
import re
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]

def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()

def require(condition, message):
    if not condition:
        raise ValueError(message)

def command(*args):
    return subprocess.check_output(args, text=True, stderr=subprocess.STDOUT).rstrip('\n')

def check(manifest):
    data = json.loads(manifest.read_text())
    spec = importlib.util.spec_from_file_location('host_build', ROOT / 'scripts/build.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    require(module.host_tools() is not None, 'host pins')
    target = data['target']; sdk = pathlib.Path(target['sdk_path']); tools = pathlib.Path(target['idf_tools_path'])
    require(command('git', '-C', str(sdk), 'rev-parse', 'HEAD') == target['sdk_commit'], 'SDK commit')
    require(not command('git', '-C', str(sdk), 'status', '--porcelain'), 'SDK worktree dirty')
    submodules = command('git', '-C', str(sdk), 'submodule', 'status', '--recursive')
    require(not any(line.startswith(('-', '+', 'U')) for line in submodules.splitlines()), 'SDK submodule mismatch')
    require(submodules + '\n' == (ROOT / 'tools/environment/idf-submodules.txt').read_text(), 'submodule inventory mismatch')
    for name, expected in target['inventories'].items():
        path = ROOT / name
        require(digest(path) == expected, 'inventory hash: ' + name)
        if name.startswith('tools/environment/sdk-requirements/'):
            require(digest(sdk / 'tools/requirements' / path.name) == expected, 'SDK requirements: ' + path.name)
    require(digest(sdk / 'tools/tools.json') == target['inventories']['tools/environment/idf-tools.json'], 'SDK tools manifest')
    for name, expected in target['archives'].items():
        require(digest(tools / 'dist' / name) == expected, 'archive hash: ' + name)
    for item in target['python_distributions'].values():
        require(digest(ROOT / 'tools/environment/wheelhouse' / item['filename']) == item['sha256'], 'wheel: ' + item['filename'])
    for name, key in [('requirements.lock.txt', 'python_lock_sha256'), ('wheels.sha256', 'wheels_sha256_manifest')]:
        require(digest(ROOT / 'tools/environment' / name) == target[key], 'lock: ' + name)
    require(digest(ROOT / 'tools/environment/wheelhouse/esptool-4.12.0.tar.gz') == target['esptool_sdist_sha256'], 'esptool source hash')
    compiler = tools / 'tools/riscv32-esp-elf/esp-14.2.0_20260121/riscv32-esp-elf/bin/riscv32-esp-elf-g++'
    require('esp-14.2.0_20260121' in command(str(compiler), '--version'), 'target compiler version')
    require(target['python_closure_replay'] == 'pass', 'Python replay pending')
    evidence = json.loads((ROOT / 'tools/environment/replay-evidence.json').read_text())
    require(evidence['lock_sha256'] == target['python_lock_sha256'] and evidence['pip_check_exit'] == 0 and evidence['idf_check_exit'] == 0, 'replay evidence')
    python = pathlib.Path(evidence['python'])
    require('Python 3.11.7' in command(str(python), '--version'), 'replay Python version')
    installed = json.loads(command(str(python), '-c', 'import importlib.metadata as m,json; print(json.dumps([(d.metadata["Name"],d.version) for d in m.distributions()]))'))
    normalize = lambda name: re.sub(r'[-_.]+', '-', name).lower()
    names = [normalize(name) for name, _ in installed]
    require(len(names) == len(set(names)), 'duplicate installed distributions')
    actual = dict(zip(names, (version for _, version in installed)))
    expected = {normalize(name): item['version'] for name, item in target['python_distributions'].items()}
    require(actual == expected, 'installed Python closure mismatch')
    require('No broken requirements found' in command(str(python), '-m', 'pip', 'check'), 'pip check')
    env = dict(os.environ, IDF_PATH=str(sdk), IDF_TOOLS_PATH=str(tools), IDF_PYTHON_ENV_PATH=str(python.parent.parent))
    require(digest(tools / 'espidf.constraints.v5.5.txt') == target['inventories']['tools/environment/espidf.constraints.v5.5.txt'], 'actual constraints hash')
    subprocess.check_output([str(python), str(sdk / 'tools/idf_tools.py'), 'check-python-dependencies'], env=env, stderr=subprocess.STDOUT)
    print('environment verified: host, SDK, archives, wheel hashes and offline replay')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--manifest', required=True, type=pathlib.Path)
    args = parser.parse_args()
    try:
        check(args.manifest)
        return 0
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError, subprocess.SubprocessError) as exc:
        print('environment gate:', exc, file=sys.stderr)
        return 3

if __name__ == '__main__':
    sys.exit(main())
