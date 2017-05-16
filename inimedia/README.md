# inimedia.py

## Build

    cd inimedia
    python setup.py build

## Install

    cd inimedia
    sudo python setup.py install

## Example


```python
myfile = inimedia.MediaFile('/data/movies/clip01/video_500000.mp4')

if myfile.has_iseg():
    # write_iseg(iseg_chunk, seg_cnt, timescale, erly_time)
    #
    # parameters
    # - iseg_chunk :: initialization segment chunk
    # - seg_cnt :: expected segment count
    # - timescale :: timescale
    # - erly_ :: earliest presentation time
    # returns
    #  - nonzero
    # throws
    #  - IOError :: if an I/O error occurs
    #  - SystemError
    test_out.write_iseg(init_chunk, 168, 10000000, 0)

# write_seg(seg_chunk, seg_idx, seg_dur)
#
# parameters
# - seg_chunk :: media segment chunk
# - seg_idx :: segment index
# - seg_dur :: segment duration
# returns
# - the file offset of the given segment
# throws
# - IOError :: if an I/O error occurs
# - IndexErrorK
# - RuntimeError
seg_offs = test_out.write_seg(seg_chunk, 0, 37546667)

test_out.close()
```
