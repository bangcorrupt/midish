/*
 * Copyright (c) 2003-2010 Alexandre Ratchov <alex@caoua.org>
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
 * 	  materials provided with the distribution.
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

#ifndef MIDISH_DBG_H
#define MIDISH_DBG_H

#ifndef NULL
#define NULL (0)
#endif

void	 dbg_puts(char *);
void	 dbg_putx(unsigned long);
void	 dbg_putu(unsigned long);
void	 dbg_puti(long);
void	 dbg_panic(void);
void	 dbg_flush(void);
void	*mem_alloc(unsigned, char *);
void	 mem_free(void *);
void	 mem_stats(void);
unsigned mem_rnd(void);

struct prof {
	char *name;
	unsigned n;
	unsigned min;
	unsigned max;
	unsigned sum;
	unsigned sumsqr;
	unsigned err;
};

void prof_reset(struct prof *, char *);
void prof_val(struct prof *, unsigned);
void prof_dbg(struct prof *);

extern unsigned mem_debug;
extern unsigned dbg_sync;

#endif /* MIDISH_DBG_H */
