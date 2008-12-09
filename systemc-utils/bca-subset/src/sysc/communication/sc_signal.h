// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// sc_signal, sc_in, sc_out

#ifndef BCASYSC_COMMUNICATION_SCSIGNAL_H
#define BCASYSC_COMMUNICATION_SCSIGNAL_H

#include "../kernel/sc_simcontext.h"
#include <vector>

namespace sc_core {

class sc_signal_sensitive_methods {
public:
  void sensitive_add(int method_id) { m_smeths.add(method_id); }
  void sensitive_merge(sc_sensitive_methods& other) { m_smeths.merge(other); }
protected:
  sc_sensitive_methods m_smeths;
  void check_sensitive_methods() { if (m_smeths.valid())  the_simcontext->check_sensitive_methods(&m_smeths); }
};

template <typename T> class sc_signal_data : public sc_signal_sensitive_methods {
protected:
  T m_val[2];
};

template <typename T, bool B> struct sc_signal_arg_type;
template <typename T> struct sc_signal_arg_type<T,true > { typedef T type; };
template <typename T> struct sc_signal_arg_type<T,false> { typedef const T& type; };
template <typename T> struct sc_signal_arg : sc_signal_arg_type<T,(sizeof(T)<=8)> {};

template <typename T> class sc_signal_accessor : public sc_signal_data<T> {
public:
  const T& read() const { return this->m_val[0]; };
  operator const T&() const { return read(); }
  void write(typename sc_signal_arg<T>::type val) {
    int ix = the_simcontext->signal_write_index();
    this->m_val[ix] = val;
    if (ix == 0)  this->check_sensitive_methods();
  }
  void operator=(const T& val) { write(val); }
};

template <typename T> class sc_signal : public sc_signal_accessor<T>, public sc_signal_update_if {
public:
  sc_signal() { the_simcontext->register_signal(*this, this->m_smeths); }
  void update() { this->m_val[0] = this->m_val[1]; }
  using sc_signal_accessor<T>::operator=;
  void operator=(const sc_signal& rhs) { operator=(rhs.read()); }
};

////////////////////////////////////////////////////////////////

template <typename T> class sc_in;

template <typename T> class sc_in_data {
public:
  sc_in_data() : m_info(new dstinfo_t) {}
  void sensitive_add(int method_id) { m_info->smeths.add(method_id); }
protected:
  struct dstinfo_t {
    ::std::vector<sc_in<T>*> ports;
    sc_sensitive_methods     smeths;
  };
  union {
    dstinfo_t    *m_info;
    sc_signal<T> *m_sig;
  };
};

template <typename T> class sc_in_reader : public sc_in_data<T> {
public:
  typedef typename sc_signal_arg<T>::type argtype_t;
  argtype_t read() const { return this->m_sig->read(); }
  operator argtype_t() const { return read(); }
};

class sc_clock_edge {
public:
  void sensitive_add(int method_id) const { the_simcontext->mark_method_as_clocked(method_id); }
};

template <typename T> class sc_in_pos {};
template <> class sc_in_pos<bool> {
public:
  const sc_clock_edge pos() const { return sc_clock_edge(); }
};

template <typename T> class sc_in : public sc_in_reader<T>, public sc_in_pos<T> {
public:
  using sc_in_reader<T>::m_info;
  using sc_in_reader<T>::m_sig;
  void bind(sc_in& src) { src.m_info->ports.push_back(this); }
  void bind(sc_signal<T>& src);
  template <typename S> void operator()(S& src) { bind(src); }
};

template <typename T> void sc_in<T>::bind(sc_signal<T>& src) {
  int n = m_info->ports.size();
  for (int i=0; i<n; ++i)  m_info->ports[i]->bind(src);
  src.sensitive_merge(m_info->smeths);
  delete m_info;
  m_sig = &src;
}

////////////////////////////////////////////////////////////////

template <typename T> class sc_out;

template <typename T> class sc_out_data {
public:
  sc_out_data() : m_info(new srcinfo_t) {}
  void sensitive_add(int method_id) { m_info->smeths.add(method_id); }
protected:
  struct srcinfo_t {
    sc_out<T>             *port;
    sc_sensitive_methods  smeths;
    srcinfo_t() : port(0) {}
  };
  union {
    srcinfo_t    *m_info;
    sc_signal<T> *m_sig;
  };
};

template <typename T> class sc_out_accessor : public sc_out_data<T> {
public:
  typedef typename sc_signal_arg<T>::type argtype_t;
  argtype_t read() const { return this->m_sig->read(); }
  operator argtype_t() const { return read(); }
  void write(argtype_t val) { this->m_sig->write(val); }
  void operator=(argtype_t val) { write(val); }
};

template <typename T> class sc_out : public sc_out_accessor<T> {
public:
  using sc_out_accessor<T>::m_info;
  using sc_out_accessor<T>::m_sig;
  void bind(sc_out& dst) { dst.m_info->port = this; }
  void bind(sc_signal<T>& dst);
  template <typename D> void operator()(D& dst) { bind(dst); }
  using sc_out_accessor<T>::operator=;
  void operator=(const sc_out& rhs) { write(rhs.read()); }
};

template <typename T> void sc_out<T>::bind(sc_signal<T>& dst) {
  if (m_info->port != 0)  m_info->port->bind(dst);
  dst.sensitive_merge(m_info->smeths);
  delete m_info;
  m_sig = &dst;
}

}; // namespace sc_core

#endif
