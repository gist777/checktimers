/* 
Steven's error functions + minor tweaks 
*/

#include "error.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

static int Syslog = 0;
static int Logfd = 0;

static void err_doit(int, int, const char *, va_list);
static ssize_t writen(int, const void *, size_t);


void err_setopt(err_opt *opt)
{
  Syslog = opt->syslog;
  if (Logfd)
    close(Logfd);
  if (opt->logfile != NULL) {
    if ((Logfd = open(opt->logfile, O_CREAT|O_APPEND|O_WRONLY,
		      S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) == -1) {
      fprintf(stderr, "open() failed on %s", opt->logfile);
      exit(1);
    }
    fcntl(Logfd, F_SETFD, FD_CLOEXEC);
  }
}

void _err_ret(const char *fmt, ...) {
  /* Nonfatal error related to a system call.
   * Print a message and return. */
  va_list ap;
  
  va_start(ap, fmt);
  err_doit(1, LOG_INFO, fmt, ap);
  va_end(ap);
  return;
}

void _err_sys(const char *fmt, ...) {
  /* Fatal error related to a system call.
   * Print a message and terminate. */
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, LOG_ERR, fmt, ap);
  va_end(ap);
  exit(EXIT_FAILURE);
}

void _err_dump(const char *fmt, ...) {
  /* Fatal error related to a system call.
   * Print a message, dump core, and terminate. */
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, LOG_ERR, fmt, ap);
  va_end(ap);
  abort();		/* dump core and terminate */
  exit(EXIT_FAILURE);   /* shouldn't get here */
}

void err_msg(const char *fmt, ...) {
  /* Nonfatal error unrelated to a system call.
   * Print a message and return. */ 
  va_list ap;
	
  va_start(ap, fmt);
  err_doit(0, LOG_INFO, fmt, ap);
  va_end(ap);
  return;
}

void err_quit(const char *fmt, ...) {
  /* Fatal error unrelated to a system call.
   * Print a message and terminate. */
  va_list ap;

  va_start(ap, fmt);
  err_doit(0, LOG_ERR, fmt, ap);
  va_end(ap);
  exit(EXIT_FAILURE);
}

static void err_doit(int errnoflag, int level, const char *fmt, va_list ap) {
  /* Print a message and return to caller.
   * Caller specifies "errnoflag" and "level". */
  int  errno_save, n;
  char buf[ERRMAXLINE];
  time_t t;
  struct tm *tm;
  char timestamp[256];

  errno_save = errno;		/* value caller might want printed */
  vsnprintf(buf, sizeof(buf), fmt, ap);	/* this is safe */
  
  n = strlen(buf);
  if (errnoflag)
    snprintf(buf+n, sizeof(buf)-n, ": %s", strerror(errno_save));
  strcat(buf, "\n");

  if (Syslog || Logfd) {
    if (Syslog)
      syslog(level, buf);
    if (Logfd) {
      t = time(NULL);
      if ((tm = localtime(&t)) == NULL)
	perror("localtime error");
      else {
	if (strftime(timestamp, sizeof(timestamp), "%b %d %T ", tm) == 0)
	  perror("strftime error");
	if (writen(Logfd, timestamp, strlen(timestamp)) == -1)
	  perror("writen error");
      }
      if (writen(Logfd, buf, strlen(buf)) == -1)
	perror("writen error");
    }
  } else {
    fflush(stdout);		/* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(stderr);
  }

  return;
}

static ssize_t writen(int fd, const void *vptr, size_t n)
{
  size_t     nleft;
  ssize_t    nwritten;
  const char *ptr;

  ptr = vptr;
  nleft = n;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) == -1) {
      if (errno == EINTR)
        nwritten = 0;           /* and call write() again */
      else
        return -1;              /* error */
    }
    
    nleft -= nwritten;
    ptr += nwritten;
  }
  return n;
}
