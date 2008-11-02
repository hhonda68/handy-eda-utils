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

module USBFS

TopDir = "/proc/bus/usb"

def find(vendor=nil, product=nil)
  vendor ||= "04b4"
  product ||= "8613"
  bus,dev,busdevline = nil,nil,nil
  File.foreach(TopDir+"/devices") do |line|
    case line
    when /^T:/
      busdevline = line
    when /^P:  Vendor=#{vendor} ProdID=#{product} /
        busdevline  or fail
      busdevline =~ /^T:\s+Bus=(\d+).*Dev\#=\s*(\d+)/  or fail
      bus, dev = $1, $2
    when /^$/
      busdevline = nil
    end
    break if bus
  end
  if (bus) then
    bus = ("0"*(3-bus.size)) + bus
    dev = ("0"*(3-dev.size)) + dev
    [bus, dev]
  else
    nil
  end
end

def devicefile(busdev_or_bus, dev=nil)
  if (dev) then
    bus = busdev_or_bus
  else
    bus,dev = busdev_or_bus
  end
  [TopDir,bus,dev].join("/")
end

end

USBFS.extend(USBFS)
