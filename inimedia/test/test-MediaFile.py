import inimedia

init_only = inimedia.MediaFile('init-only.mp4')
init_only.open()
print("init-only: expected 0, actual " + str(init_only.has_iseg()))
init_only.close()

init_sidx = inimedia.MediaFile('init-sidx.mp4')
init_sidx.open()
print("init-sidx: expected 1, actual " + str(init_sidx.has_iseg()))
init_sidx.close()

test_out = inimedia.MediaFile('test-out.mp4')
test_out.open()
print("test-out: expected 0, actual " + str(init_sidx.has_iseg()))
f = open('init-only.mp4', 'rb')
init_chunk = f.read()
f.close()
test_out.write_iseg(init_chunk, 10000000, 0, 168)
print("test-out: expected 1, actual " + str(init_sidx.has_iseg()))
test_out.close()
