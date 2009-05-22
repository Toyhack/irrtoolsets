/* 
Copyright (C) 1990 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifdef __GNUG__
#pragma implementation
#endif
#include <builtin.h>
#include <stdlib.h>
#include <stdio.h> 

void default_two_arg_error_handler(const char* kind, const char* msg)
{
  fputs(kind, stderr);
  fputs(" Error: ", stderr);
  fputs(msg, stderr);
  fputs("\n", stderr);
  abort();
}

two_arg_error_handler_t lib_error_handler = default_two_arg_error_handler;
