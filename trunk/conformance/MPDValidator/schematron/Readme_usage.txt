* Readme file for the MPD conformance software Schematron.

Software Dependencies
---------------------
This software only requires a working Java environment. No further dependencies are required. 

Usage
-----
This software validates the conformance of an MPD. 

To validate an MPD
1. run create_val_schema.sh in the schematron folder
2. To run the conformance test on an MPD, the syntax is as follows:

java -jar ../saxon9.jar -versionmsg:off -s:examples/ex01.mpd -o:output/result_ex01.xml -xsl:output/val_schema.xsl

ex01.mpd is the mpd to be Validated located in the folder examples
result_ex01 is the result of conformance check placed in the folder output
val_schema.xcl is the result of schema validator placed in the folder output

Example usage
-------------
run_samples.sh validates all files in examples folder and write the results to output folder

