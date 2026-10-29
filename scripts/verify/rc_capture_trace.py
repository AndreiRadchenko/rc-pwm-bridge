#!/usr/bin/env python3
"""Compare a frozen RC03 bench archive with normalized analyzer edges; no device I/O."""
import argparse
import csv
from decimal import Decimal, InvalidOperation
import hashlib
import json
from pathlib import Path
import sys

class IncompleteTrace(ValueError):
    pass

def read_archive(path):
    rows, header, bad = {}, None, False
    for line in Path(path).read_text().splitlines():
        fields = line.split()
        if len(fields) < 2 or fields[0] != 'rc03-trace':
            continue
        kind = fields[1]
        try:
            if kind == 'H':
                header = None
                rows, bad = {}, False
                if len(fields) == 15:
                    version, count, failed, start, end, active, bd, ed, bf, ef, mr, mp, ml = map(int, fields[2:])
                    if version == 1 and count == 128 and failed in (0,1) and active == 0 and min(start,end,bd,bf,mr,mp,ml) >= 0 and ed >= bd and ef >= bf:
                        header = dict(count=count, failed=failed or ed!=bd or ef!=bf, start=start, end=end)
            elif kind == 'R' and header is not None:
                if len(fields) != 12:
                    bad = True
                    continue
                index, seq, epoch, start, end, width, period, flags, faults, accepted = map(int, fields[2:])
                record = dict(seq=seq, epoch=epoch, start=start, end=end, width=width,
                              period=period, flags=flags, faults=faults, accepted=accepted)
                if min(index, seq, epoch, start, end, width, period, flags, faults, accepted) < 0:
                    bad = True
                if index in rows and rows[index] != record:
                    bad = True
                rows[index] = record
            elif kind == 'E' and header is not None:
                if len(fields) != 3 or int(fields[2]) != header['count']:
                    continue
                if not bad and set(rows) == set(range(header['count'])):
                    records = [rows[i] for i in range(header['count'])]
                    if records[0]['start'] != header['start'] or records[-1]['end'] != header['end']:
                        continue
                    if header['failed']:
                        raise ValueError('collector marked the window failed')
                    return records
        except (ValueError, OverflowError) as exc:
            if 'collector marked' in str(exc):
                raise
            bad = True
    raise IncompleteTrace('no complete, consistent 128-record archive cycle')

def compare(trace, analyzer, u_width, u_interval):
    for u in (u_width, u_interval):
        if not u.is_finite() or u < 0 or u > 1:
            raise IncompleteTrace('unknown or unsuitable uncertainty; require 0 <= U <= 1 us')
    records = read_archive(trace)
    with Path(analyzer).open(newline='') as source:
        reader = csv.DictReader(source)
        if reader.fieldnames != ['pulse_start_ns', 'pulse_end_ns']:
            raise IncompleteTrace('normalize analyzer CSV to pulse_start_ns,pulse_end_ns')
        refs = [(int(x['pulse_start_ns']), int(x['pulse_end_ns'])) for x in reader]
    if len(refs) != len(records):
        raise ValueError('analyzer/trace pulse counts differ')
    if records[0]["seq"] != 1:
        raise ValueError("reference window must start with the first observed rising after reset/LOW")
    errors = []
    max_width = max_interval = Decimal(0)
    for i, (record, (start, end)) in enumerate(zip(records, refs)):
        if not record['flags'] & 256 or end <= start or record['end'] <= record['start'] or record['accepted'] < record['end']:
            raise ValueError('incomplete or unordered pulse')
        if (record['flags'] & 255) not in (1,2,3,4) or record['faults'] != records[0]['faults']:
            raise ValueError('fault or inconsistent candidate status inside accuracy window')
        if i and (start <= refs[i-1][0] or record['start'] <= records[i-1]['start']
                  or record['seq'] != (records[i-1]['seq'] + 2) % (2**32)
                  or record['epoch'] != (records[i-1]['epoch'] + 1) % (2**32)):
            raise ValueError('gap/duplicate/order mismatch; do not fit or discard records')
        ew = abs(Decimal(record['width']) - Decimal(end-start)/1000)
        ei = None
        if i:
            ei = abs(Decimal(record['start']-records[i-1]['start']) - Decimal(start-refs[i-1][0])/1000)
            max_interval = max(max_interval, ei)
        max_width = max(max_width, ew)
        errors.append(dict(index=i, E_width_us=str(ew), E_interval_us=None if ei is None else str(ei)))
    return dict(status='pass' if max_width <= 5 and max_interval <= 10 else 'fail',
                samples=len(records), max_E_width_us=str(max_width), max_E_interval_us=str(max_interval),
                U_width_us=str(u_width), U_interval_us=str(u_interval),
                rule='D54-B: E <= tolerance; U documented separately', errors=errors,
                hashes={str(p):hashlib.sha256(Path(p).read_bytes()).hexdigest() for p in (trace, analyzer)})

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--trace', type=Path, required=True)
    parser.add_argument('--analyzer', type=Path, required=True)
    parser.add_argument('--u-width-us', type=Decimal, required=True)
    parser.add_argument('--u-interval-us', type=Decimal, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        result = compare(args.trace, args.analyzer, args.u_width_us, args.u_interval_us)
    except (IncompleteTrace, OSError, InvalidOperation) as exc:
        result = dict(status='blocked', reason=str(exc))
    except (ValueError, KeyError, TypeError) as exc:
        result = dict(status='fail', reason=str(exc))
    args.report.write_text(json.dumps(result, indent=2) + '\n')
    print(result['status'])
    return {'pass':0, 'fail':1, 'blocked':2}[result['status']]

if __name__ == '__main__':
    sys.exit(main())
