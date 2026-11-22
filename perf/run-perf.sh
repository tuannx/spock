#!/bin/sh
# Re-run spock extracted-kernel perf baseline; compare vs baseline.json.
# Verdict: regression >10% = warn, >25% = block.
set -e
cd "$(dirname "$0")"
CC=${CC:-cc}
$CC -O2 -std=gnu11 -Wall -Wextra -o /tmp/spock_perf_bench bench.c
/tmp/spock_perf_bench | tee /tmp/spock_perf_out.txt
python3 -c '
import json,re
base=json.load(open("baseline.json")); out=open("/tmp/spock_perf_out.txt").read()
verdict="pass"
for name,vals in base["benches"].items():
    m=re.search(r"^"+name+r" mean_ns_per_iter=([\d.]+)",out,re.M)
    if not m: print(name+": MISSING"); verdict="block"; continue
    cur=float(m.group(1)); old=vals["mean_ns_per_iter"]; d=(cur-old)/old*100
    print(f"{name}: baseline {old:.1f} ns -> current {cur:.1f} ns  delta {d:+.1f}%")
    if d>25: verdict="block"
    elif d>10 and verdict=="pass": verdict="warn"
print("verdict:",verdict)
'
