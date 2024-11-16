/*
  csInternal.h: csound internal data structures

  Copyright (C) 1991-2024 

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

#ifndef _CS_INTERNAL_H_
#define _CS_INTERNAL_H_
#ifdef __cplusplus
extern "C" {
#endif
/* The definitions and declarations in this section 
   are not available externally to plugins.
*/
#ifdef __BUILDING_LIBCSOUND
    typedef struct CSFILE_ {
    struct CSFILE_  *nxt;
    struct CSFILE_  *prv;
    int32_t type;
    int32_t  fd;
    FILE *f;
    SNDFILE *sf;
    void *cb;
    int32_t async_flag;
    int32_t items;
    int32_t pos;
    MYFLT *buf;
    int32_t bufsize;
    char fullName[1];
  } CSFILE;

  typedef struct CORFIL {
    char    *body;
    uint32_t     len;
    uint32_t     p;
  } CORFIL;

#define MARGS   (3)
#define MAX_INCLUDE_DEPTH 100
  
  typedef struct MACRO {          /* To store active macros */
    char          *name;        /* Use is by name */
    int32_t           acnt;         /* Count of arguments */
    char          *body;        /* The text of the macro */
    struct MACRO  *next;        /* Chain of active macros */
    int32_t           margs;        /* amount of space for args */
    char          *arg[MARGS];  /* With these arguments */
  } MACRO;

  typedef struct MACRON {
    int32_t             n;
    uint32_t    line;
    struct MACRO    *s;
    char            *path;
    int32_t             included;
  } MACRON;

  typedef struct in_stack_s {     /* Stack of active inputs */
    int16       is_marked_repeat;     /* 1 if this input created by 'n' stmnt */
    int16       args;                 /* Argument count for macro */
    MACRO       *mac;
    int32_t         line;
    int32       oposit;
  } IN_STACK;

  typedef struct marked_sections {
    char        *name;
    int32       posit;
    int32_t         line;
  } MARKED_SECTIONS;

  typedef struct namelst {
    char           *name;
    struct namelst *next;
  } NAMELST;

  typedef struct NAME__ {
    char          *namep;
    struct NAME__  *nxt;
    int32_t           type, count;
  } NAME;

  
  typedef union {
    uint32 dwData;
    unsigned char bData[4];
  } MIDIMESSAGE;

  /* MIDI globals */

  typedef struct midiglobals {
    MEVENT  *Midevtblk;
    int32_t     sexp;
    int32_t     MIDIoutDONE;
    int32_t     MIDIINbufIndex;
    MIDIMESSAGE MIDIINbuffer2[MIDIINBUFMAX];
    int32_t     (*MidiInOpenCallback)(CSOUND *, void **, const char *);
    int32_t     (*MidiReadCallback)(CSOUND *, void *, unsigned char *, int32_t);
    int32_t     (*MidiInCloseCallback)(CSOUND *, void *);
    int32_t     (*MidiOutOpenCallback)(CSOUND *, void **, const char *);
    int32_t     (*MidiWriteCallback)(CSOUND *, void *, const unsigned char *, int32_t);
    int32_t     (*MidiOutCloseCallback)(CSOUND *, void *);
    const char *(*MidiErrorStringCallback)(int32_t);
    void    *midiInUserData;
    void    *midiOutUserData;
    void    *midiFileData;
    void    *midiOutFileData;
    int32_t     rawControllerMode;
    char    muteTrackList[256];
    unsigned char mbuf[MBUFSIZ];
    unsigned char *bufp, *endatp;
    int16   datreq, datcnt;
  } MGLOBAL;

  typedef struct osc_mess {
    char *address;
    char *type;
    char *data;
    int32_t size;
    int32_t flag;
    struct osc_mess *nxt;
  } OSC_MESS;

  typedef struct eventnode {
    struct eventnode  *nxt;
    uint32     start_kcnt;
    EVTBLK            evt;
  } EVTNODE;

  typedef struct {
    OPDS    h;
    MYFLT   *ktempo, *istartempo;
    MYFLT   prvtempo;
  } TEMPO;

  typedef struct names {
    char    *mac;
    struct names *next;
  } NAMES;

  typedef struct threadInfo {
    struct threadInfo *next;
    void * threadId;
  } THREADINFO;

#include "sort.h"
#include "text.h"
#include "prototyp.h"
#include "cwindow.h"
#include "envvar.h"
#include "remote.h"

  /**
   * This struct will hold the current engine state after compilation
   */
  typedef struct engine_state {
    CS_VAR_POOL    *varPool;  /* global variable pool */
    CS_HASH_TABLE  *constantsPool;
    CS_HASH_TABLE  *stringPool;
    int32_t            maxopcno;
    INSTRTXT      **instrtxtp; /* instrument list      */
    INSTRTXT      instxtanchor;
    CS_HASH_TABLE *instrumentNames; /* instrument names */
    int32_t           maxinsno;
  } ENGINE_STATE;


  /* Holds UDO information, when an instrument is
     defined as a UDO
  */
  typedef struct opcodinfo {
    int32    instno;
    char    *name, *intypes, *outtypes;
    int16   inchns, outchns;
    bool newStyle;
    bool passByRef;
    CS_VAR_POOL* out_arg_pool;
    CS_VAR_POOL* in_arg_pool;
    struct instr *ip;
    struct opcodinfo *prv;
  } OPCODINFO;

  /**
   * Nen FFT interface
   */
  typedef struct _FFT_SETUP{
    int32_t N, M;
    void  *setup;
    MYFLT *buffer;
    int32_t    lib;
    int32_t    d;
    int32_t  p2;
  } CSOUND_FFT_SETUP;


  /**
   * plugin module info
   */
  typedef struct {
    char module[12];
    char type[12];
  } MODULE_INFO;

#define MAX_ALLOC_QUEUE 1024
  typedef struct _alloc_data_ {
    int32_t type;
    int32_t insno;
    EVTBLK blk;
    MCHNBLK *chn;
    MEVENT mep;
    INSDS *ip;
    OPDS *ids;
  } ALLOC_DATA;

#define MAX_MESSAGE_STR 1024
  typedef struct _message_queue_t_ {
    int32_t attr;
    char str[MAX_MESSAGE_STR];
  } message_string_queue_t;

  

#endif  /* __BUILDING_LIBCSOUND */
  
#ifdef __cplusplus
}
#endif /*  __cplusplus */

#endif // _CS_INTERNAL 
