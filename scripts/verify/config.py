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

def verify(directory, require_pump04=None):
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
    capture_required = ('CONFIG_RMT_RX_ISR_CACHE_SAFE=y', 'CONFIG_RMT_RECV_FUNC_IN_IRAM=y',
                        'CONFIG_GPIO_CTRL_FUNC_IN_IRAM=y')
    for value in capture_required:
        if value not in sdk.splitlines(): raise ValueError('missing capture config: ' + value)
    for key in ('CONFIG_PM_ENABLE=y', 'CONFIG_FREERTOS_USE_TICKLESS_IDLE=y'):
        if key in sdk.splitlines(): raise ValueError('unsupported capture power setting: ' + key)
    if not {'esp_driver_rmt','esp_driver_gpio'} <= components:
        raise ValueError('missing capture components')
    actual = {str(pathlib.Path(x['file']).resolve()) for x in commands}
    for path in ('bridge_core/rc_capture.cpp','platform_esp32/rc_capture.cpp'):
        if str(ROOT/'firmware/components'/path) not in actual:
            raise ValueError('missing capture compile object: ' + path)
    trace_on = 'CONFIG_BRIDGE_RC03_ACCURACY_TRACE=y' in sdk.splitlines()
    stress_on = 'CONFIG_BRIDGE_RC03_DIAGNOSTIC_STRESS=y' in sdk.splitlines()
    trace_source = str(ROOT/'firmware/components/bridge_core/rc_capture_trace.cpp')
    if (trace_source in actual) != trace_on or (trace_on and not stress_on):
        raise ValueError('trace config/object/stress graph mismatch')
    core = [x for x in commands if '/components/bridge_core/' in x['file']]
    if not core: raise ValueError('missing core compile commands')
    for command in core:
        opts = shlex.split(command['command'])
        standards = [v for v in opts if v.startswith('-std=')]
        if not standards or standards[-1] != '-std=c++11' or not {'-fno-rtti','-fno-exceptions'} <= set(opts):
            raise ValueError('core compiler flags do not enforce strict C++11')
    sources = [*ROOT.glob('firmware/components/bridge_core/*.cpp'), *ROOT.glob('firmware/components/platform_esp32/*.cpp'), *ROOT.glob('firmware/main/*.cpp')]
    if len(sources) < 6: raise ValueError('startup source set incomplete')
    forbidden_calls = ('esp_restart', 'esp_wifi_init', 'esp_wifi_start', 'esp_bt_controller_init',
                       'esp_bt_controller_enable', 'esp_netif_init', 'ledc_set_duty')
    for source in sources:
        text = source.read_text()
        if re.search(r'\brmt_new_rx_channel\s*\(', text) and source != ROOT/'firmware/components/platform_esp32/rc_capture.cpp':
            raise ValueError('RMT creation outside approved adapter: ' + str(source))
        for symbol in forbidden_calls:
            if re.search(r'\b' + symbol + r'\s*\(', text): raise ValueError(f'forbidden call {symbol}: {source}')
    if require_pump04 is not None:
        manifest=json.loads((directory/'pump04-manifest.json').read_text())
        if manifest['scenario']!=require_pump04 or manifest['output_duration_us']!=({'normal':0,'waveform':60000000,'stress':600000000,'accuracy':60000000}[require_pump04]) or manifest['window_duration_us']!=(601000000 if require_pump04=='stress' else 0) or manifest['initial_low_us']!=(0 if require_pump04=='normal' else 5000000):
            raise ValueError('pump04 manifest/scenario/duration mismatch')
        bench='CONFIG_BRIDGE_PUMP04_BENCH=y' in sdk.splitlines()
        if bench != (require_pump04!='normal'):raise ValueError('pump04 default-off/bench mismatch')
        if require_pump04!='normal':
            expected={'waveform':1,'stress':2,'accuracy':3}[require_pump04]
            if 'CONFIG_BRIDGE_PUMP04_SCENARIO='+str(expected) not in sdk.splitlines():raise ValueError('pump04 scenario mismatch')
        if trace_on != (require_pump04=='accuracy') or stress_on != (require_pump04 in ('stress','accuracy')):
            raise ValueError('pump04 instrumentation mismatch')
        if require_pump04=='accuracy':
            step=manifest.get('trace_step')
            if type(step) is not int or not 0<=step<=5 or 'CONFIG_BRIDGE_PUMP04_TRACE_STEP='+str(step) not in sdk.splitlines():raise ValueError('pump04 accuracy step mismatch')
        for name in ('bridge_core/pump_output.cpp','platform_esp32/pump_output.cpp'):
            if str(ROOT/'firmware/components'/name) not in actual:raise ValueError('missing pump04 object: '+name)
        for command in commands:
            if '/components/platform_esp32/' in command['file'] or '/firmware/main/' in command['file']:
                standards=[x for x in shlex.split(command['command']) if x.startswith('-std=')]
                if not standards or standards[-1]!='-std=c++17':raise ValueError('pump04 platform compiler flags')
        if 'esp_driver_ledc' not in components:raise ValueError('missing native LEDC dependency')
        for symbol in ('ledc_fade_func_install','ledc_set_duty_and_update'):
            # Discarded input sections and the cross-reference table also name
            # SDK functions that are absent from the linked image.
            if re.search(r'^\s+0x[0-9a-fA-F]+\s+'+symbol+r'\s*$',map_text,re.MULTILINE):
                raise ValueError('forbidden pump04 fade/blocking service')
    print('Automated config/graph/map/source checks passed; manual startup and watchdog/bootloader review still required.')
    for line in sdk.splitlines():
        if re.match(r'CONFIG_(?:ESP_SYSTEM_PANIC|ESP_TASK_WDT|ESP_INT_WDT|BOOTLOADER_WDT)',line): print(line)

def main():
    parser = argparse.ArgumentParser(); parser.add_argument('--build-dir', type=pathlib.Path, required=True); parser.add_argument('--require-pump04',choices=['normal','waveform','stress','accuracy']); args = parser.parse_args()
    try: verify(args.build_dir,args.require_pump04); return 0
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        print('config check failed:', exc, file=sys.stderr); return 1
if __name__ == '__main__': sys.exit(main())
