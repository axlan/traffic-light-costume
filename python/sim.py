#!/usr/bin/env -S uv run --script
# /// script
# dependencies = [
#   "pynmea2"
# ]
# ///
from pathlib import Path

import pynmea2

SCRIPT_DIR = Path(__file__).resolve().parent
DATA_FILE = SCRIPT_DIR / "../data/walk_10_19.txt"

std = 100

with open(DATA_FILE, encoding='utf-8') as fd:
    for line in fd.readlines():
        try:
            msg = pynmea2.parse(line)
            #print(repr(msg))
            if isinstance(msg, pynmea2.types.talker.RMC):
                print(f'V: {msg.spd_over_grnd:2.2f}, STD: {std:2.1f}')
            if isinstance(msg, pynmea2.types.talker.GST):
                std = msg.std_dev_latitude
        except pynmea2.ParseError as e:
            print('Parse error: {}'.format(e))
            continue
