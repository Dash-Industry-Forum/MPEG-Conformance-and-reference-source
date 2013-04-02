/* 
 * libstructures - a library for generic data structures in C
 * Copyright (C) 2005-2008 Avail Media, Inc.
 * 
 * Written by Alex Izvorski <aizvorski@gmail.com>
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

#define START_TEST(x)  int x() { int rc = 1;
#define END_TEST return rc; }
#define fail_unless(c,m) if (! (c)) { printf("* failed: %s at %s:%d\n", (m), __FILE__, __LINE__); rc = 0; }
#define fail_unless2(c,m,f,...) if (! (c)) { printf("* failed: %s at %s:%d " f "\n", (m), __FILE__, __LINE__, __VA_ARGS__); rc = 0; }
#define ok(c,m) if ((c)) { printf("ok %d - %s\n", _testnum++, m); } else { printf("failed %d - %s\n", _testnum++, m); }
