#!/usr/bin/env python3
"""Check generated evidence; source review and physical checks remain separate."""
import argparse
import json
import pathlib
import re
import shlex
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
FORBIDDEN = {'esp_wifi', 'bt', 'esp_netif', 'lwip'}
FORBIDDEN_SYMBOLS = ('esp_wifi_init', 'esp_wifi_start', 'esp_bt_controller_init',
                     'esp_bt_controller_enable', 'esp_netif_init')

def verify(directory):
    sdk = (directory / 'sdkconfig').read_text()
    graph = json.loads((directory / 'project_description.json').read_text())
    commands = json.loads((directory / 'compile_commands.json').read_text())
    map_text = (directory / 'rc_pwm_bridge.map').read_text()
    required = ('CONFIG_IDF_TARGET="esp32c3"', 'CONFIG_ESPTOOLPY_FLASHMODE_DIO=y',
                'CONFIG_ESPTOOLPY_FLASHFREQ_40M=y', 'CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y',
                'CONFIG_PARTITION_TABLE_SINGLE_APP=y', 'CONFIG_BOOTLOADER_LOG_LEVEL=0', 'CONFIG_LOG_DEFAULT_LEVEL=0',
                'CONFIG_LOG_MAXIMUM_LEVEL=0', 'CONFIG_FREERTOS_HZ=100',
                'CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y', 'CONFIG_ESP_CONSOLE_SECONDARY_NONE=y')
    for value in required:
        if value not in sdk.splitlines(): raise ValueError('missing config: ' + value)
    components = set(graph['build_components'])
    if components & FORBIDDEN: raise ValueError('forbidden components: ' + ', '.join(sorted(components & FORBIDDEN)))
    for component in FORBIDDEN:
        if re.search(r'(?<![A-Za-z0-9_])lib' + re.escape(component) + r'\.a', map_text):
            raise ValueError('forbidden linked archive: ' + component)
    for symbol in FORBIDDEN_SYMBOLS:
        if re.search(r'\b' + re.escape(symbol) + r'\b', map_text): raise ValueError('forbidden symbol: ' + symbol)
    core = [x for x in commands if x['file'].endswith('/bridge_core/startup.cpp')]
    if len(core) != 1: raise ValueError('missing or ambiguous core compile command')
    opts = shlex.split(core[0]['command'])
    standards = [v for v in opts if v.startswith('-std=')]
    if not standards or standards[-1] != '-std=c++11' or not {'-fno-rtti','-fno-exceptions'} <= set(opts):
        raise ValueError('core compiler flags do not enforce strict C++11')
    sources = [*ROOT.glob('firmware/components/bridge_core/*.cpp'), *ROOT.glob('firmware/components/platform_esp32/*.cpp'), *ROOT.glob('firmware/main/*.cpp')]
    if len(sources) < 6: raise ValueError('startup source set incomplete')
    forbidden_calls = ('esp_restart', 'esp_wifi_init', 'esp_wifi_start', 'esp_bt_controller_init',
                       'esp_bt_controller_enable', 'esp_netif_init', 'rmt_new_rx_channel', 'ledc_set_duty')
    for source in sources:
        text = source.read_text()
        for symbol in forbidden_calls:
            if re.search(r'\b' + symbol + r'\s*\(', text): raise ValueError(f'forbidden call {symbol}: {source}')
    print('Automated config/graph/map/source checks passed; manual startup and watchdog/bootloader review still required.')
    for line in sdk.splitlines():
        if re.match(r'CONFIG_(?:ESP_SYSTEM_PANIC|ESP_TASK_WDT|ESP_INT_WDT|BOOTLOADER_WDT)',line): print(line)

def main():
    parser = argparse.ArgumentParser(); parser.add_argument('--build-dir', type=pathlib.Path, required=True); args = parser.parse_args()
    try: verify(args.build_dir); return 0
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        print('config check failed:', exc, file=sys.stderr); return 1
if __name__ == '__main__': sys.exit(main())
