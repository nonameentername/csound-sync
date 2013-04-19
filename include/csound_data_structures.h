/*
 csound_data_structures.h:
 
 Copyright (C) 2013 Steven Yi
 
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
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 02111-1307 USA
 */

#include "csoundCore.h"

typedef struct _cs_str_set {
    
} CS_STR_SET;

CS_STR_SET* cs_create_str_set(CSOUND* csound);
int cs_str_set_contains(CS_STR_SET* set, char* value);
void cs_str_set_add(CSOUND* csound, CS_STR_SET* set, char* value);
void cs_str_set_remove(CSOUND* csound, CS_STR_SET* set, char* value);
void cs_str_set_delete(CSOUND* csound, CS_STR_SET* set);