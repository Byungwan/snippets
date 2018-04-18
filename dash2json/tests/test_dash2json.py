# -*- coding: utf-8 -*-

from dash2json.dash2json import dash2json


def test_dash2json():
    mpd = ("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" +
           "<MPD xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " +
           "xmlns=\"urn:mpeg:dash:schema:mpd:2011\" " +
           "xsi:schemaLocation=\"urn:mpeg:dash:schema:mpd:2011\" " +
           "profiles=\"urn:mpeg:dash:profile:isoff-live:2011\" " +
           "type=\"dynamic\" " +
           "availabilityStartTime=\"2017-09-01T06:11:36Z\" " +
           "publishTime=\"2018-04-18T01:57:46Z\" " +
           "minimumUpdatePeriod=\"PT2S\" " +
           "minBufferTime=\"PT2S\" timeShiftBufferDepth=\"PT1M40S\" " +
           "maxSegmentDuration=\"PT4S\" xmlns:cenc=\"urn:mpeg:cenc:2013\" " +
           "xmlns:mspr=\"urn:microsoft:playready\">" +
           "<Period start=\"PT0S\">" +
           "<AdaptationSet id=\"1\" lang=\"eng\" segmentAlignment=\"true\">" +
           "<ContentProtection " +
           "schemeIdUri=\"urn:mpeg:dash:mp4protection:2011\" " +
           "value=\"cenc\" " +
           "cenc:default_KID=\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\">" +
           "</ContentProtection>" +
           "<ContentProtection " +
           "schemeIdUri=\"urn:uuid:9A04F079-9840-4286-AB92-E65BE0885F95\" " +
           "value=\"2.0\" " +
           "cenc:default_KID=\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\">" +
           "<mspr:pro>dgMAAAEAAQBsAzwAVwBSAE0ASABFAEEARABFAFIAIAB4AG0AbABuA" +
           "HMAPQAiAGgAdAB0AHAAOgAvAC8AcwBjAGgAZQBtAGEAcwAuAG0AaQBjAHIAbwBz" +
           "AG8AZgB0AC4AYwBvAG0ALwBEAFIATQAvADIAMAAwADcALwAwADMALwBQAGwAYQB" +
           "5AFIAZQBhAGQAeQBIAGUAYQBkAGUAcgAiACAAdgBlAHIAcwBpAG8AbgA9ACIANA" +
           "AuADAALgAwAC4AMAAiAD4APABEAEEAVABBAD4APABQAFIATwBUAEUAQwBUAEkAT" +
           "gBGAE8APgA8AEsARQBZAEwARQBOAD4AMQA2ADwALwBLAEUAWQBMAEUATgA+ADwA" +
           "QQBMAEcASQBEAD4AQQBFAFMAQwBUAFIAPAAvAEEATABHAEkARAA+ADwALwBQAFI" +
           "ATwBUAEUAQwBUAEkATgBGAE8APgA8AEsASQBEAD4AZwA4ADAAbQBXADEAMAA1AG" +
           "cARQB6AHUAeQBGADcANwBzADQAdABCADIAdwA9AD0APAAvAEsASQBEAD4APABMA" +
           "EEAXwBVAFIATAA+AGgAdAB0AHAAOgAvAC8AcABsAGEAeQByAGUAYQBkAHkALgBk" +
           "AHIAbQBrAGUAeQBzAGUAcgB2AGUAcgAuAGMAbwBtAC8AcABsAGEAeQByAGUAYQB" +
           "kAHkALwByAGkAZwBoAHQAcwBtAGEAbgBhAGcAZQByAC4AYQBzAG0AeAA8AC8ATA" +
           "BBAF8AVQBSAEwAPgA8AEMAVQBTAFQATwBNAEEAVABUAFIASQBCAFUAVABFAFMAI" +
           "AB4AG0AbABuAHMAPQAiACIAPgA8AEMATwBOAFQARQBOAFQASQBEAD4ANwA1ADAA" +
           "NAAxAEYANQBGAC0AMwAxADcAOAAtAEEANQA0AEIALQA5ADYANwBEAC0ARAA4AEM" +
           "AQwA3AEYAQgAwAEIAOQBGAEEAPAAvAEMATwBOAFQARQBOAFQASQBEAD4APAAvAE" +
           "MAVQBTAFQATwBNAEEAVABUAFIASQBCAFUAVABFAFMAPgA8AEMASABFAEMASwBTA" +
           "FUATQA+AFkARAB1AC8AVgBiAHoAdgBvADkAUQA9ADwALwBDAEgARQBDAEsAUwBV" +
           "AE0APgA8AC8ARABBAFQAQQA+ADwALwBXAFIATQBIAEUAQQBEAEUAUgA+AA==" +
           "</mspr:pro>" +
           "</ContentProtection>" +
           "<ContentProtection " +
           "schemeIdUri=\"urn:uuid:EDEF8BA9-79D6-4ACE-A3C8-27DCD51D21ED\" " +
           "value=\"2.0\" " +
           "cenc:default_KID=\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\">" +
           "<cenc:pssh>AAAAW3Bzc2gAAAAA7e+LqXnWSs6jyCfc1R0h7QAAADsIARIQWybN" +
           "gzldTIDuyF77s4tB2xoNd2lkZXZpbmVfdGVzdCIQdQQfXzF4pUuWfdjMf7C5+io" +
           "CSEQyAA==</cenc:pssh>" +
           "</ContentProtection>" +
           "<SegmentTemplate timescale=\"10000000\" " +
           "media=\"video_eng_$Bandwidth$/$Time$.m4s\" " +
           "initialization=\"video_eng_$Bandwidth$/init.mp4\">" +
           "<SegmentTimeline>" +
           "<S t=\"15240166197695776\" d=\"37500004\"></S>" +
           "<S d=\"37499998\"></S>" +
           "<S d=\"37500000\"></S>" +
           "<S d=\"37083332\"></S>" +
           "<S d=\"37500002\"></S>" +
           "<S d=\"37500000\"></S>" +
           "<S d=\"37499998\"></S>" +
           "<S d=\"37500000\"></S>" +
           "<S d=\"37500002\"></S>" +
           "<S d=\"37500000\"></S>" +
           "</SegmentTimeline>" +
           "</SegmentTemplate>" +
           "<Representation width=\"720\" height=\"404\" " +
           "mimeType=\"video/mp4\" " +
           "codecs=\"avc1.4d401e\" startWithSAP=\"1\" " +
           "id=\"video_eng\" " +
           "bandwidth=\"2000000\"></Representation>" +
           "</AdaptationSet>" +
           "<AdaptationSet id=\"2\" lang=\"eng\" segmentAlignment=\"true\">" +
           "<ContentProtection " +
           "schemeIdUri=\"urn:mpeg:dash:mp4protection:2011\" " +
           "value=\"cenc\" " +
           "cenc:default_KID=\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\">" +
           "</ContentProtection>" +
           "<ContentProtection " +
           "schemeIdUri=\"urn:uuid:9A04F079-9840-4286-AB92-E65BE0885F95\" " +
           "value=\"2.0\" " +
           "cenc:default_KID=\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\">" +
           "<mspr:pro>dgMAAAEAAQBsAzwAVwBSAE0ASABFAEEARABFAFIAIAB4AG0AbABuA" +
           "HMAPQAiAGgAdAB0AHAAOgAvAC8AcwBjAGgAZQBtAGEAcwAuAG0AaQBjAHIAbwBz" +
           "AG8AZgB0AC4AYwBvAG0ALwBEAFIATQAvADIAMAAwADcALwAwADMALwBQAGwAYQB" +
           "5AFIAZQBhAGQAeQBIAGUAYQBkAGUAcgAiACAAdgBlAHIAcwBpAG8AbgA9ACIANA" +
           "AuADAALgAwAC4AMAAiAD4APABEAEEAVABBAD4APABQAFIATwBUAEUAQwBUAEkAT" +
           "gBGAE8APgA8AEsARQBZAEwARQBOAD4AMQA2ADwALwBLAEUAWQBMAEUATgA+ADwA" +
           "QQBMAEcASQBEAD4AQQBFAFMAQwBUAFIAPAAvAEEATABHAEkARAA+ADwALwBQAFI" +
           "ATwBUAEUAQwBUAEkATgBGAE8APgA8AEsASQBEAD4AZwA4ADAAbQBXADEAMAA1AG" +
           "cARQB6AHUAeQBGADcANwBzADQAdABCADIAdwA9AD0APAAvAEsASQBEAD4APABMA" +
           "EEAXwBVAFIATAA+AGgAdAB0AHAAOgAvAC8AcABsAGEAeQByAGUAYQBkAHkALgBk" +
           "AHIAbQBrAGUAeQBzAGUAcgB2AGUAcgAuAGMAbwBtAC8AcABsAGEAeQByAGUAYQB" +
           "kAHkALwByAGkAZwBoAHQAcwBtAGEAbgBhAGcAZQByAC4AYQBzAG0AeAA8AC8ATA" +
           "BBAF8AVQBSAEwAPgA8AEMAVQBTAFQATwBNAEEAVABUAFIASQBCAFUAVABFAFMAI" +
           "AB4AG0AbABuAHMAPQAiACIAPgA8AEMATwBOAFQARQBOAFQASQBEAD4ANwA1ADAA" +
           "NAAxAEYANQBGAC0AMwAxADcAOAAtAEEANQA0AEIALQA5ADYANwBEAC0ARAA4AEM" +
           "AQwA3AEYAQgAwAEIAOQBGAEEAPAAvAEMATwBOAFQARQBOAFQASQBEAD4APAAvAE" +
           "MAVQBTAFQATwBNAEEAVABUAFIASQBCAFUAVABFAFMAPgA8AEMASABFAEMASwBTA" +
           "FUATQA+AFkARAB1AC8AVgBiAHoAdgBvADkAUQA9ADwALwBDAEgARQBDAEsAUwBV" +
           "AE0APgA8AC8ARABBAFQAQQA+ADwALwBXAFIATQBIAEUAQQBEAEUAUgA+AA==" +
           "</mspr:pro>" +
           "</ContentProtection>" +
           "<ContentProtection " +
           "schemeIdUri=\"urn:uuid:EDEF8BA9-79D6-4ACE-A3C8-27DCD51D21ED\" " +
           "value=\"2.0\" " +
           "cenc:default_KID=\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\">" +
           "<cenc:pssh>AAAAW3Bzc2gAAAAA7e+LqXnWSs6jyCfc1R0h7QAAADsIARIQWybN" +
           "gzldTIDuyF77s4tB2xoNd2lkZXZpbmVfdGVzdCIQdQQfXzF4pUuWfdjMf7C5+io" +
           "CSEQyAA==</cenc:pssh>" +
           "</ContentProtection>" +
           "<SegmentTemplate timescale=\"10000000\" " +
           "media=\"audio_eng_$Bandwidth$/$Time$.m4s\" " +
           "initialization=\"audio_eng_$Bandwidth$/init.mp4\">" +
           "<SegmentTimeline>" +
           "<S t=\"15240166197872334\" d=\"37384444\"></S>" +
           "<S d=\"37615778\"></S>" +
           "<S d=\"37384334\"></S>" +
           "<S d=\"37152000\"></S>" +
           "<S d=\"37384222\"></S>" +
           "<S d=\"37616332\"></S>" +
           "<S d=\"37384222\"></S>" +
           "<S d=\"37616000\"></S>" +
           "<S d=\"37384112\"></S>" +
           "<S d=\"37616668\"></S>" +
           "</SegmentTimeline>" +
           "</SegmentTemplate>" +
           "<Representation mimeType=\"audio/mp4\" codecs=\"mp4a.40.2\" " +
           "startWithSAP=\"1\" id=\"audio_eng\" " +
           "bandwidth=\"128000\">" +
           "</Representation>" +
           "</AdaptationSet>" +
           "</Period>" +
           "</MPD>")
    json = dash2json(mpd)
    print(json)
    assert 'a' == json
