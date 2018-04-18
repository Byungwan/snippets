# -*- coding: utf-8 -*-

from dash2json.dash2json import dash2json
import json


def test_dash2json():
    mpd = (
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>" +
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
        "</MPD>"
    )
    expected = (
        "{\"@\":{" +
        "\"minimumUpdatePeriod\":\"PT2S\"," +
        "\"profiles\":\"urn:mpeg:dash:profile:isoff-live:2011\"," +
        "\"type\":\"dynamic\"," +
        "\"timeShiftBufferDepth\":\"PT1M40S\"," +
        "\"availabilityStartTime\":\"2017-09-01T06:11:36Z\"," +
        "\"schemaLocation\":\"urn:mpeg:dash:schema:mpd:2011\"," +
        "\"maxSegmentDuration\":\"PT4S\"," +
        "\"publishTime\":\"2018-04-18T01:57:46Z\"," +
        "\"minBufferTime\":\"PT2S\"}," +
        "\"Period\":[{" +
        "\"@\":{\"start\":\"PT0S\"}," +
        "\"AdaptationSet\":[{" +
        "\"@\":{\"id\":\"1\", \"lang\":\"eng\", \"segmentAlignment\":true}," +
        "\"ContentProtection\":[{" +
        "\"@\":{" +
        "\"value\":\"cenc\"," +
        "\"schemeIdUri\":\"urn:mpeg:dash:mp4protection:2011\"," +
        "\"default_KID\":\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\"}}," +
        "{\"@\":{" +
        "\"value\":\"2.0\"," +
        "\"schemeIdUri\":\"urn:uuid:9A04F079-9840-4286-AB92-E65BE0885F95\"," +
        "\"default_KID\":\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\"}," +
        "\"pro\":\"dgMAAAEAAQBsAzwAVwBSAE0ASABFAEEARABFAFIAIAB4AG0AbABuAHMA" +
        "PQAiAGgAdAB0AHAAOgAvAC8AcwBjAGgAZQBtAGEAcwAuAG0AaQBjAHIAbwBzAG8AZg" +
        "B0AC4AYwBvAG0ALwBEAFIATQAvADIAMAAwADcALwAwADMALwBQAGwAYQB5AFIAZQBh" +
        "AGQAeQBIAGUAYQBkAGUAcgAiACAAdgBlAHIAcwBpAG8AbgA9ACIANAAuADAALgAwAC" +
        "4AMAAiAD4APABEAEEAVABBAD4APABQAFIATwBUAEUAQwBUAEkATgBGAE8APgA8AEsA" +
        "RQBZAEwARQBOAD4AMQA2ADwALwBLAEUAWQBMAEUATgA+ADwAQQBMAEcASQBEAD4AQQ" +
        "BFAFMAQwBUAFIAPAAvAEEATABHAEkARAA+ADwALwBQAFIATwBUAEUAQwBUAEkATgBG" +
        "AE8APgA8AEsASQBEAD4AZwA4ADAAbQBXADEAMAA1AGcARQB6AHUAeQBGADcANwBzAD" +
        "QAdABCADIAdwA9AD0APAAvAEsASQBEAD4APABMAEEAXwBVAFIATAA+AGgAdAB0AHAA" +
        "OgAvAC8AcABsAGEAeQByAGUAYQBkAHkALgBkAHIAbQBrAGUAeQBzAGUAcgB2AGUAcg" +
        "AuAGMAbwBtAC8AcABsAGEAeQByAGUAYQBkAHkALwByAGkAZwBoAHQAcwBtAGEAbgBh" +
        "AGcAZQByAC4AYQBzAG0AeAA8AC8ATABBAF8AVQBSAEwAPgA8AEMAVQBTAFQATwBNAE" +
        "EAVABUAFIASQBCAFUAVABFAFMAIAB4AG0AbABuAHMAPQAiACIAPgA8AEMATwBOAFQA" +
        "RQBOAFQASQBEAD4ANwA1ADAANAAxAEYANQBGAC0AMwAxADcAOAAtAEEANQA0AEIALQ" +
        "A5ADYANwBEAC0ARAA4AEMAQwA3AEYAQgAwAEIAOQBGAEEAPAAvAEMATwBOAFQARQBO" +
        "AFQASQBEAD4APAAvAEMAVQBTAFQATwBNAEEAVABUAFIASQBCAFUAVABFAFMAPgA8AE" +
        "MASABFAEMASwBTAFUATQA+AFkARAB1AC8AVgBiAHoAdgBvADkAUQA9ADwALwBDAEgA" +
        "RQBDAEsAUwBVAE0APgA8AC8ARABBAFQAQQA+ADwALwBXAFIATQBIAEUAQQBEAEUAUg" +
        "A+AA==\"}," +
        "{\"@\":{" +
        "\"value\":\"2.0\"," +
        "\"schemeIdUri\":\"urn:uuid:EDEF8BA9-79D6-4ACE-A3C8-27DCD51D21ED\"," +
        "\"default_KID\":\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\"}," +
        "\"pssh\":\"AAAAW3Bzc2gAAAAA7e+LqXnWSs6jyCfc1R0h7QAAADsIARIQWybNgzl" +
        "dTIDuyF77s4tB2xoNd2lkZXZpbmVfdGVzdCIQdQQfXzF4pUuWfdjMf7C5+ioCSEQyA" +
        "A==\"}]," +
        "\"SegmentTemplate\":{" +
        "\"@\":{" +
        "\"media\":\"video_eng_$Bandwidth$/$Time$.m4s\"," +
        "\"initialization\":\"video_eng_$Bandwidth$/init.mp4\"," +
        "\"timescale\":\"10000000\"}," +
        "\"SegmentTimeline\":{" +
        "\"S\":[" +
        "{\"@\":{\"t\":15240166197695776, \"d\":37500004}}," +
        "{\"@\":{\"d\":37499998}}," +
        "{\"@\":{\"d\":37500000}}," +
        "{\"@\":{\"d\":37083332}}," +
        "{\"@\":{\"d\":37500002}}," +
        "{\"@\":{\"d\":37500000}}," +
        "{\"@\":{\"d\":37499998}}," +
        "{\"@\":{\"d\":37500000}}," +
        "{\"@\":{\"d\":37500002}}," +
        "{\"@\":{\"d\":37500000}}]}}," +
        "\"Representation\":[{" +
        "\"@\":{" +
        "\"width\":720," +
        "\"bandwidth\":2000000," +
        "\"mimeType\":\"video/mp4\"," +
        "\"height\":404," +
        "\"codecs\":\"avc1.4d401e\"," +
        "\"startWithSAP\":1," +
        "\"id\":\"video_eng\"}}]}," +
        "{\"@\":{" +
        "\"id\":\"2\"," +
        "\"lang\":\"eng\"," +
        "\"segmentAlignment\":true}," +
        "\"ContentProtection\":[{" +
        "\"@\":{" +
        "\"value\":\"cenc\"," +
        "\"schemeIdUri\":\"urn:mpeg:dash:mp4protection:2011\"," +
        "\"default_KID\":\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\"}}," +
        "{\"@\":{" +
        "\"value\":\"2.0\"," +
        "\"schemeIdUri\":\"urn:uuid:9A04F079-9840-4286-AB92-E65BE0885F95\"," +
        "\"default_KID\":\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\"}," +
        "\"pro\":\"dgMAAAEAAQBsAzwAVwBSAE0ASABFAEEARABFAFIAIAB4AG0AbABuAHMA" +
        "PQAiAGgAdAB0AHAAOgAvAC8AcwBjAGgAZQBtAGEAcwAuAG0AaQBjAHIAbwBzAG8AZg" +
        "B0AC4AYwBvAG0ALwBEAFIATQAvADIAMAAwADcALwAwADMALwBQAGwAYQB5AFIAZQBh" +
        "AGQAeQBIAGUAYQBkAGUAcgAiACAAdgBlAHIAcwBpAG8AbgA9ACIANAAuADAALgAwAC" +
        "4AMAAiAD4APABEAEEAVABBAD4APABQAFIATwBUAEUAQwBUAEkATgBGAE8APgA8AEsA" +
        "RQBZAEwARQBOAD4AMQA2ADwALwBLAEUAWQBMAEUATgA+ADwAQQBMAEcASQBEAD4AQQ" +
        "BFAFMAQwBUAFIAPAAvAEEATABHAEkARAA+ADwALwBQAFIATwBUAEUAQwBUAEkATgBG" +
        "AE8APgA8AEsASQBEAD4AZwA4ADAAbQBXADEAMAA1AGcARQB6AHUAeQBGADcANwBzAD" +
        "QAdABCADIAdwA9AD0APAAvAEsASQBEAD4APABMAEEAXwBVAFIATAA+AGgAdAB0AHAA" +
        "OgAvAC8AcABsAGEAeQByAGUAYQBkAHkALgBkAHIAbQBrAGUAeQBzAGUAcgB2AGUAcg" +
        "AuAGMAbwBtAC8AcABsAGEAeQByAGUAYQBkAHkALwByAGkAZwBoAHQAcwBtAGEAbgBh" +
        "AGcAZQByAC4AYQBzAG0AeAA8AC8ATABBAF8AVQBSAEwAPgA8AEMAVQBTAFQATwBNAE" +
        "EAVABUAFIASQBCAFUAVABFAFMAIAB4AG0AbABuAHMAPQAiACIAPgA8AEMATwBOAFQA" +
        "RQBOAFQASQBEAD4ANwA1ADAANAAxAEYANQBGAC0AMwAxADcAOAAtAEEANQA0AEIALQ" +
        "A5ADYANwBEAC0ARAA4AEMAQwA3AEYAQgAwAEIAOQBGAEEAPAAvAEMATwBOAFQARQBO" +
        "AFQASQBEAD4APAAvAEMAVQBTAFQATwBNAEEAVABUAFIASQBCAFUAVABFAFMAPgA8AE" +
        "MASABFAEMASwBTAFUATQA+AFkARAB1AC8AVgBiAHoAdgBvADkAUQA9ADwALwBDAEgA" +
        "RQBDAEsAUwBVAE0APgA8AC8ARABBAFQAQQA+ADwALwBXAFIATQBIAEUAQQBEAEUAUg" +
        "A+AA==\"}," +
        "{\"@\":{" +
        "\"value\":\"2.0\"," +
        "\"schemeIdUri\":\"urn:uuid:EDEF8BA9-79D6-4ACE-A3C8-27DCD51D21ED\"," +
        "\"default_KID\":\"5B26CD83-395D-4C80-EEC8-5EFBB38B41DB\"}," +
        "\"pssh\":\"AAAAW3Bzc2gAAAAA7e+LqXnWSs6jyCfc1R0h7QAAADsIARIQWybNgzl" +
        "dTIDuyF77s4tB2xoNd2lkZXZpbmVfdGVzdCIQdQQfXzF4pUuWfdjMf7C5+ioCSEQyA" +
        "A==\"}]," +
        "\"SegmentTemplate\":{" +
        "\"@\":{" +
        "\"media\":\"audio_eng_$Bandwidth$/$Time$.m4s\"," +
        "\"initialization\":\"audio_eng_$Bandwidth$/init.mp4\"," +
        "\"timescale\":\"10000000\"}," +
        "\"SegmentTimeline\":{" +
        "\"S\":[" +
        "{\"@\":{\"t\":15240166197872334, \"d\":37384444}}," +
        "{\"@\":{\"d\":37615778}}," +
        "{\"@\":{\"d\":37384334}}," +
        "{\"@\":{\"d\":37152000}}," +
        "{\"@\":{\"d\":37384222}}," +
        "{\"@\":{\"d\":37616332}}," +
        "{\"@\":{\"d\":37384222}}," +
        "{\"@\":{\"d\":37616000}}," +
        "{\"@\":{\"d\":37384112}}," +
        "{\"@\":{\"d\":37616668}}]}}," +
        "\"Representation\":[{" +
        "\"@\":{" +
        "\"codecs\":\"mp4a.40.2\"," +
        "\"startWithSAP\":1," +
        "\"bandwidth\":128000," +
        "\"mimeType\":\"audio/mp4\"," +
        "\"id\":\"audio_eng\"}}]}]}]}"
    )

    assert json.loads(expected) == json.loads(dash2json(mpd))
