import inimedia

init_only = inimedia.MediaFile('init-only.mp4')
print("init-only: expected 0, actual " + str(init_only.has_iseg()))
init_only.close()

init_sidx = inimedia.MediaFile('init-sidx.mp4')
print("init-sidx: expected 1, actual " + str(init_sidx.has_iseg()))
init_sidx.close()

test_out = inimedia.MediaFile('test-out.mp4')
test_out.open()
print("test-out: expected 0, actual " + str(init_sidx.has_iseg()))

# write iseg
f = open('init-only.mp4', 'rb')
init_chunk = f.read()
f.close()
# test_out.write_iseg(init_chunk, 168, 10000000, 0)
test_out.write_iseg(init_chunk, 200, 10000000, 0)
print("test-out: expected 1, actual " + str(init_sidx.has_iseg()))

# write seg
i = 0
for segfile in [("0.m4s", 37546667), ("37546667.m4s", 37546666)]:
    f = open(segfile[0], 'rb')
    seg_chunk = f.read()
    f.close()
    test_out.write_seg(seg_chunk, i, segfile[1])
    i += 1

test_out.close()
