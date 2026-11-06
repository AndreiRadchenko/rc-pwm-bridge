#!/usr/bin/env python3
"""Перевірка заморожених доказів04; не відкриває портів і не керує пристроєм."""
import argparse
from bisect import bisect_left, bisect_right
import csv
from decimal import Decimal, localcontext
import hashlib
import json
from pathlib import Path
import re
import sys

class Blocked(ValueError):
    pass
class Failed(ValueError):
    pass

def need(condition, message, error=Blocked):
    if not condition:
        raise error(message)

def integer(value, name, bits=64):
    need(type(value) is int and 0 <= value < (1 << bits), 'невалідне ціле поле: '+name)
    return value

def hexes(tokens):
    need(all(re.fullmatch('[0-9a-fA-F]+', x) for x in tokens), 'невалідний hexadecimal token')
    return [int(x,16) for x in tokens]

def digest(values):
    h=0xcbf29ce484222325
    for value,width in values:
        integer(value,'canonical value',width*8)
        for x in value.to_bytes(width,'big'):
            h=((h ^ x)*0x100000001b3)&((1<<64)-1)
    return h

def archives(text, metadata):
    identity=(integer(metadata.get('build_fingerprint'),'build_fingerprint'),integer(metadata.get('boot'),'boot'))
    current={}; result={}; live=[]
    for framed in text.splitlines(keepends=True):
        if not framed.endswith("\n"):continue
        line=framed.rstrip("\r\n")
        if not line.startswith(('rc04-out','rc04-win')):
            continue
        fields=line.split();marker=fields[0]
        try:
            need(len(fields)>=8,'обірваний archive frame')
            kind=fields[1]
            need(fields[-3].startswith('cf=') and fields[-2].startswith('of=') and fields[-1].startswith('pf='),'відсутній live summary')
            cf,of,pf=hexes([x.split('=',1)[1] for x in fields[-3:]])
            v,fp,boot=hexes(fields[2:5]);data=hexes(fields[5:-3])
        except Blocked:
            # A zero-wait short write may leave a line fragment before the next
            # leading newline. Discard its whole cycle; require a later H..E.
            current.pop(marker,None)
            continue
        need(cf<=0x7ff and of<=0xff and pf<=1,'невідомі live fault bits')
        live.append((cf,of,pf))
        need(v==1 and (fp,boot)==identity,'mixed boot/build/version archive')
        frame=(kind,data)
        if kind=='H':
            current[marker]=[frame]
        elif marker in current:
            # A write missing only its newline is completed by the next frame's
            # leading newline; its exact retry is a transport replay.
            if frame!=current[marker][-1]:current[marker].append(frame)
        # A partial reconnect prefix is never used as evidence; a full cycle is required.
        if kind!='E' or marker not in current:
            continue
        cycle=current.pop(marker)
        if marker in result:
            need(cycle==result[marker],'змінився immutable archive між циклами')
        result[marker]=cycle
    return result,live

def output_cycle(cycle):
    need(cycle and cycle[0][0]=='H' and cycle[-1][0]=='E','немає повного rc04-out cycle')
    h=cycle[0][1];need(len(h)==12,'rc04-out H schema')
    scenario,t0,count,first,last,frozen,overflow,firstop,firststatus,fb,fbop,fbstatus=h
    integer(t0,'output t0')
    need(1<=scenario<=3 and count<=64 and frozen==1 and overflow<=1 and fb<=1,'rc04-out header bounds')
    need(firstop<=8 and fbop<=8 and firststatus<=0xffffffff and fbstatus<=0xffffffff,'rc04-out status schema')
    need((count==0 and first==last==0) or (first==1 and last==count),'rc04-out sequence bounds')
    records=[];canonical=[]
    for index,(kind,r) in enumerate(cycle[1:-1]):
        need(kind=='R' and len(r)==10,'rc04-out R schema')
        idx,seq,op,duty,applied,reasons,status,before,after,deadline=r
        need(idx==index and seq==index+1 and 1<=op<=11 and duty<=100 and applied<=3 and reasons<=255 and status<=0xffffffff,'rc04-out record bounds/order')
        for value in (before,after,deadline):integer(value,'API timestamp')
        canonical.extend(zip(r[1:],[4,1,1,1,1,4,8,8,8]))
        records.append(r)
    need(len(records)==count and cycle[-1][1]==[count,digest(canonical)],'rc04-out count/digest mismatch')
    need(not overflow,'output evidence overflow',Failed)
    need(not firststatus and not fbstatus,'SDK error у frozen archive',Failed)
    return {'scenario':scenario,'t0_us':t0,'records':records}

def api_metrics(out, expected_fault):
    observations=[];previous=0
    for r in out['records']:
        _,_,op,_,_,reason,status,before,after,deadline=r
        need(before>=previous and after>=before,'регресія archive clock',Failed)
        previous=after
        need(status==0,'невдалий SDK operation',Failed)
        allowed=4 if expected_fault else 0
        need(reason & ~allowed==0,'неприпустима причина відмови',Failed)
        if op not in (2,3,4,5,6,7,8):
            continue
        if deadline:
            need(before>=deadline,'операція до її deadline',Failed)
            budget=1000 if reason else 2000
            need(after-deadline<=budget,'MCU API deadline порушений',Failed)
            observations.append({'seq':r[1],'kind':op,'elapsed_us':after-deadline,'budget_us':budget})
    need(bool(observations),'немає API brackets для заявленої метрики')
    return observations

def window_cycle(cycle, out):
    need([k for k,_ in cycle]==list('HBWTE'),'rc04-win frame order')
    h,base,win,timing,end=[r for _,r in cycle]
    need(len(h)==13 and len(base)==14 and len(win)==14 and len(timing)==3 and len(end)==2,'rc04-win schema')
    start,finish,startcp,endcp,finalized,seq,epoch,flags,hs,he,hp,hf,count=h
    for value in h[:5]:integer(value,'window timestamp')
    integer(seq,'watermark seq',32);integer(epoch,'watermark epoch',32)
    need(flags<=255 and all(x<=1 for x in (hs,he,hp,hf)) and count==3,'rc04-win flags/presence')
    need(finish-start==601000000 and start==out['t0_us']+5000000 and finish==out['t0_us']+606000000,'D62 bounds mismatch')
    need(hs==he==hp==1,'неповний window checkpoint/sample evidence')
    need(startcp>=start and startcp<=start+1000 and endcp>=finish and finalized>=finish+6000,'невірні checkpoint bounds')
    need(finalized<=finish+17000 and endcp<=finalized,'closure guard порушений',Failed)
    need(flags==128,'window incomplete/failed flags',Failed)
    for value in base+win[:12]:integer(value,'window count/sequence',32)
    for value in win[12:]+timing:integer(value,'window time')
    need(hf==1 or timing[2]==0,'fault timing без presence')
    need(win[0]>=2 and win[0]==win[1]+win[2]+win[3],'window counters mismatch')
    need(all(x==0 for x in win[3:8]),'window loss/fault/order evidence',Failed)
    need(timing[0]<=1000 and timing[1]<=10000 and (not hf or timing[2]<=110000),'capture budget порушений',Failed)
    canonical=list(zip(base,[4]*14))+list(zip(win,[4]*12+[8]*2))+list(zip(timing,[8]*3))
    need(end==[3,digest(canonical)],'rc04-win digest mismatch')
    need(win[12]<win[13] and start<=win[12]<finish and start<=win[13]<finish,'window sample bounds')
    return {'header':h,'counts':win[:8],'first_seq':win[8],'last_seq':win[9],'first_epoch':win[10],'last_epoch':win[11],'first_end_us':win[12],'last_end_us':win[13]}

def read_edges(path, m):
    need(type(m.get('schema_version')) is int and m['schema_version']==1,'metadata schema_version')
    begin=integer(m.get('acquisition_start_ns'),'acquisition_start_ns')
    end=integer(m.get('acquisition_end_ns'),'acquisition_end_ns');need(end>begin,'acquisition bounds')
    need(m.get('gap') is False and m.get('overflow') is False,'невідомі gaps/overflow')
    need(integer(m.get('digital_sampling_hz'),'digital_sampling_hz')>=10000000,'недостатній digital sampling')
    need(0<integer(m.get('opposite_pulse_resolution_ns'),'opposite_pulse_resolution_ns')<=1000,'недостатній glitch method')
    need(bool(m.get('instrument')) and bool(m.get('calibration')) and bool(m.get('normalization_provenance')),'відсутня method provenance')
    initial=m.get('initial_levels');need(isinstance(initial,dict) and 'gpio0' in initial,'initial levels missing')
    windows=m.get('observed_windows');need(isinstance(windows,list) and any(type(w) is list and len(w)==2 and w[0]<=begin and w[1]>=end for w in windows),'немає повного observable window')
    for channel,level in initial.items():need(channel in ('gpio0','gpio3') and type(level) is int and level in (0,1),'невірний initial state')
    result={ch:[] for ch in initial};state=dict(initial);last={ch:begin for ch in initial}
    with path.open(newline='') as f:
        reader=csv.DictReader(f);need(reader.fieldnames==['time_ns','channel','level'],'CSV schema')
        for row in reader:
            try:t=int(row['time_ns']);level=int(row['level'])
            except (ValueError,TypeError):raise Blocked('невалідний edge integer')
            ch=row['channel'];need(ch in result and level in (0,1) and begin<=t<=end and t>last[ch] and state[ch]!=level,'edge bounds/order/alternation')
            result[ch].append((t,level));last[ch]=t;state[ch]=level
    return result

def segments(edges, m, channel, start, end):
    need(channel in edges and m['acquisition_start_ns']<=start<end<=m['acquisition_end_ns'],'немає потрібного acquisition span/channel')
    series=edges[channel]
    first=bisect_right(series,(start,1));last=bisect_left(series,(end,-1))
    level=series[first-1][1] if first else m['initial_levels'][channel]
    previous=start;out=[]
    for t,after in series[first:last]+[(end,None)]:
        if previous<t:out.append((previous,t,level))
        previous=t;level=after
    return out

def decimal(value, name):
    need(type(value) in (int,float,str) and not isinstance(value,bool),'невалідний number: '+name)
    try:d=Decimal(str(value))
    except Exception:raise Blocked('невалідний decimal: '+name)
    need(d.is_finite(),'nonfinite: '+name);return d

def waveform(edges,m,out):
    plateaus=m.get('plateaus');need(isinstance(plateaus,list) and [p.get('duty') for p in plateaus]==[0,25,40,50,80,100],'потрібні шість фізичних plateau')
    nominal=100000000000 if out['scenario']==2 else 10000000000
    u_f=decimal(m.get('frequency_uncertainty_hz'),'frequency_uncertainty_hz');u_d=decimal(m.get('duty_uncertainty_pp'),'duty_uncertainty_pp');u_t=decimal(m.get('interval_uncertainty_ns'),'interval_uncertainty_ns')
    need(0<=u_f<=Decimal('.1') and 0<=u_d<=Decimal('.02') and 0<=u_t<=Decimal(nominal)*Decimal('.0001')+100000,'непридатна U для waveform')
    reports=[];previous=None
    for p in plateaus:
        begin=integer(p.get('start_ns'),'plateau start');end=integer(p.get('end_ns'),'plateau end');duty=p['duty']
        need(previous is None or previous==begin,'незіставлені plateau boundaries');previous=end
        need(abs(end-begin-nominal)<=Decimal(nominal)*Decimal('.001')+2000000,'physical plateau interval error',Failed)
        parts=segments(edges,m,'gpio0',begin,end)
        if duty in (0,100):
            need(all(level==int(duty==100) or z-a<1000 for a,z,level in parts),'static endpoint порушений',Failed)
            reports.append({'duty':duty,'static':True});continue
        lo=begin+1000000000;hi=end-1000000000
        rising=[t for t,level in edges['gpio0'] if level==1 and lo<=t<=hi]
        need(len(rising)>=2 and rising[0]-lo<=1002000 and hi-rising[-1]<=1002000,'неповне PWM stable window')
        periods=[]
        for a,z in zip(rising,rising[1:]):
            high=sum(y-x for x,y,level in segments(edges,m,'gpio0',a,z) if level)
            with localcontext() as ctx:
                ctx.prec=50;f=Decimal(1000000000)/Decimal(z-a);d=Decimal(high)*100/Decimal(z-a)
                ef=abs(f-1000);ed=abs(d-duty)
                need(ef<=1 and ed<=Decimal('.1'),'per-cycle frequency/duty поза допуском',Failed)
            periods.append({'start_ns':a,'frequency_error_hz':str(ef),'duty_error_pp':str(ed)})
        reports.append({'duty':duty,'periods':periods})
    return reports

def physical_fault(edges,m):
    start=integer(m.get('fault_start_ns'),'fault_start_ns')
    need('gpio3' in edges and any(t==start for t,_ in edges['gpio3']),'немає source fault edge')
    need(m.get('fault_before_duty') in (25,40,50,80,100),'немає active nonzero precondition')
    active_start=integer(m.get('active_start_ns'),'active_start_ns');need(active_start<start,'active precondition span')
    active=segments(edges,m,'gpio0',active_start,start)
    need(any(level==1 and z-a>=1000 for a,z,level in active),'не спостережено active output')
    need(not any(start<t<start+112000000 for t,_ in edges['gpio3']),'непідтверджений stuck episode')
    parts=segments(edges,m,'gpio0',start,m['acquisition_end_ns'])
    opposite=[(a,z) for a,z,level in parts if level==1 and z-a>=1000]
    cut=opposite[-1][1] if opposite else start
    need(m['acquisition_end_ns']-cut>=10000000000,'неповний terminal LOW guard')
    need(cut-start<=112000000,'stuck→terminal LOW latency',Failed)
    return {'source_ns':start,'cut_ns':cut,'latency_ns':cut-start}

def window_physical(w, edges, m):
    need('gpio3' in edges,'немає GPIO3 source trace')
    source=[t for t,level in edges['gpio3'] if level==0]
    mapping=m.get('window_mapping');need(isinstance(mapping,dict),'немає window/source mapping')
    i=integer(mapping.get('first_falling_index'),'first_falling_index');j=integer(mapping.get('last_falling_index'),'last_falling_index')
    need(i<j<len(source),'window source index bounds')
    for key in ('first_seq','last_seq','first_epoch','last_epoch','first_end_us','last_end_us'):
        need(mapping.get(key)==w[key],'window provenance mismatch: '+key)
    need(bool(mapping.get('anchor_provenance')),'unknown source anchor')
    need(j-i+1==w['counts'][0],'source/window count mismatch',Failed)
    uw=integer(m.get('capture_width_uncertainty_ns'),'capture_width_uncertainty_ns')
    ui=integer(m.get('capture_interval_uncertainty_ns'),'capture_interval_uncertainty_ns')
    need(uw<=1000 and ui<=1000,'capture uncertainty непридатна')
    pulses={};rise=None;previous_rise=None;period=None
    for t,level in edges['gpio3']:
        if level:
            period=t-previous_rise if previous_rise is not None else None
            previous_rise=t;rise=t
        elif rise is not None:pulses[t]=(t-rise,period)
    valid=invalid=0
    for falling in source[i:j+1]:
        need(falling in pulses,'невідомий source rising boundary')
        width,period=pulses[falling];need(period is not None,'невідомий source period')
        # Exact boundary stimuli belong to host tests (D53). Near-boundary physical
        # classification is inconclusive within the accepted capture E and method U.
        dw=5000+uw;dp=10000+ui
        width_good=900000+dw<=width<=2100000-dw
        width_bad=width<900000-dw or width>2100000+dw
        period_good=18000000+dp<=period<=22000000-dp
        period_bad=period<18000000-dp or period>22000000+dp
        need(width_good or width_bad,'неоднозначна source width біля межі')
        need(period_good or period_bad,'неоднозначний source period біля межі')
        if width_good and period_good:valid+=1
        else:invalid+=1
    need(valid==w['counts'][1] and invalid==w['counts'][2],'source valid/invalid count mismatch',Failed)
    u=integer(m.get('duration_uncertainty_ns'),'duration_uncertainty_ns')
    observed=source[j]-source[i];lower=observed-u
    need(lower>=600000000000,'недостатня фізична тривалість за D62',Failed)
    return {'observed_duration_ns':observed,'duration_uncertainty_ns':u,'covered_duration_lower_ns':lower,'source_count':j-i+1}

def voltage(m):
    a=m.get('analog');need(isinstance(a,dict),'немає analog voltage records')
    need(bool(a.get('provenance')) and bool(a.get('calibration')),'немає analog provenance')
    need(integer(a.get('bandwidth_hz'),'analog bandwidth')>=10000000 and integer(a.get('sampling_hz'),'analog sampling')>=20000000,'непридатний analog method')
    u=decimal(a.get('uncertainty_v'),'voltage U');need(0<=u<=Decimal('.05'),'voltage U непридатне')
    records=a.get('measurements');need(isinstance(records,list) and [r.get('duty') for r in records]==[0,25,40,50,80,100],'неповні analog records')
    for r in records:
        if r['duty']!=100:need(decimal(r.get('low_v'),'low_v')+u<=Decimal('.8'),'LOW voltage',Failed)
        if r['duty']!=0:
            high=decimal(r.get('high_v'),'high_v');need(high-u>=Decimal('2.4') and high+u<=Decimal('3.6'),'HIGH voltage',Failed)
    return records

def verify(archive_path,edges_path,metadata):
    need(isinstance(metadata,dict) and type(metadata.get('schema_version')) is int and metadata['schema_version']==1,'metadata schema_version')
    requested=metadata.get('metrics');need(isinstance(requested,list) and requested and all(isinstance(x,str) for x in requested) and len(set(requested))==len(requested) and set(requested)<={'api','waveform','fault','window','voltage','startup'},'явно задайте metrics')
    need(not metadata.get('cross_domain_latency_requested',False),'cross-domain latency без незалежного anchor не підтримується')
    cycles,live=archives(archive_path.read_text(),metadata)
    expected_fault='fault' in requested
    for cf,of,pf in live:
        need(not pf,'postfreeze_invalid',Failed)
        need((of & ~(4 if expected_fault else 0))==0,'live output failure',Failed)
        need(not cf or expected_fault,'live capture fault',Failed)
    result={};out=None
    if set(requested)&{'api','waveform','window'}:
        need('rc04-out' in cycles,'немає повного output archive');out=output_cycle(cycles['rc04-out'])
    if 'api' in requested:result['api']=api_metrics(out,expected_fault)
    edges=read_edges(edges_path,metadata) if set(requested)&{'waveform','fault','window','startup'} else None
    if 'waveform' in requested:result['waveform']=waveform(edges,metadata,out)
    if 'fault' in requested:result['fault']=physical_fault(edges,metadata)
    if 'window' in requested:
        need('rc04-win' in cycles,'немає повного window archive');w=window_cycle(cycles['rc04-win'],out);result['window']=window_physical(w,edges,metadata)
    if 'voltage' in requested:result['voltage']=voltage(metadata)
    if 'startup' in requested:
        spans=metadata.get('startup_windows');need(isinstance(spans,list) and spans,'немає startup windows')
        for a,z in spans:
            need(all(level==0 or y-x<1000 for x,y,level in segments(edges,metadata,'gpio0',a,z)),'startup HIGH glitch',Failed)
        result['startup']={'windows':spans}
    return result

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('archive','edges','metadata','report'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();report={'status':'blocked','metrics':{}};code=2
    try:
        report['metrics']=verify(a.archive,a.edges,json.loads(a.metadata.read_text()));report['status']='pass';code=0
    except Failed as exc:report['status']='fail';report['reason']=str(exc);code=1
    except (Blocked,OSError,ValueError,KeyError,TypeError,OverflowError) as exc:report['reason']=str(exc)
    report['hashes']={str(x):hashlib.sha256(x.read_bytes()).hexdigest() for x in (a.archive,a.edges,a.metadata) if x.is_file()}
    a.report.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print(report['status']+(': '+report['reason'] if 'reason' in report else ''));return code
if __name__=='__main__':sys.exit(main())
