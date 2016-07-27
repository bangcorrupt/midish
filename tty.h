#ifndef TTY_H
#define TTY_H

#define TTY_LINEMAX	1024
#define TTY_PROMPTMAX	64

struct pollfd;

int tty_init(void (*)(void *, int), void *, int);
void tty_done(void);
void tty_setprompt(char *);
void tty_setline(char *);
int tty_pollfd(struct pollfd *);
int tty_revents(struct pollfd *);
void tty_winch(void);
void tty_int(void);
void tty_reset(void);
void tty_write(void *, size_t);

#endif
