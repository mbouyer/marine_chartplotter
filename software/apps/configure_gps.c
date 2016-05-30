#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>


struct termios orig_t;
int term_fd = -1;
FILE *term_file = NULL;


static void
exit_handler(void)
{
	if (term_fd >= 0) {
		tcsetattr(term_fd, TCSAFLUSH,  &orig_t);
	}
}

static void
sig_hanlder(int sig)
{
	exit_handler();
	exit(1);
}

static int
check_term()
{
	fd_set rset;
	struct timeval timeout;
	int tries = 10;
	int ret;
#define BUFSIZE 40
	char buf[BUFSIZE];
	int flushed = 0;

	/* XXX discard existing inputs */
	while (!flushed) {
		FD_ZERO(&rset);
		FD_SET(term_fd, &rset);
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		ret = select(term_fd + 1, &rset, NULL, NULL, &timeout);
		switch(ret) {
		case -1:
			err(EXIT_FAILURE, "select() failed");
		case 0:
			flushed = 1;
			break;
		default:
			if (FD_ISSET(term_fd, &rset)) {
				(void)fgets(buf, BUFSIZE, term_file);
			}
		}
	}
	while (tries > 0) {
		FD_ZERO(&rset);
		FD_SET(term_fd, &rset);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		ret = select(term_fd + 1, &rset, NULL, NULL, &timeout);
		switch(ret) {
		case -1:
			err(EXIT_FAILURE, "select() failed");
			break;
		case 0:
			printf("."); fflush(stdout);
			tries--;
			break;
		default:
			if (FD_ISSET(term_fd, &rset)) {
				memset(buf, 0, BUFSIZE);
				if (fgets(buf, BUFSIZE, term_file) != NULL) {
					// printf("got: -- %s --\n", buf);
					if (strncmp(buf, "$GP", 3) == 0) {
						return 1;
					}
				}
				tries--;
				printf("."); fflush(stdout);
			}
		}
	}
	return 0;
#undef BUFSIZE
}

static void
set_term(int fd, struct termios *tp)
{
	if (tcsetattr(fd, TCSAFLUSH,  tp) != 0) {
		perror("tcsetattr");
		exit(1);
	}
	//sleep(1);
	usleep(500000);
}

static int 
open_tty(const char *name)
{
	if (term_file != NULL) {
		fclose(term_file);
		term_file = NULL;
		term_fd = -1;
	}
	term_fd = open(name, O_RDWR, 0);
	if (term_fd < 0) {
		err(EXIT_FAILURE, "open %s", name);
	}
	term_file = fdopen(term_fd, "r+");
	if (term_file == NULL) {
		err(EXIT_FAILURE, "fdopen %s", name);
	}
}

static void
gps_configure()
{
#define BUFSIZE 80
	char buf[BUFSIZE];
	int i;

	fprintf(term_file, "$PMTK220,2000*1C\r\n");
	fflush(term_file);
	for (i = 0; i < 10; i++) {
		if (fgets(buf, BUFSIZE, term_file) != NULL) {
			if (strncmp(buf, "$PMTK001,220,3",14) == 0)
				break;
		}
	}
	if (i < 10) {
		fprintf(term_file, "$PMTK251,4800*14\r\n");
		fflush(term_file);
	}
}
#undef BUFSIZE

int
main(int argc, const char *argv[])
{

	static struct termios working_t;
	int i;
	if (argc != 2) {
		fprintf(stderr, "usage: %s <tty>\n", argv[0]);
		exit(1);
	}
	open_tty(argv[1]);

	if (tcgetattr(term_fd, &orig_t) < 0) {
		perror("tcgetattr");
		exit(1);
	}
	atexit(exit_handler);
	signal(SIGINT, sig_hanlder);
	signal(SIGTERM, sig_hanlder);

	working_t = orig_t;
	working_t.c_iflag = 0;
	working_t.c_oflag &= ~(OPOST);
	working_t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | CRTSCTS | MDMBUF);
	working_t.c_cflag |= (CS8 | CLOCAL);
	working_t.c_lflag = 0;
again:
	if (cfsetspeed(&working_t, B4800) != 0)
		err(EXIT_FAILURE, "setting speed to 4800");

	set_term(term_fd, &working_t);
	if (check_term(term_fd)) {
		printf(" configured\n");
		exit(0);
	}
	if (cfsetspeed(&working_t, B9600) != 0)
		err(EXIT_FAILURE, "setting speed to 9600");
	set_term(term_fd, &working_t);
	if (check_term(term_fd)) {
		printf(" configuring"); fflush(stdout);
		gps_configure();
	}
	goto again;
}
