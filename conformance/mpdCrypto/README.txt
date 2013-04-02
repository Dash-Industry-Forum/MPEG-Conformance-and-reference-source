mpdEncryptionUtil can be used to perform one of the following tasks, per specification ISO-IEC 23009-4:
- encrypt cleartext media segments, via a simple scenario of baseline encryption scheme
- decrypt media segments, encrypted via supported encryption scheme
In each of these cases output media segments are saved to local disk, and MPD is transformed 
appropriately and also saved to local disk. However, input MPD and media segments may come either 
from local disk or from URLs.

In order to compile the utility, import an existing Java project "mpdCrypto" into Eclipse.
Eclipse JDT will compile all needed .class files automatically.

Design Limitations & Assumptions:
- A single MPD with a single Period, AdaptationSet, Representation is being processed.
- Content is assumed to be listed by a single SegmentList
- A single instance of ContentProtection descriptor of supported type is assumed in encrypted MPDs.
- At least one KeySystem of baseline type is assumed in encrypted MPDs.
- Support for generating and checking content authenticity tags is coming at a later date.
- Support for validating MPDs via Schematron is coming at a later date.
- At this time all the operations are being done serially and synchronously.
	- Later it'd be easy to pipeline, batch, and parallellize operations.

Usage: mpdEncryptionUtil -d | -e  -i <inputfile.mpd> -o <output dir> [-s] [--validate-schematron]
 -d,--decrypt                      Decrypt encrypted segments
 -e,--encrypt                      Encrypt cleartext segments
 -i,--input-file <inputfile.mpd>   MPD file path or URL, we'll fetch segments ourselves
 -o,--output-dir <output dir>      Directory to write encrypted or decrypted MPD and segments to
 -s,--sign                         Not yet supported
    --validate-schematron          Whether to validate MPD via DASH schema and schematron (default: no)

See run_tests.sh script in this directory for examples of usage.
