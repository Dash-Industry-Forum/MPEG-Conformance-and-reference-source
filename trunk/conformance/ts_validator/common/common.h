/* 
 * Copyright (C) 2012-   Alex Giladi
 * 
 * Written by Alex Giladi <alex.giladi@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef _STREAMKIT_COMMON_H_
#define _STREAMKIT_COMMON_H_

#include <stdlib.h>
#include <inttypes.h>

// fun thing to do: rewrite with C11 Generic syntax...

#define SKIT_PRINT_SINT32(arg )     fprintf( stderr,  "%s:%d (in %s) \t%s=%"PRId32"\n",  __FILE__, __LINE__, __PRETTY_FUNCTION__, #arg,  (arg ) );
#define SKIT_PRINT_UINT32(arg )     fprintf( stderr,  "%s:%d (in %s) \t%s=%"PRIu32"\n",  __FILE__, __LINE__, __PRETTY_FUNCTION__, #arg,  (arg ) );

#define SKIT_PRINT_SINT64(arg )  fprintf( stderr,  "%s:%d (in %s) \t%s=%"PRId64"\n",  __FILE__, __LINE__, __PRETTY_FUNCTION__, #arg,  (arg ) );
#define SKIT_PRINT_UINT64(arg )  fprintf( stderr,  "%s:%d (in %s) \t%s=%"PRIu64"\n",  __FILE__, __LINE__, __PRETTY_FUNCTION__, #arg,  (arg ) );

#define SKIT_PRINT_UINT32_HEX(arg ) fprintf( stderr,  "%s:%d (in %s) \t%s=0x%"PRIX32"\n",  __FILE__, __LINE__, __PRETTY_FUNCTION__, #arg,  (arg ) );
#define SKIT_PRINT_UINT64_HEX(arg ) fprintf( stderr,  "%s:%d (in %s) \t%s=0x%"PRIX64"\n",  __FILE__, __LINE__, __PRETTY_FUNCTION__, #arg,  (arg ) );

#define SKIT_PRINT_STRING( arg )   fprintf( stderr,  "%s:%d (in %s)\t%s=%s\n",  __FILE__, __LINE__, __PRETTY_FUNCTION__, #arg,  (arg ) );

#define SKIT_PRINT_DOUBLE( arg )     fprintf( stderr,  "%s:%d (in %s)\t%s=%lf\n",  __FILE__, __LINE__, __PRETTY_FUNCTION__, #arg,  (arg ) );


#define BSWAP32(X)  __builtin_bswap32((X))
#define BSWAP64(X)  __builtin_bswap64((X));

#define ARRAYSIZE(x)	((sizeof(x))/(sizeof((x)[0])))

typedef struct 
{
  uint8_t* bytes;
  size_t   len;
} buf_t;

#endif
