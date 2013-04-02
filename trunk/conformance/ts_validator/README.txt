INSTALL
Run "make" from this directory. 
The software was written in C99. GCC compiler is a recommended way of compilation.
Application of interest is tslib/apps/dash_mpeg2ts_validate. It is used to validate the DASH media segments in MPEG-2 Transport Stream format, per specification ISO-IEC 23009-1. Detailed validation is available for AVC/H.264 elementary streams encapsulated in the transport stream.

USAGE
./tslib/apps/dash_mpeg2ts_validate [options] <input bitstream>

Options:
    -v, --verbose				        print more info (up to two levels of extra verbosity)
    -i, --init-segment			        input MPEG-2 TS segment is an initialization segment 				(default: no)
    -s, --starts-with-sap <SAP type>	input MPEG-2 TS segment starts with SAP of given type				(default: none)
    -e, --extract				        extract each elementary stream into individual file				(default: no)
    -h, --help					        print this message and exit

EXAMPLE 
$ tslib/apps/dash_mpeg2ts_validate -v fileSequence0.ts

LICENSING
  (a) The validator software, dash_mpeg2ts_validate, is provided under BSD license.
  (b) The TS parsing library, tslib, is provided under BSD license.
  (c) The AVC parsing library, h264bitstream, and data structure library, libstruct, 
      are external projects with LGPL license. 
	  Their current version is modified from their current state in SourceForge, 
	  hence they will be removed as patches are accepted in their respective source code repositories. 
	  These libraries are not an intergral part of the reference software, and are not provided by Huawei 
	  or associated with it.
