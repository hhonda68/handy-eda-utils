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

/* gcc -O2 -Wall -o firmload firmload.c */
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

static int fgethex(FILE *fp)
{
  int ch = fgetc(fp);
  if (ch >= '0' && ch <= '9')  return ch-'0';
  if (ch >= 'A' && ch <= 'F')  return ch-'A'+10;
  if (ch >= 'a' && ch <= 'f')  return ch-'a'+10;
  error_and_exit("curious HEX format");
  /*NOTREACHED*/
  return 0;
}

static int fgethex2(FILE *fp)
{
  int hi, lo;
  hi = fgethex(fp);
  lo = fgethex(fp);
  return (hi<<4) | lo;
}
  
static int fgethex4(FILE *fp)
{
  int hi, lo;
  hi = fgethex2(fp);
  lo = fgethex2(fp);
  return (hi<<8) | lo;
}

static void do_firmload(int fd, int addr, int len, void *data)
{
  struct usbdevfs_ctrltransfer ctrl;
  int status;
  ctrl.bRequestType = 0x40;
  ctrl.bRequest = 0xA0;
  ctrl.wValue = addr;
  ctrl.wIndex = 0;
  ctrl.wLength = len;
  ctrl.timeout = 1000;
  ctrl.data = data;
  status = ioctl(fd, USBDEVFS_CONTROL, &ctrl);
  if (status != len) {
    error_and_exit("cannot issue control transfer");
  }
}

int main(int argc, const char *argv[])
{
  int fd, status;
  unsigned char buf[18];
  const char *devicename, *firmwarename;
  argv0 = argv[0];
  if (! (argc == 3 && strncmp(argv[1], "/proc/bus/usb/", 14) == 0)) {
    printf("usage: %s /proc/bus/usb/BBB/DDD firmwarefile.hex\n", argv[0]);
    return 1;
  }
  devicename = argv[1];
  firmwarename = argv[2];

  fd = open(devicename, O_RDWR);
  if (fd < 0)  error_and_exit("cannot open device");

  status = read(fd, buf, 18);
  if (status != 18)  error_and_exit("cannot read device info");
  if (! (buf[8] == 0xb4 && buf[9] == 0x04 && buf[10] == 0x13 && buf[11] == 0x86)) {
    error_and_exit("curious vendor/product ID");
  }

  {
    unsigned char buf[16];
    FILE *fp;
    buf[0] = 1;
    do_firmload(fd, 0xE600, 1, buf);
    fp = fopen(firmwarename, "r");
    if (fp == NULL)  error_and_exit("cannot open firmware file");
    while (1) {
      int len,addr,type,i,x, sum;
      if (fgetc(fp) != ':')  error_and_exit("curious firmware file format");
      len = fgethex2(fp);
      addr = fgethex4(fp);
      type = fgethex2(fp);
      switch (type) {
      case 0:
	if (len > 16)  error_and_exit("unsupported length (>16) in firmware file");
	sum = len + (addr>>8) + (addr&0xFF);
	for (i=0; i<len; i++) {
	  x = fgethex2(fp);
	  buf[i] = x;
	  sum += x;
	}
	x = fgethex2(fp);
	if (((sum + x) & 0xFF) != 0)  error_and_exit("firmware checksum mismatch");
	x = fgetc(fp);
	if (x == '\r')  x = fgetc(fp);
	if (x != '\n')  error_and_exit("curious EOL in firmware file");
	/*printf("addr %04x len %02x\n", addr, len);*/
	do_firmload(fd, addr, len, buf);
	break;
      case 1:
	if (! (len == 0 && addr == 0))  error_and_exit("curious END line in firmware file");
	if (fgethex2(fp) != 0xFF)  error_and_exit("curious END line in firmware file");
	break;
      default:
	error_and_exit("curious firmware file format (type field)");
	break;
      }
      if (type == 1)  break;
    }
    fclose(fp);
    buf[0] = 0;
    do_firmload(fd, 0xE600, 1, buf);
  }

  close(fd);
  return 0;
}

