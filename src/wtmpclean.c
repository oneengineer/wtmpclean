/*
 * wtmpclean.c -- A tool for dumping wtmp files and patching wtmp records.
 * Copyright (C) 2008,2009,2013 by Davide Madrisan <davide.madrisan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* http://www.gnu.org/s/libc/manual/html_node/XPG-Functions.html#XPG-Functions 
 */

/*
 * indent -i4 -nut src/wtmpclean.c
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include <arpa/inet.h>          /* inet_ntoa */
#include <errno.h>
#include <locale.h>             /* setlocale */
#include <pwd.h>                /* getpwnam */
#include <regex.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

/* FIXME */
/*#if !defined HAVE_UTMPXPOSIX
 *# include "wrappers.h"
 *#endif
 */

#ifdef HAVE_UTMPX_H
# include <utmpx.h>
#else
# ifdef HAVE_UTMP_H
#  include <utmp.h>
# endif
#endif

#include "wtmpclean.h"
#include "getopt.h"

static char *progname;


struct utp_s{
	int line_num;
	struct utmpx* utp;
};

static void die (const char *fmt, ...);
static void dumprecord (struct utmpxlist *p, int what);
static void userchk (const char *usr);

void
usage (int status)
{
    static const char *usagemsg[] = {
        PACKAGE " version " PACKAGE_VERSION,
        "A tool for dumping wtmp files and patching wtmp records.",
        "Copyright (C) 2008,2009,2013 by Davide Madrisan <davide.madrisan@gmail.com>",
        "",
        "Usage: " PACKAGE " [-l|-r] [-t \"YYYY.MM.DD HH:MM:SS\"]"
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
            " [-f <wtmpfile>]"
#endif
            " <user> [<fake>]",
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
        "  -f, --file       Modify <wtmpfile> instead of " DEFAULT_WTMP,
#endif
        "  -l, --list       Show listing of <user> logins",
        "  -r, --raw        Show the raw content of the wtmp database",
        "  -t, --time       Delete the login at the specified time",
        "",
        "Samples:",
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
        "  ./" PACKAGE " --raw -f " DEFAULT_WTMP ".1 root",
        "  ./" PACKAGE " -t \"2008.09.06 14:30:00\" jekyll hide",
        "  ./" PACKAGE " -t \"2013\\.12\\.?? 23:.*\" hide",
        "  ./" PACKAGE " -f " DEFAULT_WTMP ".1 jekyll",
#else
        "  ./" PACKAGE " root",
#endif
        "",
        "This is free software; see the source for copying conditions.  There is NO",
        "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.",
        (char *) 0
    };

    unsigned int i;

    for (i = 0; usagemsg[i]; i++)
        fprintf (status ? stderr : stdout, "%s\n", usagemsg[i]);

    exit (status);
}

static void
die (const char *fmt, ...)
{
    va_list args;

    va_start (args, fmt);
    fprintf (stderr, "%s: ", basename (progname));
    vfprintf (stderr, fmt, args);
    va_end (args);

    exit (EXIT_FAILURE);
}

static void
userchk (const char *usr)
{
    struct passwd *pw;

    if (usr && !(pw = getpwnam (usr)))
      {
          fprintf (stderr, "%s: unknown/bad user `%s'\n", progname, usr);
          exit (EXIT_FAILURE);
      }
}

char *
timetostr (const time_t time)
{
    static char s[20];          /* [2008.09.06 14:30:00] */

    if (time != 0)
        strftime (s, 20, "%Y.%m.%d %H:%M:%S", localtime (&time));
    else
        s[0] = '\0';

    return s;
}


struct utmpxlist * wtmpxdump2 (const char *wtmpfile, const char *user)
{
    struct utmpxlist *p, *curr = NULL, *next, *utmpxlist = NULL;
    struct utmpx *utp;
    char runlevel;
    int down = 0;

    if (access (wtmpfile, R_OK))
        die ("cannot access the file: %s\n", strerror (errno));

    UTMPXNAME (wtmpfile);
    setutxent ();

    while ((utp = getutxent ()))
      {

          switch (utp->ut_type)
            {
            default:
                break;
            case RUN_LVL:
                runlevel = utp->ut_pid % 256;
                if ((runlevel == '0') || (runlevel == '6'))
                    down = 1;
                break;
            case BOOT_TIME:
                down = 0;
                break;
            case USER_PROCESS:
                /*
                 * Just store the data if it is interesting enough.
                 */
                     if ((p = malloc (sizeof (struct utmpxlist))) == NULL)
                          die ("out of memory: %s\n", strerror (errno));

                      memcpy (&p->ut, utp, sizeof (struct utmpx));
                      p->delta = 0;
                      p->ltype = R_NONE;
                      p->next = NULL;
                      if (utmpxlist == NULL)
                        {
                            utmpxlist = curr = p;
                            p->prev = NULL;
                        }
                      else
                        {
                            curr->next = p;
                            p->prev = curr;
                            curr = p;
                        }
                break;
            case DEAD_PROCESS:
                for (p = curr; p; p = p->prev)
                  {
                      if (p->ut.ut_type != USER_PROCESS || p->ltype != R_NONE)
                          continue;
                      if (strncmp
                          (p->ut.ut_line, utp->ut_line, sizeof utp->ut_line))
                          continue;

                      p->eos = utp->ut_tv.tv_sec;
                      p->delta = utp->ut_tv.tv_sec - p->ut.ut_tv.tv_sec;
                      p->ltype = (down ? R_DOWN : R_NORMAL);
                  }
                break;
            }
      }

    endutxent ();
    return utmpxlist;
}

unsigned int
wtmpedit (const char *wtmpfile, const char *user, const char *fake,
          const char *timepattern, unsigned int *cleanerr)
{
    static struct utmpx *utp;
    unsigned int cleanrec;
    int rc;
    struct stat sb;
    struct utimbuf currtime;
    uid_t owner;
    gid_t group;
    regex_t regex;
    char msgbuf[100];

    if (access (wtmpfile, W_OK))
        die ("cannot access the file: %s\n", strerror (errno));

    if (stat (wtmpfile, &sb))
        die ("cannot get file status: %s\n", strerror (errno));

    if (rc = regcomp (&regex, timepattern, REG_EXTENDED | REG_NOSUB))
      {
          regerror (rc, &regex, msgbuf, sizeof (msgbuf));
          die ("regcomp() failed: %s\n", msgbuf);
      }

    currtime.actime = sb.st_atime;
    currtime.modtime = sb.st_mtime;
    owner = sb.st_uid;
    group = sb.st_gid;

    UTMPXNAME (wtmpfile);
    setutxent ();

    cleanrec = *cleanerr = 0;
    while ((utp = getutxent ()))
      {
          if (utp->ut_type == USER_PROCESS &&
              strncmp (utp->ut_user, user, sizeof utp->ut_user) == 0)
            {
                if (regexec
                    (&regex, timetostr (utp->ut_tv.tv_sec), (size_t) 0, NULL,
                     0))
                    continue;

                if (fake)
                    strncpy (utp->ut_user, fake, sizeof utp->ut_user);
                else
                  {
                      /* Simulates the job of init when a process has exited:
                       * leave ut_pid untouched, sets ut_type to DEAD_PROCESS
                       * and fills ut_user, ut_host with null bytes
                       * ex:
                       * root [11735] [pts/0] [ts/0] [10.0.0.1] [10.0.0.1] [Mon Jan 12 17:31:24 2009 CET]
                       * DEAD [11735] [pts/0] [    ] [        ] [0.0.0.0 ] [Mon Jan 12 17:31:24 2009 CET]
                       */
                      utp->ut_type = DEAD_PROCESS;
                      memset (utp->ut_user, 0, sizeof utp->ut_user);
                      memset (utp->ut_id, 0, sizeof utp->ut_id);
                      memset (utp->ut_host, 0, sizeof utp->ut_host);
                      memset (utp->ut_addr_v6, 0, sizeof utp->ut_addr_v6);
                      /*utp->ut_tv.tv_sec = utp->ut_tv.tv_usec = 0; */
                  }

                if (pututxline (utp))
                    cleanrec++;
                else
                    cleanerr++;
            }
      }

    endutxent ();
    regfree (&regex);

    if (chown (wtmpfile, owner, group) < 0)
        fprintf (stderr, "cannot preserve the ownership of the wtmp file");
    if (utime (wtmpfile, &currtime) < 0)
        fprintf (stderr, "cannot preserve access and modification times");

    return cleanrec;
}

unsigned int rand_pid() {
	return (rand() % 25051) + 2000 ;
}

unsigned int rand_usec(){
	return (rand() % 667585) + 100899;
}

void memcpy_char(char* dst, char * src){
	int n = strlen(src) + 1;
	memcpy( dst,src,sizeof(char) * n );
}

struct utp_s* get_all_entries(const char *wtmpfile,int * counter){
	struct utmpx *utp;
	*counter = 0;
	UTMPXNAME (wtmpfile);
	setutxent ();
	while ((utp = getutxent ())){
		(*counter) = (*counter) +1;
	}
	endutxent ();
	//read again
	struct utp_s * list = malloc( (*counter+10) * sizeof ( struct utp_s )  );
	UTMPXNAME (wtmpfile);
	setutxent ();
	int i=0;
	while ((utp = getutxent ())){
		list[i].line_num = i;
		list[i].utp = malloc( sizeof (struct utmpx) );
		memcpy(list[i].utp,utp,sizeof(struct utmpx));
		//deep copy
		//memcpy_char( list[i].utp->ut_line,utp->ut_line,UT_LINESIZE);
		memcpy_char( list[i].utp->ut_user,utp->ut_user);
		//memcpy_char( list[i].utp->ut_host,utp->ut_host,UT_HOSTSIZE);
		i++;
	}
	

	endutxent ();
	printf("length: %d\n",*counter);
	return list;
}

int cmp(const struct utp_s* a, const struct utp_s* b ) {
	if ( a->utp->ut_tv.tv_sec == b->utp->ut_tv.tv_sec )
		return a->line_num - b->line_num;
	else return a->utp->ut_tv.tv_sec - b->utp->ut_tv.tv_sec;
}

void write_back(struct utp_s* list,int n){
	qsort(list,n,sizeof (struct utp_s) ,cmp);
	static struct utmpx *utp;
	remove("./generated_wtmp");
	FILE *f = fopen("./generated_wtmp","w");
	fclose(f);
	UTMPXNAME ("./generated_wtmp");
	//UTMPXNAME ("./wtmp");
	setutxent();
	for (int i=0;i<n;i++){
		//printf("write %d\n",i);
		//printf("%d %s\n",list[i].line_num,list[i].utp -> ut_user);
		//updwtmpx("./generated_wtmp",list[i].utp);
        	pututxline ( list[i].utp );
	}
	endutxent();
}

void edit_entry(
		const char *wtmpfile,int pid,unsigned int original_time,unsigned int new_time,
		const char *host_name,const int ip,const int edit_dead){
	
	//find template
	int n;
	struct utp_s* list = get_all_entries(wtmpfile,&n);
	struct utmpx* target = NULL,*temp;

	for (int i=0;i<n;i++){
		temp = list[i].utp;
		int type_right = 0;
		if ( edit_dead && DEAD_PROCESS == temp->ut_type ){
			type_right = 1;
		}
		else if (!edit_dead) {
			type_right = 1;
		}

		if (  pid == temp->ut_pid && original_time == temp->ut_tv.tv_sec && type_right ){
			target = list[i].utp;
			break;
		}
	}

	if ( NULL == target){
		printf("Cannot find target entry.\n");
		exit (EXIT_FAILURE);
	}
	
	if (host_name != NULL){
		int len = strlen(host_name)+1;
		memcpy(target->ut_host,host_name,len * sizeof (char));
	}
	if (ip!=0 && DEAD_PROCESS != temp->ut_type){
		target->ut_addr_v6[0] = ip;
	}

	if (new_time !=0){
		target->ut_tv.tv_sec = new_time;
	}

	write_back(list,n);
}

void add_entry(
		const char *wtmpfile,const char *ut_line,unsigned int start_time,unsigned int end_time,
		const char *user_name,const char *host_name,const int ip){
	unsigned int start_time_u = rand_usec();
	unsigned int end_time_u = rand_usec();
	unsigned int pid = rand_pid();
	
	//find template
	int n;
	struct utp_s* list = get_all_entries(wtmpfile,&n);
	struct utmpx* template;

	for (int i=0;i<n;i++){
		if (  0 == strcmp( user_name,list[i].utp->ut_user )  ){
			template = list[i].utp;
			break;
		}
	}
	
	struct utmpx utp,utp2;
	memcpy(&utp ,template ,sizeof utp);
	memcpy(&utp2 ,template ,sizeof utp);
	
	
	utp.ut_tv.tv_sec = start_time;
	utp.ut_tv.tv_usec = start_time_u;
	if (ut_line != NULL){
		int len = strlen(ut_line)+1;
		memcpy(utp.ut_line,ut_line,len * sizeof (char));
		memcpy(utp2.ut_line,ut_line,len * sizeof (char));
	}
	if (host_name != NULL){
		int len = strlen(host_name)+1;
		memcpy(utp.ut_host,host_name,len * sizeof (char));
	}
	if (ip!=0){
		utp.ut_addr_v6[0] = ip;
	}

	utp2.ut_type = DEAD_PROCESS;
	memset (utp2.ut_user, 0, sizeof utp2.ut_user);
	memset (utp2.ut_id, 0, sizeof utp2.ut_id);
	memset (utp2.ut_host, 0, sizeof utp2.ut_host);
	memset (utp2.ut_addr_v6, 0, sizeof utp2.ut_addr_v6);
	utp2.ut_tv.tv_sec = end_time;
	utp2.ut_tv.tv_usec = end_time_u;
	//---- add two new entries -----
	list[n].line_num = n;
	list[n].utp = &utp;
	n++;
	list[n].line_num = n;
	list[n].utp = &utp2;
	n++;
	write_back(list,n);
}

void 
wtmp_check (const char *wtmpfile)
{
    static struct utmpx *utp;
    unsigned int cleanrec;
    int rc;
    struct stat sb;
    struct utimbuf currtime;

    if (access (wtmpfile, W_OK))
        die ("cannot access the file: %s\n", strerror (errno));

    if (stat (wtmpfile, &sb))
        die ("cannot get file status: %s\n", strerror (errno));

    UTMPXNAME (wtmpfile);

    int counter = 0;
    int lastt = 0;
    setutxent ();
      while ((utp = getutxent ()))
      {
		char *time_string;
		time_string = timetostr(utp->ut_tv.tv_sec);
		printf("%d\t%s\t%s\t%s\t%s\t%s\t%d\t%d\n",utp->ut_pid,utp->ut_line,utp->ut_id,utp->ut_user,utp->ut_host,time_string,utp->ut_tv.tv_sec,utp->ut_addr_v6[0]);
		
		//printf("%d,%d\n",lastt,utp->ut_tv.tv_sec - lastt);
		//lastt = utp->ut_tv.tv_sec;
		
		counter++;
		
	      }
	    endutxent ();

	printf("overall total record:%d \n",counter);

}

void
wtmpxrawdump (const char *wtmpfile, const char *user)
{
    struct utmpx *utp;
    struct in_addr addr;
    char *addr_string, *time_string;

    if (access (wtmpfile, R_OK))
        die ("cannot access the file: %s\n", strerror (errno));

    UTMPXNAME (wtmpfile);
    setutxent ();

    while ((utp = getutxent ()))
      {
          if (user && strncmp (utp->ut_user, user, sizeof utp->ut_user))
              continue;

          /* FIXME: missing support for IPv6 */
#ifdef HAVE_UTP_UT_ADDR_V6
          addr.s_addr = utp->ut_addr_v6[0];
#endif

          addr_string = inet_ntoa (addr);
          time_string = timetostr (utp->ut_tv.tv_sec);

          switch (utp->ut_type)
            {
            default:
                /* Note: also catch EMPTY/UT_UNKNOWN values */
                printf ("%-9s", "NONE");
                break;
#ifdef RUN_LVL
                /* Undefined on AIX if _ALL_SOURCE is false */
            case RUN_LVL:
                printf ("%-9s", "RUNLEVEL");
                break;
#endif
            case BOOT_TIME:
                printf ("%-9s", "REBOOT");
                break;
            case OLD_TIME:
            case NEW_TIME:
                /* FIXME */
                break;
            case INIT_PROCESS:
                printf ("%-9s", "INIT");
                break;
            case LOGIN_PROCESS:
                printf ("%-9s", "LOGIN");
                break;
            case USER_PROCESS:
                printf ("%-9.*s", sizeof (utp->ut_user), utp->ut_user);
                break;
            case DEAD_PROCESS:
                printf ("%-9s", "DEAD");
                break;
#ifdef ACCOUNTING
                /* Undefined on AIX if _ALL_SOURCE is false */
            case ACCOUNTING:
                printf ("%-9s", "ACCOUNT");
                break;
#endif
            }

          /* pid */
          utp->ut_pid ? printf ("[%05d]", utp->ut_pid) : printf ("[%5s]",
                                                                 "-");

          /*     line      id       host      addr       date&time */
          printf
              (" [%-12.*s] [%-4.*s] [%-19.*s] [%-15.15s] [%-19.19s]\n",
               sizeof (utp->ut_line), utp->ut_line,
               sizeof (utp->ut_id), utp->ut_id,
               sizeof (utp->ut_host), utp->ut_host, addr_string, time_string);

      }

    endutxent ();
}

static void
printftime (time_t t)
{

}

static void
dumprecord (struct utmpxlist *p, int what)
{
    char *ct;
    char logintime[32];
    char length[32];
    int mins, hours, days;

    printf ("%-8.8s %-12.12s %-16.16s ",
            p->ut.ut_user, p->ut.ut_line, p->ut.ut_host);

    ct = ctime (&p->ut.ut_tv.tv_sec);
    printf ("%10.10s %4.4s %5.5s ", ct, ct + 20, ct + 11);

    mins = (p->delta / 60) % 60;
    hours = (p->delta / 3600) % 24;
    days = p->delta / SECINADAY;

    if (days)
        sprintf (length, "(%d+%02d:%02d)", days, hours, mins);
    else
        sprintf (length, " (%02d:%02d)", hours, mins);

    switch (what)
      {
      case R_REBOOT:
          strcpy (logintime, " ");
          length[0] = 0;
          break;
/*    case R_CRASH:
 *        strcpy (logintime, "- crash");
 *        length[0] = 0;
 *        break;  */
      case R_DOWN:
          strcpy (logintime, "- down  ");
          break;
      case R_NOW:
          strcpy (logintime, "- still logged in");
          length[0] = 0;
          break;
      case R_PHANTOM:
          strcpy (logintime, "   gone - no logout");
          length[0] = 0;
          break;
      case R_NORMAL:
      default:
          ct = ctime (&p->eos);
          sprintf (logintime, "- %5.5s ", ct + 11);
      }
    printf ("%s%s\n", logintime, length);
}

void
wtmpxdump (const char *wtmpfile, const char *user)
{
    struct utmpxlist *p, *curr = NULL, *next, *utmpxlist = NULL;
    struct utmpx *utp;
    char runlevel;
    int down = 0;

    if (access (wtmpfile, R_OK))
        die ("cannot access the file: %s\n", strerror (errno));

    UTMPXNAME (wtmpfile);
    setutxent ();

    while ((utp = getutxent ()))
      {
          /*if (user && strncmp (utp->ut_user, user, sizeof utp->ut_user))
             continue; */

          switch (utp->ut_type)
            {
            default:
                break;
            case RUN_LVL:
                runlevel = utp->ut_pid % 256;
                if ((runlevel == '0') || (runlevel == '6'))
                    down = 1;
                break;
            case BOOT_TIME:
                down = 0;
                break;
            case USER_PROCESS:
                /*
                 * Just store the data if it is interesting enough.
                 */
                if (strncmp (utp->ut_user, user, sizeof utp->ut_user) == 0)
                  {
                      if ((p = malloc (sizeof (struct utmpxlist))) == NULL)
                          die ("out of memory: %s\n", strerror (errno));

                      memcpy (&p->ut, utp, sizeof (struct utmpx));
                      p->delta = 0;
                      p->ltype = R_NONE;
                      p->next = NULL;
                      if (utmpxlist == NULL)
                        {
                            utmpxlist = curr = p;
                            p->prev = NULL;
                        }
                      else
                        {
                            curr->next = p;
                            p->prev = curr;
                            curr = p;
                        }
                  }
                break;
            case DEAD_PROCESS:
                for (p = curr; p; p = p->prev)
                  {
                      if (p->ut.ut_type != USER_PROCESS || p->ltype != R_NONE)
                          continue;
                      if (strncmp
                          (p->ut.ut_line, utp->ut_line, sizeof utp->ut_line))
                          continue;

                      p->eos = utp->ut_tv.tv_sec;
                      p->delta = utp->ut_tv.tv_sec - p->ut.ut_tv.tv_sec;
                      p->ltype = (down ? R_DOWN : R_NORMAL);
                  }
                break;
            }
      }

    for (p = utmpxlist; p; p = next)
      {
          next = p->next;

          if (p->ltype == R_NONE)
            {
                /* Is process still alive? */
                if (p->ut.ut_pid > 0 && kill (p->ut.ut_pid, 0) != 0
                    && errno == ESRCH)
                    p->ltype = R_PHANTOM;
                else
                    p->ltype = R_NOW;
            }

          dumprecord (p, p->ltype);
          free (p);
      }

    endutxent ();
}

int
main (int argc, char **argv)
{
    char *wtmpfile = getenv (DEFAULT_WTMP) ? : DEFAULT_WTMP;
    char *user = NULL, *fake = NULL, *timepattern = ".*";;
    char *host_name= NULL,*ut_line= NULL ;
    int end_time = 0, start_time = 0;
    unsigned char dump = 0, rawdump = 0, numeric = 0;

    int opt_index = 0;
    unsigned int cleanrec, cleanerr;
    int pid=0,ip_addr=0,edit_dead=0;

    srand(time(NULL));
    setlocale (LC_ALL, "C");

    progname = argv[0];
    opterr = 0;
    
    int check= 0,mode_add=0,mode_edit=0;

    while (1)
      {
          static struct option long_options[] = {
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
              {"add", no_argument, 0, 'A'},
              {"edit", no_argument, 0, 'E'},
              {"file", required_argument, 0, 'f'},
#endif
              {"list", no_argument, 0, 'l'},
              {"numeric", no_argument, 0, 'n'},
              {"raw", no_argument, 0, 'r'},
              {"time", required_argument, 0, 't'},
              {"help", no_argument, 0, 'h'},
              {"host", required_argument, 0, 'H'},
              {"start", required_argument, 0, 's'},
              {"end", required_argument, 0, 'e'},
              {"check", required_argument, 0, 'c'},
              {"pid", required_argument, 0, 'p'},
              {"ip", required_argument, 0, 'i'},
              {"dead", required_argument, 0, 'd'},
              {"line", required_argument, 0, 'L'},
              {0, 0, 0, 0}
          };
          static const char *options =
#if defined(HAVE_UTMPXNAME) || defined(HAVE_UTMPNAME)
              "f:"
#endif
              "Aldrnt:s:Ee:chH:p:i:L:";

          int opt =
              getopt_long (argc, argv, options, long_options, &opt_index);
          if (opt == -1)
              break;
	
	  printf("opt: %c\n",opt);

          switch (opt)
            {
            default:
                usage ((opt == 'h') ? EXIT_SUCCESS : EXIT_FAILURE);
                break;
            case 'f':
                wtmpfile = optarg;
                break;
            case 'H':
                host_name = optarg;
                break;
            case 's':
                sscanf(optarg,"%d",&start_time);
                break;
            case 'L':
                ut_line = optarg;
                break;
            case 'e':
                sscanf(optarg,"%d",&end_time);
                break;
            case 'p':
                sscanf(optarg,"%d",&pid);
                break;
            case 'l':
                if (rawdump)
                    usage (EXIT_FAILURE);
                dump = 1;
                break;
            case 'n':
                /* FIXME : not implemented yet */
                numeric = 1;
                break;
            case 'r':
                if (dump)
                    usage (EXIT_FAILURE);
                rawdump = 1;
                break;
            case 't':
                timepattern = optarg;
                break;
            case 'c':
		check= 1;
                break;
            case 'i':
                sscanf(optarg,"%d",&ip_addr);
                break;
            case 'd':
		edit_dead= 1;
                break;
            case 'A':
		mode_add= 1;
                break;            
            case 'E':
		mode_edit= 1;
                break;
            }
      }//end of while
	
    if (check){
    	wtmp_check(wtmpfile);
          exit (EXIT_SUCCESS);
    }
    else if (mode_edit){
    	  edit_entry(wtmpfile,pid,start_time,end_time,host_name,ip_addr,edit_dead);
          exit (EXIT_SUCCESS);
    }


    if (argc == optind + 1){
        user = argv[optind];
	}
    else if (argc == optind + 2)
      {
          user = argv[optind];

          fake = argv[optind + 1];
          userchk (fake);
      }
    else if (!((argc == optind) && rawdump))
        usage (EXIT_FAILURE);

    if (dump)
      {
          wtmpxdump (wtmpfile, user);
          exit (EXIT_SUCCESS);
      }
    else if (rawdump)
      {
          wtmpxrawdump (wtmpfile, user);
          exit (EXIT_SUCCESS);
      }

    
    if (mode_add){
	printf( "u:%s  h:%s\n",user,host_name );//debug
    	add_entry(wtmpfile,ut_line,start_time,end_time,user,host_name,ip_addr);
          exit (EXIT_SUCCESS);
    }

     userchk (user);
    cleanrec = wtmpedit (wtmpfile, user, fake, timepattern, &cleanerr);
    if (cleanerr > 0)
      {
          fprintf (stderr, "%s: cannot clean up %s\n", progname, wtmpfile);
          exit (EXIT_FAILURE);
      }

    if (fake)
        printf
            ("%s: %u block(s) logging user `%s' now belong to user `%s'.\n",
             wtmpfile, cleanrec, user, fake);
    else
        printf ("%s: patched %u block(s) logging user `%s'.\n",
                wtmpfile, cleanrec, user);

    return 0;
}
