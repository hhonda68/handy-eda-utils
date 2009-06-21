// -*- c++ -*-
#ifndef ADD_H
#define ADD_H

#include "common.h"
#include "addgeneric.h"
#include "add32.h"

template <int W> struct Add : AddGeneric<W> {};
template <> struct Add<32> : Add32 {};

#endif
