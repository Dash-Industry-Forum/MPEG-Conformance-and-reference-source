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



OSErr BitBuffer_Init(BitBuffer *bb, UInt8 *p, UInt32 length)
{
	int err = noErr;
	
	if (length > 0x0fffffff) {
		err = paramErr;
		goto bail;
	}

	bb->ptr = (void*)p;
	bb->length = length;
	
	bb->cptr = (void*)p;
	bb->cbyte = *bb->cptr;
	bb->curbits = 8;
	
	bb->bits_left = length * 8;
	
	bb->prevent_emulation   = 0;
	bb->emulation_position  = (bb->cbyte == 0 ? 1 : 0);

bail:
	return err;
}



OSErr GetBytes(BitBuffer *bb, UInt32 nBytes, UInt8 *p)
{
	OSErr err = noErr;
	unsigned int i;
	
	for (i = 0; i < nBytes; i++) {
		*p++ = (UInt8)GetBits(bb, 8, &err);
		if (err) break;
	}
	
	return err;
}

OSErr SkipBytes(BitBuffer *bb, UInt32 nBytes)
{
	OSErr err = noErr;
	unsigned int i;
	
	for (i = 0; i < nBytes; i++) {
		GetBits(bb, 8, &err);
		if (err) break;
	}
	
	return err;
}


UInt32 NumBytesLeft(BitBuffer *bb)
{
	UInt32 numBytes;
	
	numBytes = ((bb->bits_left + 7) / 8);
	return numBytes;
}

UInt32 GetBits(BitBuffer *bb, UInt32 nBits, OSErr *errout)
{
	OSErr err = noErr;
	int myBits;
	int myValue = 0; 
	int myResidualBits;
	int leftToRead;
	
	if (nBits==0) goto bail;
	
	if (nBits > bb->bits_left || 0 == bb->bits_left) {
		err = outOfDataErr;
		goto bail;
	}
	
    if (bb->curbits <= 0) {
        bb->cbyte = *++bb->cptr;
        bb->curbits = 8;
		
		if (bb->prevent_emulation != 0) {
			if ((bb->emulation_position >= 2) && (bb->cbyte == 3)) {
				bb->cbyte = *++bb->cptr;
				bb->bits_left -= 8;
				bb->emulation_position = 0;
				if (nBits>bb->bits_left) {
					err = outOfDataErr;
					goto bail;
				}
			}
			else if (bb->cbyte == 0) bb->emulation_position += 1;
			else bb->emulation_position = 0;
		}
	}
	
	if (nBits > bb->curbits)
		myBits = bb->curbits;
	else
		myBits = nBits;
		
	myValue = (bb->cbyte>>(8-myBits));
	myResidualBits = bb->curbits - myBits;
	leftToRead = nBits - myBits;
	bb->bits_left -= myBits;
	
	bb->curbits = myResidualBits;
	bb->cbyte = ((bb->cbyte) << myBits) & 0xff;

	if (leftToRead > 0) {
		UInt32 newBits;
		newBits = GetBits(bb, leftToRead, &err);
		myValue = (myValue<<leftToRead) | newBits;
	}
	
bail:	
	if (errout) *errout = err;
	return myValue;
}


UInt32 PeekBits(BitBuffer *bb, UInt32 nBits, OSErr *errout)
{
	OSErr err = noErr;
	BitBuffer curbb = *bb;
	int myBits;
	int myValue = 0;
	int myResidualBits;
	int leftToRead;
	
	if (nBits == 0) goto bail;
	
	if (nBits>bb->bits_left) {
		err = outOfDataErr;
		goto bail;
	}

	if (bb->curbits <= 0) {
		bb->cbyte = *++bb->cptr;
		bb->curbits = 8;
	}
	
	if (nBits > bb->curbits)
		myBits = bb->curbits;
	else
		myBits = nBits;
		
	myValue = (bb->cbyte>>(8-myBits));
	myResidualBits = bb->curbits - myBits;
	leftToRead = nBits - myBits;
	
	bb->curbits = myResidualBits;
	bb->cbyte = ((bb->cbyte) << myBits) & 0xff;
	
	if (leftToRead > 0) {
		UInt32 newBits;
		newBits = PeekBits(bb, leftToRead, &err);
		myValue = (myValue<<leftToRead) | newBits;
	}
	
bail:
	*bb = curbb;
	if (errout) *errout = err;
	return myValue;
}




OSErr GetDescriptorTagAndSize(BitBuffer *bb, UInt32 *tagOut, UInt32 *sizeOut)
{
	OSErr err = noErr;
	UInt32 tag = 0;
	UInt32 size = 0;
	UInt32 collectSize = 0;
	
	tag = GetBits(bb, 8, &err); if (err) goto bail;

	size = 0;
	collectSize = 0x80;
	while (collectSize & 0x80) {
		collectSize = GetBits(bb, 8, &err); if (err) goto bail;
		size <<= 7;
		size |= (collectSize & 0x7f);
	}

bail:
	*tagOut = tag;
	*sizeOut = size;
	
	return err;
}

#define kMPEG4_Video_StartCodeLength	32
#define kMPEG4_Video_StartCodeMask		0xFFFFFF00
#define kMPEG4_Video_StartCodeValue		0x00000100
#define kMPEG4_Video_StartCodeTagMask	0x000000FF


Boolean BitBuffer_IsVideoStartCode(BitBuffer *bb)
{
	OSErr			err = noErr;
	Boolean			isStartCode = false;
	UInt32			temp32;
	
	temp32 = PeekBits(bb, kMPEG4_Video_StartCodeLength, &err);
	if (err != noErr ) goto bail;
	if ((temp32 & kMPEG4_Video_StartCodeMask) == kMPEG4_Video_StartCodeValue) {
		isStartCode = true;
	}
bail:
	return isStartCode;
}

OSErr BitBuffer_GetVideoStartCode(BitBuffer *bb, unsigned char *outStartCode)
{
	OSErr			err = noErr;
	Boolean			isStartCode = false;
	UInt32			temp32;
	
	temp32 = GetBits(bb, kMPEG4_Video_StartCodeLength, &err);
	if (err != noErr ) goto bail;
	if ((temp32 & kMPEG4_Video_StartCodeMask) == kMPEG4_Video_StartCodeValue) {
		*outStartCode = temp32 & kMPEG4_Video_StartCodeTagMask;
	}
bail:
	return err;
}

UInt32 read_golomb_uev(BitBuffer *bb, OSErr *errout)
{
	OSErr err = noErr;
	
	UInt32 power = 1;
	UInt32 value = 0;
	UInt32 leading = 0;
	UInt32 nbits = 0;
	
	leading = GetBits(bb, 1, &err);  if (err) goto bail;
	
	while (leading == 0) { 
		power = power << 1;
		nbits++;
		leading = GetBits(bb, 1, &err);  if (err) goto bail;
	}
	
	if (nbits > 0) {
		value = GetBits( bb, nbits, &err); if (err) goto bail;
	}
	
bail:
	if (errout) *errout = err;
	return (power - 1 + value);
}

SInt32 read_golomb_sev(BitBuffer *bb, OSErr *errout)
{
	OSErr err = noErr;
	UInt32 uev;
	SInt32 val;
	
	uev = read_golomb_uev( bb, &err ); if (err) goto bail;
	if (uev & 1)
		val = (uev + 1)/2;
		else
		val = -1 * (uev/2);
bail:
	if (errout) *errout = err;
	return val;
}

UInt32 strip_trailing_zero_bits(BitBuffer *bb, OSErr *errout)
{
	OSErr err = noErr;
	UInt8 bit_check = 1;
	UInt8* byte_ptr;
	UInt32 trailing = 0, bits;
	
	bits = bb->bits_left;
	byte_ptr = bb->cptr;
	
	bits -= bb->curbits;
	byte_ptr++;
	
	byte_ptr += (bits / 8);
	bits = bits % 8;
	if (bits == 0) { bits = 8; byte_ptr--; }
	
	bit_check = 1 << (8- bits);
	
	while (( *byte_ptr & bit_check ) == 0) {
		trailing++;
		if (bit_check == 0x80) {
			if ((--byte_ptr) < bb->cptr) { 
				err = outOfDataErr;
				goto bail;
			}
			bit_check = 1;
		} else bit_check = bit_check << 1;
		bb->bits_left -= 1;
	}
	bail:
	if (errout != NULL) *errout = err;
	return trailing;
}