
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include "N2K/NMEA2000.h"
#include "N2K/nmea2000_defs_tx.h"
#include "lv_edisplay/edisplay.h"

static nmea2000 *n2kp;

static void
usage(void)
{
	std::cerr << "usage: " << getprogname() << " <canif>" << std::endl;
	exit(1);
}


int main(int argc, char ** argv)
{
	if (argc != 2) {
		usage();
	}
	edisplay_init();
	n2kp = new nmea2000(argv[1]);
	n2kp->Init();
	edisplay_run();
	exit(0);
}
