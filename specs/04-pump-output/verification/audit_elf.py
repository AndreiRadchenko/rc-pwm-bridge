#!/usr/bin/env python3
"""Статичний ISR/section/frame review фінальних ELF; не запускає MCU."""
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[3]
PREFIX = '/tmp/scopa01-idf-tools/tools/riscv32-esp-elf/esp-14.2.0_20260121/riscv32-esp-elf/bin/riscv32-esp-elf-'
V = Path(__file__).resolve().parent

def audit(directory):
    elf=directory/'rc_pwm_bridge.elf'
    table=subprocess.check_output([PREFIX+'objdump','-t','-C',str(elf)],text=True)
    dis=subprocess.check_output([PREFIX+'objdump','-d','-C',str(elf)],text=True)
    symbols={}; by_address={}
    for line in table.splitlines():
        m=re.match(r'^([0-9a-f]+)\s+(.+?)\s+(\.[\w.]+|\*ABS\*)\s+([0-9a-f]+)\s+(.+)$',line)
        if m:
            address,flags,section,size,name=m.groups()
            item=dict(address=address,section=section,size=int(size,16),flags=flags)
            symbols[name]=item
            if 'F' in flags or section=='*ABS*':by_address[int(address,16)]=name
    bodies={}; name=None
    for line in dis.splitlines():
        m=re.match(r'^([0-9a-f]+) <(.+)>:$',line)
        if m:
            name=m[2];bodies[name]=[];by_address[int(m[1],16)]=name
        elif name:bodies[name].append(line)
    roots=['rmt_rx_default_isr','gpio_intr_service','platform::RcCapture::gpio_edge(void*)','platform::RcCapture::rx_done(rmt_channel_t*, rmt_rx_done_event_data_t const*, void*)','systimer_ticks_to_us']
    todo=list(roots);closure={}
    while todo:
        name=todo.pop()
        if name in closure:continue
        assert name in symbols, name
        item=symbols[name].copy();calls=set();indirect=[]
        for line in bodies.get(name,[]):
            pieces=line.split('\t')
            if len(pieces)<3:continue
            asm='\t'.join(pieces[2:]).strip();mnemonic=asm.split()[0] if asm else ''
            if mnemonic=='jalr' and not re.search(r'\b[0-9a-f]{8} <',asm):indirect.append(line.strip())
            if mnemonic in ('jal','jalr','j','call','tail','beq','bne','blt','bge','beqz','bnez','bltz','bgez','bltu','bgeu'):
                m=re.search(r'\b([0-9a-f]{8}) <',asm)
                if m and int(m[1],16) in by_address:
                    target=by_address[int(m[1],16)]
                    if target!=name:calls.add(target)
        item.update(calls=sorted(calls),indirect=indirect)
        frames=[int(x) for x in re.findall(r'\baddi\s+sp,sp,-([0-9]+)', '\n'.join(bodies.get(name,[]))) ]
        item['largest_explicit_frame_bytes']=max(frames,default=0)
        assert item['section'] in ('.iram0.text','*ABS*'),(name,item)
        closure[name]=item;todo.extend(calls)
    # Actual symbol presence, unlike discarded input sections/cross-reference map.
    for forbidden in ('ledc_fade_func_install','ledc_set_duty_and_update','esp_wifi_init','esp_bt_controller_init'):
        assert forbidden not in symbols,forbidden
    data={n:x for n,x in symbols.items() if n.startswith('app_main::') or n in ('gpio_context','systimer_hal')}
    output=dict(elf=str(elf.relative_to(ROOT)),roots=roots,closure=closure,data=data,
                app_frames={n:max([int(x) for x in re.findall(r'\baddi\s+sp,sp,-([0-9]+)','\n'.join(body))],default=0) for n,body in bodies.items() if n.startswith(('platform::RcCapture::step(', 'platform::LocalDiagnostics::step(', 'platform::RcCapture::output_service('))})
    return output

if __name__=='__main__':
    results=[]
    for build in json.loads((V/'build-results-reviewed.json').read_text()):
        assert build['exit']==0
        result=audit(ROOT/build['directory'])
        path=V/('iram-audit-'+build['variant']+'.json');path.write_text(json.dumps(result,indent=2)+'\n')
        results.append(dict(variant=build['variant'],functions=len(result['closure']),artifact=path.name))
    (V/'iram-results.json').write_text(json.dumps(results,indent=2)+'\n')
    print(json.dumps(results,indent=2))
