# perf — spock

Standalone perf harness for the pure-C kernels extracted from spock.
The extension itself is **not** built here: on the measurement machine
(2026-11-21) `which pg_config` exits 1, `/usr/lib/postgresql` and
`~/.pgrx` are absent, and every `src/*.c` includes `postgres.h`.
Replication/apply/output-plugin paths need a live PostgreSQL cluster
(see `tests/regress`, `tests/tap`) and are out of scope for this baseline.

## Kernels (`bench.c`)

| bench | source |
|---|---|
| `lsn_parse_format` | pg_lsn_in/out algorithm used by `str_to_lsn()` in `src/spock_common.c` |
| `json_escape` | `escape_json()` semantics used in `src/spock_change_log.c` |
| `native_int32_codec` | pqformat big-endian int32 primitives used by `src/spock_proto_native.c` |

## Run

```sh
perf/run-perf.sh   # rebuilds, reruns, prints delta % vs baseline.json + verdict
```

Verdict thresholds: regression >10% = warn, >25% = block.
Baseline in `baseline.json` is the median of 6 run means taken 2026-11-21
on a heavily loaded shared VM — expect large run-to-run variance; compare
medians of several reruns, not a single run.

## When a PostgreSQL server is available

Build with PGXS (`make USE_PGXS=1`), then benchmark end-to-end logical
replication throughput (rows/sec through the output plugin + apply worker)
and `tests/regress` wall time; add those as separate benches alongside
these kernel numbers rather than replacing them.
