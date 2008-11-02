/*
 * Generic EZUSB firmware
 *
 * Copyright (c) 2008 the handy-eda-utils develeper(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 *
 * Last modified on May 2007.
 */

/* gcc -O2 -Wall -o ep6in ep6in.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>			/* open */
#include <sys/stat.h>			/* open */
#include <fcntl.h>			/* open */
#include <sys/ioctl.h>			/* ioctl */
#include <unistd.h>			/* read,close */
#include <linux/usb.h>			/* USB_DIR_IN */
#include <linux/usbdevice_fs.h>		/* usbfs */

#define	NR_BANK		2		/* 1 = SYNC, 2.. = ASYNC */

static const char *argv0;

static void error_and_exit(const char *message)
{
  if (errno == 0) {
    fprintf(stderr, "%s: %s.\n", argv0, message);
  } else {
    fprintf(stderr, "%s: ", argv0);
    perror(message);
  }
  exit(2);
}

static void help_and_exit(void)
{
  fprintf(stderr, "usage: %s /proc/bus/usb/BBB/DDD [nr_packets] | hexdump -C\n", argv0);
  exit(1);
}

#define	SZ_DATABUF		16384

static unsigned char databuf[NR_BANK][SZ_DATABUF] __attribute__ ((aligned(4096)));

int main(int argc, const char *argv[])
{
  int fd, status, nr_packets;
  unsigned int interface;
  argv0 = argv[0];
  if (! ((argc == 2 || argc == 3) && strncmp(argv[1], "/proc/bus/usb/", 14) == 0)) {
    help_and_exit();
  }
  nr_packets = (argc == 2) ? 1 : atoi(argv[2]);
  if (nr_packets < 1)  help_and_exit();
  if (isatty(1))  help_and_exit();

  fd = open(argv[1], O_RDWR);
  if (fd < 0)  error_and_exit("cannot open device");

  {
    /* assume firm.ihex */
    unsigned char buf[18];
    status = read(fd, buf, 18);
    if (status != 18)  error_and_exit("cannot read device info");
    if (! (buf[8] == 0xb4 && buf[9] == 0x04 && buf[10] == 0x13 && buf[11] == 0x86)) {
      error_and_exit("curious vendor/product ID");
    }
  }

  interface = 0;
  status = ioctl(fd, USBDEVFS_CLAIMINTERFACE, &interface);
  if (status != 0)  error_and_exit("cannot claim interface");

  {
    struct usbdevfs_setinterface intf;
    intf.interface = 0;
    intf.altsetting = 1;
    status = ioctl(fd, USBDEVFS_SETINTERFACE, &intf);
    if (status != 0)  error_and_exit("cannot set interface");
  }

#if NR_BANK == 1
  while (nr_packets != 0) {
    struct usbdevfs_bulktransfer bulk;
    int npak = (nr_packets >= SZ_DATABUF/512) ? SZ_DATABUF/512 : nr_packets;
    bulk.ep = USB_DIR_IN | 0x06;
    bulk.len = npak*512;
    bulk.timeout = 10000;
    bulk.data = &databuf[0][0];
    status = ioctl(fd, USBDEVFS_BULK, &bulk);
    if (status < 0)  error_and_exit("cannot bulk-read");
    if (bulk.len != npak*512)  error_and_exit("curious received_length");
    if (fwrite(databuf[0], npak*512, 1, stdout) != 1)  error_and_exit("cannot fwrite");
    nr_packets -= npak;
  }
#else
  {
    struct usbdevfs_urb urb[NR_BANK];
    int	ix = 0;
    int	rix;
    while (nr_packets != 0) {
      int npak = (nr_packets >= SZ_DATABUF/512) ? SZ_DATABUF/512 : nr_packets;
      struct usbdevfs_urb *urbp;
      if (ix >= NR_BANK) {
	urbp = NULL;
	status = ioctl(fd, USBDEVFS_REAPURB, &urbp);
	if (status < 0)  error_and_exit("cannot reap-urb");
	if (urbp != &urb[ix&(NR_BANK-1)])  error_and_exit("curious reap-urb result");
	if (urbp->status != 0)  error_and_exit("reap-urb result status error");
	if (urbp->actual_length != urbp->buffer_length) {
	  error_and_exit("curious actual_length");
	}
	if (fwrite(urbp->buffer, urbp->buffer_length, 1, stdout) != 1) {
	  error_and_exit("cannot fwrite");
	}
      }
      urbp = &urb[ix&(NR_BANK-1)];
      urbp->type = USBDEVFS_URB_TYPE_BULK;
      urbp->endpoint = USB_DIR_IN | 0x06;
      urbp->status = 0;
      urbp->flags = 0;
      urbp->buffer = &databuf[ix&(NR_BANK-1)][0];
      urbp->buffer_length = npak*512;
      urbp->actual_length = 0;
      urbp->start_frame = 0;
      urbp->number_of_packets = 0;
      urbp->error_count = 0;
      urbp->signr = 0;
      urbp->usercontext = NULL;
      status = ioctl(fd, USBDEVFS_SUBMITURB, urbp);
      if (status < 0)  error_and_exit("cannot submit-urb");
      nr_packets -= npak;
      ix ++;
    }
    rix = (ix < NR_BANK) ? 0 : ix-NR_BANK;
    while (rix < ix) {
      struct usbdevfs_urb *urbp = NULL;
      status = ioctl(fd, USBDEVFS_REAPURB, &urbp);
      if (status < 0)  error_and_exit("cannot reap-urb");
      if (urbp != &urb[rix&(NR_BANK-1)])  error_and_exit("curious reap-urb result");
      if (urbp->status != 0)  error_and_exit("reap-urb result status error");
      if (urbp->actual_length != urbp->buffer_length) {
	error_and_exit("curious actual_length");
      }
      if (fwrite(urbp->buffer, urbp->buffer_length, 1, stdout) != 1) {
	error_and_exit("cannot fwrite");
      }
      rix ++;
    }
  }
#endif

  status = ioctl(fd, USBDEVFS_RELEASEINTERFACE, &interface);
  if (status != 0)  error_and_exit("cannot release interface");

  close(fd);
  return 0;
}

