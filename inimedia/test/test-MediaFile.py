import inimedia

init_only = inimedia.MediaFile('init-only.mp4')
iseg = init_only.has_iseg()
print("init-only: expected iseg 0, actual " + str(iseg))
if iseg == 0:
    print(' ... OK')
else:
    print(' ... NG')
init_only.close()

init_sidx = inimedia.MediaFile('init-sidx.mp4')
iseg = init_sidx.has_iseg()
print("init-sidx: expected iseg 1, actual " + str(iseg))
if iseg == 1:
    print(' ... OK')
else:
    print(' ... NG')
init_sidx.close()

test_out = inimedia.MediaFile('test-out.mp4')
test_out.open()

# write iseg
f = open('init-only.mp4', 'rb')
init_chunk = f.read()
f.close()
test_out.write_iseg(init_chunk, 168, 10000000, 0)
iseg = test_out.has_iseg()
print("test_out: expected iseg 1, actual " + str(iseg))
if iseg == 1:
    print(' ... OK')
else:
    print(' ... NG')

# write seg
i = 0
for segfile in [("0.m4s", 37546667, 2699), ("37546667.m4s", 37546666, 54561)]:
    f = open(segfile[0], 'rb')
    seg_chunk = f.read()
    f.close()
    off = test_out.write_seg(seg_chunk, i, segfile[1])
    print("test-out: expected offset " + str(segfile[2])
          + ", actual " + str(off))
    if segfile[2] == off:
        print(' ... OK')
    else:
        print(' ... NG')
    i += 1

test_out.close()
