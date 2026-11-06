import copy
import csv
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('pump_trace',ROOT/'scripts/verify/pump_output_trace.py')
p=importlib.util.module_from_spec(spec);spec.loader.exec_module(p)

def fnv(data):
    h=0xcbf29ce484222325
    for b in data:h=((h^b)*0x100000001b3)&0xffffffffffffffff
    return h

def line(marker,kind,values):
    return marker+' '+kind+' 1 1 2 '+' '.join(format(x,'x') for x in values)+' cf=0 of=0 pf=0\n'

def archive(scenario=1,delay=20,status=0):
    deadline=105000000 if scenario==2 else 15000000
    records=[[1,1,0,1,0,0,0,0,0],[2,3,25,1,0,status,deadline,deadline+delay,deadline]]
    raw=b''.join(struct.pack('>IBBBBIQQQ',*r) for r in records)
    h=[scenario,0,len(records),1,len(records),1,0,3 if status else 0,status,0,0,0]
    return line('rc04-out','H',h)+''.join(line('rc04-out','R',[i]+r) for i,r in enumerate(records))+line('rc04-out','E',[len(records),fnv(raw)])

def method(metrics):
    return dict(schema_version=1,metrics=metrics,build_fingerprint=1,boot=2,
        acquisition_start_ns=0,acquisition_end_ns=75000000000,initial_levels={'gpio0':0},
        observed_windows=[[0,75000000000]],gap=False,overflow=False,digital_sampling_hz=10000000,
        opposite_pulse_resolution_ns=1000,instrument='synthetic host fixture',calibration='synthetic',
        normalization_provenance='generated integer ns',frequency_uncertainty_hz='.1',
        duty_uncertainty_pp='.02',interval_uncertainty_ns=1000000)

class PumpTraceTests(unittest.TestCase):
    def invoke(self,text,edges,m):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);a=root/'archive';e=root/'edges.csv';a.write_text(text)
            with e.open('w',newline='') as f:
                w=csv.writer(f);w.writerow(['time_ns','channel','level']);w.writerows(sorted(edges))
            return p.verify(a,e,m)

    def test_fnv_known_vector(self):
        self.assertEqual(fnv(b'hello'),0xa430d84680aabd0b)
        self.assertEqual(p.digest([(0x68656c6c6f,5)]),0xa430d84680aabd0b)

    def test_api_archive_reconnect_integrity_and_failures(self):
        m=method(['api']);text=archive()
        self.assertIn('api',self.invoke(text+text+'rc04-out H 1 1 2',[],m))
        self.assertIn('api',self.invoke('rc04-out R 1 1\n'+text,[],m))
        self.assertIn('api',self.invoke(text.split('rc04-out E')[0]+'rc04-out R 1\n'+text,[],m))
        replay=text.splitlines(keepends=True)
        self.assertIn('api',self.invoke(''.join(replay[:2]+[replay[1]]+replay[2:]),[],m))
        with self.assertRaises(p.Blocked):self.invoke(text.replace(' E 1 1 2 2 ',' E 1 1 2 3 '),[],m)
        with self.assertRaises(p.Blocked):self.invoke(text.replace(' R 1 1 2 0 ',' R 1 1 3 0 '),[],m)
        with self.assertRaises(p.Failed):self.invoke(archive(delay=2001),[],m)
        with self.assertRaises(p.Failed):self.invoke(archive(status=0xfffffff9),[],m)
        with self.assertRaises(p.Blocked):self.invoke(text.split('rc04-out E')[0],[],m)
        m['cross_domain_latency_requested']=True
        with self.assertRaises(p.Blocked):self.invoke(text,[],m)

    def test_static_empty_trace_requires_initial_state_and_bounds(self):
        m=method(['startup']);m['startup_windows']=[[1000000,12000000000]]
        self.assertIn('startup',self.invoke('',[],m))
        bad=copy.deepcopy(m);del bad['initial_levels']
        with self.assertRaises(p.Blocked):self.invoke('',[],bad)
        bad=copy.deepcopy(m);bad['gap']=True
        with self.assertRaises(p.Blocked):self.invoke('',[],bad)
        self.invoke('',[(5000000000,'gpio0',1),(5000000999,'gpio0',0)],m)
        with self.assertRaises(p.Failed):self.invoke('',[(5000000000,'gpio0',1),(5000001000,'gpio0',0)],m)

    def test_terminal_low_112ms_boundary_and_no_repeat(self):
        m=method(['fault']);m.update(initial_levels={'gpio0':1,'gpio3':0},fault_start_ns=1000000000,active_start_ns=1000000,fault_before_duty=100)
        edges=[(1000000000,'gpio3',1),(1112000000,'gpio0',0)]
        self.assertEqual(self.invoke('',edges,m)['fault']['latency_ns'],112000000)
        with self.assertRaises(p.Failed):self.invoke('',[(1000000000,'gpio3',1),(1112000001,'gpio0',0)],m)
        with self.assertRaises(p.Failed):self.invoke('',edges+[(3000000000,'gpio0',1),(3000001000,'gpio0',0)],m)

    @staticmethod
    def waveform():
        edges=[];plateaus=[]
        for i,duty in enumerate([0,25,40,50,80,100]):
            start=5000000000+i*10000000000;end=start+10000000000
            plateaus.append(dict(start_ns=start,end_ns=end,duty=duty))
            if 0<duty<100:
                for t in range(start,end,1000000):edges.extend([(t,'gpio0',1),(t+10000*duty,'gpio0',0)])
            elif duty==100:edges.extend([(start,'gpio0',1),(end,'gpio0',0)])
        return edges,plateaus

    def test_every_period_not_average_and_duty_boundary(self):
        edges,plateaus=self.waveform();m=method(['waveform']);m['plateaus']=plateaus
        self.assertEqual(len(self.invoke(archive(),edges,m)['waveform']),6)
        at=17000250000
        exact=[(t+1000 if t==at else t,ch,level) for t,ch,level in edges]
        self.invoke(archive(),exact,m)
        bad=[(t+1001 if t==at else t,ch,level) for t,ch,level in edges]
        with self.assertRaises(p.Failed):self.invoke(archive(),bad,m)
        m['frequency_uncertainty_hz']='.100001'
        with self.assertRaises(p.Blocked):self.invoke(archive(),edges,m)

    def test_analog_not_inferred_from_logic_levels(self):
        m=method(['voltage'])
        with self.assertRaises(p.Blocked):self.invoke('',[],m)
        m['analog']=dict(provenance='synthetic',calibration='synthetic',bandwidth_hz=10000000,sampling_hz=20000000,uncertainty_v='.05',measurements=[dict(duty=d,low_v='.75',high_v='2.45') for d in [0,25,40,50,80,100]])
        self.invoke('',[],m)
        m['analog']['measurements'][1]['high_v']='2.44999'
        with self.assertRaises(p.Failed):self.invoke('',[],m)

    @staticmethod
    def stress():
        edges=[]
        for n in range(30800):
            t=n*20000000+500000;edges.extend([(t,'gpio3',1),(t+1000000,'gpio3',0)])
        first,last=250,30299;count=last-first+1
        first_us=first*20000+1500;last_us=last*20000+1500
        h=[5000000,606000000,5000000,606006000,606006000,61601,30801,128,1,1,1,0,3]
        base=[0]*14;win=[count,count,0,0,0,0,0,0,2*first+1,2*last+1,first+1,last+1,first_us,last_us];timing=[100,100,0]
        raw=struct.pack('>14I',*base)+struct.pack('>12I2Q',*win)+struct.pack('>3Q',*timing)
        text=archive(2)+line('rc04-win','H',h)+line('rc04-win','B',base)+line('rc04-win','W',win)+line('rc04-win','T',timing)+line('rc04-win','E',[3,fnv(raw)])
        m=method(['window']);m.update(acquisition_end_ns=616000000000,observed_windows=[[0,616000000000]],initial_levels={'gpio0':0,'gpio3':0},duration_uncertainty_ns=60000000,capture_width_uncertainty_ns=1000,capture_interval_uncertainty_ns=1000)
        m['window_mapping']=dict(first_falling_index=first,last_falling_index=last,first_seq=win[8],last_seq=win[9],first_epoch=win[10],last_epoch=win[11],first_end_us=first_us,last_end_us=last_us,anchor_provenance='synthetic known first rising')
        return text,edges,m

    def test_D62_duration_lower_bound_and_presence(self):
        text,edges,m=self.stress();r=self.invoke(text,edges,m)['window'];self.assertGreaterEqual(r['covered_duration_lower_ns'],600000000000)
        observed=r['observed_duration_ns'];m['duration_uncertainty_ns']=observed-600000000000
        self.invoke(text,edges,m)
        m['duration_uncertainty_ns']+=1
        with self.assertRaises(p.Failed):self.invoke(text,edges,m)
        m.pop('duration_uncertainty_ns')
        with self.assertRaises(p.Blocked):self.invoke(text,edges,m)
        m['duration_uncertainty_ns']=60000000
        with self.assertRaises(p.Failed):self.invoke(text.replace('pf=0','pf=1'),edges,m)
        m['window_mapping']['last_falling_index']-=1
        with self.assertRaises(p.Failed):self.invoke(text,edges,m)

    def test_cli_exit_codes(self):
        from unittest.mock import patch
        for text,expected in [(archive(),0),(archive(status=7),1),('',2)]:
            with tempfile.TemporaryDirectory() as d:
                root=Path(d);(root/'a').write_text(text);(root/'e').write_text('time_ns,channel,level\n');(root/'m').write_text(json.dumps(method(['api'])))
                with patch.object(p.sys,'argv',['trace','--archive',str(root/'a'),'--edges',str(root/'e'),'--metadata',str(root/'m'),'--report',str(root/'r')]):self.assertEqual(p.main(),expected)
                self.assertEqual(json.loads((root/'r').read_text())['status'],['pass','fail','blocked'][expected])

if __name__=='__main__':unittest.main()
