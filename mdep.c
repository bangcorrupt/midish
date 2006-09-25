/*
 * Copyright (c) 2003-2006 Alexandre Ratchov <alex@caoua.org>
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

/*
 * machine and OS dependent code
 */
 
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "default.h"
#include "mux.h"
#include "rmidi.h"
#include "cons.h"
#include "mdep.h"
#include "user.h"
#include "exec.h"
#include "dbg.h"

#ifndef RC_NAME
#define RC_NAME		"midishrc"
#endif

#ifndef RC_DIR
#define RC_DIR		"/etc"
#endif

#define MIDI_BUFSIZE	1024

void
cons_mdep_sighandler(int s) {
	if (s == SIGINT) {
		cons_breakcnt++;
	}
}

void
cons_mdep_init(void) {
	struct sigaction sa;

	sa.sa_handler = cons_mdep_sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sa, NULL) < 0) {
		perror("cons_mdep_init: sigaction");
		exit(1);
	}
}

void
cons_mdep_done(void) {
	struct sigaction sa;

	sa.sa_handler = SIG_DFL;
	if (sigaction(SIGINT, &sa, NULL) < 0) {
		perror("cons_mdep_done: sigaction");
		exit(1);
	}
}

void
mux_mdep_init(void) {
	struct sigaction sa;
	struct mididev *i;
	int mode;

	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &sa, NULL) < 0) {
		perror("mux_mdep_init: sigaction");
		exit(1);
	}
	
	for (i = mididev_list; i != NULL; i = i->next) {
		if (i->mode == MIDIDEV_MODE_IN) {
			mode = O_RDONLY;
		} else if (i->mode == MIDIDEV_MODE_OUT) {
			mode = O_WRONLY;
		} else if (i->mode == (MIDIDEV_MODE_IN | MIDIDEV_MODE_OUT)) {
			mode = O_RDWR;
		} else {
			dbg_puts("mux_mdep_init: not allowed mode\n");
			dbg_panic();
		}
		RMIDI(i)->mdep.fd = open(RMIDI(i)->mdep.path, mode, 0666);
		if (RMIDI(i)->mdep.fd < 0) {
			perror(RMIDI(i)->mdep.path);
			RMIDI(i)->mdep.idying = 1;
			RMIDI(i)->mdep.odying = 1;
		} else {
			RMIDI(i)->mdep.idying = 0;
			RMIDI(i)->mdep.odying = 0;
		}		
	}
}


void
mux_mdep_done(void) {
	struct sigaction sa;
	struct mididev *i;

	for (i = mididev_list; i != NULL; i = i->next) {
		if (RMIDI(i)->mdep.fd < 0) {
			continue;
		}
		while(close(RMIDI(i)->mdep.fd) < 0) {
			if (errno != EINTR) {
				perror(RMIDI(i)->mdep.path);
				break;
			}
		}
	}

	sa.sa_handler = SIG_DFL;
	if (sigaction(SIGPIPE, &sa, NULL) < 0) {
		perror("mux_mdep_done: sigaction");
		exit(1);
	}
}


void
mux_mdep_run(void) {
	nfds_t ifds;
	int res;
	struct timeval tv, tv_last;
	struct pollfd fds[DEFAULT_MAXNDEVS];
	struct mididev *dev, *index2dev[DEFAULT_MAXNDEVS];
	static unsigned char midibuf[MIDI_BUFSIZE];
	long delta_usec;
	unsigned i;

	ifds = 0;
	for (dev = mididev_list; dev != NULL; dev = dev->next) {
		if (!(dev->mode & MIDIDEV_MODE_IN)) {
			continue;
		}
		fds[ifds].fd = RMIDI(dev)->mdep.fd;
		fds[ifds].events = POLLIN;
		index2dev[ifds] = dev;
		ifds++;
	}
		
	if (gettimeofday(&tv_last, NULL) < 0) {
		perror("mux_run: initial gettimeofday() failed\n");
		exit(1);
	}

	cons_err("press control-C to finish");
	while (!cons_break()) {
		res = poll(fds, ifds, 1); 		/* 1ms timeout */
		if (res < 0) {
			if (errno == EINTR) {
				continue;
			}
			perror("mux_run: poll failed");
			return;
		}
		
		for (i = 0; i < ifds; i++) {
			if (fds[i].revents & POLLIN) {
				dev = index2dev[i];
				res = read(fds[i].fd, midibuf, MIDI_BUFSIZE);
				if (res < 0) {
					perror(RMIDI(dev)->mdep.path);
					RMIDI(dev)->mdep.idying = 1;
					mux_errorcb(dev->unit);
					continue;
				}
				if (dev->isensto > 0) {
					dev->isensto = MIDIDEV_ISENSTO;
				}
				rmidi_inputcb(RMIDI(dev), midibuf, res);
			}
		}
		
		if (gettimeofday(&tv, NULL) < 0) {
			perror("mux_run: gettimeofday failed");
			return;
		}

		/*
		 * number of micro-seconds between now
		 * and the last timeout we called poll(). 
		 * Warning: because of system clock changes this value
		 * can be negative.
		 */
		delta_usec = 1000000L * (tv.tv_sec - tv_last.tv_sec);
		delta_usec += tv.tv_usec - tv_last.tv_usec;
		if (delta_usec > 0) {
			tv_last = tv;
			/*
			 * update the current position, 
			 * (time unit = 24th of microsecond
			 */
			mux_timercb(24 * delta_usec);
		}
	}
}

	/* 
	 * sleeps for 'millisecs' milliseconds
	 * useful when sending system exclusive messages
	 * IMPORTANT : must never be called inside mux_run 
	 */

void 
mux_sleep(unsigned millisecs) {
	while (poll(NULL, (nfds_t)0, millisecs) < 0) {
		if (errno == EINTR) {
			continue;
		}
		perror("mux_sleep: poll failed");
		exit(1);
	}
}

void
rmidi_mdep_init(struct rmidi *o) {
}

void
rmidi_mdep_done(struct rmidi *o) {
}

void
rmidi_flush(struct rmidi *o) {
	int res;
	unsigned start, stop;
	
	if (!RMIDI(o)->mdep.odying) {
		start = 0;
		stop = o->oused;
		while (start < stop) {
			res = write(o->mdep.fd, o->obuf, o->oused);
			if (res < 0) {
				perror(RMIDI(o)->mdep.path);
				RMIDI(o)->mdep.odying = 1;
				break;
			}
			start += res;
		}
		if (o->oused) {
			o->mididev.osensto = MIDIDEV_OSENSTO;
		}
	}
	o->oused = 0;
}

unsigned
exec_runrcfile(struct exec *o) {
	char *home;
	char name[PATH_MAX];
	struct stat st;
	
	home = getenv("HOME");
	if (home != NULL) {
		snprintf(name, PATH_MAX, "%s" "/" "." RC_NAME, home);
		if (stat(name, &st) == 0) {
			return exec_runfile(o, name);
		}
	}
	if (stat(RC_DIR "/" RC_NAME, &st) == 0) {
		return exec_runfile(o, RC_DIR "/" RC_NAME);
	}
	return 1;
}

extern char *optarg;
extern int optind;

unsigned
user_getopts(int *pargc, char ***pargv) {
	int ch;

	while ((ch = getopt(*pargc, *pargv, "bv")) != -1) {
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
	*pargc -= optind;
	*pargv += optind;
	if (*pargc >= 1) {
	err:
		fputs("usage: midish [-bv]\n", stderr);
		return 0;
	}
	return 1;
}
