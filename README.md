# Byungwan's snippet collections

## xml2box

### Build

    gcc -Wall -O2 $(xml2-config --cflags) -o xml2box xml2box.c $(xml2-config --libs)

### Usage

    xml2box [XML-FILE] [BOX-FILE]

### XML Format

```xml
<?xml version="1.0" encoding="UTF-8"?>
<IsoMediaFile Name="audio_eng_112987.mp4">
  <SegmentIndexBox reference_ID="1" timescale="10000000"
                   earliest_presentation_time="0" first_offset="0">
    <BoxInfo Size="2056" Type="sidx"/>
    <FullBoxInfo Version="1" Flags="0x0"/>
    <Reference type="0" size="192049" duration="37546667"
               startsWithSAP="1" SAP_type="0" SAPDeltaTime="0"/>
    <Reference type="0" size="248499" duration="37546666"
               startsWithSAP="1" SAP_type="0" SAPDeltaTime="0"/>
    <!-- ... -->
    <Reference type="0" size="42226" duration="15146666"
               startsWithSAP="1" SAP_type="0" SAPDeltaTime="0"/>
  </SegmentIndexBox>
</IsoMediaFile>
```

## inimedia.py

### Build

    cd inimedia
    python setup.py build

### Install

    cd inimedia
    sudo python setup.py install

### Example


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
