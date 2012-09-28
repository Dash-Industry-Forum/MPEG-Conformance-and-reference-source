/*


############################################################################
##                Test Tools Source Code License Notice                   ##
############################################################################
                 Internet Streaming Media Alliance (ISMA)

          TEST TOOLS SOURCE CODE RELATED TO THE ISMA SPECIFICATION

ISMA Member Apple ('Submitting Member'), submitting the 
Contributed Code in this Test Tools source code has agreed to the
following terms and conditions by including this notice. 'Contributed Code'
means the code originally written by, or code modifications made by, the 
Submitting Member included in this Test Tools source code.  This submission
by Apple was registered as Contribution IN009.

COMPANYNAME1, COMPANYNAME2, COMPANYNAME3, COMPANYNAME4, and COMPANYNAME5
('Previous Submitting Members'), which previously submitted contributed 
code to this Test Tools source code, have agreed to the following terms 
and conditions in their respective submissions.

--------- ISMA Members downloading and/or using this Test Tool ------------
 PLEASE READ BEFORE COPYING, INSTALLING, OR USING.  By loading or using 
 the Test Tools, you agree to the terms and conditions herein.  If you do 
 not wish to so agree, do not load, install, or use this Test Tool.  
---------------------------------------------------------------------------

1.   Terms and Conditions:

1.1  Submitting Member hereby grants to ISMA an IRREVOCABLE, world-wide,
     royalty-free, non-exclusive license:
     (a) to intellectual property rights (Copyright and Patent) in the
         Contributed Code in this Test Tools submission Licensable 
         by the Submitting Member.

        'Licensable' means having the right to grant, to the maximum
         extent possible, whether at the time of the initial grant or
         subsequently acquired, any and all of the rights conveyed herein.

     (b) to use, reproduce and DISTRIBUTE, the submitted Test Tools
         as ISMA deems appropriate.

1.2  Submitting Member hereby grants to all ISMA Members a world-wide,
     royalty-free, non-exclusive license:
     (a) to intellectual property rights (Copyright and Patent) in the
         Contributed Code in this Test Tools submission Licensable 
         by the Submitting Member.
     (b) to use, reproduce and modify the Test Tools for the non-Commercial
         Use of self-testing the ISMA Member's own products in reference
         to the ISMA specifications.

        'Commercial Use' means distribution or otherwise making the
         sumbitted source code available to a non-ISMA Member.

     (c) to make derivative works of the Test Tools that improve or extend
         the utility of the Test Tools so long as the ISMA Member that
         creates such derivative work submits it back to ISMA, subject to
         the same terms and conditions described herein.
     (d) which does not include the right to sublicense or assign, or the
         right to incorporate the Test Tools in the ISMA Member's own products.
     (e) for as long as the Member remains an ISMA Member.

1.3  Submitting Member, Previous Submitting Members and ISMA, MAKE NO WARRANTY 
     OF NONINFRINGEMENT OF THE INTELLECTUAL PROPERTY RIGHTS OF THIRD PARTIES.  
     ISMA Member agrees that ISMA, Submitting Member, and the Previous Submitting
     Members shall NOT be liable or held responsible if use of the Test Tools 
     is found to infringe the intellectual property rights of third parties.

1.4  Submitting Member represents that it holds a good faith belief that the
     Test Tools source code submitted may be of assistance to ISMA Members
     in self-testing their products with reference to the ISMA Specification.
     HOWEVER, SUBMITTING MEMBER, PREVIOUS SUBMITTING MEMBERS, AND ISMA MAKE
     NO WARRANTIES, EXPRESS OR IMPLIED AND, IN PARTICULAR, NO WARRANTY OF 
     MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. ISMA MEMBERS 
     UTILIZING THE TEST TOOLS DO SO AT THEIR OWN RISK.  SUBMITTING MEMBER, 
     PREVIOUS SUBMITTING MEMBERS, AND ISMA DISCLAIM ALL RESPONSIBILITY 
     FOR DAMAGES OR LIABILITY OF ANY KIND TO MEMBERS.

1.5  If the Submitting Member has knowledge that a license under a third 
     party's intellectual property rights may be required to exercise the
     rights granted by Submitting Member under Sections 1.1(b) or 1.2(b,c,d,e),
     the Submitting Member agrees to include a text file with the Test Tools 
     submitted titled 'LEGAL-3PARTY' which describes the claim and the party 
     making the claim in sufficient detail that a user will know whom to contact.

1.6  Submitting Member and Previous Submitting Members assume no 
     obligation to support or to update the submitted Test Tools 
     beyond the requirements of 1.5

1.7  ISMA assumes no obligation to support or to update any Test Tools and
     reserves the right to remove from the ISMA web site at any time any 
     Test Tools it finds to be infringing or otherwise problematical.
*/



#include "ValidateMP4.h"
#if STAND_ALONE_APP
	#include "console.h"
#endif

#if 1
#define myTAB "  "
#else
#define myTAB "\t"
#endif

ValidateGlobals vg = {0};


static int keymatch (const char * arg, const char * keyword, int minchars);

//#define STAND_ALONE_APP 1  //  #define this if you're using a source level debugger (i.e. Visual C++ in Windows)
							  //  also, near the beginning of main(), hard-code your arguments (e.g. your test file)

//==========================================================================================

/*
 * Case-insensitive matching of possibly-abbreviated keyword switches.
 * keyword is the constant keyword (must be lower case already),
 * minchars is length of minimum legal abbreviation.
 */

static int keymatch (const char * arg, const char * keyword, int minchars)
{
  register int ca, ck;
  register int nmatched = 0;

  while ((ca = *arg++) != '\0') {
    if ((ck = *keyword++) == '\0')
      return false;		/* arg longer than keyword, no good */
    if (isupper(ca))		/* force arg to lcase (assume ck is already) */
      ca = tolower(ca);
    if (ca != ck)
      return false;		/* no good */
    nmatched++;			/* count matched characters */
  }
  /* reached end of argument; fail if it's too short for unique abbrev */
  if (nmatched < minchars)
    return false;
  return true;			/* A-OK */
}

//==========================================================================================
//_MSL_IMP_EXP_C extern int ccommand(char ***);

#define getNextArgStr( _str_, _str_err_str_ ) \
		argn++; \
		arg = argv[argn]; \
		if( nil == arg ) \
		{ \
			fprintf( stderr, "Expected " _str_err_str_ " got end of args\n" ); \
			err = -1; \
			goto usageError; \
		} \
		if( arg[0] == '-' ) \
		{ \
			fprintf( stderr, "Expected " _str_err_str_ " next arg\n" ); \
			err = -1; \
			goto usageError; \
		} \
		strcpy(*(_str_), arg); 
		

#if !STAND_ALONE_APP
int main(int argc, char *argv[]);
int main(int argc, char *argv[])
{
#else
int main(void);
int main(void)
{
	char *argv[] = {
		"ValidateMP4",
		"<mpeg4-file-path>"
		};
	int argc = sizeof(argv)/sizeof(char*);
#endif
	int argn;
	int gotInputFile = false;
    bool gotSegmentInfoFile = false;
    bool gotleafInfoFile = false;
	int err;
	char gInputFileFullPath[1024];
	char leafInfoFileName[1024];
    char sapType[1024];
	int usedefaultfiletype = true;
	
	FILE *infile = nil;
	atomOffsetEntry aoe = {0};

	vg.warnings = true;
//	vg.qtwarnings = true;
//	vg.print_atompath = true;
//	strcpy( vg.atompath, "moov-1:trak-1:mdia-1:minf-1:stbl-1:stsd-1" );
//	vg.print_atom = true;
//	vg.print_fulltable = true;
//	vg.print_sample = true;
//	vg.print_sampleraw = true;
//	vg.print_hintpayload = true;
    vg.visualProfileLevelIndication = 255;
    vg.checkSegAlignment = false;
    vg.checkSubSegAlignment = false;
    vg.isoLive = false;
    vg.isoondemand = false;
    vg.isomain = false;
    vg.bss = false;
    vg.subRepLevel = false;
    vg.startWithSAP = -1;
    // this is simply the wrong place for this;  it's not a program parameter, it's the mpeg-4
    //   profile/level indication as found in the video stream.
    // But neither movie info nor track info are available at the right points.  Ugh [dws]



		
	// Check the parameters
	for( argn = 1; argn < argc; argn++ )
	{
		const char *arg = argv[argn];
		
		if( '-' != arg[0] )
		{
			char *extensionstartp = nil;
			
			if (gotInputFile) {
				fprintf( stderr, "Unexpected argument \"%s\"\n", arg );
				err = -1;
				goto usageError;
			}
			strcpy(gInputFileFullPath, arg);
			gotInputFile = true;
			
#ifdef USE_STRCASECMP
	#define rStrCaseCmp(a,b)		strcasecmp(a,b)
#else
	#define rStrCaseCmp(a,b)		my_stricmp(a,b)
#endif			
			extensionstartp = strrchr(gInputFileFullPath,'.');
			if (extensionstartp) {
				if (rStrCaseCmp(extensionstartp,".mp4") == 0) {
					vg.filetype = filetype_mp4;
					usedefaultfiletype = false;
				}
			}
			
			continue;
		}
		
		arg++;	// skip '-'
		
		if( keymatch( arg, "help", 1 ) ) {
			goto usageError;
		} else if( keymatch( arg, "warnings", 1 ) ) {
			vg.warnings = true;
		} else if ( keymatch( arg, "filetype", 1 ) ) {
			getNextArgStr( &vg.filetypestr, "filetype" );
		} else if ( keymatch( arg, "atompath", 1 ) ) {
			getNextArgStr( &vg.atompath, "atompath" );
		} else if ( keymatch( arg, "checklevel", 1 ) ) {
			getNextArgStr( &vg.checklevelstr, "checklevel" );
		} else if ( keymatch( arg, "printtype", 1 ) ) {
			getNextArgStr( &vg.printtypestr, "printtype" );
        } else if ( keymatch( arg, "infofile", 1 ) ) {
                getNextArgStr( &vg.segmentOffsetInfo, "infofile" ); gotSegmentInfoFile = true;
        } else if ( keymatch( arg, "segal", 5 ) ) {
                vg.checkSegAlignment = true;
        } else if ( keymatch( arg, "ssegal", 6 ) ) {
            vg.checkSubSegAlignment = true;
        } else if ( keymatch( arg, "isolive", 7 ) ) {
                vg.isoLive = true;
        } else if ( keymatch( arg, "isoondemand", 7 ) ) {
                vg.isoondemand = true;
        } else if ( keymatch( arg, "isomain", 7 ) ) {
                vg.isomain = true;
        } else if ( keymatch( arg, "level", 5 ) ) {
                vg.subRepLevel = true;
        } else if ( keymatch( arg, "startwithsap", 6 ) ) {
                getNextArgStr( &sapType, "startwithsap" );vg.startWithSAP = atoi(sapType);
        } else if ( keymatch( arg, "bss", 3 ) ) {
                vg.bss = true; vg.checkSegAlignment = true; //The conditions required for setting the @segmentAlignment attribute to a value other than 'false' for the Adaptation Set are fulfilled.
        } else if ( keymatch( arg, "leafinfo", 1 ) ) {
                getNextArgStr( &leafInfoFileName, "leafinfo" ); gotleafInfoFile = true;
		} else if ( keymatch( arg, "samplenumber", 1 ) ) {
			getNextArgStr( &vg.samplenumberstr, "samplenumber" );



		} else {
			fprintf( stderr, "Unexpected option \"%s\"\n", arg);
			err = -1;
			goto usageError;
		}
	}
	

	//=====================
	// Process input parameters
	
	if ((usedefaultfiletype && (vg.filetypestr[0] == 0)) ||				// default to mp4
		      (strcmp(vg.filetypestr, "mp4") == 0)) {
		vg.filetype = filetype_mp4;
	} else if (strcmp(vg.filetypestr, "mp4v") == 0) {
		vg.filetype = filetype_mp4v;
	} else if (vg.filetype == 0) {
		fprintf( stderr, "Invalid filetype\n" );
		err = -1;
		goto usageError;
	}

	if (vg.checklevelstr[0] == 0) {
		vg.checklevel = 1;				// default
	} else {
		vg.checklevel = atoi(vg.checklevelstr);
		if (vg.checklevel < 1) {
			fprintf( stderr, "Invalid check level\n" );
			goto usageError;
		}
	}

	if (vg.printtypestr[0] == 0) {
		// default is not to print anything
	} else {
		char instr[256];
		char *tokstr;

		strcpy(instr, vg.printtypestr);
		tokstr = strtok(instr,"+");
		while (tokstr) {
			if        (keymatch(tokstr, "atompath", 5)) {
				vg.print_atompath = true;
			} else if (keymatch(tokstr, "atom", 4)) {
				vg.print_atom = true;
			} else if (keymatch(tokstr, "fulltable", 1)) {
				vg.print_fulltable = true;
			} else if (keymatch(tokstr, "sampleraw", 7)) {
				vg.print_sampleraw = true;
			} else if (keymatch(tokstr, "sample", 6)) {
				vg.print_sample = true;
			} else if (keymatch(tokstr, "hintpayload", 1)) {
				vg.print_hintpayload = true;
			} else {
				fprintf( stderr, "Invalid print type option\n" );
				goto usageError;
			}
			tokstr = strtok(nil,"+");
		}
	}

	if (vg.samplenumberstr[0] == 0) {
		vg.samplenumber = 0;			// zero means print them all if you print any
	} else {
		vg.samplenumber = atoi(vg.samplenumberstr);
		if (vg.samplenumber < 1) goto usageError;
	}

	//=====================

	if (!gotInputFile) {
		err = -1;
		fprintf( stderr, "No input file specified\n" );
		goto usageError;
	}

    infile = fopen(gInputFileFullPath, "rb");
	if (!infile) {
		err = -1;
		fprintf( stderr, "Could not open input file \"%s\"\n", gInputFileFullPath );
		goto usageError;
	}

	fprintf(stdout,"\n\n\n<!-- Source file is '%s' -->\n", gInputFileFullPath);

	vg.inFile = infile;
	vg.inOffset = 0;
	err = fseek(infile, 0, SEEK_END);
	if (err) goto bail;
	vg.inMaxOffset = ftell( infile );
	if (vg.inMaxOffset < 0) {
		err = vg.inMaxOffset;
		goto bail;
	}

	aoe.type = 'file';
	aoe.size = vg.inMaxOffset;
	aoe.offset = 0;
	aoe.atomStartSize = 0;
	aoe.maxOffset = aoe.size;
	
	vg.fileaoe = &aoe;		// used when you need to read file & size from the file
	
    if(gotSegmentInfoFile)
    {
        int numSegments = 0;
        
        for(int ii = 0 ; ii < 2 ; ii++)
        {
            FILE *segmentOffsetInfoFile = fopen(vg.segmentOffsetInfo, "rb");
        	if (!segmentOffsetInfoFile) {
        		err = -1;
        		fprintf( stderr, "Could not open segment info file \"%s\"\n", vg.segmentOffsetInfo );
        		goto usageError;
        	}

            if(ii == 1)
            {
                vg.segmentSizes = (UInt64 *)malloc(sizeof(UInt64)*numSegments);
                vg.segmentInfoSize = numSegments;
                vg.simsInStyp = (bool *)malloc(sizeof(bool)*numSegments);
            }

            numSegments = 0;

            while(1)
            {
                int temp1;
                UInt64 temp2;
                int ret = fscanf(segmentOffsetInfoFile,"%d %lld\n",&temp1,&temp2);
                if(ret < 2)
                    break;
                
                if(ii == 1)
                {
                    vg.segmentSizes[numSegments] = temp2;
                    vg.simsInStyp[numSegments] = false;
                }
                numSegments++;
                if(numSegments == 1 && temp1 > 0)
                    vg.initializationSegment=false;
                else
                    vg.initializationSegment=true;
                
            }

            if(numSegments == 0)
                {
                    err = -1;
                    fprintf( stderr, "Empty segment info file \"%s\"\n", vg.segmentOffsetInfo );
                    goto usageError;
                }

            fclose(segmentOffsetInfoFile);
        }
        vg.brandDASH = true;    //Either this, or for non-segmented file = self-intializing segment, brand DASH shall be in ftyp
    }
    else
    {
        vg.segmentSizes = (UInt64 *)malloc(sizeof(UInt64)*1);
        vg.segmentInfoSize = 1;
        vg.initializationSegment=false;
        vg.segmentSizes[0] = aoe.size;
        vg.simsInStyp = (bool *)malloc(sizeof(bool)*1);
        vg.simsInStyp[0] = false;
        vg.brandDASH = false;   //For non-segmented file = self-intializing segment, brand DASH shall be in ftyp and this will be re-initialized
    }
    vg.processedStypes = 0;
    vg.accessUnitDurationNonIndexedTrack = 0;

    if(vg.checkSegAlignment || vg.checkSubSegAlignment || vg.bss)
    {
        if(gotleafInfoFile)
            loadLeafInfo(leafInfoFileName);
        else
        {
            printf("Segment/Subsegment alignment check request, leaf info file not found!\n");
            vg.checkSegAlignment = vg.checkSubSegAlignment = false;
        }
    }
		
	if (vg.filetype == filetype_mp4v) {
		err = ValidateElementaryVideoStream( &aoe, nil );
	} else {
		err = ValidateFileAtoms( &aoe, nil );
		fprintf(stdout,"<!#- Finished testing file '%s' -->\n", gInputFileFullPath);
	}
    
	goto bail;
	
	//=====================

usageError:
	fprintf( stderr, "Usage: %s [-filetype <type>] "
								"[-printtype <options>] [-checklevel <level>] [-infofile <Segment Info File>] [-leafinfo <Leaf Info File>] [-segal] [-ssegal] [-startwithsap] [-level] [-isolive] [-isoondemand] [-isomain]\n", "ValidateMP4" );
	fprintf( stderr, "            [-samplenumber <number>] [-verbose <options> [-help] inputfile\n" );
	fprintf( stderr, "    -a[tompath] <atompath> - limit certain operations to <atompath> (e.g. moov-1:trak-2)\n" );
	fprintf( stderr, "                     this effects -checklevel and -printtype (default is everything) \n" );
	fprintf( stderr, "    -p[rinttype] <options> - controls output (combine options with +) \n" );
	fprintf( stderr, "                     atompath - output the atompath for each atom \n" );
	fprintf( stderr, "                     atom - output the contents of each atom \n" );
	fprintf( stderr, "                     fulltable - output those long tables (e.g. samplesize tables)  \n" );
	fprintf( stderr, "                     sample - output the samples as well \n" );
	fprintf( stderr, "                                 (depending on the track type, this is the same as sampleraw) \n" );
	fprintf( stderr, "                     sampleraw - output the samples in raw form \n" );
	fprintf( stderr, "                     hintpayload - output payload for hint tracks \n" );
	fprintf( stderr, "    -c[hecklevel] <level> - increase the amount of checking performed \n" );
	fprintf( stderr, "                     1: check the moov container (default -atompath is ignored) \n" );
	fprintf( stderr, "                     2: check the samples \n" );
	fprintf( stderr, "                     3: check the payload of hint track samples \n" );
	fprintf( stderr, "    -i[nfofile] <Segment Info File> - Offset file generated by assembler \n" );
	fprintf( stderr, "    -leafinfo <Leaf Info File> - Information file generated by this software (named leafinfo.txt) for another representation, provided to run for cross-checks of alignment\n" );
	fprintf( stderr, "    -segal  -         Check Segment alignment based on <Leaf Info File>\n" );
	fprintf( stderr, "    -ssegal -         Check Subegment alignment based on <Leaf Info File>\n" );
	fprintf( stderr, "    -isolive          Make checks specific for media segments conforming to ISO Base media file format live profile\n" );
	fprintf( stderr, "    -isoondemand      Make checks specific for media segments conforming to ISO Base media file format On Demand profile\n" );
	fprintf( stderr, "    -isomain          Make checks specific for media segments conforming to ISO Base media file format main profile\n" );
	fprintf( stderr, "    -startwithsap     Check for a specific SAP type as announced in the MPD\n" );
	fprintf( stderr, "    -level            SubRepresentation@level checks\n" );
	fprintf( stderr, "    -bss              Make checks specific for bitstream switching\n" );
	fprintf( stderr, "    -s[amplenumber] <number> - limit sample checking or printing operations to sample <number> \n" );
	fprintf( stderr, "                     most effective in combination with -atompath (default is all samples) \n" );

	fprintf( stderr, "    -h[elp] - print this usage message \n" );


	//=====================

bail:
	if (infile) {
		fclose(infile);
	}
	
	return err;
}

void loadLeafInfo(char *leafInfoFileName)
{
    FILE *leafInfoFile = fopen(leafInfoFileName,"rt");
    if(leafInfoFile == NULL)
    {
        printf("Leaf info file %s not found, alignment wont be checked!\n",leafInfoFileName);
        vg.checkSegAlignment = vg.checkSubSegAlignment = false;
        vg.bss = false;
        return;
    }
    
    fscanf(leafInfoFile,"%lu\n",&vg.accessUnitDurationNonIndexedTrack);
    
    fscanf(leafInfoFile,"%u\n",&vg.numControlTracks);
    
    vg.controlLeafInfo = (LeafInfo **)malloc(vg.numControlTracks*sizeof(LeafInfo *));
    vg.numControlLeafs = (unsigned int *)malloc(vg.numControlTracks*sizeof(unsigned int));
    vg.trackTypeInfo = (TrackTypeInfo *)malloc(vg.numControlTracks*sizeof(TrackTypeInfo));

    for(unsigned int i = 0 ; i < vg.numControlTracks ; i++)
    {
        fscanf(leafInfoFile,"%lu %lu\n",&vg.trackTypeInfo[i].track_ID,&vg.trackTypeInfo[i].componentSubType);
    }
    
    for(unsigned int i = 0 ; i < vg.numControlTracks ; i++)
    {
        fscanf(leafInfoFile,"%u\n",&(vg.numControlLeafs[i]));

        vg.controlLeafInfo[i] = (LeafInfo *)malloc(vg.numControlLeafs[i]*sizeof(LeafInfo));
        
        for(UInt32 j = 0 ; j < vg.numControlLeafs[i] ; j++)
            fscanf(leafInfoFile,"%d %Lf %Lf\n",(int *)&vg.controlLeafInfo[i][j].firstInSegment,&vg.controlLeafInfo[i][j].earliestPresentationTime,&vg.controlLeafInfo[i][j].lastPresentationTime);
            
    }

    fclose(leafInfoFile);
}


//==========================================================================================

#include <stdarg.h>
// change here if you want to send both types of output to stdout to get interleaved output
#if 1
	#define _stdout stdout
	#define _stderr stderr
#else
	#define _stdout stdout
	#define _stderr stdout
#endif

void toggleprintatom( Boolean onOff )
{

	// need hackaround to print certain things - can't figure out arg scheme
	// true is on, false is off
	vg.printatom = onOff;
//	if( vg.printatom )
//		fprintf( _stdout, "--> turning ON vg.printatom \n");
//	else
//		fprintf( _stdout, "--> turning OFF vg.printatom \n" );

}

void toggleprintatomdetailed( Boolean onOff )
{

	// need hackaround to print certain things - can't figure out arg scheme
	// true is on, false is off
	vg.printatom = onOff;
	vg.print_fulltable = onOff;
//	if( vg.printatom )
//		fprintf( _stdout, "--> turning ON vg.printatom and vg.print_fulltable \n" );
//	else
//		fprintf( _stdout, "--> turning OFF vg.printatom and vg.print_fulltable \n" );

}

void toggleprintsample( Boolean onOff )
{

	// need hackaround to print certain things - can't figure out arg scheme
	// true is on, false is off
	vg.printsample = onOff;

}


void atomprintnotab(const char *formatStr, ...)
{
	va_list 		ap;
	va_start(ap, formatStr);
	
	if (vg.printatom) {
		vfprintf( _stdout, formatStr, (va_list)(char *)ap );
	}
	
	va_end(ap);
}

void atomprint(const char *formatStr, ...)
{
	va_list 		ap;
	va_start(ap, formatStr);
	
	if (vg.printatom) {
		long tabcnt = vg.tabcnt;
		while (tabcnt--) {
			fprintf(_stdout,myTAB);
		}
		vfprintf( _stdout, formatStr, (va_list)(char *)ap );
	}
	
	va_end(ap);
}

void atomprinthexdata(char *dataP, UInt32 size)
{
	char hexstr[4] = "12 ";
	int widthCnt = 0;
	char c;
	static char hc[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	
	while (size) {
		c = *dataP++;
		hexstr[0] = hc[(c>>4)&0x0F];
		hexstr[1] = hc[(c   )&0x0F];
		if (widthCnt == 0) {
			atomprint(hexstr);
		} else { 
			atomprintnotab(hexstr);
		}
		if (++widthCnt >= 16) {
			widthCnt = 0;
			atomprint("\n");
		}
		size--;
	}
	if (widthCnt != 0)
		atomprint("\n");
}



void atomprintdetailed(const char *formatStr, ...)
{
	va_list 		ap;
	va_start(ap, formatStr);
	
	if (vg.printatom && vg.print_fulltable) {
		long tabcnt = vg.tabcnt;
		while (tabcnt--) {
			fprintf(_stdout,myTAB);
		}
		vfprintf( _stdout, formatStr, (va_list)(char *)ap );
	}
	
	va_end(ap);
}

void sampleprint(const char *formatStr, ...)
{
	va_list 		ap;
	va_start(ap, formatStr);
	
	if (vg.printsample) {
		long tabcnt = vg.tabcnt;
		while (tabcnt--) {
			fprintf(_stdout,myTAB);
		}
		vfprintf( _stdout, formatStr, (va_list)(char *)ap );
	}
	
	va_end(ap);
}

void sampleprintnotab(const char *formatStr, ...)
{
	va_list 		ap;
	va_start(ap, formatStr);
	
	if (vg.printsample) {
		vfprintf( _stdout, formatStr, (va_list)(char *)ap );
	}
	
	va_end(ap);
}

void sampleprinthexdata(char *dataP, UInt32 size)
{
	char hexstr[4] = "12 ";
	int widthCnt = 0;
	char c;
	static char hc[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	
	while (size) {
		c = *dataP++;
		hexstr[0] = hc[(c>>4)&0x0F];
		hexstr[1] = hc[(c   )&0x0F];
		if (widthCnt == 0) {
			sampleprint(hexstr);
		} else { 
			sampleprintnotab(hexstr);
		}
		if (++widthCnt >= 16) {
			widthCnt = 0;
			sampleprint("\n");
		}
		size--;
	}
	if (widthCnt != 0)
		sampleprint("\n");
}


void sampleprinthexandasciidata(char *dataP, UInt32 size)
{
	char hexstr[4] = "12 ";  
	char asciiStr[17];
	int widthCnt = 0;
	char threeSpaces[4] = "   ";
	char c;
	static char hc[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
		
	
	// similar to sampleprinthexdata() but also prints ascii characters to the right of hex dump
	//   (ala Mac OS X's HexDump or 9's MacsBug; if the character is not ascii, it will print a '.' )
	// sampleprintnotab("\n");
	
	asciiStr[16] = 0;
	while (size) {
		c = *dataP++;
		hexstr[0] = hc[(c>>4)&0x0F];
		hexstr[1] = hc[(c   )&0x0F];
		
		if( isprint( c ) && c != 0 )
			asciiStr[ widthCnt ] = c ;
		else
			asciiStr[ widthCnt ] = '.';	
			
		if (widthCnt == 0) {
			sampleprint(hexstr);
		} else { 
			sampleprintnotab(hexstr);
		}
		if (++widthCnt >= 16) {
			sampleprintnotab( threeSpaces );  // some space between hex chars and ascii chars
			sampleprintnotab( asciiStr );  // prints the ascii characters to the right of hex dump

			widthCnt = 0;
			sampleprintnotab("\n");
		}
		
		size--;
	}
	if (widthCnt != 0){
		
			// for the last line, fill out the rest of the hex row with blanks
			//   and fill the unused right end of asciiStr with blanks
		while( widthCnt < 16 ){
			sampleprintnotab( threeSpaces );
			asciiStr[ widthCnt++ ] = ' ';		
		}
		
		
		sampleprintnotab( threeSpaces );
		sampleprintnotab( asciiStr );
		sampleprintnotab("\n");
	}
	
	// sampleprintnotab("\n");

}


void warnprint(const char *formatStr, ...)
{
	va_list 		ap;
	va_start(ap, formatStr);
	
	if (vg.warnings)
		vfprintf( _stderr, formatStr, (va_list)(char *)ap );
	
	va_end(ap);
}


void errprint(const char *formatStr, ...)
{
	va_list 		ap;
	va_start(ap, formatStr);
	
	fprintf( _stderr, "### error: %s \n###        ",vg.curatompath);
	vfprintf( _stderr, formatStr, (va_list)(char *)ap );
	
	va_end(ap);
}

int my_stricmp(const char* p, const char* q)
{
	while (tolower(*p) == tolower(*q) && *p && *q)
	{
		p++;
		q++;
	}
	
	return tolower(*p) - tolower(*q);
}

int mapStringToUInt32(char *src, UInt32 *target)
{
    if(src == NULL || target == NULL)
    {
        fprintf(stderr, "mapStringToUInt32: NULL pointer exception");
        return -1;
    }

    char *tmp = (char *)target;

    //Write in reverse (big-endian) format)

    tmp[0] = src[3];
    tmp[1] = src[2];
    tmp[2] = src[1];
    tmp[3] = src[0];

    return 0;

}

char *ostypetostr(UInt32 num)
{
	static char str[sizeof(num)+1] = {0};
	
	str[0] = (num >> 24) & 0xff;
	str[1] = (num >> 16) & 0xff;
	str[2] = (num >>  8) & 0xff;
	str[3] = (num >>  0) & 0xff;

	return str;
}

char *ostypetostr_r(UInt32 num, char * buffer)
{
	buffer[0] = (num >> 24) & 0xff;
	buffer[1] = (num >> 16) & 0xff;
	buffer[2] = (num >>  8) & 0xff;
	buffer[3] = (num >>  0) & 0xff;
	buffer[4] = 0;

	return buffer;
}

//  careful about using more than one call to this in the same print statement, they end up all being the same
//    for cases where you need it more than once in the same print statment, use int64toxstr_r() instead
char *int64toxstr(UInt64 num)
{
	static char str[20];
	UInt32 hi,lo;
	
	hi = num>>32;
	lo = num&(0xffffffff);
	if (hi) {
		sprintf(str,"0x%lx%8.8lx",hi,lo);
	} else {
		sprintf(str,"0x%lx",lo);
	}
	return str;
}

char *int64toxstr_r(UInt64 num, char * str)
{
	UInt32 hi,lo;
	
	hi = num>>32;
	lo = num&(0xffffffff);
	if (hi) {
		sprintf(str,"0x%lx%8.8lx",hi,lo);
	} else {
		sprintf(str,"0x%lx",lo);
	}
	return str;
}

//  careful about using more than one call to this in the same print statement, they end up all being the same
//    for cases where you need it more than once in the same print statment, use int64toxstr_r() instead
char *int64todstr(UInt64 num)
{
	static char str[40];
	UInt32 hi,lo;
	
	hi = num>>32;
	lo = num&(0xffffffff);
	
	if (hi) 
		sprintf(str,"%ld%8.8ld",hi,lo);
	else
		sprintf(str,"%ld",lo);
	return str;
}


char *int64todstr_r(UInt64 num, char * str)
{
	UInt32 hi,lo;
	
	hi = num>>32;
	lo = num&(0xffffffff);
	
	if (hi) 
		sprintf(str,"%ld%8.8ld",hi,lo);
	else
		sprintf(str,"%ld",lo);
	return str;
}

//  careful about using more than one call to this in the same print statement, they end up all being the same
char *langtodstr(UInt16 num)
{
	static char str[5];

	str[4] = 0;
	
	if (num==0) {
		str[0] = str[1] = str[2] = ' ';
	}
	else {
		str[0] = ((num >> 10) & 0x1F) + 0x60;
		str[1] = ((num >> 5 ) & 0x1F) + 0x60;
		str[2] = ( num        & 0x1F) + 0x60;
	}
			
	return str;
}


//  careful about using more than one call to this in the same print statement, they end up all being the same
//    for cases where you need it more than once in the same print statment, use fixed16str_r() instead
char *fixed16str(SInt16 num)
{
	static char str[40];
	float f;
	
	f = num;
	f /= 0x100;
	
	sprintf(str,"%f",f);

	return str;
}

char *fixed16str_r(SInt16 num, char * str)
{
	float f;
	
	f = num;
	f /= 0x100;
	
	sprintf(str,"%f",f);

	return str;
}


//  careful about using more than one call to this in the same print statement, they end up all being the same
//    for cases where you need it more than once in the same print statment, use fixed32str_r() instead
char *fixed32str(SInt32 num)
{
	static char str[40];
	double f;
	
	f = num;
	f /= 0x10000;
	
	sprintf(str,"%lf",f);

	return str;
}

char *fixed32str_r(SInt32 num, char * str)
{
	double f;
	
	f = num;
	f /= 0x10000;
	
	sprintf(str,"%lf",f);

	return str;
}



//  careful about using more than one call to this in the same print statement, they end up all being the same
//    for cases where you need it more than once in the same print statment, use fixedU32str_r() instead
char *fixedU32str(UInt32 num)
{
	static char str[40];
	double f;
	
	f = num;
	f /= 0x10000;
	
	sprintf(str,"%lf",f);

	return str;
}

char *fixedU32str_r(UInt32 num, char * str)
{
	double f;
	
	f = num;
	f /= 0x10000;
	
	sprintf(str,"%lf",f);

	return str;
}

    //  copy non-terminated C string (chars) to terminated C string (str)
void copyCharsToStr( char *chars, char *str, UInt16 count ){
    SInt16 i;
    
    for( i = 0; i < count; ++i )
        str[i] = chars[i];
        
    str[ count ] = 0;

}

//==========================================================================================

void addEscapedChar( char *str, char c );
void addEscapedChar( char *str, char c )
{
	char addc[4] = {0};
	
	if ((('a' <= c) && (c <= 'z'))
		|| (('A' <= c) && (c <= 'Z'))
		|| (('0' <= c) && (c <= '9'))
	//	add extra chars here
	//  we want to escape - & . for now 
		) {
		addc[0] = c;
	} else {
		char n;
		
		addc[0] = '%';
		
		n = ((c >> 4) & 0x0F);
		if (n < 10)
			n = n + '0';
		else
			n = (n - 10) + 'a';
		addc[1] = n;
		
		n = ((c) & 0x0F);
		if (n < 10)
			n = n + '0';
		else
			n = (n - 10) + 'a';
		addc[2] = n;
	}
		
	strcat(str, addc);
}

void addAtomToPath( atompathType workingpath, OSType atomId, long atomIndex, atompathType curpath )
{
	strcpy( curpath, workingpath );
	if (workingpath[0])
		strcat( workingpath, ":");
	addEscapedChar(workingpath, (atomId>>24) & 0xff);
	addEscapedChar(workingpath, (atomId>>16) & 0xff);
	addEscapedChar(workingpath, (atomId>> 8) & 0xff);
	addEscapedChar(workingpath, (atomId>> 0) & 0xff);
	strcat( workingpath, "-");
	sprintf(&workingpath[strlen(workingpath)],"%ld",atomIndex);
}

void restoreAtomPath( atompathType workingpath, atompathType curpath )
{
	strcpy( workingpath, curpath );
}

//===========================================================
//  Validate a Video Elementary Stream
//===========================================================

OSErr ValidateElementaryVideoStream( atomOffsetEntry *aoe, void *refcon )
{
#pragma unused(refcon)
	TrackInfoRec tir = {0};
	OSErr err = noErr;
	UInt32 startCode;
	UInt32 prevStartCode;
	UInt64 offset1 = aoe->offset;
	UInt64 offset2, offset3;
	UInt32 sampleNum = 0;
	BitBuffer bb;
	Ptr dataP;
	UInt32 dataSize;
	UInt32 refcons[2];
	
	if (vg.checklevel < checklevel_samples)
		vg.checklevel = checklevel_samples;

	tir.sampleDescriptionCnt = 1;
	tir.validatedSampleDescriptionRefCons = &refcons[0];
	
	err = GetFileStartCode( aoe, &prevStartCode, offset1, &offset2 );
	if (err) {
		fprintf(stderr,"### did NOT find ANY start codes\n");
		goto bail;
	}
	
	do {
		err = GetFileStartCode( aoe, &startCode, offset2, &offset3 );
		
		if (err) {
			offset3 = aoe->maxOffset;
		}
		
		if (err || (startCode == 0x000001B6) || (startCode == 0x000001B3)) {
			if (!err && prevStartCode == 0x000001B3) {
				goto nextone;
			}
			
			dataSize = offset3 - offset1;
			BAILIFNIL( dataP = (Ptr)malloc(dataSize), allocFailedErr );
			err = GetFileData( vg.fileaoe, dataP, offset1, dataSize, nil );
			
			err = BitBuffer_Init(&bb, (UInt8 *)dataP, dataSize);

			if (sampleNum == 0) {
				atomprint("<Video_Sample_Description offset=\"%s\" size=\"%d\"",
								int64toxstr(offset1),dataSize); vg.tabcnt++;
					Validate_vide_ES_Bitstream( &bb, &tir );
				--vg.tabcnt; atomprint("</Video_Sample_Description>\n");
			} else {
				atomprint("<Video_Sample sample_num=\"%d\" offset=\"%s\" size=\"%d\"",
								sampleNum, int64toxstr(offset1),dataSize); vg.tabcnt++;
					Validate_vide_sample_Bitstream( &bb, &tir );
				--vg.tabcnt; atomprint("</Video_Sample_Description>\n");
			}
			
			sampleNum++;
			offset1 = offset2 = offset3;
		}
nextone:
		prevStartCode = startCode;
		offset2 = offset3 + 4;
	} while (!err);
	
	
	
bail:
	return err;
}


