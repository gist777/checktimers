#ifndef	__defined_error_h
#define	__defined_error_h

#define	ERRMAXLINE	4096	    /* max text line length */

#define err_ret(fmt, ...) _err_ret(fmt "[%s,%d]", ##__VA_ARGS__, __FILE__, __LINE__)
#define err_sys(fmt, ...) _err_sys(fmt "[%s,%d]", ##__VA_ARGS__, __FILE__, __LINE__)
#define err_dump(fmt, ...) _err_dump(fmt "[%s,%d]", ##__VA_ARGS__, __FILE__, __LINE__)

typedef struct {
  int syslog;     /* set to non-zero to use syslog */
  char *logfile;  /* set to a filename to log to */
} err_opt;

/* Error function prototypes */
void err_setopt(err_opt *);          /* set logging options */
void _err_ret(const char *, ...);    /* print errno message and return */
void _err_sys(const char *, ...);    /* print errno message and exit */
void _err_dump(const char *, ...);   /* print errno message and core */
void err_msg(const char *, ...);     /* print message and return */
void err_quit(const char *, ...);    /* print message and exit */

#endif
