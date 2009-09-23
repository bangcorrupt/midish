/*
 * Copyright (c) 2003-2007 Alexandre Ratchov <alex@caoua.org>
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

#include <stdio.h>
#include <unistd.h>

#include "dbg.h"
#include "str.h"
#include "cons.h"
#include "ev.h"
#include "mux.h"
#include "track.h"
#include "frame.h"
#include "song.h"
#include "user.h"
#include "filt.h"
#include "mididev.h"
#include "defs.h"
#include "sysex.h"
#include "textio.h"

int
main(int argc, char **argv)
{
	int ch;
	unsigned exitcode;

	while ((ch = getopt(argc, argv, "bv")) != -1) {
		switch (ch) {
		case 'b':
			user_flag_batch = 1;
			break;
		case 'v':
			user_flag_verb = 1;
			break;
		default:
			goto err;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc >= 1) {
	err:
		fputs("usage: midish [-bv]\n", stderr);
		return 0;
	}

	cons_init();
	textio_init();
	evctl_init();
	seqev_pool_init(DEFAULT_MAXNSEQEVS);
	state_pool_init(DEFAULT_MAXNSTATES);
	chunk_pool_init(DEFAULT_MAXNCHUNKS);
	sysex_pool_init(DEFAULT_MAXNSYSEXS);
	seqptr_pool_init(DEFAULT_MAXNSEQPTRS);
	mididev_listinit();

	exitcode = user_mainloop();

	mididev_listdone();
	seqptr_pool_done();
	sysex_pool_done();
	chunk_pool_done();
	state_pool_done();
	seqev_pool_done();
	evctl_done();
	textio_done();
	cons_done();

	mem_stats();
	return exitcode ? 0 : 1;
}
