#!/usr/bin/env python3
import re
import sys
from pathlib import Path

if len(sys.argv) != 2:
    print("Uso: python3 scripts/check_acceptance_log.py <monitor.log>", file=sys.stderr)
    raise SystemExit(2)

text = Path(sys.argv[1]).read_text(encoding="utf-8", errors="replace")
if not re.search(r"API ERROR SELF-TEST.*result=PASS", text):
    print("FAIL: no se encontro el self-test de errores aprobado.", file=sys.stderr)
    raise SystemExit(1)

pattern = re.compile(
    r"RX sender=(\d+) seq=(\d+) dt=(\d+) ms RSSI=(-?\d+) dBm SNR=(-?\d+) dB"
)
sessions = []
current = []
previous_sequence = None
for match in pattern.finditer(text):
    record = tuple(int(value) for value in match.groups())
    sequence = record[1]
    if previous_sequence is not None and sequence <= 3 and sequence < previous_sequence:
        if current:
            sessions.append(current)
        current = []
    current.append(record)
    previous_sequence = sequence
if current:
    sessions.append(current)

if not sessions:
    print("FAIL: no se encontraron tramas RX parseables.", file=sys.stderr)
    raise SystemExit(1)

records = sessions[-1]
unique = []
seen = set()
for record in records:
    sequence = record[1]
    if sequence not in seen:
        unique.append(record)
        seen.add(sequence)

missing = sum(max(0, current[1] - previous[1] - 1) for previous, current in zip(unique, unique[1:]))
duplicates = len(records) - len(unique)
received = len(unique)
expected = received + missing
delivery = 100.0 * received / expected if expected else 0.0
rssi = [record[3] for record in unique]
snr = [record[4] for record in unique]
intervals = [record[2] for record in unique if record[2] > 0]
interval_mean = round(sum(intervals) / len(intervals)) if intervals else 0

print(
    f"ACCEPTANCE parsed_from_log sessions={len(sessions)} samples={received} "
    f"missing={missing} duplicates={duplicates} delivery={delivery:.1f}% "
    f"RSSI mean/min/max={round(sum(rssi)/len(rssi))}/{min(rssi)}/{max(rssi)} dBm "
    f"SNR mean/min/max={round(sum(snr)/len(snr))}/{min(snr)}/{max(snr)} dB "
    f"DT mean/min/max={interval_mean}/{min(intervals) if intervals else 0}/"
    f"{max(intervals) if intervals else 0} ms"
)

failures = []
if received < 100:
    failures.append(f"se requieren 100 tramas de la sesion actual y hay {received}")
if delivery < 90.0:
    failures.append(f"delivery {delivery:.1f}% menor a 90.0%")
if not 2500 <= interval_mean <= 3500:
    failures.append(f"DT medio {interval_mean} ms fuera de 2500..3500 ms")
if failures:
    print("FAIL: " + "; ".join(failures), file=sys.stderr)
    raise SystemExit(1)
print("PASS: evidencia IDS beacon aceptada.")
