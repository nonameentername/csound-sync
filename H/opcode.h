/*
    opcode.h:

    Copyright (C) 2024 by Victor Lazzarini

    This file is part of Csound.

    The Csound Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
    02110-1301 USA
*/

#pragma once

#include "csoundCore.h"
#include "aops.h"

typedef struct _opinfo {
  OPDS h;
  OPCODEREF *ref;
} OPINFO;

int32_t opcode_info(CSOUND *csound, OPINFO *p);
int32_t opcode_ref(CSOUND *csound, ASSIGN *p);
int32_t opcode_object_info(CSOUND *csound, OPINFO *p);
int32_t opcode_dealloc(CSOUND *csound, AOP *p);
int32_t create_opcode_simple(CSOUND *csound, AOP *p);
