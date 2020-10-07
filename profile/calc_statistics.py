#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: sts=4 sw=4 et
# Description: profiling stat calculator
# Author: Tony Pavlov (untoxa)
# SPDX-License-Identifier: MIT

import sys
import math

from BGB_toolkit import load_noi, calc_profiling_stats

def calc_percentile(data, percentile):
    size = len(data)
    return 0 if size == 0 else sorted(data)[int(math.ceil((size * percentile) / 100)) - 1]

if len(sys.argv) < 2:
    exit('USAGE: profile.py <profiler_log> <symbols>')

# loading symbols
symbols = load_noi(sys.argv[2]) if len(sys.argv) > 2 else {}
   
# calculating the stat
stat = calc_profiling_stats(sys.argv[1], double_speed=False, all_data=True, symbols=symbols)

# pretty-print the stat
for k,v in stat.items():
    print('{:50s}  MIN:{:-8d} AVG:{:-10.2f} 95P:{:-8d} MAX:{:-8d} TOTAL: {:s} NCALLS: {:d}'.format(k, v['min'], v['totalclk'] / v['ncalls'], calc_percentile(v['data'], 95), v['max'], '0x{:016x}'.format(v['totalclk']), v['ncalls']))
