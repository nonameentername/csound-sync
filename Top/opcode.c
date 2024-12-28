/*
  opcode.c:

  Copyright (C) 1997 John ffitch
  (C) 2005 Istvan Varga

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

/* OPCODE.C */
/* Print opcodes in system */

/* John ffitch -- 26 Jan 97 */
/*  4 april 02 -- ma++ */
/*  restructure to retrieve externally  */
/* And suppressing deprecated Oct 2015 -- JPff */
#include "csoundCore.h"
#include "csound_standard_types.h"
#include "opcode.h"
#include <ctype.h>
#include "interlocks.h"

static int32_t opcode_cmp_func(const void *a, const void *b) {
  opcodeListEntry *ep1 = (opcodeListEntry *)a;
  opcodeListEntry *ep2 = (opcodeListEntry *)b;
  int32_t retval;

  if ((retval = strcmp(ep1->opname, ep2->opname)) != 0)
    return retval;
  if ((retval = strcmp(ep1->outypes, ep2->outypes)) != 0)
    return retval;
  if ((retval = strcmp(ep1->intypes, ep2->intypes)) != 0)
    return retval;
  if (ep1 < ep2)
    return -1;
  if (ep1 > ep2)
    return 1;

  return 0;
}

/**
 * Gets an alphabetically sorted list of all opcodes.
 * Should be called after externals are loaded by csoundCompile().
 * Returns the number of opcodes, or a negative error code on failure.
 * Make sure to call csoundDisposeOpcodeList() when done with the list.
 */

PUBLIC int32_t csoundNewOpcodeList(CSOUND *csound, opcodeListEntry **lstp) {
  void *lst = NULL;
  OENTRY *ep;
  char *s;
  size_t nBytes = (size_t)0;
  int32_t i, cnt = 0;
  CONS_CELL *head, *items, *temp;

  (*lstp) = NULL;
  if (UNLIKELY(csound->opcodes == NULL))
    return -1;

  head = items = cs_hash_table_values(csound, csound->opcodes);

  /* count the number of opcodes, and bytes to allocate */
  while (items != NULL) {
    temp = items->value;
    while (temp != NULL) {
      ep = temp->value;
      if (ep->opname != NULL && ep->opname[0] != '\0' &&
          isalpha(ep->opname[0]) && ep->outypes != NULL &&
          ep->intypes != NULL) {
        cnt++;
#ifdef JPFF
        if (strchr(ep->intypes, 'x'))
          printf("%s, type %d %s -> %s\n", ep->opname,
                 (ep->init && ep->perf ? 3
                  : ep->init           ? 1
                                       : 2) ep->intypes,
                 ep->outypes);
#endif
        nBytes += sizeof(opcodeListEntry);
        for (i = 0; ep->opname[i] != '\0' && ep->opname[i] != '.'; i++)
          ;
        nBytes += (size_t)i;
        nBytes += strlen(ep->outypes);
        nBytes += strlen(ep->intypes);
        nBytes += 3; /* for null characters */
      }
      temp = temp->next;
    }
    items = items->next;
  }
  nBytes += sizeof(opcodeListEntry);
  /* allocate memory for opcode list */
  lst = csound->Malloc(csound, nBytes);
  if (UNLIKELY(lst == NULL))
    return CSOUND_MEMORY;
  (*lstp) = (opcodeListEntry *)lst;
  /* store opcodes in list */
  items = head;
  s = (char *)lst + ((int32_t)sizeof(opcodeListEntry) * (cnt + 1));
  cnt = 0;
  while (items != NULL) {
    temp = items->value;
    while (temp != NULL) {
      ep = temp->value;

      if (ep->opname != NULL && ep->opname[0] != '\0' &&
          isalpha(ep->opname[0]) && ep->outypes != NULL &&
          ep->intypes != NULL) {
        for (i = 0; ep->opname[i] != '\0' && ep->opname[i] != '.'; i++)
          s[i] = ep->opname[i];
        s[i++] = '\0';
        ((opcodeListEntry *)lst)[cnt].opname = s;
        s += i;
        strcpy(s, ep->outypes);
        ((opcodeListEntry *)lst)[cnt].outypes = s;
        s += ((int32_t)strlen(ep->outypes) + 1);
#ifdef JPFF
        if (strlen(ep->outypes) == 0)
          printf("***potential WI opcode %s\n", ep->opname);
#endif
        strcpy(s, ep->intypes);
        ((opcodeListEntry *)lst)[cnt].intypes = s;
        s += ((int32_t)strlen(ep->intypes) + 1);
        ((opcodeListEntry *)lst)[cnt].flags = ep->flags;
        // if (ep->flags&_QQ) printf("DEPRICATED: %s\n", ep->opname);
        // if (ep->flags&_QQ) *deprec++;
        cnt++;
      }
      temp = temp->next;
    }
    items = items->next;
  }
  ((opcodeListEntry *)lst)[cnt].opname = NULL;
  ((opcodeListEntry *)lst)[cnt].outypes = NULL;
  ((opcodeListEntry *)lst)[cnt].intypes = NULL;
  ((opcodeListEntry *)lst)[cnt].flags = 0;

  cs_cons_free(csound, head);

  /* sort list */
  qsort(lst, (size_t)cnt, sizeof(opcodeListEntry), opcode_cmp_func);

  /* return the number of opcodes */
  return cnt;
}

PUBLIC void csoundDisposeOpcodeList(CSOUND *csound, opcodeListEntry *lst) {
  csound->Free(csound, lst);
}

void list_opcodes(CSOUND *csound, int32_t level) {
  opcodeListEntry *lst;
  const char *sp = "                    "; /* length should be 20 */
  int32_t j, k;
  int32_t cnt, len = 0, xlen = 0;
  int32_t count = 0;

  cnt = csoundNewOpcodeList(csound, &lst);
  if (UNLIKELY(cnt <= 0)) {
    csound->ErrorMsg(csound, Str("Error creating opcode list"));
    csoundDisposeOpcodeList(csound, lst);
    return;
  }

  for (j = 0, k = -1; j < cnt; j++) {
    if ((level & 1) == 0) { /* Print in 4 columns */
      if (j > 0 && strcmp(lst[j - 1].opname, lst[j].opname) == 0)
        continue;
      if ((level & 2) == 0 && ((lst[j].flags & _QQ) != 0)) {
        // printf("dropping %s\n", lst[j].opname);
        continue;
      }
      k++;
      xlen = 0;
      if (!(k & 3))
        csound->Message(csound, "\n");
      else {
        if (len > 19) {
          xlen = len - 19;
          len = 19;
        }
        csound->Message(csound, "%s", sp + len);
      }
      csound->Message(csound, "%s", lst[j].opname);
      len = (int32_t)strlen(lst[j].opname) + xlen;
    } else {
      char *ans = lst[j].outypes, *arg = lst[j].intypes;
      if ((level & 2) == 0 && ((lst[j].flags & _QQ) != 0)) {
        // printf("dropping %s\n", lst[j].opname);
        continue;
      }
      csound->Message(csound, "%s", lst[j].opname);
      len = (int32_t)strlen(lst[j].opname);
      if (len > 11) {
        xlen = len - 11;
        len = 11;
      }
      csound->Message(csound, "%s", sp + (len + 8));
      if (ans == NULL || *ans == '\0')
        ans = "(null)";
      if (arg == NULL || *arg == '\0')
        arg = "(null)";
      csound->Message(csound, "%s", ans);
      len = (int32_t)strlen(ans) + xlen;
      len = (len < 11 ? len : 11);
      xlen = 0;
      csound->Message(csound, "%s", sp + (len + 8));
      csound->Message(csound, "%s\n", arg);
    }
    count++;
  }
  csound->Message(csound, "\n");
  csound->Message(csound, Str("%d opcodes\n\n"), count);
  csoundDisposeOpcodeList(csound, lst);
}

struct oentries *find_opcode2(CSOUND *, char *);
// opcodes for OpcodeRef and Opcode types

int32_t opcode_ref(CSOUND *csound, ASSIGN *p) {
   OPCODEREF *pp = (OPCODEREF *) p->r;
   STRINGDAT *str = (STRINGDAT *) p->a;
   if(find_opcode(csound, str->data))
   pp->entries = find_opcode2(csound, str->data);
   else return csound->InitError(csound,
                                 "could not find opcode %s",
                                 str->data);
   return OK;
}

/**
 * print info on opcode ref (overloads, types)
 *
 * opcodeinfo opc:OpcodeRef
 */
int32_t opcode_info(CSOUND *csound, OPINFO *p) {
  OENTRY *ep = p->ref->entries->entries[0];
  int n, nep =  p->ref->entries->count;
  csound->Message(csound, "%s: %d overloads\n",
       get_opcode_short_name(csound, ep->opname),
                nep);
  for(n = 0; n < nep; n++) {
    ep = p->ref->entries->entries[n];
    csound->Message(csound, "(%d)\t%s\tout-types: %s\tin-types: %s\n",
                    n, ep->opname, ep->outypes, ep->intypes);
  }
  return OK;
}

/** Create opcode from an OpcodeRef
    opc:Opcode create ref:OpcodeRef[,overload:i]
 */
int32_t create_opcode_simple(CSOUND *csound, AOP *p) {
    OPCODEREF *ref = (OPCODEREF *) p->a;
    if(ref->entries != NULL) {
    OPCODEOBJ *obj = (OPCODEOBJ *) p->r;
    int32_t n = (int32_t) (*p->b >= 0 ? *p->b : 0);
    OENTRY *entry =
      ref->entries->entries[n < ref->entries->count ? n : ref->entries->count-1];
    obj->init_flag = 0;
    // set opcode object
    obj->size = entry->dsblksiz;
    obj->dataspace = (OPDS *) csound->Calloc(csound, obj->size);
    if(obj->dataspace != NULL) {
    // fill OPDS with as much information as we have now, rest on init
      obj->dataspace->insdshead = p->h.insdshead;
    obj->dataspace->optext = csound->Calloc(csound, sizeof(OPTXT));
    if(obj->dataspace->optext != NULL) {
      obj->dataspace->optext->t.oentry = entry;
      obj->dataspace->optext->t.opcod = entry->opname;
    }
    obj->dataspace->init = entry->init;
    obj->dataspace->perf = entry->perf;
    obj->dataspace->deinit = entry->deinit;
    return OK;
    } return csound->InitError(csound, "could not allocate opcode object");
   }
   return csound->InitError(csound, "invalid opcode reference");
}

int32_t opcode_dealloc(CSOUND *csound, AOP *p) {
   OPCODEOBJ *obj = (OPCODEOBJ *) p->r;
   if(obj->dataspace) {
    if(obj->init_flag && obj->dataspace->deinit != NULL)
      obj->dataspace->deinit(csound, obj->dataspace);
    csound->Free(csound, obj->dataspace->optext);
    if(!obj->udo_flag) 
      csound->Free(csound, obj->dataspace);
    obj->dataspace = NULL;
   }
   return OK;
}

/**
 * print info on opcode (name, types)
 *
 * opcodeinfo opc:Opcode
 */
int32_t opcode_object_info(CSOUND *csound, OPINFO *p) {
  OPCODEOBJ *obj = (OPCODEOBJ *) p->ref;
  if(obj->dataspace != NULL) {
  OENTRY *ep = obj->dataspace->optext->t.oentry;
    csound->Message(csound, "%s\tout-types: %s\tin-types: %s\n",
                   ep->opname, ep->outypes, ep->intypes);
  }
  return OK;
}

void *find_or_add_constant(CSOUND *csound, CS_HASH_TABLE *constantsPool,
                           const char *name, MYFLT value);

MYFLT *set_constant(CSOUND *csound, const char *name, MYFLT value) {
  return (MYFLT *) (find_or_add_constant(csound, csound->engineState.constantsPool,
                               name, value) + CS_VAR_TYPE_OFFSET);
}

#include "udo.h"
/** 
 * Set up arguments for opcode using OENTRY type lists
 * check every out and in arg and connect it
 * returns an error if args do not match
 */
int32_t setup_args(CSOUND *csound, OPCODEOBJ *obj, OPDS *h,
                   MYFLT *args[], int32_t no, int32_t ni){
  TEXT *t = &(obj->dataspace->optext->t);
  OENTRY *ep = t->oentry;
  char *types;
  CS_TYPE *argtype;
  int32_t n = 0, i = 0, opt = 0;
  size_t len;
  MYFLT **outargs;
  MYFLT **inargs;
  if(obj->udo_flag) {
    // udo args located at the end of udo struct
   UOPCODE *udo = (UOPCODE *) obj->dataspace;
   outargs = udo->ar;
  } else {
   // opcode args located after OPDS struct
    outargs = (MYFLT **) (obj->dataspace + 1);
  }
  
  // out args first
  types = ep->outypes;
  // opcode input args located after after outargs
  inargs = outargs + strlen(types);
  while(*types != '\0') {
    // deal with multipe out args first
    if(*types == '*') {
      // expecting only a single char here
      // connect all args
      for(;n < no; n++) outargs[n] = args[n];
      break;
    }
    // other multi output args have one letter
    // per optional output, indicating size
    // of output pointer array
    else if(*types == 'm') {
      // get the size of output array
      len = strlen(types);
      for(; i < len; i++) {
        if(n < no) {
          argtype = csoundGetTypeForArg(args[n]);
          // if output arg exists, try to connect it
          if(argtype != &CS_VAR_TYPE_A) {
            csound->Message(csound, "Output arg %d, expected type: %s, got: %s\n",
                            i+1,CS_VAR_TYPE_A.varTypeName, argtype->varTypeName);
            return NOTOK;
          }
          outargs[i] = args[n++];
        } else // otherwise set it to NULL
          outargs[i] = NULL;
      }
      break; // no further output expected
    }
    // same for all other multi output types
    else if(*types == 'z') {
      len = strlen(types);
      for(; i < len; i++) {
        if(n < no) {
          argtype = csoundGetTypeForArg(args[n]);
          if(argtype != &CS_VAR_TYPE_K){
            csound->Message(csound, "Output arg %d, expected type: %s, got: %s\n",
                            i+1, CS_VAR_TYPE_K.varTypeName, argtype->varTypeName);
            return NOTOK;
          }
          outargs[i] = args[n++];
        } else outargs[i] = NULL;
      }
      break;
    }
    else if(*types == 'I') {
      len = strlen(types);
      for(; i < len; i++) {
        if(n < no) {
          argtype = csoundGetTypeForArg(args[n]);
          if(argtype != &CS_VAR_TYPE_I){
            csound->Message(csound, "Output arg %d, expected type: %s, got: %s\n",
                            i+1, CS_VAR_TYPE_I.varTypeName, argtype->varTypeName);
            return NOTOK;
          }
          outargs[i] = args[n++];
        } else outargs[i] = NULL;        
      }
      break;
    }
    else if(*types == 'X') {
      len = strlen(types);
      for(; i < len; i++) {
        if(n < no) {  
          argtype = csoundGetTypeForArg(args[n]);
          if(argtype != &CS_VAR_TYPE_A && argtype != &CS_VAR_TYPE_K &&
             argtype != &CS_VAR_TYPE_I ){
            csound->Message(csound,
                            "Output arg %d, expected types: %s, %s, or %s, got: %s\n",
                            i+1, CS_VAR_TYPE_A.varTypeName, CS_VAR_TYPE_K.varTypeName,
                            CS_VAR_TYPE_I.varTypeName, argtype->varTypeName);
            return NOTOK;
          }
          outargs[i] = args[n++];
        } else outargs[i] = NULL;         
      }
      break;
    }
    else if(*types == 'N') {
      len = strlen(types);
      for(; i < len; i++) {
        if(n < no) { 
          argtype = csoundGetTypeForArg(args[n]);
          if(argtype != &CS_VAR_TYPE_A && argtype != &CS_VAR_TYPE_K &&
             argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_S){
            csound->Message(csound,
                            "Output arg %d, expected types: "
                            "%s, %s, %s, or %s, got: %s\n",
                            i+1, CS_VAR_TYPE_A.varTypeName,
                            CS_VAR_TYPE_K.varTypeName,
                            CS_VAR_TYPE_I.varTypeName, CS_VAR_TYPE_S.varTypeName,
                            argtype->varTypeName);
            return NOTOK;
          }
          
          outargs[i] = args[n++];
        } else outargs[i] = NULL;        
      }
      break;
    }   
    else if(*types == 'F') {
      len = strlen(types);
      for(; i < len; i++) {
        if(n < no) {
          argtype = csoundGetTypeForArg(args[n]);  
          if(argtype != &CS_VAR_TYPE_F) {
            csound->Message(csound, "Output arg %d, expected type: %s, got: %s\n",
                            i+1, CS_VAR_TYPE_F.varTypeName, argtype->varTypeName);
            return NOTOK;
          }
          outargs[i] = args[n++];
        } else outargs[i] = NULL;          
      }
      break;
    }
    // now individual arg types
    // skip type delimiter
    if(*types == ':') types++;
    argtype = csoundGetTypeForArg(args[n]);
    len = strlen(argtype->varTypeName);
    if(strncmp(argtype->varTypeName, types, len) != 0) {
      char typ[64];
      memcpy(typ, types, len);
      typ[len] = '\0';
      csound->Message(csound, "Output arg %d, expect type: %s, got %s\n", i+1, typ,
                      argtype->varTypeName);
      return NOTOK;
    }
    outargs[i++] = args[n++];
    // next type
    types += len;
    // skip type delimiter
    if(*types == ';') types++;
  }
  // n is the outarg count
  if(n != no) {
    // arg number mismatch?
    csound->Message(csound, "Output arg number mismatch, expected %d, got %d\n",
                    no, n);
    return NOTOK;
  }
  // set argcount for opcode
  t->outArgCount = no;
  // connect TEXT args
  t->outArgs = h->optext->t.outArgs;
  t->outlist = h->optext->t.outlist;
  // now check inargs
  n++;    // skip opc obj arg
  i = 0;  // set inarg count to 0
  types = ep->intypes;
  while(*types != '\0') {
    // now deal with multiple inargs
    // unlike input, these are single letter
    if(*types == '*') {
      // connect all remaining args
      for(; i < ni; n++, i++) {
        argtype = csoundGetTypeForArg(args[n]);
        inargs[i] = args[n];
      }
      break; // no further inputs expected
    }
    // same for all other multi input types, but with type checks
    else if(*types == 'M') {
      for(; i < ni; n++, i++) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_A && argtype != &CS_VAR_TYPE_K &&
           argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_C &&
           argtype != &CS_VAR_TYPE_P){
          csound->Message(csound, "Input arg %d, expected types: "
                          "%s, %s, or %s, got: %s\n",
                          i+1,CS_VAR_TYPE_A.varTypeName,
                          CS_VAR_TYPE_K.varTypeName,
                          CS_VAR_TYPE_I.varTypeName, argtype->varTypeName);
          return NOTOK;
        }
        inargs[i] = args[n];
      }
      break; 
    }
    else if(*types == 'N') {
      for(; i < ni; n++, i++) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_A && argtype != &CS_VAR_TYPE_K &&
           argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_S &&
           argtype != &CS_VAR_TYPE_C && argtype != &CS_VAR_TYPE_P){
          csound->Message(csound,
                          "Input arg %d, expected types: "
                          "%s, %s, %s, or %s, got: %s\n",
                          i+1,CS_VAR_TYPE_A.varTypeName,
                          CS_VAR_TYPE_K.varTypeName,
                          CS_VAR_TYPE_I.varTypeName, CS_VAR_TYPE_S.varTypeName,
                          argtype->varTypeName);
          return NOTOK;
        }
        inargs[i] = args[n];
      }
      break;
    }    
    else if(*types == 'm') {
      for(; i < ni; n++, i++) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_C &&
           argtype != &CS_VAR_TYPE_P){
          csound->Message(csound, "Input arg %d, expected type: %s, got: %s\n",
                          i+1,CS_VAR_TYPE_I.varTypeName, argtype->varTypeName);
          return NOTOK;
        }
        inargs[i] = args[n];
      }
      break;
    }
    else if(*types == 'y') {
      for(; i < ni; n++, i++) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_A){
          csound->Message(csound, "Input arg %d, expected type: %s, got: %s\n",
                          i+1,CS_VAR_TYPE_A.varTypeName, argtype->varTypeName);
          return NOTOK;
        }
        inargs[i] = args[n];
      }
      break;
    }
    else if(*types == 'z') { 
      for(; i < ni; n++, i++) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_K && argtype != &CS_VAR_TYPE_C &&
           argtype != &CS_VAR_TYPE_P && argtype != &CS_VAR_TYPE_I){
          csound->Message(csound, "Input arg %d, expected type: %s, got: %s\n",
                          i+1,CS_VAR_TYPE_K.varTypeName, argtype->varTypeName);
          return NOTOK;
        }
        inargs[i] = args[n];
      }
      break;
    }
    else if(*types == 'W') {
      for(; i < ni; n++, i++) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_S){
          csound->Message(csound, "Input arg %d, expected type: %s, got: %s\n",
                          i+1,CS_VAR_TYPE_S.varTypeName, argtype->varTypeName);
          return NOTOK;
        }
        inargs[i] = args[n];
      }
      break;
    }
    else if(*types == 'Z') {
      for(; i < ni; n++, i++) {
        argtype = csoundGetTypeForArg(args[n]);
        if(n%2 && argtype != &CS_VAR_TYPE_A) {
          csound->Message(csound, "Input arg %d, expected type: %s, got: %s\n",
                          i+1, CS_VAR_TYPE_A.varTypeName,
                          argtype->varTypeName);
          return NOTOK;
        }
        if(n%2 == 0 && argtype != &CS_VAR_TYPE_K && argtype != &CS_VAR_TYPE_I
                        && argtype != &CS_VAR_TYPE_C &&
                        argtype != &CS_VAR_TYPE_P) {
          csound->Message(csound, "Input arg %d, expected type: %s, got: %s\n",
                          i+1, CS_VAR_TYPE_K.varTypeName, argtype->varTypeName);

          return NOTOK;
        }
        inargs[i] = args[n];   
      }
      break;
    }
    else if(*types == 'x') {
      argtype = csoundGetTypeForArg(args[n]);
      if(argtype != &CS_VAR_TYPE_A && argtype != &CS_VAR_TYPE_K &&
         argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_C &&
         argtype != &CS_VAR_TYPE_P){
        csound->Message(csound, "Input arg %d, expected types: %s, %s, or %s, got: %s\n",
                        i+1, CS_VAR_TYPE_A.varTypeName, CS_VAR_TYPE_K.varTypeName,
                        CS_VAR_TYPE_I.varTypeName, argtype->varTypeName);
        return NOTOK;
      }
      inargs[i++] = args[n++]; 
      types++;
    }
    else if(*types == 'T') {
      argtype = csoundGetTypeForArg(args[n]);
      if(argtype != &CS_VAR_TYPE_S && argtype != &CS_VAR_TYPE_I
         && argtype != &CS_VAR_TYPE_C && argtype != &CS_VAR_TYPE_P){
        csound->Message(csound, "Input arg %d, expected types: %s or %s, got: %s\n",
                        i+1, CS_VAR_TYPE_I.varTypeName,
                        CS_VAR_TYPE_S.varTypeName,argtype->varTypeName);
        return NOTOK;
      }
      inargs[i++] = args[n++]; 
      types++;  
    }
    else if(*types == 'U') {
      argtype = csoundGetTypeForArg(args[n]);
      if(argtype != &CS_VAR_TYPE_S && argtype != &CS_VAR_TYPE_I &&
         argtype != &CS_VAR_TYPE_K && argtype != &CS_VAR_TYPE_C &&
         argtype != &CS_VAR_TYPE_P){
        csound->Message(csound, "Input arg %d, expected types: %s, %s, or %s, got: %s\n",
                        i+1, CS_VAR_TYPE_K.varTypeName, CS_VAR_TYPE_I.varTypeName,
                        CS_VAR_TYPE_S.varTypeName, argtype->varTypeName);
        return NOTOK;
      }
      inargs[i++] = args[n++]; 
      types++;
    }
    else if(*types == '.') {
      inargs[i++] = args[n++]; 
      types++;
    }
    // now deal with optional types
    // we have to connect to constants if
    // no arg is provided.
    else if(*types == '?') {
      // increment opt count if no args are passed
      opt += args[n] ? 0 : 1;
      // connect arg if passed, else lookup const
      inargs[i++] = args[n] ? args[n++] :
        set_constant(csound, "0", 0); 
      types++;
    }
    // same for other optional types
    else if(*types == 'o' || *types == 'O') {
      if(args[n] != NULL) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_K &&
           argtype != &CS_VAR_TYPE_C && argtype != &CS_VAR_TYPE_P) {
          csound->Message(csound, "Input arg %d, expected types: %s or %s, got: %s\n",
                          i+1,CS_VAR_TYPE_I.varTypeName, CS_VAR_TYPE_K.varTypeName,
                          argtype->varTypeName);
          return NOTOK;
        }
        inargs[i++] = args[n++];
          }
      else {
        inargs[i++] = set_constant(csound, "0", 0);       
          opt++;
      }
      types++;
    }
    else if(*types == 'p' || *types == 'P') {
      if(args[n] != NULL) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_K &&
           argtype != &CS_VAR_TYPE_C && argtype != &CS_VAR_TYPE_P) {
          csound->Message(csound, "Input arg %d, expected types: %s or %s, got: %s\n",
                          i+1,CS_VAR_TYPE_I.varTypeName, CS_VAR_TYPE_K.varTypeName,
                          argtype->varTypeName);
          return NOTOK;
        }
        inargs[i++] = args[n++];
          }
      else {
        inargs[i++] = set_constant(csound, "1", 1);       
          opt++;
      }
      types++;
    }
    else if(*types == 'q') {
      if(args[n] != NULL) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_C &&
           argtype != &CS_VAR_TYPE_P) {
          csound->Message(csound, "Input arg %d, expected types: %s got: %s\n",
                          i+1,CS_VAR_TYPE_I.varTypeName, argtype->varTypeName);
          return NOTOK;
        }
        inargs[i++] = args[n++];
          }
      else {
        inargs[i++] = set_constant(csound, "10", 10);       
          opt++;
      }
      types++;
    }
    else if(*types == 'v' || *types == 'V') {
      if(args[n] != NULL) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_K &&
           argtype != &CS_VAR_TYPE_C && argtype != &CS_VAR_TYPE_P) {
          csound->Message(csound, "Input arg %d, expected types: %s or %s, got: %s\n",
                          i+1,CS_VAR_TYPE_I.varTypeName, CS_VAR_TYPE_K.varTypeName,
                          argtype->varTypeName);
          return NOTOK;
        }
        inargs[i++] = args[n++];
          }
      else {
        inargs[i++] = set_constant(csound, ".5", 0.5);       
          opt++;
      }
      types++;
    }
    else if(*types == 'j' || *types == 'J') {
      if(args[n] != NULL) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_K &&
           argtype != &CS_VAR_TYPE_C && argtype != &CS_VAR_TYPE_P) {
          csound->Message(csound, "Input arg %d, expected types: %s or %s, got: %s\n",
                          i+1,CS_VAR_TYPE_I.varTypeName, CS_VAR_TYPE_K.varTypeName,
                          argtype->varTypeName);
          return NOTOK;
        }
        inargs[i++] = args[n++];
          }
      else {
        inargs[i++] = set_constant(csound, "-1", -1);      
        opt++;
      }
      types++;
    }
    else if(*types == 'h') {
      if(args[n] != NULL) {
        argtype = csoundGetTypeForArg(args[n]);
        if(argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_C &&
           argtype != &CS_VAR_TYPE_P) {
          csound->Message(csound, "Input arg %d, expected type: %s got: %s\n",
                          i+1,CS_VAR_TYPE_I.varTypeName, argtype->varTypeName);
          return NOTOK;
        }
        inargs[i++] = args[n++];
          }
      else {
        inargs[i++] = set_constant(csound, "127", 127);       
          opt++;
      }
      types++;
    }
    else if(*types == 'k') {
      argtype = csoundGetTypeForArg(args[n]);
      if(argtype != &CS_VAR_TYPE_P && argtype != &CS_VAR_TYPE_C
         && argtype != &CS_VAR_TYPE_I && argtype != &CS_VAR_TYPE_K) {
        csound->Message(csound, "Input arg %d, expected type: k got: %s\n",
                        i+1,argtype->varTypeName);
        return NOTOK;
      }
      inargs[i++] = args[n++];
      types++;
    }
    else if(*types == 'i') {
      argtype = csoundGetTypeForArg(args[n]);
      if(argtype != &CS_VAR_TYPE_P && argtype != &CS_VAR_TYPE_C
         && argtype != &CS_VAR_TYPE_I) {
        csound->Message(csound, "Input arg %d, expected type: i got: %s\n",
                        i+1,argtype->varTypeName);
        return NOTOK;
      }
      inargs[i++] = args[n++];
      types++;
    }
    else {             
      // now arg types with no special cases
      // skip type delimiter
      if(*types == ':') types++;
      argtype = csoundGetTypeForArg(args[n]);
      len = strlen(argtype->varTypeName);
      if(strncmp(argtype->varTypeName, types, len) != 0) {
        char typ[64];
        memcpy(typ, types, len);
        typ[len] = '\0';
        csound->Message(csound, "Input arg %d, expected type: %s got: %s\n",
                        i+1, typ, argtype->varTypeName);
        return NOTOK;
      }
      inargs[i++] = args[n++];
      // next type
      types += len;
      // skip type delimiter if any
      if(*types == ';') types++;
    }
  }
  // check that input args match. 
  if(ni != i - opt) {
    csound->Message(csound, "Input arg number mismatch, expected %d, got %d\n",
                    ni, i - opt);    
    return NOTOK;
  }
  t->inArgCount = ni;
  // connect TEXT args, skipping opcode obj
  t->outArgs = h->optext->t.outArgs + 1;
  t->outlist = h->optext->t.outlist + 1;
  return OK;
}

/**
 * connect caller locn to opcode TXT
 */
void set_line_num_and_loc(OPRUN *p) {
   OPCODEOBJ *obj = (OPCODEOBJ *) p->args[p->OUTCOUNT];
   obj->dataspace->optext->t.linenum = p->h.optext->t.linenum;
   obj->dataspace->optext->t.linenum = p->h.optext->t.locn;
}

int32_t context_check(CSOUND *csound, OPCODEOBJ *obj, OPDS *h) {
  if(obj->dataspace->insdshead == h->insdshead) return OK;
  INSDS *ip = obj->dataspace->insdshead;
  INSDS *ctx = h->insdshead;
  // different SR always fails check
  if(ip->esr != ctx->esr) return NOTOK;
  // ksmps less than ctx fails check
  if(ip->ksmps < ctx->ksmps) return NOTOK;
  // otherwise there is no context incompatibility
  return OK;
}

/* this opcode connects all args to opcode obj and
   optionally runs init function 
*/
int32_t opcode_object_init(CSOUND *csound, OPRUN *p) {
  OPCODEOBJ *obj = (OPCODEOBJ *) p->args[p->OUTCOUNT];
  if(context_check(csound, obj, &(p->h)) != OK)
    return csound->InitError(csound, "incompatible context, "
                             "cannot initialise opcode obj for %s\n",
                             obj->dataspace->optext->t.oentry->opname);
  set_line_num_and_loc(p);
  obj->udo_flag = obj->dataspace->optext->t.oentry->useropinfo ? 1 : 0;
  if(setup_args(csound, obj, &(p->h), p->args, p->OUTCOUNT,
                p->INCOUNT - 1) == OK) {
    obj->init_flag = 1;
    if(obj->dataspace->init != NULL)
      return obj->dataspace->init(csound, obj->dataspace);
    else return OK;
  }
  return csound->InitError(csound, "mismatching arguments\n"
                           "for opcode obj %s\t"
                            "outypes: %s\tintypes: %s",
                           obj->dataspace->optext->t.oentry->opname,
                           obj->dataspace->optext->t.oentry->outypes,
                           obj->dataspace->optext->t.oentry->intypes); 
}

/* this opcode runs a perf pass on an initialised object
*/
int32_t opcode_object_perf(CSOUND *csound, OPRUN *p) {
  OPCODEOBJ *obj = (OPCODEOBJ *) p->args[p->OUTCOUNT];
  if(context_check(csound, obj, &(p->h)) != OK)
    return csound->InitError(csound, "incompatible context, "
                             "cannot perform opcode obj for %s\n",
                             obj->dataspace->optext->t.oentry->opname);
  set_line_num_and_loc(p);
  if(obj->dataspace->perf != NULL && obj->init_flag)
    return obj->dataspace->perf(csound, obj->dataspace);
  else return OK;
}



