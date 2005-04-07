/*
 * Copyright (c) 2003-2005 Alexandre Ratchov
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 * 	- Redistributions of source code must retain the above
 * 	  copyright notice, this list of conditions and the 
 * 	  following disclaimer.
 * 
 * 	- Redistributions in binary form must reproduce the above
 * 	  copyright notice, this list of conditions and the 
 * 	  following disclaimer in the documentation and/or other 
 * 	  materials  provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef SEQ_MUX_H
#define SEQ_MUX_H

#define MUX_STARTWAIT		0	/* waiting for a start event */
#define MUX_START 		1	/* just got a start */
#define MUX_FIRST		2	/* just got the first tic */
#define MUX_NEXT		3	/* just got the next tic */
#define MUX_STOPWAIT		4	/* waiting for the stop event */
#define MUX_STOP		5	/* nothing to do */

#define MUX_LINESIZE		1024

struct ev_s;

void mux_init(void (*cb)(void *, struct ev_s *), void *addr);
void mux_done(void);
void mux_run(void);
void mux_flush(void);
void mux_putev(struct ev_s *);
void mux_sendraw(unsigned unit, unsigned char *buf, unsigned len);
unsigned mux_getphase(void);
void mux_chgtempo(unsigned long tmr_ticlength);
void mux_chgticrate(unsigned tpu);
void mux_startwait(void);
void mux_stopwait(void);

void mux_mdep_init(void);
void mux_mdep_done(void);

void mux_timercb(unsigned long delta);
void mux_evcb(unsigned, struct ev_s *ev);

extern unsigned mux_ticsperunit;

#endif /* SEQ_MUX_H */
