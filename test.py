#!/usr/bin/python

import sys
import matrixbuilder6 as mb

'''
m = mb.frame_writer(sys.argv[1], sys.argv[2], "u32", 10000)
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    value = v[2].replace(":","|")
    print value
    m.append(uid, value, len(value))
del m
'''
r = mb.frame_reader(sys.argv[1], sys.argv[2])
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    print uid, r.get(uid, 20)
del r


'''
m = mb.frame_writer(sys.argv[1], sys.argv[2], "int_float", 10000)
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    value = v[2]
    m.append(uid, value, len(value))
del m
r = mb.frame_reader(sys.argv[1], sys.argv[2])
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    print uid, r.dic(uid, 20)
del r
'''

'''
m = mb.frame_writer(sys.argv[1], sys.argv[2], "type_int", 10000)
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    value = v[2]
    m.append(uid, value, len(value))
del m
r = mb.frame_reader(sys.argv[1], sys.argv[2])
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    print uid, r.get(uid, 20)
del r
'''

'''
m = mb.frame_writer(sys.argv[1], sys.argv[2], "string", 10000)
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    value = v[2]
    m.append(uid, value, len(value))
del m
'''
r = mb.frame_reader(sys.argv[1], sys.argv[2])
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    print uid, r.get(uid, 20)
del r



'''
m = mb.series_writer(sys.argv[1], "score", 1000, 4)
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    value = v[2]
    m.append(uid, value, len(value))
del m
r = mb.series_reader(sys.argv[1])
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    print uid, r.get(uid, 20)
del r
'''


'''
m = mb.series_writer(sys.argv[1], "int", 1000)
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    value = v[2]
    pos = value.find(":")
    value = value[:pos] + "|"
    m.append(uid, value, len(value))
del m
r = mb.series_reader(sys.argv[1])
for line in sys.stdin:
    v = line.strip().split('\t')
    uid = int(v[0])
    print uid, r.get(uid, 20)
del r
'''


