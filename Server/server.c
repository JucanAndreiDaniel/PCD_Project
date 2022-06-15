#include <errno.h>
#include <pthread.h>
#include <stddef.h>
//#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void *unix_main(void *args);
void *inet_main(void *args);
void *rest_main(void *args);

#define UNIXSOCKET "/tmp/server.sock"
#define INETPORT 8081
#define RESTPORT 8080

pthread_mutex_t curmtx = PTHREAD_MUTEX_INITIALIZER;

int main()
{
  int iport, rport;

  pthread_t unixthr, /* UNIX Thread: the UNIX server component */
      inetthr,       /* INET Thread: the INET server component */
      restthr;       /* REST Thread: the REST server component */
  //	workerthr ;  /* The Worker Thread: use it for WORK tasks (various) */

  /*
    mainwnd = initscr () ;
    noecho () ;
    cbreak () ;
    if (has_colors()) start_color () ;
    init_pair (1, COLOR_RED, COLOR_BLACK) ;
  //  refresh () ;
  */
  unlink(UNIXSOCKET);

  pthread_create(&unixthr, NULL, unix_main,
                 UNIXSOCKET); /* Transmite SOCKET-ul utilizat */
  iport = INETPORT;
  pthread_create(&inetthr, NULL, inet_main, &iport);

  rport = RESTPORT;
  pthread_create(&restthr, NULL, rest_main, &rport);

  pthread_join(unixthr, NULL);
  pthread_join(inetthr, NULL);
  pthread_join(restthr, NULL);

  /*
    getch () ;
    endwin () ;
    */
  unlink(UNIXSOCKET);
  return 0;
}
