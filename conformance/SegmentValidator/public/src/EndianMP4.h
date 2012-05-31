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

#ifndef __ENDIAN__
#define __ENDIAN__

#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif

/*
    This file provides Endian Flipping routines for dealing with converting data
    between Big-Endian and Little-Endian machines.  These routines are useful
    when writing code to compile for both Big and Little Endian machines and  
    which must handle other endian number formats, such as reading or writing 
    to a file or network packet.
    
    These routines are named as follows:
    
        Endian<U><W>_<S>to<D>

    where
        <U> is whether the integer is signed ('S') or unsigned ('U')
        <W> is integer bit width: 16, 32, or 64 
        <S> is the source endian format: 'B' for big, 'L' for little, or 'N' for native
        <D> is the destination endian format: 'B' for big, 'L' for little, or 'N' for native
    
    For example, to convert a Big Endian 32-bit unsigned integer to the current native format use:
        
        long i = EndianU32_BtoN(data);
        
    This file is set up so that the function macro to nothing when the target runtime already
    is the desired format (e.g. on Big Endian machines, EndianU32_BtoN() macros away).
            
    If long long's are not supported, you cannot get 64-bit quantities as a single value.
    The macros are not defined in that case.
    
    
    
                                <<< W A R N I N G >>>
    
    It is very important not to put any autoincrements inside the macros.  This 
    will produce erroneous results because each time the address is accessed in the macro, 
    the increment occurs.
    
 */

SInt16 
EndianS16_BtoN                  (SInt16                 value);

SInt16 
EndianS16_NtoB                  (SInt16                 value);

SInt16 
EndianS16_LtoN                  (SInt16                 value);

SInt16 
EndianS16_NtoL                  (SInt16                 value);

SInt16 
EndianS16_LtoB                  (SInt16                 value);

SInt16 
EndianS16_BtoL                  (SInt16                 value);

UInt16 
EndianU16_BtoN                  (UInt16                 value);

UInt16 
EndianU16_NtoB                  (UInt16                 value);

UInt16 
EndianU16_LtoN                  (UInt16                 value);

UInt16 
EndianU16_NtoL                  (UInt16                 value);

UInt16 
EndianU16_LtoB                  (UInt16                 value);

UInt16 
EndianU16_BtoL                  (UInt16                 value);

UInt32 
EndianU24_BtoN                  (UInt16                 value);

SInt32 
EndianS32_BtoN                  (SInt32                 value);

SInt32 
EndianS32_NtoB                  (SInt32                 value);

SInt32 
EndianS32_LtoN                  (SInt32                 value);

SInt32 
EndianS32_NtoL                  (SInt32                 value);

SInt32 
EndianS32_LtoB                  (SInt32                 value);

SInt32 
EndianS32_BtoL                  (SInt32                 value);

UInt32 
EndianU32_BtoN                  (UInt32                 value);

UInt32 
EndianU32_NtoB                  (UInt32                 value);

UInt32 
EndianU32_LtoN                  (UInt32                 value);

UInt32 
EndianU32_NtoL                  (UInt32                 value);

UInt32 
EndianU32_LtoB                  (UInt32                 value);

UInt32 
EndianU32_BtoL                  (UInt32                 value);

#if !TYPE_LONGLONG
/*
   Note: If these Int64 functions ever were implemented in a library,
         we would need two libraries, one for compilers that
         support long long and one for other compilers.
*/
SInt64 
EndianS64_BtoN                  (SInt64                 value);

SInt64 
EndianS64_NtoB                  (SInt64                 value);

SInt64 
EndianS64_LtoN                  (SInt64                 value);

SInt64 
EndianS64_NtoL                  (SInt64                 value);

SInt64 
EndianS64_LtoB                  (SInt64                 value);

SInt64 
EndianS64_BtoL                  (SInt64                 value);

UInt64 
EndianU64_BtoN                  (UInt64                 value);

UInt64 
EndianU64_NtoB                  (UInt64                 value);

UInt64 
EndianU64_LtoN                  (UInt64                 value);

UInt64 
EndianU64_NtoL                  (UInt64                 value);

UInt64 
EndianU64_LtoB                  (UInt64                 value);

UInt64 
EndianU64_BtoL                  (UInt64                 value);

#endif  /* !TYPE_LONGLONG */

/*
   These types are used for structures that contain data that is
   always in BigEndian format.  This extra typing prevents little
   endian code from directly changing the data, thus saving much
   time in the debugger.
*/

#if TARGET_RT_LITTLE_ENDIAN

struct BigEndianLong {
    long                            bigEndianValue;
};
typedef struct BigEndianLong            BigEndianLong;

struct BigEndianUnsignedLong {
    unsigned long                   bigEndianValue;
};
typedef struct BigEndianUnsignedLong    BigEndianUnsignedLong;

struct BigEndianShort {
    short                           bigEndianValue;
};
typedef struct BigEndianShort           BigEndianShort;

struct BigEndianUnsignedShort {
    unsigned short                  bigEndianValue;
};
typedef struct BigEndianUnsignedShort   BigEndianUnsignedShort;

struct BigEndianFixed {
    Fixed                           bigEndianValue;
};
typedef struct BigEndianFixed           BigEndianFixed;

struct BigEndianUnsignedFixed {
    UnsignedFixed                   bigEndianValue;
};
typedef struct BigEndianUnsignedFixed   BigEndianUnsignedFixed;

struct BigEndianOSType {
    OSType                          bigEndianValue;
};
typedef struct BigEndianOSType          BigEndianOSType;
#else

typedef long                            BigEndianLong;
typedef unsigned long                   BigEndianUnsignedLong;
typedef short                           BigEndianShort;
typedef unsigned short                  BigEndianUnsignedShort;
typedef Fixed                           BigEndianFixed;
typedef UnsignedFixed                   BigEndianUnsignedFixed;
typedef OSType                          BigEndianOSType;
#endif  /* TARGET_RT_LITTLE_ENDIAN */


/*
    Macro away no-op functions
*/
#if TARGET_RT_BIG_ENDIAN
 #define EndianS16_BtoN(value)               (value)
    #define EndianS16_NtoB(value)               (value)
    #define EndianU16_BtoN(value)               (value)
    #define EndianU16_NtoB(value)               (value)
    #define EndianU24_BtoN(value)               (value)
    #define EndianS32_BtoN(value)               (value)
    #define EndianS32_NtoB(value)               (value)
    #define EndianU32_BtoN(value)               (value)
    #define EndianU32_NtoB(value)               (value)
    #define EndianS64_BtoN(value)               (value)
    #define EndianS64_NtoB(value)               (value)
    #define EndianU64_BtoN(value)               (value)
    #define EndianU64_NtoB(value)               (value)
#else
  #define EndianS16_LtoN(value)               (value)
    #define EndianS16_NtoL(value)               (value)
    #define EndianU16_LtoN(value)               (value)
    #define EndianU16_NtoL(value)               (value)
    #define EndianS32_LtoN(value)               (value)
    #define EndianS32_NtoL(value)               (value)
    #define EndianU32_LtoN(value)               (value)
    #define EndianU32_NtoL(value)               (value)
    #define EndianS64_LtoN(value)               (value)
    #define EndianS64_NtoL(value)               (value)
    #define EndianU64_LtoN(value)               (value)
    #define EndianU64_NtoL(value)               (value)
#endif



/*
    Map native to actual
*/
#if TARGET_RT_BIG_ENDIAN
   #define EndianS16_LtoN(value)               EndianS16_LtoB(value)
  #define EndianS16_NtoL(value)               EndianS16_BtoL(value)
  #define EndianU16_LtoN(value)               EndianU16_LtoB(value)
  #define EndianU16_NtoL(value)               EndianU16_BtoL(value)
  #define EndianS32_LtoN(value)               EndianS32_LtoB(value)
  #define EndianS32_NtoL(value)               EndianS32_BtoL(value)
  #define EndianU32_LtoN(value)               EndianU32_LtoB(value)
  #define EndianU32_NtoL(value)               EndianU32_BtoL(value)
  #define EndianS64_LtoN(value)               EndianS64_LtoB(value)
  #define EndianS64_NtoL(value)               EndianS64_BtoL(value)
  #define EndianU64_LtoN(value)               EndianU64_LtoB(value)
  #define EndianU64_NtoL(value)               EndianU64_BtoL(value)
#else
    #define EndianS16_BtoN(value)               EndianS16_BtoL(value)
  #define EndianS16_NtoB(value)               EndianS16_LtoB(value)
  #define EndianU16_BtoN(value)               EndianU16_BtoL(value)
  #define EndianU16_NtoB(value)               EndianU16_LtoB(value)
  #define EndianU24_BtoN(value)               EndianU24_BtoL(value)
  #define EndianS32_BtoN(value)               EndianS32_BtoL(value)
  #define EndianS32_NtoB(value)               EndianS32_LtoB(value)
  #define EndianU32_BtoN(value)               EndianU32_BtoL(value)
  #define EndianU32_NtoB(value)               EndianU32_LtoB(value)
  #define EndianS64_BtoN(value)               EndianS64_BtoL(value)
  #define EndianS64_NtoB(value)               EndianS64_LtoB(value)
  #define EndianU64_BtoN(value)               EndianU64_BtoL(value)
  #define EndianU64_NtoB(value)               EndianU64_LtoB(value)
#endif



/*
    Implement ÅLtoB and ÅBtoL
*/
#define EndianS16_LtoB(value)              ((SInt16)Endian16_Swap(value))
#define EndianS16_BtoL(value)                ((SInt16)Endian16_Swap(value))
#define EndianU16_LtoB(value)                ((UInt16)Endian16_Swap(value))
#define EndianU16_BtoL(value)                ((UInt16)Endian16_Swap(value))
#define EndianU24_BtoL(value)                ((UInt32)Endian24_Swap(value))
#define EndianS32_LtoB(value)                ((SInt32)Endian32_Swap(value))
#define EndianS32_BtoL(value)                ((SInt32)Endian32_Swap(value))
#define EndianU32_LtoB(value)                ((UInt32)Endian32_Swap(value))
#define EndianU32_BtoL(value)                ((UInt32)Endian32_Swap(value))
#define EndianS64_LtoB(value)                ((SInt64)Endian64_Swap((UInt64)value))
#define EndianS64_BtoL(value)                ((SInt64)Endian64_Swap((UInt64)value))
#define EndianU64_LtoB(value)                ((UInt64)Endian64_Swap(value))
#define EndianU64_BtoL(value)                ((UInt64)Endian64_Swap(value))


/*
    Implement low level Å_Swap functions.
  
       extern UInt16 Endian16_Swap(UInt16 value);
     extern UInt32 Endian32_Swap(UInt32 value);
     extern UInt64 Endian64_Swap(UInt64 value);
     
   Note: Depending on the processor, you might want to implement
        these as function calls instead of macros.
   
*/

#define Endian16_Swap(value)                 \
     (((((UInt16)value)<<8) & 0xFF00)   | \
      ((((UInt16)value)>>8) & 0x00FF))

/* we give a 32-bit value in, but only want the first 3 bytes as a 24-bit value, so this
      would be wrong
 define Endian24_Swap(value)                 \
     (((((UInt32)value)<<16) & 0x00FF0000)  | \
      ((((UInt32)value)    ) & 0x0000FF00)  | \
      ((((UInt32)value)>>16) & 0x000000FF))
*/
      
#define Endian24_Swap(value)                     \
     (  \
      ((((UInt32)value)<< 8) & 0x00FF0000)  | \
      ((((UInt32)value)>> 8) & 0x0000FF00)  | \
      ((((UInt32)value)>>24) & 0x000000FF))

#define Endian32_Swap(value)                     \
     (((((UInt32)value)<<24) & 0xFF000000)  | \
      ((((UInt32)value)<< 8) & 0x00FF0000)  | \
      ((((UInt32)value)>> 8) & 0x0000FF00)  | \
      ((((UInt32)value)>>24) & 0x000000FF))


#if TYPE_LONGLONG
 #if defined(_MSC_VER)
        /* the inline macros crash MSDEV's optimizer on Windows. */
        extern UInt64 Endian64_Swap(UInt64 value);
 #else
      #define Endian64_Swap(value)                             \
             (((((UInt64)value)<<56) & 0xFF00000000000000ULL)  | \
               ((((UInt64)value)<<40) & 0x00FF000000000000ULL)  | \
               ((((UInt64)value)<<24) & 0x0000FF0000000000ULL)  | \
               ((((UInt64)value)<< 8) & 0x000000FF00000000ULL)  | \
               ((((UInt64)value)>> 8) & 0x00000000FF000000ULL)  | \
               ((((UInt64)value)>>24) & 0x0000000000FF0000ULL)  | \
               ((((UInt64)value)>>40) & 0x000000000000FF00ULL)  | \
               ((((UInt64)value)>>56) & 0x00000000000000FFULL))
  #endif
#else
   /* 
        Note: When using C compilers that don't support "long long",
             Endian64_Swap must be implemented as glue. 
  */
 #ifdef __cplusplus
     inline static UInt64 Endian64_Swap(UInt64 value)
       {
          UInt64 temp;
           ((UnsignedWide*)&temp)->lo = Endian32_Swap(((UnsignedWide*)&value)->hi);
           ((UnsignedWide*)&temp)->hi = Endian32_Swap(((UnsignedWide*)&value)->lo);
           return temp;
       }
  #else
      extern UInt64 Endian64_Swap(UInt64 value);
 #endif
#endif




#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ENDIAN__ */

