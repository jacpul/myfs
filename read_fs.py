#!/usr/bin/python
import json
import struct
import sys

NUM_B = 64
NUM_I = 80
T = " "*2

def B(i):
    return i*4096

def I(i):
    return (i*4)

fp = open("fs.iso", "rb")

data = fp.read()

def rint(data, i):
    return to_int(data[I(i):I(i+1)])

def to_int(l):
    return struct.unpack("<L", ''.join(l))[0]

def b_to_s(a):
    if a == '\x00':
        return "0"
    elif a == '\x01':
        return "1"
    return "I"

def read_inode_bmap():
    bmap = list(data[B(1):B(1)+80])
    bmap = map(b_to_s, bmap)
    return bmap

def read_data_bmap():
    bmap = list(data[B(2):B(2)+64])
    bmap = map(b_to_s, bmap)
    return bmap

def get_inode_size(n):
    inode = list(data[B(3)+(n*256):B(3)+((n+1)*256)])
    return to_int(inode[I(1):I(2)])


def get_inode_blocks(n):
    inode = list(data[B(3)+(n*256):B(3)+((n+1)*256)])
    bs = []
    for i in range(to_int(inode[I(2):I(3)])):
        bs += [to_int(inode[I(3+i):I(4+i)])]
    return bs


def get_blocks_data(l, s):
    if s == 0:
        return []
    d = []
    for b in l[:-1]:
        d += list(data[B(b):B(b+1)])
    # THIS DOES NOT WORK.
    d += list(data[B(l[-1]):B(l[-1])+(s % B(1))])
    return d


def read_inode(n):
    print "Inode ", n, "@", B(3)+(n*256), hex(B(3)+(n*256))
    inode = list(data[B(3)+(n*256):B(3)+((n+1)*256)])
    #print inode
    t = inode[I(0):I(1)][0]
    if t == '\x00':
        print T, "Type: dir"
    elif t == '\x01':
        print T, "Type: file"
    else:
        print T, "Type: unknown"

    print T, "Size:", to_int(inode[I(1):I(2)])
    bs = to_int(inode[I(2):I(3)])
    print T, "Blocks:", bs
    print T, "Pointers:"
    for i in range(bs):
        print T, T, to_int(inode[I(3+i):I(4+i)])


def get_inode(n):
    inode = list(data[B(3)+(n*256):B(3)+((n+1)*256)])
    node = {"type": 2}
    t = inode[I(0):I(1)][0]
    if t == '\x00':
        node["type"] = 0
    elif t == '\x01':
        node["type"] = 1

    node["size"] = to_int(inode[I(1):I(2)])
    node["bs"] = to_int(inode[I(2):I(3)])
    node["pointers"] = []
    for i in range(node["bs"]):
        node["pointers"] += [to_int(inode[I(3+i):I(4+i)])]

    return node

def get_dir(n):
    dir_data = []
    s = get_inode_size(n)
    bs = get_inode_blocks(n)

    data = get_blocks_data(bs, s)
    while data:
        inum = rint(data, 0)
        reclen = rint(data, 1)
        strlen = rint(data, 2)
        name = ''.join(data[I(3):I(3)+strlen-1])
        dir_data += [{"inum": inum, "reclen":reclen, "strlen":strlen, "name":name}]
        data = data[I(3)+reclen:]

    return {"n": n, "content": dir_data}

def get_file(n):
    s = get_inode_size(n)
    bs = get_inode_blocks(n)
    data = get_blocks_data(bs, s)
    return {"n": n, "content": map(lambda x: hex(ord(x))[2:].zfill(2), data)}


def read_dir(n):
    print "Dir at Inode ", n, "@", B(3)+(n*256), hex(B(3)+(n*256))
    s = get_inode_size(n)
    print "Size:", s
    bs = get_inode_blocks(n)
    print "Stored in blocks:", bs

    data = get_blocks_data(bs, s)
    while data:
        inum = rint(data, 0)
        reclen = rint(data, 1)
        strlen = rint(data, 2)
        print T, "Inum: %d Reclen: %d Strlen: %d Name: '%s'" % (inum, reclen, strlen, ''.join(data[I(3):I(3)+strlen-1]))
        data = data[I(3)+reclen:]


def read_file(n):
    print "File at Inode ", n, "@", B(3)+(n*256), hex(B(3)+(n*256))
    s = get_inode_size(n)
    print "Size:", s
    bs = get_inode_blocks(n)
    print "Stored in blocks:", bs

    data = get_blocks_data(bs, s)
    print "Content: '%s'" % (''.join(data))


def dump_fs():
    out = {}
    out["imap"] = read_inode_bmap()
    out["dmap"] = read_data_bmap()

    out["inodes"] = []
    out["data"] = []
    for i in range(NUM_I):
        node = dict()
        if out["imap"][i] == '1':
            node = get_inode(i)
            if "type" in node and node["type"] == 0:
                out["data"] += [get_dir(i)]
            if "type" in node and node["type"] == 1:
                out["data"] += [get_file(i)]
        out["inodes"] += [node]
    #out[""]
    return out


if __name__ in "__main__":
    last = sys.argv[-1]
    others = sys.argv[1:-1]
    if "--dump" in sys.argv:
        fs = dump_fs()
        if "-p" in sys.argv:
            print json.dumps(fs, indent=4, sort_keys=True)
        else:
            print json.dumps(fs)
    else:
        last = int(last)

        if "--bmap" in others or "-b" in others:
            print read_inode_bmap()
            print read_data_bmap()
        if "-i" in others:
            read_inode(last)
        if "-d" in others:
            read_dir(last)
        if "-f" in others:
            read_file(last)
