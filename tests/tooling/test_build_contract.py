import copy
import importlib.util
import json
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('build', ROOT / 'scripts/build.py')
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)
PROFILE = json.loads((ROOT / 'firmware/boards/c3-supermini-pinout-01.json').read_text())

class ProfileTests(unittest.TestCase):
    def check_profile(self, data, valid):
        with tempfile.TemporaryDirectory() as d:
            p = pathlib.Path(d) / 'profile.json'
            p.write_text(json.dumps(data))
            if not valid:
                with self.assertRaises((ValueError, KeyError, OSError)):
                    build.validate_profile(p)
                self.assertFalse((pathlib.Path(d)/'generated').exists())
            else:
                result, digest = build.validate_profile(p)
                header = build.generate_profile_header(d, result, digest)
                self.assertIn('BRIDGE_PROFILE_PWM_PIN 0', header.read_text())
                self.assertIn(digest[:16], header.read_text())

    def test_approved_pending_bench(self):
        self.assertEqual(PROFILE['approval']['bench'], 'pending')
        self.check_profile(PROFILE, True)

    def test_rejected_variants(self):
        for key, value in [('chip_model','ESP32'),('flash_bytes',0),('flash_mode',None),
                           ('flash_frequency_hz',None),('partition_layout',None),('pwm_pin',3),
                           ('rc_pin',0),('dir_pin',1),('direction_mode','gpio')]:
            with self.subTest(key=key):
                data = copy.deepcopy(PROFILE); data[key]=value; self.check_profile(data, False)
        data=copy.deepcopy(PROFILE); data['approval']['design']='draft'; self.check_profile(data,False)
        data=copy.deepcopy(PROFILE); del data['evidence']['pinout']; self.check_profile(data,False)
        with self.assertRaises(OSError): build.validate_profile('/nonexistent/profile.json')


class WrapperTests(unittest.TestCase):
    def invoke(self, args):
        from unittest.mock import patch
        with patch.object(build.sys, 'argv', ['build.py'] + args):
            return build.main()

    def test_bad_cli(self):
        with self.assertRaises(SystemExit) as e: self.invoke(['target','--build-dir','unused'])
        self.assertEqual(e.exception.code, 2)

    def test_busy_directory_is_untouched(self):
        with tempfile.TemporaryDirectory() as d:
            p=pathlib.Path(d)/'keep'; p.write_text('keep')
            self.assertEqual(self.invoke(['host','--build-dir',d]),3)
            self.assertEqual(p.read_text(),'keep')

    def test_host_and_target_return_codes_without_device_actions(self):
        from unittest.mock import patch
        from types import SimpleNamespace
        h={'cmake':{'path':'/fake/cmake'},'ninja':{'path':'/fake/ninja'},'clang':{'path':'/fake/clang'}}
        with tempfile.TemporaryDirectory() as d, patch.object(build,'host_tools',return_value=h), patch.object(build.subprocess,'run',return_value=SimpleNamespace(returncode=0)) as run:
            self.assertEqual(self.invoke(['host','--build-dir',str(pathlib.Path(d)/'host')]),0)
            self.assertEqual(self.invoke(['target','--board-profile',str(ROOT/'firmware/boards/c3-supermini-pinout-01.json'),'--build-dir',str(pathlib.Path(d)/'target')]),0)
            commands=[call.args[0] for call in run.call_args_list]
            self.assertTrue(all(not set(cmd)&{'flash','erase-flash','monitor','reset'} for cmd in commands))
            self.assertTrue((pathlib.Path(d)/'target/generated/board_profile.hpp').exists())
        with tempfile.TemporaryDirectory() as d, patch.object(build,'host_tools',return_value=h), patch.object(build.subprocess,'run',side_effect=[SimpleNamespace(returncode=0), SimpleNamespace(returncode=1)]):
            self.assertEqual(self.invoke(['target','--board-profile',str(ROOT/'firmware/boards/c3-supermini-pinout-01.json'),'--build-dir',str(pathlib.Path(d)/'failed-target')]),1)
        with tempfile.TemporaryDirectory() as d, patch.object(build,'host_tools',return_value=h), patch.object(build.subprocess,'run',return_value=SimpleNamespace(returncode=1)):
            self.assertEqual(self.invoke(['host','--build-dir',str(pathlib.Path(d)/'failed')]),1)
            self.assertEqual(self.invoke(['target','--board-profile',str(ROOT/'firmware/boards/c3-supermini-pinout-01.json'),'--build-dir',str(pathlib.Path(d)/'blocked')]),3)
            self.assertFalse((pathlib.Path(d)/'blocked').exists())

    def test_invalid_profile_rejected_before_configure(self):
        from unittest.mock import patch
        h={'cmake':{'path':'/fake/cmake'},'ninja':{'path':'/fake/ninja'},'clang':{'path':'/fake/clang'}}
        with tempfile.TemporaryDirectory() as d, patch.object(build,'host_tools',return_value=h), patch.object(build.subprocess,'run') as run:
            bad=pathlib.Path(d)/'bad.json'; data=copy.deepcopy(PROFILE); data['pwm_pin']=3; bad.write_text(json.dumps(data))
            dest=pathlib.Path(d)/'out'
            self.assertEqual(self.invoke(['target','--board-profile',str(bad),'--build-dir',str(dest)]),3)
            run.assert_not_called(); self.assertFalse(dest.exists())

class ConfigTests(unittest.TestCase):
    def test_missing_and_forbidden_evidence(self):
        spec2 = importlib.util.spec_from_file_location('config_verify', ROOT/'scripts/verify/config.py')
        module = importlib.util.module_from_spec(spec2); spec2.loader.exec_module(module)
        with tempfile.TemporaryDirectory() as d:
            path=pathlib.Path(d)
            with self.assertRaises(OSError): module.verify(path)
            (path/'sdkconfig').write_text('\n'.join(['CONFIG_IDF_TARGET="esp32c3"','CONFIG_ESPTOOLPY_FLASHMODE_DIO=y','CONFIG_ESPTOOLPY_FLASHFREQ_40M=y','CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y','CONFIG_PARTITION_TABLE_SINGLE_APP=y','CONFIG_BOOTLOADER_LOG_LEVEL=0','CONFIG_LOG_DEFAULT_LEVEL=0','CONFIG_LOG_MAXIMUM_LEVEL=0','CONFIG_FREERTOS_HZ=100','CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y','CONFIG_ESP_CONSOLE_SECONDARY_NONE=y']))
            graph={'build_components':['main','esp_wifi']}
            (path/'project_description.json').write_text(json.dumps(graph))
            (path/'compile_commands.json').write_text('[]'); (path/'rc_pwm_bridge.map').write_text('map')
            with self.assertRaisesRegex(ValueError,'forbidden components'): module.verify(path)

if __name__ == '__main__': unittest.main()
