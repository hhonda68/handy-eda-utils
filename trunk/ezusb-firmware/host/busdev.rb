#!/usr/bin/ruby -w
#
# Generic EZUSB firmware
#
# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on May 2007.

require "usbfs.rb"

if (ARGV.size >= 1 && (ARGV[0] == "-f" || ARGV[0] == "--fullpath")) then
  Opt_fullpath = true
  ARGV.shift
else
  Opt_fullpath = false
end

if (ARGV.size == 2 && ARGV[0] =~ /^[0-9a-f]{4}$/ && ARGV[1] =~ /^[0-9a-f]{4}$/) then
  vendor,product = ARGV
elsif (ARGV.size == 0)
  vendor,product = nil,nil
else
  STDERR.print("usage: #{$0} [--fullpath] [VendorID ProductID]\n")
  exit 1
end

busdev = USBFS.find(vendor, product)
if busdev then
  bus,dev = busdev
  if (Opt_fullpath) then
    print(USBFS::TopDir,"/",bus,"/",dev,"\n")
  else
    print(bus," ",dev,"\n")
  end
  exit 0
else
  STDERR.print("No device (vendor=#{vendor}, product=#{product})\n")
  exit 2
end

