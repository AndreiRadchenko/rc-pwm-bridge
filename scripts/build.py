#!/usr/bin/env python3
"""Build the independent host suite; never operates a device."""
import argparse
import hashlib
import json
import os
import pathlib
import platform
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
MANIFEST = ROOT / 'tools/environment/environment.lock.json'

def host_tools():
    try:
        h = json.loads(MANIFEST.read_text())['host']
        if platform.system() != 'Darwin' or platform.machine() != 'arm64':
            raise ValueError('host is not Darwin arm64')
        for name in ('clang', 'python', 'cmake', 'ninja'):
            item = h[name]
            if hashlib.sha256(pathlib.Path(item['path']).read_bytes()).hexdigest() != item['sha256']:
                raise ValueError(name + ' hash mismatch')
        checks = [([h['clang']['path'], '--version'], 'clang-2100.1.1.101'),
                  ([h['python']['path'], '--version'], 'Python 3.11.7'),
                  ([h['cmake']['path'], '--version'], 'cmake version 3.30.2'),
                  ([h['ninja']['path'], '--version'], '1.12.1'),
                  (['xcrun', '--show-sdk-version'], '26.5'),
                  (['pkgutil', '--pkg-info=com.apple.pkg.CLTools_Executables'], '26.6.0.0.1781586589')]
        for cmd, expected in checks:
            if expected not in subprocess.check_output(cmd, text=True, stderr=subprocess.STDOUT):
                raise ValueError('version mismatch: ' + cmd[0])
        return h
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as exc:
        print('host environment gate:', exc, file=sys.stderr)
        return None

def validate_profile(path):
    """Accept only the reviewed design; pending physical gates are not build approval."""
    raw = pathlib.Path(path).read_bytes()
    data = json.loads(raw)
    expected = {'schema_version': 1, 'variant_id': 'c3-supermini-pinout-01',
                'unit_id': 'c3-supermini-188b0e93032c', 'chip_model': 'ESP32-C3',
                'chip_package': 'QFN32', 'chip_revision': 'v0.4', 'flash_bytes': 4194304,
                'flash_vendor': 'XMC', 'flash_jedec_id': '20:4016', 'rc_pin': 3,
                'pwm_pin': 0, 'direction_mode': 'hardwired_low', 'flash_mode': 'dio',
                'flash_frequency_hz': 40000000, 'partition_layout': 'idf_single_factory_1m'}
    for key, value in expected.items():
        if data.get(key) != value or (isinstance(value, int) and type(data.get(key)) is not int):
            raise ValueError('invalid/unapproved profile field: ' + key)
    if any(key in data for key in ('dir_pin', 'dir_level')):
        raise ValueError('DIR GPIO is forbidden')
    if data.get('approval', {}).get('design') != 'design_approved' or data.get('approval', {}).get('source') != 'D18-D20':
        raise ValueError('design approval missing')
    evidence = data.get('evidence', {})
    sources = {'pinout': ('hardware/esp32-c3-pinout.jpg', 'c0c770ef639612bcfeca7ed7472c4d0b190032b43332c84b52fda81f3133a07c'),
               'photo': ('hardware/esp32-supermini.jpg', '36ab8c15ab2626a4c55ddf45ceb3a02d9be5e18dd465bc9602f31cc8927ee041')}
    for name, (relative, expected_hash) in sources.items():
        if evidence.get(name) != relative or evidence.get(name + '_sha256') != expected_hash or hashlib.sha256((ROOT / relative).read_bytes()).hexdigest() != expected_hash:
            raise ValueError('provenance mismatch: ' + name)
    if evidence.get('design') != 'docs/decisions.md D17-D20' or evidence.get('rom') != 'specs/01-foundation/verification/board-review.md' or not (ROOT / evidence['rom']).is_file():
        raise ValueError('design/ROM provenance missing')
    flash = data['flash_settings_evidence']
    if hashlib.sha256((ROOT / flash['sfdp']).read_bytes()).hexdigest() != 'b431a14ba8c39a05e5fe8c14924d8f33649ea28a48c7c0214364a516bee1daa9':
        raise ValueError('SFDP provenance mismatch')
    if flash['partition_csv_sha256'] != 'e829387dc93b09b655e5ba558bb317a798ab7378834facad2621602126d7b510':
        raise ValueError('partition provenance mismatch')
    return data, hashlib.sha256(raw).hexdigest()

def generate_profile_header(directory, data, fingerprint):
    path = pathlib.Path(directory) / 'generated' / 'board_profile.hpp'
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('#pragma once\n#define BRIDGE_PROFILE_PWM_PIN %d\n#define BRIDGE_PROFILE_RC_PIN %d\n#define BRIDGE_PROFILE_FINGERPRINT 0x%sULL\n' %
                    (data['pwm_pin'], data['rc_pin'], fingerprint[:16]))
    return path

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('mode', choices=['host', 'target'])
    parser.add_argument('--build-dir', required=True)
    parser.add_argument('--board-profile')
    args = parser.parse_args()
    if args.mode == 'target' and not args.board_profile:
        parser.error('target requires --board-profile')
    if args.mode == 'host' and args.board_profile:
        parser.error('host does not accept --board-profile')
    directory = pathlib.Path(args.build_dir).resolve()
    if directory.exists():
        print('build directory already exists:', directory, file=sys.stderr)
        return 3
    h = host_tools()
    if h is None:
        return 3
    if args.mode == 'target':
        try:
            data, fingerprint = validate_profile(args.board_profile)
            env_check = subprocess.run([sys.executable, str(ROOT / 'scripts/environment/check.py'), '--manifest', str(MANIFEST)], check=False)
            if env_check.returncode:
                return 3
            identity = json.loads(MANIFEST.read_text())['target']
            sdk = pathlib.Path(identity['sdk_path'])
            toolroot = pathlib.Path(identity['idf_tools_path'])
            python = pathlib.Path(json.loads((ROOT / 'tools/environment/replay-evidence.json').read_text())['python'])
            header = generate_profile_header(directory, data, fingerprint)
            env = os.environ.copy()
            env.update({'IDF_PATH': str(sdk), 'IDF_TOOLS_PATH': str(toolroot), 'IDF_PYTHON_ENV_PATH': str(python.parent.parent), 'IDF_COMPONENT_MANAGER': '0'})
            bins = [str(pathlib.Path(h['cmake']['path']).parent), str(pathlib.Path(h['ninja']['path']).parent),
                    str(toolroot / 'tools/riscv32-esp-elf/esp-14.2.0_20260121/riscv32-esp-elf/bin'), str(python.parent)]
            env['PATH'] = os.pathsep.join(bins + [env.get('PATH', '')])
            cmd = [str(python), str(sdk / 'tools/idf.py'), '-C', str(ROOT / 'firmware'), '-B', str(directory),
                   '-D', 'SDKCONFIG=' + str(directory / 'sdkconfig'),
                   '-D', 'SDKCONFIG_DEFAULTS=' + str(ROOT / 'firmware/sdkconfig.defaults'),
                   '-D', 'BOARD_PROFILE_HEADER=' + str(header), 'build']
            return 0 if subprocess.run(cmd, env=env, check=False).returncode == 0 else 1
        except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
            print('target environment/profile gate:', exc, file=sys.stderr)
            return 3
    cmake = h['cmake']['path']
    commands = [[cmake, '-S', str(ROOT / 'tests/host'), '-B', str(directory), '-G', 'Ninja',
                 '-DCMAKE_MAKE_PROGRAM=' + h['ninja']['path'],
                 '-DCMAKE_CXX_COMPILER=' + str(pathlib.Path(h['clang']['path']).with_name('clang++'))],
                [cmake, '--build', str(directory)]]
    for cmd in commands:
        try:
            if subprocess.run(cmd, check=False).returncode:
                return 1
        except OSError as exc:
            print(exc, file=sys.stderr)
            return 1
    return 0

if __name__ == '__main__':
    sys.exit(main())
