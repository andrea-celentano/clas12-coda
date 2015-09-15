
/* codaterm.c - modified xterm */

/*
./Linux_i686/bin/codaterm -into xyz11 -e "daq_comp_nossh.tcl et_start -n 1000 -s 50000 -f /tmp/et_sys_bla"

./Linux_x86_64/bin/codaterm -fg black -bg lightblue -into bbb0101 -e "daq_comp_nossh.tcl et_start -n 1000 -s 50000 -f /tmp/et_sys_blabla"

*/

/* sergey */
#define DO_EXPECT
/*???
#include <curses.h>
#include <term.h>
*/

#define RES_OFFSET(field)	XtOffsetOf(XTERM_RESOURCE, field)

#include <version.h>
#include <xterm.h>

#include <X11/cursorfont.h>
#include <X11/Xlocale.h>



#if OPT_TOOLBAR

#if defined(HAVE_LIB_XAW)
#include <X11/Xaw/Form.h>
#elif defined(HAVE_LIB_XAW3D)
#include <X11/Xaw3d/Form.h>
#elif defined(HAVE_LIB_NEXTAW)
#include <X11/neXtaw/Form.h>
#endif

#endif /* OPT_TOOLBAR */


#if OPT_LABEL

#if defined(HAVE_LIB_XAW)
#include <X11/Xaw/Form.h>
#elif defined(HAVE_LIB_XAW3D)
#include <X11/Xaw3d/Form.h>
#elif defined(HAVE_LIB_NEXTAW)
#include <X11/neXtaw/Form.h>
#endif

#endif /* OPT_LABEL */




/*sergey*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
/*sergey*/




#include <pwd.h>
#include <ctype.h>

#include <data.h>
#include <error.h>
#include <menu.h>
#include <main.h>
#include <xstrings.h>
#include <xterm_io.h>



#define USE_POSIX_SIGNALS

#ifdef linux
#define USE_SYSV_PGRP
#define USE_SYSV_SIGNALS
#define WTMP
#ifdef __GLIBC__
#if (__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1))
#include <pty.h>
#endif
#endif
#endif


#include <grp.h>


#ifndef TTY_GROUP_NAME
#define TTY_GROUP_NAME "tty"
#endif

#include <sys/stat.h>

#ifdef linux
#define HAS_SAVED_IDS_AND_SETEUID
#endif

/* Xpoll.h and <sys/param.h> on glibc 2.1 systems have colliding NBBY's */
#if defined(__GLIBC__) && ((__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 1)))
#ifndef NOFILE
#define NOFILE OPEN_MAX
#endif
#endif


#if defined(BSD) && (BSD >= 199103)
#define WTMP
#define HAS_SAVED_IDS_AND_SETEUID
#endif

#include <stdio.h>

#ifdef __hpux
#include <sys/utsname.h>
#endif /* __hpux */

#if defined(apollo) && (OSMAJORVERSION == 10) && (OSMINORVERSION < 4)
#define ttyslot() 1
#endif /* apollo */

#if defined(UTMPX_FOR_UTMP)
#define UTMP_STR utmpx
#else
#define UTMP_STR utmp
#endif

#if defined(USE_UTEMPTER)

#elif defined(UTMPX_FOR_UTMP)

#include <utmpx.h>
#define setutent setutxent
#define getutid getutxid
#define endutent endutxent
#define pututline pututxline

#elif defined(HAVE_UTMP)

#include <utmp.h>
#if defined(_CRAY) && (OSMAJORVERSION < 8)
extern struct utmp *getutid __((struct utmp * _Id));
#endif

#endif

#include <lastlog.h>

#if !defined(UTMP_FILENAME)
#if defined(UTMP_FILE)
#define UTMP_FILENAME UTMP_FILE
#elif defined(_PATH_UTMP)
#define UTMP_FILENAME _PATH_UTMP
#else
#define UTMP_FILENAME "/etc/utmp"
#endif
#endif

#ifndef LASTLOG_FILENAME
#ifdef _PATH_LASTLOG
#define LASTLOG_FILENAME _PATH_LASTLOG
#else
#define LASTLOG_FILENAME "/usr/adm/lastlog"	/* only on BSD systems */
#endif
#endif

#if !defined(WTMP_FILENAME)
#if defined(WTMP_FILE)
#define WTMP_FILENAME WTMP_FILE
#elif defined(_PATH_WTMP)
#define WTMP_FILENAME _PATH_WTMP
#elif defined(SYSV)
#define WTMP_FILENAME "/etc/wtmp"
#else
#define WTMP_FILENAME "/usr/adm/wtmp"
#endif
#endif

#include <signal.h>

#if defined(sco) || (defined(ISC) && !defined(_POSIX_SOURCE))
#undef SIGTSTP			/* defined, but not the BSD way */
#endif

#ifdef SIGTSTP
#include <sys/wait.h>
#endif

#ifdef X_NOT_POSIX
extern long lseek();
#if defined(USG) || defined(SVR4)
extern unsigned sleep();
#else
extern void sleep();
#endif
extern char *ttyname();
#endif

#ifdef SYSV
extern char *ptsname(int);
#endif

#ifdef __cplusplus
extern "C" {
#endif

    extern int tgetent(char *ptr, char *name);
    extern char *tgetstr(char *name, char **ptr);

#ifdef __cplusplus
}
#endif

static SIGNAL_T reapchild(int n);
static int spawn(void);
static void remove_termcap_entry(char *buf, char *str);

static int get_pty(int *pty, char *from);
static void get_terminal(void);
static void resize(TScreen * s, char *oldtc, char *newtc);
static void set_owner(char *device, int uid, int gid, int mode);
static Bool added_utmp_entry = False;
static Bool xterm_exiting = False;

/*
** Ordinarily it should be okay to omit the assignment in the following
** statement. Apparently the c89 compiler on AIX 4.1.3 has a bug, or does
** it? Without the assignment though the compiler will init command_to_exec
** to 0xffffffff instead of NULL; and subsequent usage, e.g. in spawn() to
** SEGV.
*/
static char **command_to_exec = NULL;

#ifdef DO_EXPECT
static char **command_to_expect = NULL;
#endif


#define TERMCAP_ERASE "kb"
#define VAL_INITIAL_ERASE A2E(8)

/* choose a nice default value for speed - if we make it too low, users who
 * mistakenly use $TERM set to vt100 will get padding delays
 */
#ifdef B38400			/* everyone should define this */
#define VAL_LINE_SPEED B38400
#else /* ...but xterm's used this for a long time */
#define VAL_LINE_SPEED B9600
#endif

/* allow use of system default characters if defined and reasonable */
#ifndef CBRK
#define CBRK 0
#endif
#ifndef CDSUSP
#define CDSUSP CONTROL('Y')
#endif
#ifndef CEOF
#define CEOF CONTROL('D')
#endif
#ifndef CEOL
#define CEOL 0
#endif
#ifndef CFLUSH
#define CFLUSH CONTROL('O')
#endif
#ifndef CINTR
#define CINTR 0177
#endif
#ifndef CKILL
#define CKILL '@'
#endif
#ifndef CLNEXT
#define CLNEXT CONTROL('V')
#endif
#ifndef CNUL
#define CNUL 0
#endif
#ifndef CQUIT
#define CQUIT CONTROL('\\')
#endif
#ifndef CRPRNT
#define CRPRNT CONTROL('R')
#endif
#ifndef CSTART
#define CSTART CONTROL('Q')
#endif
#ifndef CSTOP
#define CSTOP CONTROL('S')
#endif
#ifndef CSUSP
#define CSUSP CONTROL('Z')
#endif
#ifndef CSWTCH
#define CSWTCH 0
#endif
#ifndef CWERASE
#define CWERASE CONTROL('W')
#endif


/* The following structures are initialized in main() in order
** to eliminate any assumptions about the internal order of their
** contents.
*/
static struct termio d_tio;


/*
 * SYSV has the termio.c_cc[V] and ltchars; BSD has tchars and ltchars;
 * SVR4 has only termio.c_cc, but it includes everything from ltchars.
 * POSIX termios has termios.c_cc, which is similar to SVR4.
 */
static int override_tty_modes = 0;
/* *INDENT-OFF* */
struct _xttymodes {
    char *name;
    size_t len;
    int set;
    char value;
} ttymodelist[] = {
    { "intr",	4, 0, '\0' },	/* tchars.t_intrc ; VINTR */
#define XTTYMODE_intr 0
    { "quit",	4, 0, '\0' },	/* tchars.t_quitc ; VQUIT */
#define XTTYMODE_quit 1
    { "erase",	5, 0, '\0' },	/* sgttyb.sg_erase ; VERASE */
#define XTTYMODE_erase 2
    { "kill",	4, 0, '\0' },	/* sgttyb.sg_kill ; VKILL */
#define XTTYMODE_kill 3
    { "eof",	3, 0, '\0' },	/* tchars.t_eofc ; VEOF */
#define XTTYMODE_eof 4
    { "eol",	3, 0, '\0' },	/* VEOL */
#define XTTYMODE_eol 5
    { "swtch",	5, 0, '\0' },	/* VSWTCH */
#define XTTYMODE_swtch 6
    { "start",	5, 0, '\0' },	/* tchars.t_startc */
#define XTTYMODE_start 7
    { "stop",	4, 0, '\0' },	/* tchars.t_stopc */
#define XTTYMODE_stop 8
    { "brk",	3, 0, '\0' },	/* tchars.t_brkc */
#define XTTYMODE_brk 9
    { "susp",	4, 0, '\0' },	/* ltchars.t_suspc ; VSUSP */
#define XTTYMODE_susp 10
    { "dsusp",	5, 0, '\0' },	/* ltchars.t_dsuspc ; VDSUSP */
#define XTTYMODE_dsusp 11
    { "rprnt",	5, 0, '\0' },	/* ltchars.t_rprntc ; VREPRINT */
#define XTTYMODE_rprnt 12
    { "flush",	5, 0, '\0' },	/* ltchars.t_flushc ; VDISCARD */
#define XTTYMODE_flush 13
    { "weras",	5, 0, '\0' },	/* ltchars.t_werasc ; VWERASE */
#define XTTYMODE_weras 14
    { "lnext",	5, 0, '\0' },	/* ltchars.t_lnextc ; VLNEXT */
#define XTTYMODE_lnext 15
    { "status", 6, 0, '\0' },	/* VSTATUS */
#define XTTYMODE_status 16
    { NULL,	0, 0, '\0' },	/* end of data */
};
/* *INDENT-ON* */

#define TMODE(ind,var) if (ttymodelist[ind].set) var = ttymodelist[ind].value

static int parse_tty_modes(char *s, struct _xttymodes *modelist);

#if (defined(AIXV3) && (OSMAJORVERSION < 4)) && !(defined(getutid))
extern struct utmp *getutid();
#endif /* AIXV3 */

static char etc_lastlog[] = LASTLOG_FILENAME;
static char etc_wtmp[] = WTMP_FILENAME;

/*
 * Some people with 4.3bsd /bin/login seem to like to use login -p -f user
 * to implement xterm -ls.  They can turn on USE_LOGIN_DASH_P and turn off
 * WTMP and USE_LASTLOG.
 */
#ifdef USE_LOGIN_DASH_P
#ifndef LOGIN_FILENAME
#define LOGIN_FILENAME "/bin/login"
#endif
static char bin_login[] = LOGIN_FILENAME;
#endif

static int inhibit;
static char passedPty[PTYCHARLEN + 1];	/* name if pty if slave */

static int Console;
#include <X11/Xmu/SysUtil.h>	/* XmuGetHostname */
#define MIT_CONSOLE_LEN	12
#define MIT_CONSOLE "MIT_CONSOLE_"
static char mit_console_name[255 + MIT_CONSOLE_LEN + 1] = MIT_CONSOLE;
static Atom mit_console;

static sigjmp_buf env;

/* used by VT (charproc.c) */

static XtResource application_resources[] =
{
    Sres("name", "Name", xterm_name, DFT_TERMTYPE),
    Sres("iconGeometry", "IconGeometry", icon_geometry, NULL),
    Sres(XtNtitle, XtCTitle, title, NULL),
    Sres(XtNiconName, XtCIconName, icon_name, NULL),
    Sres("termName", "TermName", term_name, NULL),
    Sres("ttyModes", "TtyModes", tty_modes, NULL),
    Bres("hold", "Hold", hold_screen, FALSE),
    Bres("utmpInhibit", "UtmpInhibit", utmpInhibit, FALSE),
    Bres("messages", "Messages", messages, TRUE),
    Bres("sunFunctionKeys", "SunFunctionKeys", sunFunctionKeys, FALSE),
#if OPT_SUNPC_KBD
    Bres("sunKeyboard", "SunKeyboard", sunKeyboard, FALSE),
#endif
#if OPT_HP_FUNC_KEYS
    Bres("hpFunctionKeys", "HpFunctionKeys", hpFunctionKeys, FALSE),
#endif
    Bres("ptyInitialErase", "PtyInitialErase", ptyInitialErase, FALSE),
    Bres("backarrowKeyIsErase", "BackarrowKeyIsErase", backarrow_is_erase, FALSE),
    Bres("waitForMap", "WaitForMap", wait_for_map, FALSE),
    Bres("useInsertMode", "UseInsertMode", useInsertMode, FALSE),
#if OPT_ZICONBEEP
    Ires("zIconBeep", "ZIconBeep", zIconBeep, 0),
#endif
#if OPT_SAME_NAME
    Bres("sameName", "SameName", sameName, TRUE),
#endif
    Bres("sessionMgt", "SessionMgt", sessionMgt, TRUE),
};


/* sergey: defines menu items !? */
static char *fallback_resources[] =
{
    "*SimpleMenu*menuLabel.vertSpace: 100",
    "*SimpleMenu*HorizontalMargins: 16",
    "*SimpleMenu*Sme.height: 16",
    "*SimpleMenu*Cursor: left_ptr",
    "*mainMenu.Label:  Main OOptions (no app-defaults)",
    "*vtMenu.Label:  VT Opptions (no app-defaults)",
    "*fontMenu.Label:  VV Fonts (no app-defaults)",
    NULL
};
/*sergey: it comes from /usr/share/X11/app-defaults/XTerm !!!!!!!!!*/



/* Command line options table.  Only resources are entered here...there is a
   pass over the remaining options after XrmParseCommand is let loose. */
/* *INDENT-OFF* */
static XrmOptionDescRec optionDescList[] = {
{"-geometry",	"*vt100.geometry",XrmoptionSepArg,	(caddr_t) NULL},
{"-132",	"*c132",	XrmoptionNoArg,		(caddr_t) "on"},
{"+132",	"*c132",	XrmoptionNoArg,		(caddr_t) "off"},
{"-ah",		"*alwaysHighlight", XrmoptionNoArg,	(caddr_t) "on"},
{"+ah",		"*alwaysHighlight", XrmoptionNoArg,	(caddr_t) "off"},
{"-aw",		"*autoWrap",	XrmoptionNoArg,		(caddr_t) "on"},
{"+aw",		"*autoWrap",	XrmoptionNoArg,		(caddr_t) "off"},
#ifndef NO_ACTIVE_ICON
{"-ai",		"*activeIcon",	XrmoptionNoArg,		(caddr_t) "off"},
{"+ai",		"*activeIcon",	XrmoptionNoArg,		(caddr_t) "on"},
#endif /* NO_ACTIVE_ICON */
{"-b",		"*internalBorder",XrmoptionSepArg,	(caddr_t) NULL},
{"-bc",		"*cursorBlink",	XrmoptionNoArg,		(caddr_t) "on"},
{"+bc",		"*cursorBlink",	XrmoptionNoArg,		(caddr_t) "off"},
{"-bcf",	"*cursorOffTime",XrmoptionSepArg,	(caddr_t) NULL},
{"-bcn",	"*cursorOnTime",XrmoptionSepArg,	(caddr_t) NULL},
{"-bdc",	"*colorBDMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+bdc",	"*colorBDMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-cb",		"*cutToBeginningOfLine", XrmoptionNoArg, (caddr_t) "off"},
{"+cb",		"*cutToBeginningOfLine", XrmoptionNoArg, (caddr_t) "on"},
{"-cc",		"*charClass",	XrmoptionSepArg,	(caddr_t) NULL},
{"-cm",		"*colorMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+cm",		"*colorMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-cn",		"*cutNewline",	XrmoptionNoArg,		(caddr_t) "off"},
{"+cn",		"*cutNewline",	XrmoptionNoArg,		(caddr_t) "on"},
{"-cr",		"*cursorColor",	XrmoptionSepArg,	(caddr_t) NULL},
{"-cu",		"*curses",	XrmoptionNoArg,		(caddr_t) "on"},
{"+cu",		"*curses",	XrmoptionNoArg,		(caddr_t) "off"},
{"-dc",		"*dynamicColors",XrmoptionNoArg,	(caddr_t) "off"},
{"+dc",		"*dynamicColors",XrmoptionNoArg,	(caddr_t) "on"},
{"-fb",		"*boldFont",	XrmoptionSepArg,	(caddr_t) NULL},
{"-fbb",	"*freeBoldBox", XrmoptionNoArg,		(caddr_t)"off"},
{"+fbb",	"*freeBoldBox", XrmoptionNoArg,		(caddr_t)"on"},
{"-fbx",	"*forceBoxChars", XrmoptionNoArg,	(caddr_t)"off"},
{"+fbx",	"*forceBoxChars", XrmoptionNoArg,	(caddr_t)"on"},
#ifndef NO_ACTIVE_ICON
{"-fi",		"*iconFont",	XrmoptionSepArg,	(caddr_t) NULL},
#endif /* NO_ACTIVE_ICON */
#ifdef XRENDERFONT
{"-fa",		"*faceName",	XrmoptionSepArg,	(caddr_t) NULL},
{"-fs",		"*faceSize",	XrmoptionSepArg,	(caddr_t) NULL},
#endif
#if OPT_INPUT_METHOD
{"-fx",		"*ximFont",	XrmoptionSepArg,	(caddr_t) NULL},
#endif
#if OPT_HIGHLIGHT_COLOR
{"-hc",		"*highlightColor", XrmoptionSepArg,	(caddr_t) NULL},
#endif
#if OPT_HP_FUNC_KEYS
{"-hf",		"*hpFunctionKeys",XrmoptionNoArg,	(caddr_t) "on"},
{"+hf",		"*hpFunctionKeys",XrmoptionNoArg,	(caddr_t) "off"},
#endif
{"-hold",	"*hold",	XrmoptionNoArg,		(caddr_t) "on"},
{"+hold",	"*hold",	XrmoptionNoArg,		(caddr_t) "off"},
{"-ie",		"*ptyInitialErase", XrmoptionNoArg,	(caddr_t) "on"},
{"+ie",		"*ptyInitialErase", XrmoptionNoArg,	(caddr_t) "off"},
{"-j",		"*jumpScroll",	XrmoptionNoArg,		(caddr_t) "on"},
{"+j",		"*jumpScroll",	XrmoptionNoArg,		(caddr_t) "off"},
#if OPT_C1_PRINT
{"-k8",		"*allowC1Printable", XrmoptionNoArg,	(caddr_t) "on"},
{"+k8",		"*allowC1Printable", XrmoptionNoArg,	(caddr_t) "off"},
#endif
/* parse logging options anyway for compatibility */
{"-l",		"*logging",	XrmoptionNoArg,		(caddr_t) "on"},
{"+l",		"*logging",	XrmoptionNoArg,		(caddr_t) "off"},
{"-lf",		"*logFile",	XrmoptionSepArg,	(caddr_t) NULL},
{"-ls",		"*loginShell",	XrmoptionNoArg,		(caddr_t) "on"},
{"+ls",		"*loginShell",	XrmoptionNoArg,		(caddr_t) "off"},
{"-mb",		"*marginBell",	XrmoptionNoArg,		(caddr_t) "on"},
{"+mb",		"*marginBell",	XrmoptionNoArg,		(caddr_t) "off"},
{"-mc",		"*multiClickTime", XrmoptionSepArg,	(caddr_t) NULL},
{"-mesg",	"*messages",	XrmoptionNoArg,		(caddr_t) "off"},
{"+mesg",	"*messages",	XrmoptionNoArg,		(caddr_t) "on"},
{"-ms",		"*pointerColor",XrmoptionSepArg,	(caddr_t) NULL},
{"-nb",		"*nMarginBell",	XrmoptionSepArg,	(caddr_t) NULL},
{"-nul",	"*underLine",	XrmoptionNoArg,		(caddr_t) "off"},
{"+nul",	"*underLine",	XrmoptionNoArg,		(caddr_t) "on"},
{"-pc",		"*boldColors",	XrmoptionNoArg,		(caddr_t) "on"},
{"+pc",		"*boldColors",	XrmoptionNoArg,		(caddr_t) "off"},
{"-rw",		"*reverseWrap",	XrmoptionNoArg,		(caddr_t) "on"},
{"+rw",		"*reverseWrap",	XrmoptionNoArg,		(caddr_t) "off"},
{"-s",		"*multiScroll",	XrmoptionNoArg,		(caddr_t) "on"},
{"+s",		"*multiScroll",	XrmoptionNoArg,		(caddr_t) "off"},
{"-sb",		"*scrollBar",	XrmoptionNoArg,		(caddr_t) "on"},
{"+sb",		"*scrollBar",	XrmoptionNoArg,		(caddr_t) "off"},
#ifdef SCROLLBAR_RIGHT
{"-leftbar",	"*rightScrollBar", XrmoptionNoArg,	(caddr_t) "off"},
{"-rightbar",	"*rightScrollBar", XrmoptionNoArg,	(caddr_t) "on"},
#endif
{"-rvc",	"*colorRVMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+rvc",	"*colorRVMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-sf",		"*sunFunctionKeys", XrmoptionNoArg,	(caddr_t) "on"},
{"+sf",		"*sunFunctionKeys", XrmoptionNoArg,	(caddr_t) "off"},
{"-si",		"*scrollTtyOutput", XrmoptionNoArg,	(caddr_t) "off"},
{"+si",		"*scrollTtyOutput", XrmoptionNoArg,	(caddr_t) "on"},
{"-sk",		"*scrollKey",	XrmoptionNoArg,		(caddr_t) "on"},
{"+sk",		"*scrollKey",	XrmoptionNoArg,		(caddr_t) "off"},
{"-sl",		"*saveLines",	XrmoptionSepArg,	(caddr_t) NULL},
#if OPT_SUNPC_KBD
{"-sp",		"*sunKeyboard", XrmoptionNoArg,		(caddr_t) "on"},
{"+sp",		"*sunKeyboard", XrmoptionNoArg,		(caddr_t) "off"},
#endif
{"-t",		"*tekStartup",	XrmoptionNoArg,		(caddr_t) "on"},
{"+t",		"*tekStartup",	XrmoptionNoArg,		(caddr_t) "off"},
{"-ti",		"*decTerminalID",XrmoptionSepArg,	(caddr_t) NULL},
{"-tm",		"*ttyModes",	XrmoptionSepArg,	(caddr_t) NULL},
{"-tn",		"*termName",	XrmoptionSepArg,	(caddr_t) NULL},
{"-ulc",	"*colorULMode",	XrmoptionNoArg,		(caddr_t) "off"},
{"+ulc",	"*colorULMode",	XrmoptionNoArg,		(caddr_t) "on"},
{"-ut",		"*utmpInhibit",	XrmoptionNoArg,		(caddr_t) "on"},
{"+ut",		"*utmpInhibit",	XrmoptionNoArg,		(caddr_t) "off"},
{"-im",		"*useInsertMode", XrmoptionNoArg,	(caddr_t) "on"},
{"+im",		"*useInsertMode", XrmoptionNoArg,	(caddr_t) "off"},
{"-vb",		"*visualBell",	XrmoptionNoArg,		(caddr_t) "on"},
{"+vb",		"*visualBell",	XrmoptionNoArg,		(caddr_t) "off"},
{"-pob",	"*popOnBell",	XrmoptionNoArg,		(caddr_t) "on"},
{"+pob",	"*popOnBell",	XrmoptionNoArg,		(caddr_t) "off"},
{"-wf",		"*waitForMap",	XrmoptionNoArg,		(caddr_t) "on"},
{"+wf",		"*waitForMap",	XrmoptionNoArg,		(caddr_t) "off"},
#if OPT_ZICONBEEP
{"-ziconbeep", "*zIconBeep", XrmoptionSepArg, (caddr_t) NULL},
#endif
#if OPT_SAME_NAME
{"-samename",	"*sameName",	XrmoptionNoArg,		(caddr_t) "on"},
{"+samename",	"*sameName",	XrmoptionNoArg,		(caddr_t) "off"},
#endif
{"-sm",		"*sessionMgt",	XrmoptionNoArg,		(caddr_t) "on"},
{"+sm",		"*sessionMgt",	XrmoptionNoArg,		(caddr_t) "off"},
/* options that we process ourselves */
{"-help",	NULL,		XrmoptionSkipNArgs,	(caddr_t) NULL},
{"-version",	NULL,		XrmoptionSkipNArgs,	(caddr_t) NULL},
{"-class",	NULL,		XrmoptionSkipArg,	(caddr_t) NULL},
{"-e",		NULL,		XrmoptionSkipLine,	(caddr_t) NULL},
{"-into",	NULL,		XrmoptionSkipArg,	(caddr_t) NULL},

#ifdef DO_EXPECT
/*sergey: expect-like option*/
{"-expect",	NULL,		XrmoptionSkipLine,	(caddr_t) NULL},
#endif

/* bogus old compatibility stuff for which there are
   standard XtOpenApplication options now */
{"%",		"*tekGeometry",	XrmoptionStickyArg,	(caddr_t) NULL},
{"#",		".iconGeometry",XrmoptionStickyArg,	(caddr_t) NULL},
{"-T",		".title",	XrmoptionSepArg,	(caddr_t) NULL},
{"-n",		"*iconName",	XrmoptionSepArg,	(caddr_t) NULL},
{"-r",		"*reverseVideo",XrmoptionNoArg,		(caddr_t) "on"},
{"+r",		"*reverseVideo",XrmoptionNoArg,		(caddr_t) "off"},
{"-rv",		"*reverseVideo",XrmoptionNoArg,		(caddr_t) "on"},
{"+rv",		"*reverseVideo",XrmoptionNoArg,		(caddr_t) "off"},
{"-w",		".borderWidth", XrmoptionSepArg,	(caddr_t) NULL},
};




static OptionHelp xtermOptions[] = {
{ "-version",              "print the version number" },
{ "-help",                 "print out this message" },
{ "-display displayname",  "X server to contact" },
{ "-geometry geom",        "size (in characters) and position" },
{ "-/+rv",                 "turn on/off reverse video" },
{ "-bg color",             "background color" },
{ "-fg color",             "foreground color" },
{ "-bd color",             "border color" },
{ "-bw number",            "border width in pixels" },
{ "-fn fontname",          "normal text font" },
{ "-fb fontname",          "bold text font" },
{ "-/+fbb",                "turn on/off normal/bold font comparison inhibit"},
{ "-/+fbx",                "turn off/on linedrawing characters"},
#ifdef XRENDERFONT
{ "-fa pattern",           "FreeType font-selection pattern" },
{ "-fs size",              "FreeType font-size" },
#endif
#if OPT_INPUT_METHOD
{ "-fx fontname",          "XIM fontset" },
#endif
{ "-iconic",               "start iconic" },
{ "-name string",          "client instance, icon, and title strings" },
{ "-class string",         "class string (XTerm)" },
{ "-title string",         "title string" },
{ "-xrm resourcestring",   "additional resource specifications" },
{ "-/+132",                "turn on/off 80/132 column switching" },
{ "-/+ah",                 "turn on/off always highlight" },
#ifndef NO_ACTIVE_ICON
{ "-/+ai",                 "turn off/on active icon" },
{ "-fi fontname",          "icon font for active icon" },
#endif /* NO_ACTIVE_ICON */
{ "-b number",             "internal border in pixels" },
{ "-/+bc",                 "turn on/off text cursor blinking" },
{ "-bcf milliseconds",     "time text cursor is off when blinking"},
{ "-bcn milliseconds",     "time text cursor is on when blinking"},
{ "-/+bdc",                "turn off/on display of bold as color"},
{ "-/+cb",                 "turn on/off cut-to-beginning-of-line inhibit" },
{ "-cc classrange",        "specify additional character classes" },
{ "-/+cm",                 "turn off/on ANSI color mode" },
{ "-/+cn",                 "turn on/off cut newline inhibit" },
{ "-cr color",             "text cursor color" },
{ "-/+cu",                 "turn on/off curses emulation" },
{ "-/+dc",                 "turn off/on dynamic color selection" },
#if OPT_HIGHLIGHT_COLOR
{ "-hc color",             "selection background color" },
#endif
#if OPT_HP_FUNC_KEYS
{ "-/+hf",                 "turn on/off HP Function Key escape codes" },
#endif
{ "-/+hold",               "turn on/off logic that retains window after exit" },
{ "-/+ie",                 "turn on/off initialization of 'erase' from pty" },
{ "-/+im",                 "use insert mode for TERMCAP" },
{ "-/+j",                  "turn on/off jump scroll" },
#if OPT_C1_PRINT
{ "-/+k8",                 "turn on/off C1-printable classification"},
#endif
#ifdef ALLOWLOGGING
{ "-/+l",                  "turn on/off logging" },
{ "-lf filename",          "logging filename" },
#else
{ "-/+l",                  "turn on/off logging (not supported)" },
{ "-lf filename",          "logging filename (not supported)" },
#endif
{ "-/+ls",                 "turn on/off login shell" },
{ "-/+mb",                 "turn on/off margin bell" },
{ "-mc milliseconds",      "multiclick time in milliseconds" },
{ "-/+mesg",               "forbid/allow messages" },
{ "-ms color",             "pointer color" },
{ "-nb number",            "margin bell in characters from right end" },
{ "-/+nul",                "turn off/on display of underlining" },
{ "-/+aw",                 "turn on/off auto wraparound" },
{ "-/+pc",                 "turn on/off PC-style bold colors" },
{ "-/+rw",                 "turn on/off reverse wraparound" },
{ "-/+s",                  "turn on/off multiscroll" },
{ "-/+sb",                 "turn on/off scrollbar" },
#ifdef SCROLLBAR_RIGHT
{ "-rightbar",             "force scrollbar right (default left)" },
{ "-leftbar",              "force scrollbar left" },
#endif
{ "-/+rvc",                "turn off/on display of reverse as color" },
{ "-/+sf",                 "turn on/off Sun Function Key escape codes" },
{ "-/+si",                 "turn on/off scroll-on-tty-output inhibit" },
{ "-/+sk",                 "turn on/off scroll-on-keypress" },
{ "-sl number",            "number of scrolled lines to save" },
#if OPT_SUNPC_KBD
{ "-/+sp",                 "turn on/off Sun/PC Function/Keypad mapping" },
#endif
{ "-ti termid",            "terminal identifier" },
{ "-tm string",            "terminal mode keywords and characters" },
{ "-tn name",              "TERM environment variable name" },
{ "-/+ulc",                "turn off/on display of underline as color" },
{ "-/+ut",                 "turn on/off utmp support" },
{ "-/+vb",                 "turn on/off visual bell" },
{ "-/+pob",                "turn on/off pop on bell" },
{ "-/+wf",                 "turn on/off wait for map before command exec" },
{ "-e command args ...",   "command to execute" },

#ifdef DO_EXPECT
/*sergey*/
{ "-expect command|response,[command|response,][command|response]", "command|respond to execute, up to 3" },
#endif

{ "#geom",                 "icon window geometry" },
{ "-T string",             "title name for window" },
{ "-n string",             "icon name for window" },
{ "-C",                    "intercept console messages" },
{ "-Sccn",                 "slave mode on \"ttycc\", file descriptor \"n\"" },
{ "-into windowId",        "use the window id given to -into as the parent window rather than the default root window" },
#if OPT_ZICONBEEP
{ "-ziconbeep percent",    "beep and flag icon of window having hidden output" },
#endif
#if OPT_SAME_NAME
{ "-/+samename",           "turn on/off the no-flicker option for title and icon name" },
#endif
{ "-/+sm",                 "turn on/off the session-management support" },

{ NULL, NULL }};
/* *INDENT-ON* */

static char *message[] =
{
  "Fonts should be fixed width and, if both normal and bold are specified, should",
  "have the same size.  If only a normal font is specified, it will be used for",
  "both normal and bold text (by doing overstriking).  The -e option, if given,",
  "must appear at the end of the command line, otherwise the user's default shell",
  "will be started.  Options that start with a plus sign (+) restore the default.",
  NULL
};

/*
 * Decode a key-definition.  This combines the termcap and ttyModes, for
 * comparison.  Note that octal escapes in ttyModes are done by the normal
 * resource translation.  Also, ttyModes allows '^-' as a synonym for disabled.
 */
static int
decode_keyvalue(char **ptr, int termcap)
{
  char *string = *ptr;
  int value = -1;

  TRACE(("...decode '%s'\n", string));
  if (*string == '^')
  {
	switch (*++string)
    {
	case '?':
	    value = A2E(127);
	    break;
	case '-':
	    /*printf("minusssssssssss\n");sergey*/
	    if (!termcap)
        {
		  errno = 0;
		  value = _POSIX_VDISABLE;
		  if (value == -1)
          {
		    value = fpathconf(0, _PC_VDISABLE);
		    if (value == -1)
            {
			  if (errno != 0) break;	/* skip this (error) */
			  value = 0377;
		    }
		  }
		  break;
	    }
	    /* FALLTHRU */
	default:
	    value = CONTROL(*string);
	    break;
	}
	++string;
  }
  else if (termcap && (*string == '\\'))
  {
	char *d;
	int temp = strtol(string + 1, &d, 8);
	if (temp > 0 && d != string)
    {
	    value = temp;
	    string = d;
	}
  }
  else
  {
	value = CharOf(*string);
	++string;
  }
  *ptr = string;
  return value;
}

/*
 * If we're linked to terminfo, tgetent() will return an empty buffer.  We
 * cannot use that to adjust the $TERMCAP variable.
 */
static Boolean
get_termcap(char *name, char *buffer, char *resized)
{
  register TScreen *screen = &term->screen;

  *buffer = 0;		/* initialize, in case we're using terminfo's tgetent */

  if (name != 0)
  {
	if (tgetent(buffer, name) == 1)
    {
	  TRACE(("get_termcap(%s) succeeded (%s)\n", name,
		   (*buffer
		    ? "ok:termcap, we can update $TERMCAP"
		    : "assuming this is terminfo")));
	  if (*buffer)
      {
		if (!TEK4014_ACTIVE(screen))
        {
		  resize(screen, buffer, resized);
		}
	  }
	  return True;
	}
    else
    {
	  *buffer = 0;	/* just in case */
	}
  }
  return False;
}

static int
abbrev(char *tst, char *cmp, size_t need)
{
  size_t len = strlen(tst);
  return ((len >= need) && (!strncmp(tst, cmp, len)));
}

static void
Syntax(char *badOption)
{
  OptionHelp *opt;
  OptionHelp *list = sortedOpts(xtermOptions, optionDescList, XtNumber(optionDescList));
  int col;

  fprintf(stderr, "%s:  bad command line option \"%s\"\r\n\n", ProgramName, badOption);

  fprintf(stderr, "usage:  %s", ProgramName);
  col = 8 + strlen(ProgramName);
  for (opt = list; opt->opt; opt++)
  {
	int len = 3 + strlen(opt->opt);		/* space [ string ] */
	if (col + len > 79)
    {
	  fprintf(stderr, "\r\n   ");		/* 3 spaces */
	  col = 3;
	}
	fprintf(stderr, " [%s]", opt->opt);
	col += len;
  }

  fprintf(stderr, "\r\n\nType %s -help for a full description.\r\n\n", ProgramName);
  exit(1);
}

static void
Version(void)
{
  printf("%s(%d)\n", XFREE86_VERSION, XTERM_PATCH);
  fflush(stdout);
}

static void
Help(void)
{
  OptionHelp *opt;
  OptionHelp *list = sortedOpts(xtermOptions, optionDescList, XtNumber(optionDescList));
  char **cpp;

  fprintf(stderr, "%s(%d) usage:\n    %s [-options ...] [-e command args]\n\n", XFREE86_VERSION, XTERM_PATCH, ProgramName);
  fprintf(stderr, "where options include:\n");
  for (opt = list; opt->opt; opt++)
  {
	fprintf(stderr, "    %-28s %s\n", opt->opt, opt->desc);
  }

  putc('\n', stderr);
  for (cpp = message; *cpp; cpp++)
  {
	fputs(*cpp, stderr);
	putc('\n', stderr);
  }
  putc('\n', stderr);
  fflush(stderr);
}

/* ARGSUSED */
static Boolean
ConvertConsoleSelection(Widget w GCC_UNUSED,
			Atom * selection GCC_UNUSED,
			Atom * target GCC_UNUSED,
			Atom * type GCC_UNUSED,
			XtPointer * value GCC_UNUSED,
			unsigned long *length GCC_UNUSED,
			int *format GCC_UNUSED)
{
  /* we don't save console output, so can't offer it */
  return False;
}

static void
die_callback(Widget w GCC_UNUSED,
	     XtPointer client_data GCC_UNUSED,
	     XtPointer call_data GCC_UNUSED)
{
  Cleanup(0);
}

static void
save_callback(Widget w GCC_UNUSED,
	      XtPointer client_data GCC_UNUSED,
	      XtPointer call_data)
{
  XtCheckpointToken token = (XtCheckpointToken) call_data;
  /* we have nothing to save */
  token->save_success = True;
}


/*
 * DeleteWindow(): Action proc to implement ICCCM delete_window.
 */
/* ARGSUSED */
static void
DeleteWindow(Widget w,
	     XEvent * event GCC_UNUSED,
	     String * params GCC_UNUSED,
	     Cardinal * num_params GCC_UNUSED)
{
  do_hangup(w, (XtPointer) 0, (XtPointer) 0);
}

/* ARGSUSED */
static void
KeyboardMapping(Widget w GCC_UNUSED,
		XEvent * event,
		String * params GCC_UNUSED,
		Cardinal * num_params GCC_UNUSED)
{
  switch (event->type)
  {
    case MappingNotify:
	XRefreshKeyboardMapping(&event->xmapping);
	break;
  }
}

XtActionsRec actionProcs[] =
{
  {"DeleteWindow", DeleteWindow},
  {"KeyboardMapping", KeyboardMapping},
};

/*
 * Some platforms use names such as /dev/tty01, others /dev/pts/1.  Parse off
 * the "tty01" or "pts/1" portion, and return that for use as an identifier for
 * utmp.
 */
static char *
my_pty_name(char *device)
{
  size_t len = strlen(device);
  Boolean name = False;

  while (len != 0)
  {
	int ch = device[len - 1];
	if (isdigit(ch))
    {
	  len--;
	}
    else if (ch == '/')
    {
	  if (name) break;
	  len--;
	}
    else if (isalpha(ch))
    {
	  name = True;
	  len--;
	}
    else
    {
	  break;
	}
  }
  TRACE(("my_pty_name(%s) -> '%s'\n", device, device + len));
  /*printf("my_pty_name(%s) -> '%s'\n", device, device + len);sergey*/

  return(device + len);
}

/*
 * If the name contains a '/', it is a "pts/1" case.  Otherwise, return the
 * last few characters for a utmp identifier.
 */
static char *
my_pty_id(char *device)
{
  char *name = my_pty_name(device);
  char *leaf = x_basename(name);

  if (name == leaf)		/* no '/' in the name */
  {
	int len = strlen(leaf);
	if (PTYCHARLEN < len) leaf = leaf + (len - PTYCHARLEN);
  }
  TRACE(("my_pty_id  (%s) -> '%s'\n", device, leaf));
  /*printf("my_pty_id  (%s) -> '%s'\n", device, leaf);sergey*/

  return(leaf);
}

/*
 * Set the tty/pty identifier
 */
static void
set_pty_id(char *device, char *id)
{
  char *name = my_pty_name(device);
  char *leaf = x_basename(name);

  if (name == leaf)
  {
	strcpy(my_pty_id(device), id);
  }
  else
  {
	strcpy(leaf, id);
  }
  TRACE(("set_pty_id(%s) -> '%s'\n", id, device));
  /*printf("set_pty_id(%s) -> '%s'\n", id, device);sergey*/
}

/*
 * The original -S option accepts two characters to identify the pty, and a
 * file-descriptor (assumed to be nonzero).  That is not general enough, so we
 * check first if the option contains a '/' to delimit the two fields, and if
 * not, fall-thru to the original logic.
 */
static Boolean
ParseSccn(char *option)
{
  char *leaf = x_basename(option);
  Boolean code = False;

  if (leaf != option)
  {
	if (leaf - option > 1
	    && leaf - option <= PTYCHARLEN
	    && sscanf(leaf, "%d", &am_slave) == 1)
    {
	  size_t len = leaf - option - 1;
	  /*
	   * If the given length is less than PTYCHARLEN, that is
       * all right because the calling application may be
	   * giving us a path for /dev/pts, which would be
	   * followed by one or more decimal digits.
       *
       * For fixed-width fields, it is up to the calling
	   * application to provide leading 0's, if needed.
	   */
	  strncpy(passedPty, option, len);
	  passedPty[len] = 0;
	  code = True;
	}
  }
  else
  {
	code = (sscanf(option, "%c%c%d",
		       passedPty, passedPty + 1, &am_slave) == 3);
  }

  TRACE(("ParseSccn(%s) = '%s' %d (%s)\n", option,
	   passedPty, am_slave, code ? "OK" : "ERR"));

  /*printf("ParseSccn(%s) = '%s' %d (%s)\n", option,
		 passedPty, am_slave, code ? "OK" : "ERR");sergey*/

  return(code);
}


/*
 * From "man utmp":
 * xterm and other terminal emulators directly create a USER_PROCESS record
 * and generate the ut_id by using the last two letters of /dev/ttyp%c or by
 * using p%d for /dev/pts/%d.  If they find a DEAD_PROCESS for this id, they
 * recycle it, otherwise they create a new entry.  If they can, they will mark
 * it as DEAD_PROCESS on exiting and it is advised that they null ut_line,
 * ut_time, ut_user and ut_host as well.
 *
 * Generally ut_id allows no more than 3 characters (plus null), even if the
 * pty implementation allows more than 3 digits.
 */
static char *
my_utmp_id(char *device)
{
  static char result[PTYCHARLEN + 4];
  char *name = my_pty_name(device);
  char *leaf = x_basename(name);

  if (name == leaf) 		/* no '/' in the name */
  {
	int len = strlen(leaf);
	if (PTYCHARLEN < len) leaf = leaf + (len - PTYCHARLEN);
	strcpy(result, leaf);
  }
  else
  {
	sprintf(result, "p%s", leaf);
  }

  TRACE(("my_utmp_id  (%s) -> '%s'\n", device, result));
  /*printf("my_utmp_id  (%s) -> '%s'\n", device, result);sergey*/

  return(result);
}





typedef void (*sigfunc) (int);

/* make sure we sure we ignore SIGCHLD for the cases parent
   has just been stopped and not actually killed */

static sigfunc
posix_signal(int signo, sigfunc func)
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NOCLDSTOP | SA_RESTART;
  if (sigaction(signo, &act, &oact) < 0) return (SIG_ERR);

  return (oact.sa_handler);
}






#ifdef DO_EXPECT

/********/
/*sergey*/

#include <signal.h>
#include <string.h>

#define NCODACOMMANDS 3

int  coda_ncommands = 0;
char coda_command[NCODACOMMANDS][128];

int  coda_nresponses = 0;
char coda_response[NCODACOMMANDS][128];

#ifdef OPT_LABEL
char coda_label[128];

void
codaRegisterLabel(char *text)
{
  strcpy(coda_label, text);
  printf("codaRegisterLabel >%s<\n",coda_label);
}

char *
codaGetLabel()
{
  return(coda_label);
}
#endif

void
codaRegisterCommand(char *command, char *response)
{
  if(coda_ncommands >= NCODACOMMANDS)
  {
    printf("codaRegisterCommand ERROR: cannot register more then %d commands\n",NCODACOMMANDS);
  }
  else
  {
    strcpy(coda_command[coda_ncommands], command);
    strcpy(coda_response[coda_nresponses], response);
    coda_ncommands ++;
    coda_nresponses ++;
  }
}

void
codaSendCommand(int fd)
{
  int len;
  if(coda_ncommands)
  {
    /* add '\r' to the end of command */
    len = strlen(coda_command[coda_ncommands-1]);
    coda_command[coda_ncommands-1][len] = '\r';
    coda_command[coda_ncommands-1][len+1] = '\0';

    /* send command */
    unparseputs(coda_command[coda_ncommands-1], fd);
    coda_ncommands --;
  }
}

void
codaCheckResponse(char *str, int len)
{
  int ii;

  if(coda_nresponses)
  {
    if(strstr(str, coda_response[coda_nresponses-1]) != NULL)
    {
      printf("\nsubstring >%s< found in string >",coda_response[coda_nresponses-1]);
      for(ii=0; ii<len; ii++) printf("%c",str[ii]);
      printf("<\n");
      coda_nresponses --;
    }
    else
    {
      printf("\nsubstring >%s< NOT found in string >",coda_response[coda_nresponses-1]);
      for(ii=0; ii<len; ii++) printf("%c",str[ii]);
      printf("<\n");
    }
  }
}


static int
listSplit(char *list, char *separator, int *argc, char argv[NCODACOMMANDS][128])
{
  char *p, str[1024];
  strcpy(str,list);
  p = strtok(str,separator);
  *argc = 0;
  while(p != NULL)
  {
    /*printf("1[%d]: >%s< (%d)\n",*argc,p,strlen(p));*/
    strcpy((char *)&argv[*argc][0], (char *)p);
    /*printf("2[%d]: >%s< (%d)\n",*argc,(char *)&argv[*argc][0],strlen((char *)&argv[*argc][0]));*/
    (*argc) ++;
    if( (*argc) >= NCODACOMMANDS)
	{
      printf("listSplit ERROR: too many args\n");
      return(0);
	}
    p = strtok(NULL,separator);
  }

  return(0);
}

#endif







int
Xhandler(Widget w, XtPointer p, XEvent *e, Boolean *b)
{
#ifdef DEBUG
  printf("codaterm Xhandler reached\n");
#endif

  if (e->type == DestroyNotify)
  {
    printf("CODATERM:X window was destroyed\n");
    exit(0);
  }

  return(0);
}


void
messageHandler(char *message)
{
  printf("\n--> codaterm::messageHandler reached, message >%s< =================================\n\n",message);

  switch (message[0])
  {
  case 'c':
    printf("\ncodaterm::messageHandler: using config >%s<\n\n",(char *)&message[2]);
    CodaTermSelectConfig(&message[2]);
    break;
  case 'e':
    /*EditorSelectExp(toplevel,&message[2]);*/
    break;
  case 's':
    {
      int state;
      char name[50];
      sscanf(&message[2],"%d %s",&state, name);
      /*setCompState(name,state);*/
    }
    break;

  case 'b':
    printf("--> recieved >%s<\n",message);
    break;

  default:
  printf("unknown message : %s\n",message);
  
  }
}





static int embedded;
static char embedded_name[128];
/*sergey*/





int codaterm(int argc, char *argv[]);


void
codatermtest()
{
  int myargc;
  char *myargv[7];

  myargc = 0;
  myargv[myargc++] = strdup( "codaterm" );
  myargv[myargc++] = strdup( "-into" );
  myargv[myargc++] = strdup( "00_00" );

  codaterm(myargc, myargv);
}

void
codaterm1()
{
  int myargc;
  char *myargv[7];

  myargc = 7;
  myargv[0] = strdup( "codaterm" );
  myargv[1] = strdup( "-into" );
  myargv[2] = strdup( "01_01" );
  myargv[3] = strdup( "-expect" );
  myargv[4] = strdup( "ssh adcecal5:adcecal5" );
  myargv[5] = strdup( "stop_coda_process -p coda_roc_gef -match \"adcecal5 ROC\"" );
  myargv[6] = strdup( "coda_roc_gef -s clastest -o \"adcecal5 ROC\"" );

  codaterm(myargc, myargv);
}



int
codaterm(int argc, char *argv[])
{
  Widget form_top, menu_top;
  register TScreen *screen;
  int ac, mode;
  char *my_class = DEFCLASS;
  Window winToEmbedInto = None;
  Arg arg[20];

  int ii;
  printf("\ncodaterm: argc=%d\n",argc);
  for(ii=0; ii<argc; ii++)
  {
    printf("codaterm: argv[%d] >%s<\n",ii,argv[ii]);
  }
  printf("\n\n");

  ProgramName = argv[0];

  /* extra length in case longer tty name like /dev/ttyq255 */
  ttydev = (char *) malloc(sizeof(TTYDEV) + 80);
  if (!ttydev)
  {
	fprintf(stderr, "%s: unable to allocate memory for ttydev or ptydev\n", ProgramName);
	exit(1);
  }
  strcpy(ttydev, TTYDEV); /* sergey: fills with default value, will be overwritten later */


  /* Do these first, since we may not be able to open the display */
  TRACE_OPTS(xtermOptions, optionDescList, XtNumber(optionDescList));
  TRACE_ARGV("Before XtOpenApplication", argv);
  if (argc > 1)
  {
	int n;
	int unique = 2;
	Boolean quit = True;

	for (n = 1; n < argc; n++)
    {
	  TRACE(("parsing %s\n", argv[n]));
	  if (abbrev(argv[n], "-version", unique))
      {
		Version();
	  }
      else if (abbrev(argv[n], "-help", unique))
      {
		Help();
	  }
      else if (abbrev(argv[n], "-class", 3))
      {
		if ((my_class = argv[++n]) == 0)
        {
		  Help();
		}
        else
        {
		  quit = False;
		}
		unique = 3;
	  }
      else
      {
		quit = False;
		unique = 3;
	  }
	}
	if (quit) exit(0);
  }

  /* This dumps core on HP-UX 9.05 with X11R5 */
  XtSetLanguageProc(NULL, NULL, NULL);


  /* Initialization is done here rather than above in order
   * to prevent any assumptions about the order of the contents
   * of the various terminal structures (which may change from
   * implementation to implementation).
   */
  d_tio.c_iflag = ICRNL | IXON;
  d_tio.c_oflag = OPOST | ONLCR | TAB3;
  d_tio.c_cflag = VAL_LINE_SPEED | CS8 | CREAD | PARENB | HUPCL;
  d_tio.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK;
  d_tio.c_lflag |= ECHOKE | IEXTEN;
  d_tio.c_lflag |= ECHOCTL | IEXTEN;
  d_tio.c_line = 0;
  d_tio.c_cc[VINTR] = CONTROL('C');	/* '^C' */
  d_tio.c_cc[VERASE] = 0x7f;	/* DEL  */
  d_tio.c_cc[VKILL] = CONTROL('U');	/* '^U' */
  d_tio.c_cc[VQUIT] = CQUIT;	/* '^\' */
  d_tio.c_cc[VEOF] = CEOF;	/* '^D' */
  d_tio.c_cc[VEOL] = CEOL;	/* '^@' */
  d_tio.c_cc[VMIN] = 1;
  d_tio.c_cc[VTIME] = 0;

  d_tio.c_cc[VLNEXT] = CLNEXT;
  d_tio.c_cc[VWERASE] = CWERASE;
  d_tio.c_cc[VREPRINT] = CRPRNT;
  d_tio.c_cc[VDISCARD] = CFLUSH;
  d_tio.c_cc[VSTOP] = CSTOP;
  d_tio.c_cc[VSTART] = CSTART;
  d_tio.c_cc[VSUSP] = CSUSP;

  /* now, try to inherit tty settings */
  {
	int i;

	for (i = 0; i <= 2; i++)
    {
	  struct termio deftio;
	  if (ioctl(i, TCGETA, &deftio) == 0)
	  {
		d_tio.c_cc[VINTR] = deftio.c_cc[VINTR];
		d_tio.c_cc[VQUIT] = deftio.c_cc[VQUIT];
		d_tio.c_cc[VERASE] = deftio.c_cc[VERASE];
		d_tio.c_cc[VKILL] = deftio.c_cc[VKILL];
		d_tio.c_cc[VEOF] = deftio.c_cc[VEOF];
		d_tio.c_cc[VEOL] = deftio.c_cc[VEOL];
		d_tio.c_cc[VEOL2] = deftio.c_cc[VEOL2];
		d_tio.c_cc[VLNEXT] = deftio.c_cc[VLNEXT];

		d_tio.c_cc[VWERASE] = deftio.c_cc[VWERASE];
		d_tio.c_cc[VREPRINT] = deftio.c_cc[VREPRINT];
		d_tio.c_cc[VDISCARD] = deftio.c_cc[VDISCARD];
		d_tio.c_cc[VSTOP] = deftio.c_cc[VSTOP];
		d_tio.c_cc[VSTART] = deftio.c_cc[VSTART];
		d_tio.c_cc[VSUSP] = deftio.c_cc[VSUSP];
		break;
	  }
	}
  }


  d_tio.c_cc[VSUSP] = CSUSP;
  d_tio.c_cc[VREPRINT] = '\377';
  d_tio.c_cc[VDISCARD] = '\377';
  d_tio.c_cc[VWERASE] = '\377';
  d_tio.c_cc[VLNEXT] = '\377';



  /* Init the Toolkit. */
  {
	uid_t euid = geteuid();
	gid_t egid = getegid();
	uid_t ruid = getuid();
	gid_t rgid = getgid();

	if (setegid(rgid) == -1)
    {
	  (void) fprintf(stderr, "setegid(%d): %s\n", (int)rgid, strerror(errno));
	}

	if (seteuid(ruid) == -1)
    {
	  (void) fprintf(stderr, "seteuid(%d): %s\n", (int)ruid, strerror(errno));
	}

	XtSetErrorHandler(xt_error);

	{
      int iii;
	  printf("befor XtOpenApplication(): argc=%d\n",argc);
      for(iii=0; iii<argc; iii++) printf("   argv[%d] >%s<\n",iii,argv[iii]);
	}
	toplevel = XtOpenApplication(&app_con, my_class,
				     optionDescList,
				     XtNumber(optionDescList),
					 &argc, argv, fallback_resources,
				     sessionShellWidgetClass,
				     NULL, 0);

    printf("main: toplevel=0x%08x\n",toplevel);

	XtSetErrorHandler((XtErrorHandler) 0);

	XtGetApplicationResources(toplevel, (XtPointer) & resource,
				  application_resources,
				  XtNumber(application_resources), NULL, 0);

	if (seteuid(euid) == -1)
    {
	  (void) fprintf(stderr, "seteuid(%d): %s\n", (int)euid, strerror(errno));
	}

	if (setegid(egid) == -1)
    {
	  (void) fprintf(stderr, "setegid(%d): %s\n", (int)egid, strerror(errno));
	}

  }

  waiting_for_initial_map = resource.wait_for_map;


  /*
   * ICCCM delete_window.
   */
  XtAppAddActions(app_con, actionProcs, XtNumber(actionProcs));




  /*sergey*/
  printf("\n\nresource.tty_modes=%d\n",resource.tty_modes);
   
  /*printf("\ngeometry=%d\n",resource.geometry);*/

  printf("\n\n");




  /*
   * fill in terminal modes
   */
  if (resource.tty_modes)
  {
	int n = parse_tty_modes(resource.tty_modes, ttymodelist);
	if (n < 0)
    {
	  fprintf(stderr, "%s:  bad tty modes \"%s\"\n", ProgramName, resource.tty_modes);
	}
    else if (n > 0)
    {
	  override_tty_modes = 1;
	}
  }

  zIconBeep = resource.zIconBeep;
  zIconBeep_flagged = False;
  if (zIconBeep > 100 || zIconBeep < -100)
  {
	zIconBeep = 0;		/* was 100, but I prefer to defaulting off. */
	fprintf(stderr,
		"a number between -100 and 100 is required for zIconBeep.  0 used by default\n");
  }

  sameName = resource.sameName;

  hold_screen = resource.hold_screen ? 1 : 0;
  xterm_name = resource.xterm_name;
  if (strcmp(xterm_name, "-") == 0) xterm_name = DFT_TERMTYPE;
  if (resource.icon_geometry != NULL)
  {
	int scr, junk;
	int ix, iy;
	Arg args[2];

	printf("NEVER HERE\n");

	for (scr = 0;		/* yyuucchh */
	     XtScreen(toplevel) != ScreenOfDisplay(XtDisplay(toplevel), scr);
	     scr++) ;

	args[0].name = XtNiconX;
	args[1].name = XtNiconY;
	XGeometry(XtDisplay(toplevel), scr, resource.icon_geometry, "",
		  0, 0, 0, 0, 0, &ix, &iy, &junk, &junk);
	args[0].value = (XtArgVal) ix;
	args[1].value = (XtArgVal) iy;
	XtSetValues(toplevel, args, 2);
  }

  XtSetValues(toplevel, ourTopLevelShellArgs, number_ourTopLevelShellArgs);


  /* Parse the rest of the command line */
  TRACE_ARGV("After XtOpenApplication", argv);
  for (argc--, argv++; argc > 0; argc--, argv++)
  {
	if (**argv != '-')
	{
	  /*printf("syntax 1\n");sergey*/
	  Syntax(*argv);
	}

	TRACE(("parsing %s\n", argv[0]));
	switch (argv[0][1])
    {
	case 'h':		/* -help */
	    Help();
	    continue;

	case 'v':		/* -version */
	    Version();
	    continue;

	case 'C':
	    {
		struct stat sbuf;

		/* Must be owner and have read/write permission.
		   xdm cooperates to give the console the right user. */
		if (!stat("/dev/console", &sbuf) &&
		    (sbuf.st_uid == getuid()) &&
		    !access("/dev/console", R_OK | W_OK))
        {
		  Console = TRUE;
		}
        else
		{
		  Console = FALSE;
		}
	    }
	    continue;

	case 'S':
	    if (!ParseSccn(*argv + 2))
		{
	      /*printf("syntax 2\n");sergey*/
          Syntax(*argv);
		}
	    continue;
#ifdef DEBUG
	case 'D':
	    debug = TRUE;
	    continue;
#endif /* DEBUG */

	case 'c':		/* -class param */
	    if (strcmp(argv[0] + 1, "class") == 0)
		argc--, argv++;
	    else
		{
	      /*printf("syntax 3\n");sergey*/
		  Syntax(*argv);
		}
	    continue;

	case 'e':
	  printf("-e: argc=%d, argv >%s<\n",argc,*argv);/*sergey*/
		if (argc <= 1)
		{
	      /*printf("syntax 4\n");*/
		  Syntax(*argv);
		}
#ifdef DO_EXPECT
        if(!strncmp(*argv,"-expect",7))
		{
          int ii, jj, ncommands;
          int  listArgc;
          char listArgv[2][128];	

	      command_to_expect = ++argv;
          argc--;

          /* count commands */
          ncommands = 0;
		  for(ii=0; ii<argc; ii++)
		  {
            if(command_to_expect[ii] == NULL) break;
            ncommands ++;
		  }
          printf("ncommands=%d\n",ncommands);

          /* sparse commands and call 'codaRegisterCommand' in backward order */
		  for(ii=ncommands-1; ii>=0; ii--)
		  {
            printf(">>> expect command [%d] >%s<\n",ii,(char *)command_to_expect[ii]);

	 	    /* parse */
 	        listSplit(command_to_expect[ii],":",&listArgc,listArgv);
            for(jj=0; jj<listArgc; jj++)
            {
              printf("split1[%d] >%s<\n",jj,listArgv[jj]);
	        }
            if(listArgc==2) codaRegisterCommand(listArgv[0],listArgv[1]);
            else            codaRegisterCommand(listArgv[0],"");
		  }

#ifdef OPT_LABEL
          /* form label text from responses */
          coda_label[0] = '\0';
		  for(ii=ncommands-1; ii>=0; ii--)
		  {
            strcat(coda_label,coda_response[ii]);
            if(ii>0) strcat(coda_label,":");
		  }
#endif


		  /* ex.:
codaterm -expect 'ssh adcecal5:adcecal5' 'stop_coda_process -p coda_roc_gef -match "adcecal5 ROC":adcecal5' 'coda_roc_gef -s clastest -o "adcecal5 ROC":adcecal5'
		   */

		  /*
  codaRegisterCommand("coda_roc_gef -s clastest -o \"adcecal5 ROC\"","adcecal5");
  codaRegisterCommand("stop_coda_process -p coda_roc_gef -match \"adcecal5 ROC\"","adcecal5");
  codaRegisterCommand("ssh adcecal5","adcecal5");
		  */


		  break;          
		}
#endif
	    command_to_exec = ++argv;
        /*printf(">>> command_to_exec >%s<\n",(char *)command_to_exec[0]);sergey*/
	    break; /* '-e' must be last argument !!! */

	case 'i':
	    if (argc <= 1)
        {
	      /*printf("syntax 5\n");sergey*/
		  Syntax(*argv);
	    }
        else
        {
		  char *endPtr;
		  --argc;
		  ++argv;
          strcpy(embedded_name, argv[0]);
          printf(">>> embedded window requested >%s<\n",embedded_name);
		  winToEmbedInto = (Window) strtol(argv[0], &endPtr, 10);
          embedded = 1;
	    }
	    continue/*break*/;

	default:
	    /*printf("syntax 6\n");sergey*/
	    Syntax(*argv);
	}
	break;
  }


  /*
  SetupMenus(toplevel, &form_top, &menu_top);
  */

/*#ifdef OPT_LABEL*/


  ac = 0;
  top = XmCreateFrame (toplevel, "work_area", arg, ac);
  {
    Widget label_w;
    ac = 0;
    XtSetArg (arg[ac], XmNframeChildType, XmFRAME_TITLE_CHILD); ac++;
    XtSetArg (arg[ac], XmNchildVerticalAlignment, XmALIGNMENT_CENTER); ac++;
    label_w = XmCreateLabelGadget (top, /*"!!!TITLE!!!"*/codaGetLabel(), arg, ac);
    XtManageChild (label_w);
  }

  /*
  form_top = XtVaCreateManagedWidget("form", formWidgetClass, top, (XtPointer) 0);
  */
  form_top = top;
  /*xtermAddInput(form_top); do nothing*/


/*#endif*/



  /* this causes the Realize method to be called */
  term = (XtermWidget) XtVaCreateManagedWidget("vt100", xtermWidgetClass, form_top,
#if OPT_TOOLBAR
						 XtNmenuBar, menu_top,
						 XtNresizable, True,
						 XtNfromVert, menu_top, /* sergey: comment it out to make menu small*/
						 XtNleft, XawChainLeft,
						 XtNright, XawChainRight,
						 XtNbottom, XawChainBottom,
#endif
#if OPT_LABEL   
						 XtNresizable, True,		   
											   /*
						 XtNtop, XawChainTop,
						 XtNleft, XawChainLeft,
						 XtNright, XawChainRight,
						 XtNbottom, XawChainBottom,
											   */
											   XmNtopAttachment, XmATTACH_FORM,
											   XmNbottomAttachment, XmATTACH_FORM,
											   XmNleftAttachment, XmATTACH_FORM,
											   XmNrightAttachment, XmATTACH_FORM,

#endif
                         (XtPointer) 0);


  printf("main: term=0x%08x\n",term);
 

  /* this causes the initialize method to be called */
  init_keyboard_type(keyboardIsSun, resource.sunFunctionKeys);
  init_keyboard_type(keyboardIsVT220, resource.sunKeyboard);

  screen = &term->screen;

  inhibit = 0;
#ifdef ALLOWLOGGING
  if (term->misc.logInhibit) inhibit |= I_LOG;
#endif
  if (term->misc.signalInhibit) inhibit |= I_SIGNAL;

  if (resource.sessionMgt)
  {
	TRACE(("Enabling session-management callbacks\n"));
	XtAddCallback(toplevel, XtNdieCallback, die_callback, NULL);
	XtAddCallback(toplevel, XtNsaveCallback, save_callback, NULL);
  }

  /*
   * Set title and icon name if not specified
   */
  if (command_to_exec)
  {
	Arg args[2];

	if (!resource.title)
    {
	  if (command_to_exec)
      {
  	    resource.title = x_basename(command_to_exec[0]);
	  }			/* else not reached */
	}

	if (!resource.icon_name) resource.icon_name = resource.title;

	XtSetArg(args[0], XtNtitle, resource.title);
	XtSetArg(args[1], XtNiconName, resource.icon_name);

	TRACE(("setting:\n\ttitle \"%s\"\n\ticon \"%s\"\n\tbased on command \"%s\"\n",
	       resource.title,
	       resource.icon_name,
	       *command_to_exec));

	XtSetValues(toplevel, args, 2);
  }


#ifdef DEBUG
  {
	/* Set up stderr properly.  Opening this log file cannot be
	   done securely by a privileged xterm process (although we try),
	   so the debug feature is disabled by default. */
	char dbglogfile[45];
	int i = -1;
	if (debug)
    {
	  timestamp_filename(dbglogfile, "xterm.debug.log.");
	  if (creat_as(getuid(), getgid(), False, dbglogfile, 0666))
      {
		i = open(dbglogfile, O_WRONLY | O_TRUNC);
	  }
	}
	if (i >= 0)
    {
	  dup2(i, 2);

	  /* mark this file as close on exec */
	  (void) fcntl(i, F_SETFD, 1);
	}
  }
#endif /* DEBUG */


  /* open a terminal for client */
  get_terminal();


  spawn();


  /* Child process is out there, let's catch its termination */

  (void) posix_signal(SIGCHLD, reapchild);

  /* Realize procs have now been executed */

  if (am_slave >= 0)	/* Write window id so master end can read and use */
  {
	char buf[80];

	buf[0] = '\0';
	sprintf(buf, "%lx\n", XtWindow(XtParent(CURRENT_EMU(screen))));
	write(screen->respond, buf, strlen(buf));
  }


  screen->inhibit = inhibit;

  if (0 > (mode = fcntl(screen->respond, F_GETFL, 0))) SysError(ERROR_F_GETFL);

  mode |= O_NDELAY;

  if (fcntl(screen->respond, F_SETFL, mode)) SysError(ERROR_F_SETFL);

  FD_ZERO(&pty_mask);
  FD_ZERO(&X_mask);
  FD_ZERO(&Select_mask);
  FD_SET(screen->respond, &pty_mask);
  FD_SET(ConnectionNumber(screen->display), &X_mask);
  FD_SET(screen->respond, &Select_mask);
  FD_SET(ConnectionNumber(screen->display), &Select_mask);
  max_plus1 = ((screen->respond < ConnectionNumber(screen->display))
		 ? (1 + ConnectionNumber(screen->display))
		 : (1 + screen->respond));

#ifdef DEBUG
  if (debug) printf("debugging on\n");
#endif /* DEBUG */

  XSetErrorHandler(xerror);
  XSetIOErrorHandler(xioerror);

#ifdef ALLOWLOGGING
  if (term->misc.log_on)
  {
	StartLog(screen);
  }
#endif

  if(embedded) /*(winToEmbedInto != None)*/
  {
	/*???????????? here ? or later ? or does not need it ?*/
	/*XtRealizeWidget(toplevel);*/
	

	/*
	 * This should probably query the tree or check the attributes of
	 * winToEmbedInto in order to verify that it exists, but I'm still not
	 * certain what is the best way to do it -GPS
	 */
	  /*sergey: we get 'winToEmbedInto' from creg and it called 'parent', see below
	  XReparentWindow(XtDisplay(toplevel),
			XtWindow(toplevel),
			winToEmbedInto, 0, 0);
	  */
  }






  {
    int ac, ix;
    Arg args[10];
    Widget w;
    char parent_name[100];
    char my_name[100];
    char cmd[100];
    parent = 0;

#ifdef USE_CREG


    /*printf("CREG 1\n");*/
    if (embedded)
    {
	  printf("wwwwwwwwwwwwwwwwwwwww CREG wwwwwwwwwwwwwww (-embed)\n");

      sprintf(parent_name,"%s_WINDOW",embedded_name);
      sprintf(my_name,"%s_MY_WINDOW",embedded_name);
      printf("parent_name >%s<, my_name >%s<\n",parent_name,my_name);
      
      /* at that point we assume that window 'parent_name' is exist already (for example opened by 'rocs') */

      parent = CODAGetAppWindow(XtDisplay(toplevel),parent_name);

      printf("main: parent=0x%08x\n",parent);
    }


    /*printf("CREG 2\n");*/
    if (parent)
    {
      sprintf(cmd,"r:0x%08x 0x%08x",XtWindow(toplevel),parent);     
      printf("cmd >%s<\n",cmd);


      /* assume 'rocs' did 'codaSendInit(toplevel,"ALLROCS")' */
      /* second parameter is the same as in parent's call 'codaSendInit(toplevel,"ALLROCS")' */
      /*coda_Send(XtDisplay(toplevel),"RUNCONTROL",cmd);*/


	  
      coda_Send(XtDisplay(toplevel), parent_name, cmd);
	  










	  /* initialize callbacks from 'rocs'; use name ROCS */
	  /*sergey: 'rocs' will be able to do 'coda_Send(..,"CODATERM",..)'*/
      CodaTerm(toplevel,1);

/*sergey: was parent_name, it deleted and created again XX_XX_WINDOW in CODARegistry, as result next time embedding does not work !!! */
/*needed to call Xhandler only ??? */
      codaSendInit(toplevel, my_name);
      codaRegisterMsgCallback(messageHandler);
      XtAddEventHandler(toplevel, StructureNotifyMask, False, Xhandler, NULL); /*Xhandler will exit if window was destroyed*/




	  /* in creg
	  XtAddEventHandler( (Widget)w1, (EventMask)StructureNotifyMask, (Boolean)False, (XtEventHandler)resizeHandler, (XtPointer)target);
	  */




	  /*
      ac = 0;
      XtSetArg(args[ac], XmNoverrideRedirect,False); ac++;
      XtSetValues (toplevel, args, ac);
	  

      XtRealizeWidget(toplevel);
	  */


  printf("XReparentWindow called 2 parent=0x%08x\n",parent);
      XReparentWindow(XtDisplay(toplevel), XtWindow(toplevel), parent, 0, 0);



	  /*no effect ?
      XReparentWindow(XtDisplay(toplevel), XtWindow(toplevel), parent, 0, 0);
	  */

	  /* works but overlaping
      XReparentWindow(XtDisplay(toplevel), XtWindow(top), parent, 0, 0);
	  */


      {
		/*
        Widget w1;
        Widget w2;
        Display *dis;

        dis = XtDisplay(toplevel);
        w2 = XtWindowToWidget(dis, parent);
        w1 = XtParent(w2);
		*/
        /*XtAddEventHandler(toplevel, StructureNotifyMask, False, Xhandler, NULL);*/
	  }





	  /*
XWithdrawWindow(XtDisplay(toplevel), XtWindow(toplevel), 0);
	  */

	  /*
      XtUnmanageChild(toplevel);
	  */
      /* ??? have to make shell 'toplevel' invisible, otherwise following 'XReparentWindow' call will leave it outside */


      /*XtRealizeWidget(top);*/
	  /*
	  XtUnmapWidget(toplevel);
      XtUnrealizeWidget(toplevel);
	  */


      /*XReparentWindow(XtDisplay(toplevel), XtWindow(top), toplevel, 0, 0);*/

      /*XtUnrealizeWidget(toplevel);*/
    }
    else
	{

  printf("ERRRRRRRRRRRRRRRRRRRRRRRRRRRRRR parent=%d\n",parent);
	}
#else
	printf("uuuuuuuuuuuuuuu NO CREG uuuuuuuuuuuuuuuuuuu\n");
#endif
	;


	/* need that ???????????????? 
#ifdef USE_CREG
    {
      ac = 0;
      XtSetArg(args[ac], XmNoverrideRedirect,False); ac++;
      XtSetValues (toplevel, args, ac);

	  CodaTerm(toplevel,0);
      XtRealizeWidget(toplevel);
    }
#endif
	*/

	/*??????????????*/
	/*XtRealizeWidget(toplevel);*/
	

  }

  /*printf("CREG 5\n");*/

  /* register commands in an order OPPOSITE to execution one !!! */

  /*
  codaRegisterCommand("coda_roc_gef -s clastest -o \"adcecal5 ROC\"","adcecal5");

  codaRegisterCommand("stop_coda_process -p coda_roc_gef -match \"adcecal5 ROC\"","adcecal5");

  codaRegisterCommand("ssh adcecal5","adcecal5");
  */



  for (;;)
  {
	VTRun();
  }
}























/*
 * This function opens up a pty master and stuffs its value into pty.
 *
 * If it finds one, it returns a value of 0.  If it does not find one,
 * it returns a value of !0.  This routine is designed to be re-entrant,
 * so that if a pty master is found and later, we find that the slave
 * has problems, we can re-enter this function and get another one.
 */
static int
get_pty(int *pty, char *from GCC_UNUSED)
{
  int result = 1;

  /* GNU libc 2 allows us to abstract away from having to know the
       master pty device name. */
  if ((*pty = getpt()) >= 0)
  {
	char *name = ptsname(*pty);
	if (name != 0)	/* if filesystem is trashed, this may be null */
	{
	  strcpy(ttydev, name);
      /*printf("get_pty: ttydev >%s<\n",ttydev);sergey*/
	  result = 0;
	}
  }

  TRACE(("get_pty(ttydev=%s, ptydev=%s) %s fd=%d\n",
	   ttydev != 0 ? ttydev : "?",
	   ptydev != 0 ? ptydev : "?",
	   result ? "FAIL" : "OK",
	   pty != 0 ? *pty : -1));

  return(result);
}




static void
get_terminal(void)
/*
 * sets up X and initializes the terminal structure except for term.buf.fildes.
 */
{
  register TScreen *screen = &term->screen;

  screen->arrow = make_colored_cursor(XC_left_ptr,
					screen->mousecolor,
					screen->mousecolorback);
}

/*
 * The only difference in /etc/termcap between 4014 and 4015 is that
 * the latter has support for switching character sets.  We support the
 * 4015 protocol, but ignore the character switches.  Therefore, we
 * choose 4014 over 4015.
 *
 * Features of the 4014 over the 4012: larger (19) screen, 12-bit
 * graphics addressing (compatible with 4012 10-bit addressing),
 * special point plot mode, incremental plot mode (not implemented in
 * later Tektronix terminals), and 4 character sizes.
 * All of these are supported by xterm.
 */


/* The VT102 is a VT100 with the Advanced Video Option included standard.
 * It also adds Escape sequences for insert/delete character/line.
 * The VT220 adds 8-bit character sets, selective erase.
 * The VT320 adds a 25th status line, terminal state interrogation.
 * The VT420 has up to 48 lines on the screen.
 */

static char *vtterm[] =
{
    DFT_TERMTYPE,		/* for people who want special term name */
    "xterm",			/* the prefered name, should be fastest */
    "vt102",
    "vt100",
    "ansi",
    "dumb",
    0
};



/* ARGSUSED */
static SIGNAL_T
hungtty(int i GCC_UNUSED)
{
    siglongjmp(env, 1);
    SIGNAL_RETURN;
}

/*
 * declared outside OPT_PTY_HANDSHAKE so HsSysError() callers can use
 */
static int pc_pipe[2];		/* this pipe is used for parent to child transfer */
static int cp_pipe[2];		/* this pipe is used for child to parent transfer */

/*
 * temporary hack to get xterm working on att ptys
 */
static void
HsSysError(int pf GCC_UNUSED, int error)
{
  fprintf(stderr, "%s: fatal pty error %d (errno=%d) on tty %s\n",
	    xterm_name, error, errno, ttydev);
  exit(error);
}

void
first_map_occurred(void)
{
  return;
}









extern char **environ;

static void
set_owner(char *device, int uid, int gid, int mode)
{
    if (chown(device, uid, gid) < 0) {
	if (errno != ENOENT
	    && getuid() == 0) {
	    fprintf(stderr, "Cannot chown %s to %d,%d: %s\n",
		    device, uid, gid, strerror(errno));
	}
    }
    chmod(device, mode);
}

static int
spawn(void)
/*
 *  Inits pty and tty and forks a login process.
 *  Does not close fd Xsocket.
 *  If slave, the pty named in passedPty is already open for use
 */
{
  register TScreen *screen = &term->screen;
  int initial_erase = VAL_INITIAL_ERASE;
  int rc;
  int tty = -1;
  struct termio tio;
  char termcap[TERMCAP_SIZE];
  char newtc[TERMCAP_SIZE];
  char *ptr, *shname, *shname_minus;
  int i, no_dev_tty = FALSE;
  char **envnew;		/* new environment */
  int envsize;		/* elements in new environment */
  char buf[64];
  char *TermName = NULL;
  TTYSIZE_STRUCT ts;
  struct passwd *pw = NULL;
  char *login_name = NULL;
  struct UTMP_STR utmp;
  struct UTMP_STR *utret;
  struct lastlog lastlog;

  screen->uid = getuid();
  screen->gid = getgid();

  termcap[0] = '\0';
  newtc[0] = '\0';

  /* so that TIOCSWINSZ || TIOCSIZE doesn't block */
  signal(SIGTTOU, SIG_IGN);

  if (am_slave >= 0)
  {
	screen->respond = am_slave;

	set_pty_id(ttydev, passedPty); /* set the tty/pty identifier */

	setgid(screen->gid);
	setuid(screen->uid);
  }
  else
  {
	Bool tty_got_hung;

	/*
	 * Sometimes /dev/tty hangs on open (as in the case of a pty
	 * that has gone away).  Simply make up some reasonable
	 * defaults.
	 */

	signal(SIGALRM, hungtty);
	alarm(2);		/* alarm(1) might return too soon */
	if (!sigsetjmp(env, 1))
    {
	  tty = open("/dev/tty", O_RDWR);
	  alarm(0);
	  tty_got_hung = False;
	}
    else
    {
	  tty_got_hung = True;
	  tty = -1;
      errno = ENXIO;
	}
	initial_erase = VAL_INITIAL_ERASE;
	signal(SIGALRM, SIG_DFL);

	/*
	 * Check results and ignore current control terminal if
	 * necessary.  ENXIO is what is normally returned if there is
	 * no controlling terminal, but some systems (e.g. SunOS 4.0)
	 * seem to return EIO.  Solaris 2.3 is said to return EINVAL.
	 * Cygwin returns ENOENT.
	 */
	no_dev_tty = FALSE;
	if (tty < 0)
    {
	  if (tty_got_hung || errno == ENXIO || errno == EIO || errno == ENODEV ||
		                  errno == EINVAL || errno == ENOTTY || errno == EACCES)
      {
		no_dev_tty = TRUE;
		tio = d_tio;
	  }
      else
      {
		SysError(ERROR_OPDEVTTY);
	  }
	}
    else
    {
	  /* Get a copy of the current terminal's state,
	   * if we can.  Some systems (e.g., SVR4 and MacII)
	   * may not have a controlling terminal at this point
       * if started directly from xdm or xinit,
	   * in which case we just use the defaults as above.
	   */

	  if ((rc = ioctl(tty, TCGETA, &tio)) == -1) tio = d_tio;

	  /*
	   * If ptyInitialErase is set, we want to get the pty's
       * erase value.  Just in case that will fail, first get
       * the value from /dev/tty, so we will have something
       * at least.
       */

	  if (resource.ptyInitialErase)
      {
		initial_erase = tio.c_cc[VERASE];
		TRACE(("%s initial_erase:%d (from /dev/tty)\n",
		       rc == 0 ? "OK" : "FAIL",
		       initial_erase));
	  }

	  close(tty);
	  /* tty is no longer an open fd! */
	  tty = -1;
	}


	if (get_pty(&screen->respond, XDisplayString(screen->display)))
    {
	  SysError(ERROR_PTYS);
	}

	if (resource.ptyInitialErase)
    {
	  struct termio my_tio;
	  if ((rc = ioctl(screen->respond, TCGETA, &my_tio)) == 0) initial_erase = my_tio.c_cc[VERASE];
	    TRACE(("%s initial_erase:%d (from pty)\n",
		   (rc == 0) ? "OK" : "FAIL",
		   initial_erase));
	}
  }

  /* avoid double MapWindow requests */
  XtSetMappedWhenManaged(XtParent(CURRENT_EMU(screen)), False);

  wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW", False);

  if (!TEK4014_ACTIVE(screen)) VTInit(); /* realize now so know window size for tty driver */

  if (Console)
  {
	/*
	 * Inform any running xconsole program
	 * that we are going to steal the console.
	 */
	XmuGetHostname(mit_console_name + MIT_CONSOLE_LEN, 255);
	mit_console = XInternAtom(screen->display, mit_console_name, False);
	/* the user told us to be the console, so we can use CurrentTime */
	XtOwnSelection(XtParent(CURRENT_EMU(screen)),
		       mit_console, CurrentTime,
		       ConvertConsoleSelection, NULL, NULL);
  }

  {
	envnew = vtterm;
	ptr = termcap;
  }

  /*
   * This used to exit if no termcap entry was found for the specified
   * terminal name.  That's a little unfriendly, so instead we'll allow
   * the program to proceed (but not to set $TERMCAP) if the termcap
   * entry is not found.
   */
  if (!get_termcap(TermName = resource.term_name, ptr, newtc))
  {
	char *last = NULL;
	TermName = *envnew;
	while (*envnew != NULL)
    {
	  if ((last == NULL || strcmp(last, *envnew))
		  && get_termcap(*envnew, ptr, newtc))
      {
		TermName = *envnew;
		break;
	  }
	  last = *envnew;
	  envnew++;
	}
  }

  /*
   * Check if ptyInitialErase is not set.  If so, we rely on the termcap
   * (or terminfo) to tell us what the erase mode should be set to.
   */
  TRACE(("resource ptyInitialErase is %sset\n", resource.ptyInitialErase ? "" : "not "));
  if (!resource.ptyInitialErase)
  {
	char temp[1024], *p = temp;
	char *s = tgetstr(TERMCAP_ERASE, &p);
	TRACE(("...extracting initial_erase value from termcap\n"));
	if (s != 0)
    {
	    initial_erase = decode_keyvalue(&s, True);
	}
  }
  TRACE(("...initial_erase:%d\n", initial_erase));

  TRACE(("resource backarrowKeyIsErase is %sset\n", resource.backarrow_is_erase ? "" : "not "));
  if (resource.backarrow_is_erase)	/* see input.c */
  {
	if (initial_erase == 127)
    {
	  term->keyboard.flags &= ~MODE_DECBKM;
	}
    else
    {
	  term->keyboard.flags |= MODE_DECBKM;
	  term->keyboard.reset_DECBKM = 1;
	}
	TRACE(("...sets DECBKM %s\n", (term->keyboard.flags & MODE_DECBKM) ? "on" : "off"));
  }
  else
  {
	term->keyboard.reset_DECBKM = 2;
  }

  /* tell tty how big window is */
  {
	TTYSIZE_ROWS(ts) = screen->max_row + 1;
	TTYSIZE_COLS(ts) = screen->max_col + 1;
	ts.ws_xpixel = FullWidth(screen);
	ts.ws_ypixel = FullHeight(screen);
  }

  i = SET_TTYSIZE(screen->respond, ts);
  TRACE(("spawn SET_TTYSIZE %dx%d return %d\n", TTYSIZE_ROWS(ts), TTYSIZE_COLS(ts), i));


  if (am_slave < 0)
  {
	TRACE(("Forking...\n"));

	if ((screen->pid = fork()) == -1) SysError(ERROR_FORK);


    /********************************/
	/* begin if in child after fork */

	if (screen->pid == 0)
    {
	  /*
	   * now in child process
       */
	  TRACE_CHILD
	  int pgrp = setsid();	/* variable may not be used... */


	  int ptyfd = 0;
	  char *pty_name = 0;

	  setpgrp();
	  grantpt(screen->respond);
	  unlockpt(screen->respond);
	  if ((pty_name = ptsname(screen->respond)) == 0)
      {
		SysError(ERROR_PTSNAME);
	  }
	  if ((ptyfd = open(pty_name, O_RDWR)) < 0)
      {
		SysError(ERROR_OPPTSNAME);
	  }

	  tty = ptyfd;
	  close(screen->respond);

	  /* tell tty how big window is */
	  {
		TTYSIZE_ROWS(ts) = screen->max_row + 1;
		TTYSIZE_COLS(ts) = screen->max_col + 1;
		ts.ws_xpixel = FullWidth(screen);
		ts.ws_ypixel = FullHeight(screen);
	  }

	  {
		struct group *ttygrp;
		if ((ttygrp = getgrnam(TTY_GROUP_NAME)) != 0)
        {
		  /* change ownership of tty to real uid, "tty" gid */
		  set_owner(ttydev, screen->uid, ttygrp->gr_gid,
			      (resource.messages ? 0620 : 0600));
		}
        else
        {
		  /* change ownership of tty to real group and user id */
		  set_owner(ttydev, screen->uid, screen->gid,
			      (resource.messages ? 0622 : 0600));
		}
		endgrent();
	  }

	  /*
	   * set up the tty modes
	   */
	  {


		/* If the control tty had its modes screwed around with,
		   eg. by lineedit in the shell, or emacs, etc. then tio
		   will have bad values.  Let's just get termio from the
		   new tty and tailor it.  */
		if (ioctl(tty, TCGETA, &tio) == -1) SysError(ERROR_TIOCGETP);
		tio.c_lflag |= ECHOE;

		/* Now is also the time to change the modes of the
		 * child pty.
		 */
		/* input: nl->nl, don't ignore cr, cr->nl */
		tio.c_iflag &= ~(INLCR | IGNCR);
		tio.c_iflag |= ICRNL;
		/* ouput: cr->cr, nl is not return, no delays, ln->cr/nl */

		tio.c_oflag &=
		    ~(OCRNL
		      | ONLRET
		      | NLDLY
		      | CRDLY
		      | TABDLY
		      | BSDLY
		      | VTDLY
		      | FFDLY);

		tio.c_oflag |= ONLCR;
		tio.c_oflag |= OPOST;

		tio.c_cflag &= ~(CBAUD);
		tio.c_cflag |= VAL_LINE_SPEED;

		tio.c_cflag &= ~CSIZE;
		if (screen->input_eight_bits) tio.c_cflag |= CS8;
		else                          tio.c_cflag |= CS7;

		/* enable signals, canonical processing (erase, kill, etc),
		 * echo
		 */
		tio.c_lflag |= ISIG | ICANON | ECHO | ECHOE | ECHOK;
		tio.c_lflag |= ECHOKE | IEXTEN;
		tio.c_lflag |= ECHOCTL | IEXTEN;

		/* reset EOL to default value */
		tio.c_cc[VEOL] = CEOL;	/* '^@' */
		/* certain shells (ksh & csh) change EOF as well */
		tio.c_cc[VEOF] = CEOF;	/* '^D' */

		tio.c_cc[VLNEXT] = CLNEXT;
		tio.c_cc[VWERASE] = CWERASE;
		tio.c_cc[VREPRINT] = CRPRNT;
		tio.c_cc[VDISCARD] = CFLUSH;
		tio.c_cc[VSTOP] = CSTOP;
		tio.c_cc[VSTART] = CSTART;
		tio.c_cc[VSUSP] = CSUSP;
		if (override_tty_modes)
        {
		  /* sysv-specific */
		  TMODE(XTTYMODE_intr, tio.c_cc[VINTR]);
		  TMODE(XTTYMODE_quit, tio.c_cc[VQUIT]);
		  TMODE(XTTYMODE_erase, tio.c_cc[VERASE]);
		  TMODE(XTTYMODE_kill, tio.c_cc[VKILL]);
		  TMODE(XTTYMODE_eof, tio.c_cc[VEOF]);
		  TMODE(XTTYMODE_eol, tio.c_cc[VEOL]);
		  TMODE(XTTYMODE_susp, tio.c_cc[VSUSP]);
		  TMODE(XTTYMODE_rprnt, tio.c_cc[VREPRINT]);
		  TMODE(XTTYMODE_flush, tio.c_cc[VDISCARD]);
		  TMODE(XTTYMODE_weras, tio.c_cc[VWERASE]);
		  TMODE(XTTYMODE_lnext, tio.c_cc[VLNEXT]);
		  TMODE(XTTYMODE_start, tio.c_cc[VSTART]);
		  TMODE(XTTYMODE_stop, tio.c_cc[VSTOP]);
		}

		if (ioctl(tty, TCSETA, &tio) == -1) HsSysError(cp_pipe[1], ERROR_TIOCSETP);

		if (Console)
        {
		  int on = 1;
		  if (ioctl(tty, TIOCCONS, (char *) &on) == -1)
			fprintf(stderr, "%s: cannot open console: %s\n",
				xterm_name, strerror(errno));
		}
	  }

	  signal(SIGCHLD, SIG_DFL);
	  signal(SIGHUP, SIG_IGN);

	  /* restore various signals to their defaults */
	  signal(SIGINT, SIG_DFL);
	  signal(SIGQUIT, SIG_DFL);
	  signal(SIGTERM, SIG_DFL);

	  /*
	   * If we're not asked to make the parent process set the
	   * terminal's erase mode, and if we had no ttyModes resource,
       * then set the terminal's erase mode from our best guess.
       */
	  TRACE(("check if we should set erase to %d:%s\n\tptyInitialErase:%d,\n\toveride_tty_modes:%d,\n\tXTTYMODE_erase:%d\n",
		   initial_erase,
		   (!resource.ptyInitialErase
		    && !override_tty_modes
		    && !ttymodelist[XTTYMODE_erase].set)
		   ? "YES" : "NO",
		   resource.ptyInitialErase,
		   override_tty_modes,
		   ttymodelist[XTTYMODE_erase].set));

	  if (!resource.ptyInitialErase
		&& !override_tty_modes
		&& !ttymodelist[XTTYMODE_erase].set)
      {
		int old_erase;

		if (ioctl(tty, TCGETA, &tio) == -1) tio = d_tio;
		old_erase = tio.c_cc[VERASE];
		tio.c_cc[VERASE] = initial_erase;
		rc = ioctl(tty, TCSETA, &tio);

		TRACE(("%s setting erase to %d (was %d)\n",
		       rc ? "FAIL" : "OK", initial_erase, old_erase));
	  }


	  /* copy the environment before Setenving */
	  for (i = 0; environ[i] != NULL; i++) ;

	  /* compute number of xtermSetenv() calls below */
	  envsize = 1;	/* (NULL terminating entry) */
	  envsize += 3;	/* TERM, WINDOWID, DISPLAY */
	  envsize += 1;	/* LOGNAME */
	  envsize += 1;	/* TERMCAP */

	  envnew = (char **) calloc((unsigned) i + envsize, sizeof(char *));
	  memmove((char *) envnew, (char *) environ, i * sizeof(char *));
	  environ = envnew;
	  xtermSetenv("TERM=", TermName);
	  if (!TermName) *newtc = 0;

	  sprintf(buf, "%lu", ((unsigned long) XtWindow(XtParent(CURRENT_EMU(screen)))));
	  xtermSetenv("WINDOWID=", buf);

	  /* put the display into the environment of the shell */
	  xtermSetenv("DISPLAY=", XDisplayString(screen->display));

	  signal(SIGTERM, SIG_DFL);


	  /*sergey: switching std's !? */
	  /* this is the time to go and set up stdin, out, and err */
	  {

		/*printf here goes to old xterm*/

		/* dup the tty */
		for (i = 0; i <= 2; i++)
		{
		  if (i != tty)
          {
			(void) close(i);
			(void) dup(tty);
		  }
		}

		/*printf here goes to new xterm*/

		/* and close the tty */
		if (tty > 2) (void) close(tty);
	  }
	  /*sergey*/


	  pw = getpwuid(screen->uid);
	  login_name = NULL;
	  if (pw && pw->pw_name)
      {
		/*
		 * If the value from getlogin() differs from the value we
		 * get by looking in the password file, check if it does
		 * correspond to the same uid.  If so, allow that as an
		 * alias for the uid.
		 *
		 * Of course getlogin() will fail if we're started from
		 * a window-manager, since there's no controlling terminal
		 * to fuss with.  In that case, try to get something useful
		 * from the user's $LOGNAME or $USER environment variables.
		 */
		if (((login_name = getlogin()) != NULL
		     || (login_name = getenv("LOGNAME")) != NULL
		     || (login_name = getenv("USER")) != NULL)
		    && strcmp(login_name, pw->pw_name))
        {
		  struct passwd *pw2 = getpwnam(login_name);
		  if (pw2 != 0 && pw->pw_uid != pw2->pw_uid)
          {
			login_name = NULL;
		  }
		}

		if (login_name == NULL) login_name = pw->pw_name;
		if (login_name != NULL) login_name = x_strdup(login_name);
	  }
	  if (login_name != NULL)
      {
		xtermSetenv("LOGNAME=", login_name);	/* for POSIX */
	  }

	  /* Set up our utmp entry now.  We need to do it here
	   * for the following reasons:
       *   - It needs to have our correct process id (for
       *     login).
	   *   - If our parent was to set it after the fork(),
	   *     it might make it out before we need it.
       *   - We need to do it before we go and change our
       *     user and group id's.
       */
	  (void) setutent();
	  /* set up entry to search for */
	  bzero((char *) &utmp, sizeof(utmp));
	  (void) strncpy(utmp.ut_id, my_utmp_id(ttydev), sizeof(utmp.ut_id));

	  utmp.ut_type = DEAD_PROCESS;

	  /* position to entry in utmp file */
	  /* Test return value: beware of entries left behind: PSz 9 Mar 00 */
	  if (!(utret = getutid(&utmp)))
      {
		(void) setutent();
		utmp.ut_type = USER_PROCESS;
		if (!(utret = getutid(&utmp)))
        {
		  (void) setutent();
		}
	  }

	  /* set up the new entry */
	  utmp.ut_type = USER_PROCESS;
      utmp.ut_xstatus = 2;
      (void) strncpy(utmp.ut_user,
			   (login_name != NULL) ? login_name : "????",
			   sizeof(utmp.ut_user));
	  /* why are we copying this string again?  (see above) */
	  (void) strncpy(utmp.ut_id, my_utmp_id(ttydev), sizeof(utmp.ut_id));
      (void) strncpy(utmp.ut_line,
			   my_pty_name(ttydev), sizeof(utmp.ut_line));

	  (void) strncpy(buf, DisplayString(screen->display), sizeof(buf));
	  (void) strncpy(utmp.ut_host, buf, sizeof(utmp.ut_host));
      (void) strncpy(utmp.ut_name,
			   (login_name) ? login_name : "????",
			   sizeof(utmp.ut_name));

	  utmp.ut_pid = getpid();
	  utmp.ut_session = getsid(0);
      utmp.ut_xtime = time((time_t *) 0);
      utmp.ut_tv.tv_usec = 0;

	  /* write out the entry */
	  if (!resource.utmpInhibit)
      {
		errno = 0;
		rc = (pututline(&utmp) == 0);
		TRACE(("pututline: %d %d %s\n",
		       resource.utmpInhibit,
		       errno, (rc != 0) ? strerror(errno) : ""));
	  }

	  if (term->misc.login_shell) updwtmp(etc_wtmp, &utmp);

	  /* close the file */
	  (void) endutent();

	  if (term->misc.login_shell && (i = open(etc_lastlog, O_WRONLY)) >= 0)
      {
		bzero((char *) &lastlog, sizeof(struct lastlog));
		(void) strncpy(lastlog.ll_line,
			       my_pty_name(ttydev),
			       sizeof(lastlog.ll_line));
		(void) strncpy(lastlog.ll_host,
			       XDisplayString(screen->display),
			       sizeof(lastlog.ll_host));
		time(&lastlog.ll_time);
		lseek(i, (long) (screen->uid * sizeof(struct lastlog)), 0);
		write(i, (char *) &lastlog, sizeof(struct lastlog));
		close(i);
	  }

	  (void) setgid(screen->gid);
	  if (setuid(screen->uid))
      {
		SysError(ERROR_SETUID);
	  }

	  if (!TEK4014_ACTIVE(screen) && *newtc)
      {
		strcpy(termcap, newtc);
		resize(screen, termcap, newtc);
	  }
	  if (term->misc.titeInhibit && !term->misc.tiXtraScroll)
      {
		remove_termcap_entry(newtc, "ti=");
		remove_termcap_entry(newtc, "te=");
	  }

	  /* work around broken termcap entries */
	  if (resource.useInsertMode)
      {
		remove_termcap_entry(newtc, "ic=");
		/* don't get duplicates */
		remove_termcap_entry(newtc, "im=");
		remove_termcap_entry(newtc, "ei=");
		remove_termcap_entry(newtc, "mi");
		if (*newtc) strcat(newtc, ":im=\\E[4h:ei=\\E[4l:mi:");
	  }
	  if (*newtc)
      {
		unsigned len;
		remove_termcap_entry(newtc, TERMCAP_ERASE "=");
		len = strlen(newtc);
		if (len != 0 && newtc[len - 1] == ':') len--;
		sprintf(newtc + len, ":%s=\\%03o:",
			TERMCAP_ERASE,
			initial_erase & 0377);
		xtermSetenv("TERMCAP=", newtc);
	  }

	  /* need to reset after all the ioctl bashing we did above */
	  signal(SIGHUP, SIG_DFL);

	  if(((ptr = getenv("SHELL")) == NULL || *ptr == 0) &&
		 ((pw == NULL && (pw = getpwuid(screen->uid)) == NULL) ||
		  *(ptr = pw->pw_shell) == 0))
		
		ptr = "/bin/sh";

	  shname = x_basename(ptr);

      /* if command need to be executed, start shell and command here */
	  /* examples: codaterm -e "ssh adcecal5; tcsh"
                   codaterm -e 'ssh adcecal5 && tcsh'
      */
	  if (command_to_exec)
      {
		TRACE(("spawning command \"%s\"\n", *command_to_exec));
	    /*printf("spawning command \"%s\", command_to_exec=0x%08x\n", *command_to_exec, command_to_exec);*/

        printf("1 shname >%s<\n",shname); /* tcsh */

		printf("1 execvp command >%s<\n",*command_to_exec);
	    execvp(*command_to_exec, command_to_exec);
	    printf("1 execvp exit, command_to_exec[1]=%d\n",command_to_exec[1]);

		if (command_to_exec[1] == 0)
	    {
	      printf("1 execlp runs command '%s -c %s'\n",shname,command_to_exec[0]);
		  execlp(ptr, shname, "-c", command_to_exec[0], 0);
	      printf("1 execlp exit\n");
	    }

		printf("1 exit\n");
        sleep(3);

		/* print error message on screen */
	    fprintf(stderr, "%s: Can't execvp %s: %s\n",
			xterm_name, *command_to_exec, strerror(errno));
	  }

	  shname_minus = (char *) malloc(strlen(shname) + 2);
	  (void) strcpy(shname_minus, "-");
	  (void) strcat(shname_minus, shname);

      /*printf("2 shname >%s<\n",shname);*/ /* tcsh */
      /*printf("2 shname_minus >%s<\n",shname_minus);*/ /* -tcsh */

      /*printf("2 execlp\n");*/

	  /* sergey: start actual shell; will stuck there until 'shname' exits */
	  execlp(ptr, (term->misc.login_shell ? shname_minus : shname), (void *) 0);
      /*printf("2 execlp exit\n");*/


	  /* Exec failed. */
	  fprintf(stderr, "%s: Could not exec %s: %s\n", xterm_name,
		    ptr, strerror(errno));
	  (void) sleep(5);

	  exit(ERROR_EXEC);
	}
	/* end if in child after fork */
    /******************************/


  }
  /* end if no slave */

  /*
   * still in parent (xterm process)
   */
  signal(SIGHUP, SIG_IGN);

  /*
   * Unfortunately, System V seems to have trouble divorcing the child process
   * from the process group of xterm.  This is a problem because hitting the
   * INTR or QUIT characters on the keyboard will cause xterm to go away if we
   * don't ignore the signals.  This is annoying.
   */
  signal(SIGINT, Exit);
  signal(SIGQUIT, Exit);
  signal(SIGTERM, Exit);
  signal(SIGPIPE, Exit);


  return(0);
}				/* end spawn */




SIGNAL_T
Exit(int n)
{
  register TScreen *screen = &term->screen;

  struct UTMP_STR utmp;
  struct UTMP_STR *utptr;

  /* don't do this more than once */
  if (xterm_exiting) SIGNAL_RETURN;
  xterm_exiting = True;

  /* cleanup the utmp entry we forged earlier */
  if (!resource.utmpInhibit)
  {
	utmp.ut_type = USER_PROCESS;
	(void) strncpy(utmp.ut_id, my_utmp_id(ttydev), sizeof(utmp.ut_id));
	(void) setutent();
	utptr = getutid(&utmp);
	/* write it out only if it exists, and the pid's match */
	if (utptr && (utptr->ut_pid == screen->pid))
    {
	  utptr->ut_type = DEAD_PROCESS;
	  utptr->ut_session = getsid(0);
	  utptr->ut_xtime = time((time_t *) 0);
	  utptr->ut_tv.tv_usec = 0;
	  (void) pututline(utptr);

	  strncpy(utmp.ut_line, utptr->ut_line, sizeof(utmp.ut_line));
	  if (term->misc.login_shell) updwtmp(etc_wtmp, utptr);
	}
	(void) endutent();
  }

  close(screen->respond);	/* close explicitly to avoid race with slave side */
#ifdef ALLOWLOGGING
  if (screen->logging) CloseLog(screen);
#endif

  if (am_slave < 0)
  {
	/* restore ownership of tty and pty */
	set_owner(ttydev, 0, 0, 0666);
  }
  exit(n);
  SIGNAL_RETURN;
}


/* ARGSUSED */
static void
resize(TScreen * screen, register char *oldtc, char *newtc)
{
  register char *ptr1, *ptr2;
  register size_t i;
  register int li_first = 0;
  register char *temp;

  TRACE(("resize %s\n", oldtc));
  if ((ptr1 = x_strindex(oldtc, "co#")) == NULL)
  {
	strcat(oldtc, "co#80:");
	ptr1 = x_strindex(oldtc, "co#");
  }
  if ((ptr2 = x_strindex(oldtc, "li#")) == NULL)
  {
	strcat(oldtc, "li#24:");
	ptr2 = x_strindex(oldtc, "li#");
  }
  if (ptr1 > ptr2)
  {
	li_first++;
	temp = ptr1;
	ptr1 = ptr2;
	ptr2 = temp;
  }
  ptr1 += 3;
  ptr2 += 3;
  strncpy(newtc, oldtc, i = ptr1 - oldtc);
  temp = newtc + i;
  sprintf(temp, "%d", (li_first
			 ? screen->max_row + 1
			 : screen->max_col + 1));
  temp += strlen(temp);
  ptr1 = strchr(ptr1, ':');
  strncpy(temp, ptr1, i = ptr2 - ptr1);
  temp += i;
  sprintf(temp, "%d", (li_first
			 ? screen->max_col + 1
			 : screen->max_row + 1));
  ptr2 = strchr(ptr2, ':');
  strcat(temp, ptr2);
  TRACE(("   ==> %s\n", newtc));
}


/*
 * Does a non-blocking wait for a child process.  If the system
 * doesn't support non-blocking wait, do nothing.
 * Returns the pid of the child, or 0 or -1 if none or error.
 */
int
nonblocking_wait(void)
{
  pid_t pid;

  pid = waitpid(-1, NULL, WNOHANG);

  return pid;
}


/* ARGSUSED */
static SIGNAL_T
reapchild(int n GCC_UNUSED)
{
  int olderrno = errno;
  int pid;

  pid = wait(NULL);

  /* cannot re-enable signal before waiting for child
   * because then SVR4 loops.  Sigh.  HP-UX 9.01 too.
   */
  (void) signal(SIGCHLD, reapchild);

  do
  {
	if (pid == term->screen.pid)
    {
#ifdef DEBUG
	  if (debug) fputs("Exiting\n", stderr);
#endif
	  if (!hold_screen) Cleanup(0);
	}
  } while ((pid = nonblocking_wait()) > 0);

  errno = olderrno;
  SIGNAL_RETURN;
}


static void
remove_termcap_entry(char *buf, char *str)
{
  char *base = buf;
  char *first = base;
  int count = 0;
  size_t len = strlen(str);

  TRACE(("*** remove_termcap_entry('%s', '%s')\n", str, buf));

  while (*buf != 0)
  {
	if (!count && !strncmp(buf, str, len))
    {
	  while (*buf != 0)
      {
		if (*buf == '\\') buf++;
		else if (*buf == ':') break;

		if (*buf != 0) buf++;
	  }
	  while ((*first++ = *buf++) != 0) ;
	  TRACE(("...removed_termcap_entry('%s', '%s')\n", str, base));
	  return;
	}
    else if (*buf == '\\')
    {
	  buf++;
	}
    else if (*buf == ':')
    {
	  first = buf;
	  count = 0;
	}
    else if (!isspace(CharOf(*buf)))
    {
	  count++;
	}
	if (*buf != 0) buf++;
  }
  TRACE(("...cannot remove\n"));
}

/*
 * parse_tty_modes accepts lines of the following form:
 *
 *         [SETTING] ...
 *
 * where setting consists of the words in the modelist followed by a character
 * or ^char.
 */
static int
parse_tty_modes(char *s, struct _xttymodes *modelist)
{
  struct _xttymodes *mp;
  int c;
  int count = 0;

  TRACE(("parse_tty_modes\n"));
  while (1)
  {
	while (*s && isascii(CharOf(*s)) && isspace(CharOf(*s))) s++;
	if (!*s) return count;

	for (mp = modelist; mp->name; mp++)
    {
	  if (strncmp(s, mp->name, mp->len) == 0) break;
	}
	if (!mp->name) return -1;

	s += mp->len;
	while (*s && isascii(CharOf(*s)) && isspace(CharOf(*s))) s++;
	if (!*s) return -1;

	if ((c = decode_keyvalue(&s, False)) != -1)
    {
	  mp->value = c;
	  mp->set = 1;
      count++;
	  TRACE(("...parsed #%d: %s=%#x\n", count, mp->name, c));
	}
  }
}


int
GetBytesAvailable(int fd)
{
  long arg;
  ioctl(fd, FIONREAD, (char *) &arg);
  return (int) arg;
}


/* Utility function to try to hide system differences from
   everybody who used to call killpg() */

int
kill_process_group(int pid, int sig)
{
  TRACE(("kill_process_group(pid=%d, sig=%d)\n", pid, sig));
  return kill(-pid, sig);
}

