#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: sts=4 sw=4 et
# Description: BGB toolkit
# Author: Tony Pavlov (untoxa)
# SPDX-License-Identifier: MIT

import sys
from struct import unpack
from array import array

def load_noi(filename):
    symbols = {}
    with open(filename) as f:
        line = f.readline()
        while line:
            decoded_line = [x.strip() for x in line.split(' ')]
            if decoded_line[0] == 'DEF':
                symbols[int(decoded_line[2], 16)] = decoded_line[1]
            line = f.readline()
    return symbols

def load_nogmb_map(filename):
    symbols = {}
    with open(filename) as f:
        current_bank = 0
        skip = True
        line = f.readline()
        while line:
            decoded_line = [x.strip() for x in line.split('\t')]
            if decoded_line[0][:4].lower() == 'area':
                decoded_area = [x.strip() for x in decoded_line[0].split(' ')]
                areaname = decoded_area[1]
                if areaname[:6].lower() == '_code_':
                    decoded_area = [x.strip() for x in areaname.split('_')]
                    current_bank = int(decoded_area[2]) if len(decoded_area) >= 3 else 0
                else:
                    current_bank = 0
                skip = False
            elif len(decoded_line[0]) == 0:
                pass
            else:    
                skip = True
            if not skip:
                if len(decoded_line[0]) == 0 and len(decoded_line[1]) == 0:
                    if len(decoded_line) >= 4:
                        addr = (current_bank << 16) | int(decoded_line[3], 16)
                        symbols[addr] = decoded_line[2]
            line = f.readline()
    return symbols

def load_nogmb_symbols(filename, **kv):
    resolve_banks = kv.get('resolve_banks', False)
    symbols = {}
    with open(filename) as f:
        current_bank = 0
        line = f.readline()
        while line:
            if line[0] == ';':
                if resolve_banks and line[:14].lower() == '; area: _code_':
                    decoded_line = [x.strip() for x in line[9:].split('_')]
                    current_bank = int(decoded_line[1]) if len(decoded_line) > 1 else 0
                else:
                    current_bank = 0
            else:
                decoded_line = [x.strip() for x in line.split(' ')]
                addr = decoded_line[0].split(':')
                addr = (current_bank << 16) | int(addr[1], 16)
                symbols[addr] = decoded_line[1]
            line = f.readline()
    return symbols
  
def calc_profiling_stats(filename, **kv):
    double_speed = kv.get('double_speed', False)
    symbols = kv.get('symbols', {})
    extended = kv.get('all_data', False)
    stat = {}  
    stack = [] 
    path =  []
    profile_addend = 18 if double_speed else 36
    with open(filename) as f:
        line = f.readline()
        while line:
            decoded_line = [x.strip() for x in line.split(',')]
            if decoded_line[0] == "PROFILE":
                if int(decoded_line[3], 16) == 3:
                    caller = ((int(decoded_line[2], 16) << 8) | int(decoded_line[1], 16)) - 3
                    
                    # check if caller is in bank
                    if caller >= 0x4000 and caller < 0x8000:
                        caller = caller | (int(decoded_line[5], 16) << 16)
                    
                    path.append(symbols.get(caller, hex(caller)))
                    stack.append({'caller':caller, 'addend':profile_addend, 'clk':int(decoded_line[4], 16)})
                elif int(decoded_line[3], 16) == 4:
                    itm = stack.pop()

                    clk = int(decoded_line[4], 16) - itm['clk'] - itm['addend']
                    clk = int(clk if double_speed else clk / 2)
                    caller = itm['caller'];
                    
                    # update statistics item
                    statitm = stat.setdefault(':'.join(path), {'min':clk, 'max':clk})
                    statitm['ncalls'] = statitm.setdefault('ncalls', 0) + 1
                    statitm['totalclk'] = statitm.setdefault('totalclk', 0) + clk
                    statitm['min'] = min(statitm.setdefault('min', 0), clk)
                    statitm['max'] = max(statitm.setdefault('max', 0), clk)                    
                    if extended: 
                        all_data = statitm.setdefault('data', [])
                        all_data.append(clk)

                    # shortening the path
                    path.pop()
                    
                    # make a correction for underlying function
                    if len(stack) > 0:
                        itm = stack[-1]
                        itm['addend'] = itm['addend'] + (profile_addend * 2)
            line = f.readline()
    return stat

def read_bgb_snspshot(filename):
    def readasciiz(f):
        res = b''
        ch = f.read(1)
        while (ch != b'') and (ch != b'\x00'):
            res = res + ch
            ch = f.read(1)
        return res.decode('u8')

    def readblock(f):
        len = unpack('<L', f.read(4))[0]
        if len == 0:
            return None
        if len == 1:
            return [len, unpack('B', f.read(1))[0]]
        elif len == 2:
            return [len, unpack('<H', f.read(2))[0]]
        elif len == 4:
            return [len, unpack('<L', f.read(4))[0]]
        elif len == 8:
            return [len, unpack('<Q', f.read(8))[0]]
        else:
            data = array('B')
            data.fromfile(f, len)
            return data

    snapshot = {}
    with open(filename, 'rb') as f:
        name = readasciiz(f)
        while (name):
            data = readblock(f)
            snapshot[name] = data
            name = readasciiz(f)
    return snapshot
