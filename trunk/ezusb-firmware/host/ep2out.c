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

/* gcc -O2 -Wall -o ep2out ep2out.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>			/* open */
#include <sys/stat.h>			/* open */
#include <fcntl.h>			/* open */
#include <sys/ioctl.h>			/* ioctl */
#include <unistd.h>			/* read,close */
#include <linux/usb.h>			/* USB_DIR_OUT */
#include <linux/usbdevice_fs.h>		/* usbfs */

#define	NR_BANK		2		/* 1 = SYNC, 2.. = ASYNC */
/* Performance on Pentium4 3.2GHz PC
 *
 * NR_BANK                 1      2      4
 * ------------------------------------------
 * Time to send 32MB      1.5s   1.0s   1.0s
 */

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
  printf("usage: %s [--autopad] /proc/bus/usb/BBB/DDD datafile\n", argv0);
  exit(1);
}

#define	SZ_DATABUF		16384

static unsigned char databuf[NR_BANK][SZ_DATABUF] __attribute__ ((aligned(4096)));

int main(int argc, const char *argv[])
{
  FILE *fp;
  int fd, status;
  long size;
  unsigned int interface;
  int opt_autopad = 0;
  argv0 = argv[0];
  if (argc >= 2 && strcmp(argv[1], "--autopad") == 0) {
    opt_autopad = 1;
    argc --;
    argv ++;
  }
  if (! (argc == 3 && strncmp(argv[1], "/proc/bus/usb/", 14) == 0)) {
    help_and_exit();
  }
  fp = fopen(argv[2], "rb");
  if (fp == NULL)  error_and_exit("cannot fopen datafile");
  if (fseek(fp, 0L, SEEK_END) != 0)  error_and_exit("cannot fseek datafile");
  size = ftell(fp);
  if (size < 0)  error_and_exit("cannot ftell datafile");
  if (! opt_autopad) {
    if ((size & 0x1ff) != 0)  error_and_exit("curious datafile size (not multiple of 512)");
  }
  if (fseek(fp, 0L, SEEK_SET) != 0)  error_and_exit("cannot fseek datafile");

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
  while (size != 0) {
    struct usbdevfs_bulktransfer bulk;
    int len = (size >= SZ_DATABUF) ? SZ_DATABUF : size;
    if (fread(databuf[0], len, 1, fp) != 1)  error_and_exit("cannot fread datafile");
    /*printf("%ld %d ...\n", size, len);*/
    size -= len;
    while ((len & 0x1ff) != 0)  databuf[0][len++] = 0;
    bulk.ep = 0x02;
    bulk.len = len;
    bulk.timeout = 3000;
    bulk.data = &databuf[0][0];
    status = ioctl(fd, USBDEVFS_BULK, &bulk);
    if (status < 0)  error_and_exit("cannot bulk-write");
  }
#else
  {
    struct usbdevfs_urb urb[NR_BANK];
    int	ix = 0;
    int	rix;
    while (size != 0) {
      int len = (size >= SZ_DATABUF) ? SZ_DATABUF : size;
      struct usbdevfs_urb *urbp;
      if (ix >= NR_BANK) {
	urbp = NULL;
	status = ioctl(fd, USBDEVFS_REAPURB, &urbp);
	if (status < 0)  error_and_exit("cannot reap-urb");
	if (urbp != &urb[ix&(NR_BANK-1)])  error_and_exit("curious reap-urb result");
	if (urbp->status != 0)  error_and_exit("reap-urb result status error");
      }
      if (fread(databuf[ix&(NR_BANK-1)], len, 1, fp) != 1) {
	error_and_exit("cannot fread datafile");
      }
      size -= len;
      while ((len & 0x1ff) != 0)  databuf[ix&(NR_BANK-1)][len++] = 0;
      urbp = &urb[ix&(NR_BANK-1)];
      urbp->type = USBDEVFS_URB_TYPE_BULK;
      urbp->endpoint = USB_DIR_OUT | 0x02;
      urbp->status = 0;
      urbp->flags = 0;
      urbp->buffer = &databuf[ix&(NR_BANK-1)][0];
      urbp->buffer_length = len;
      urbp->actual_length = len;
      urbp->start_frame = 0;
      urbp->number_of_packets = 0;
      urbp->error_count = 0;
      urbp->signr = 0;
      urbp->usercontext = NULL;
      status = ioctl(fd, USBDEVFS_SUBMITURB, urbp);
      if (status < 0)  error_and_exit("cannot submit-urb");
      ix ++;
    }
    rix = (ix < NR_BANK) ? 0 : ix-NR_BANK;
    while (rix < ix) {
      struct usbdevfs_urb *urbp = NULL;
      status = ioctl(fd, USBDEVFS_REAPURB, &urbp);
      if (status < 0)  error_and_exit("cannot reap-urb");
      if (urbp != &urb[rix&(NR_BANK-1)])  error_and_exit("curious reap-urb result");
      if (urbp->status != 0)  error_and_exit("reap-urb result status error");
      rix ++;
    }
  }
#endif

  fclose(fp);

  status = ioctl(fd, USBDEVFS_RELEASEINTERFACE, &interface);
  if (status != 0)  error_and_exit("cannot release interface");

  close(fd);
  return 0;
}

