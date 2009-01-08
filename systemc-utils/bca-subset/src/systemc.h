// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#ifndef BCASYSC_SYSTEMC_H
#define BCASYSC_SYSTEMC_H

#include "systemc"

using ::sc_dt::sc_int;
using ::sc_dt::sc_uint;

using ::sc_core::sc_in;
using ::sc_core::sc_out;
using ::sc_core::sc_signal;
using ::sc_core::sc_clock;
using ::sc_core::sc_start;
using ::sc_core::sc_stop;
using ::sc_core::sc_set_time_resolution;
using ::sc_core::sc_time_stamp;
using ::sc_core::sc_time;
using ::sc_core::SC_NS;
using ::sc_core::wait;
using ::sc_core::sc_module;
using ::sc_core::sc_module_name;
using ::sc_core::sc_trace_file;

#include <iostream>

using ::std::ios;
using ::std::streambuf;
using ::std::streampos;
using ::std::streamsize;
using ::std::iostream;
using ::std::istream;
using ::std::ostream;
using ::std::cin;
using ::std::cout;
using ::std::cerr;
using ::std::endl;
using ::std::flush;
using ::std::dec;
using ::std::hex;
using ::std::oct;
using ::std::fstream;
using ::std::ifstream;
using ::std::ofstream;
using ::std::size_t;
using ::std::memchr;
using ::std::memcmp;
using ::std::memcpy;
using ::std::memmove;
using ::std::memset;
using ::std::strcat;
using ::std::strncat;
using ::std::strchr;
using ::std::strrchr;
using ::std::strcmp;
using ::std::strncmp;
using ::std::strcpy;
using ::std::strncpy;
using ::std::strcspn;
using ::std::strspn;
using ::std::strlen;
using ::std::strpbrk;
using ::std::strstr;
using ::std::strtok;

#endif
