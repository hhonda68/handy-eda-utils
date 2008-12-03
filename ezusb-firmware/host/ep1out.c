/*
 * Generic EZUSB firmware
 *
 * Copyright (c) 2008 the handy-eda-utils developer(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 *
 * Last modified on May 2007.
 */

/* gcc -O2 -Wall -o ep1out ep1out.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>			/* open */
#include <sys/stat.h>			/* open */
#include <fcntl.h>			/* open */
#include <sys/ioctl.h>			/* ioctl */
#include <unistd.h>			/* read,close */
#include <linux/usbdevice_fs.h>		/* usbfs */

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
  printf("usage: %s /proc/bus/usb/BBB/DDD dat0(hex) dat1 dat2 ...\n", argv0);
  exit(1);
}

static int gethexbyte(const char *str)
{
  int x, ch;
  ch = str[0];
  if (ch >= '0' && ch <= '9') {
    x = ch - '0';
  } else if (ch >= 'a' && ch <= 'f') {
    x = ch - 'a' + 10;
  } else if (ch >= 'A' && ch <= 'F') {
    x = ch - 'A' + 10;
  } else {
    return -1;
  }
  x <<= 4;
  ch = str[1];
  if (ch >= '0' && ch <= '9') {
    x |= ch - '0';
  } else if (ch >= 'a' && ch <= 'f') {
    x |= ch - 'a' + 10;
  } else if (ch >= 'A' && ch <= 'F') {
    x |= ch - 'A' + 10;
  } else {
    return -1;
  }
  if (str[2] != '\0')  return -1;
  return x;
}

#define	SZ_DATA		63

int main(int argc, const char *argv[])
{
  int fd, status;
  unsigned int interface;
  unsigned char databuf[SZ_DATA];
  argv0 = argv[0];
  if (! (argc >= 3 && strncmp(argv[1], "/proc/bus/usb/", 14) == 0 && argc <= 2+SZ_DATA)) {
    help_and_exit();
  }
  {
    int i, x;
    for (i=2; i<argc; i++) {
      x = gethexbyte(argv[i]);
      if (x < 0)  help_and_exit();
      databuf[i-2] = x;
    }
  }

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

  {
    struct usbdevfs_bulktransfer bulk;
    bulk.ep = 0x01;
    bulk.len = argc-2;
    bulk.timeout = 1000;
    bulk.data = databuf;
    status = ioctl(fd, USBDEVFS_BULK, &bulk);
    if (status < 0)  error_and_exit("cannot bulk-write");
  }

  status = ioctl(fd, USBDEVFS_RELEASEINTERFACE, &interface);
  if (status != 0)  error_and_exit("cannot release interface");

  close(fd);
  return 0;
}

