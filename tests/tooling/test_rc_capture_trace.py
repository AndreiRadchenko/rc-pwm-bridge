import importlib.util
from pathlib import Path
from decimal import Decimal
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('trace_compare', ROOT/'scripts/verify/rc_capture_trace.py')
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

class TraceComparisonTests(unittest.TestCase):
    def fixture(self, directory, delta_ns=4600):
        trace = directory/'trace.txt'
        ref = directory/'analyzer.csv'
        lines = ['rc03-trace H 1 128 0 1000 2542500 0 0 0 0 0 0 0 0']
        refs = ['pulse_start_ns,pulse_end_ns']
        for i in range(128):
            start = 1000+i*20000
            lines.append(f'rc03-trace R {i} {i*2+1} {i+1} {start} {start+1500} 1500 20000 769 0 {start+6500}')
            a = 1000000000+i*20000000
            refs.append(f'{a},{a+1500000-delta_ns}')
        lines.append('rc03-trace E 128')
        trace.write_text('\n'.join(lines)+'\n')
        ref.write_text('\n'.join(refs)+'\n')
        return trace, ref

    def test_direct_error_rule_not_guard_band(self):
        with tempfile.TemporaryDirectory() as d:
            t,a = self.fixture(Path(d))
            result = mod.compare(t,a,Decimal('.8'),Decimal('.8'))
            self.assertEqual(result['status'], 'pass')
            self.assertEqual(Decimal(result['max_E_width_us']), Decimal('4.6'))
            self.assertEqual(len(result['errors']),128)

    def test_outside_limit_no_rounding(self):
        with tempfile.TemporaryDirectory() as d:
            t,a = self.fixture(Path(d),5001)
            self.assertEqual(mod.compare(t,a,Decimal(1),Decimal(1))['status'], 'fail')

    def test_partial_cycle_then_recovered_reader(self):
        with tempfile.TemporaryDirectory() as d:
            t,a = self.fixture(Path(d),0)
            full = t.read_text()
            t.write_text(full[:300]+'\n'+full)
            self.assertEqual(mod.compare(t,a,Decimal(1),Decimal(1))['status'], 'pass')

    def test_gap_count_and_uncertainty(self):
        with tempfile.TemporaryDirectory() as d:
            t,a = self.fixture(Path(d),0)
            with self.assertRaises(mod.IncompleteTrace):
                mod.compare(t,a,Decimal('1.01'),Decimal(1))
            t.write_text(t.read_text().replace('rc03-trace R 42 ', 'rc03-trace R 43 '))
            with self.assertRaises(mod.IncompleteTrace):
                mod.compare(t,a,Decimal(1),Decimal(1))

    def test_reference_count_mismatch(self):
        with tempfile.TemporaryDirectory() as d:
            t,a = self.fixture(Path(d),0)
            a.write_text('\n'.join(a.read_text().splitlines()[:-1])+'\n')
            with self.assertRaises(ValueError):
                mod.compare(t,a,Decimal(1),Decimal(1))

    def test_failed_collector_and_different_session_records(self):
        with tempfile.TemporaryDirectory() as d:
            t,a = self.fixture(Path(d),0)
            t.write_text(t.read_text().replace('H 1 128 0 ', 'H 1 128 1 '))
            with self.assertRaisesRegex(ValueError,'collector marked'):
                mod.compare(t,a,Decimal(1),Decimal(1))
            t,a = self.fixture(Path(d),0)
            t.write_text(t.read_text().replace('R 5 11 6 ', 'R 5 11 1 '))
            with self.assertRaisesRegex(ValueError,'gap/duplicate/order'):
                mod.compare(t,a,Decimal(1),Decimal(1))

    def test_interval_error_is_checked_for_each_pair(self):
        with tempfile.TemporaryDirectory() as d:
            t,a = self.fixture(Path(d),0)
            rows=a.read_text().splitlines()
            start,end=map(int,rows[65].split(','))
            rows[65]=f'{start+10001},{end+10001}'
            a.write_text('\n'.join(rows)+'\n')
            result=mod.compare(t,a,Decimal(1),Decimal(1))
            self.assertEqual(result['status'],'fail')
            self.assertEqual(result['max_E_interval_us'],'10.001')

if __name__ == '__main__':
    unittest.main()
