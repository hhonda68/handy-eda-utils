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

namespace sc_core {

#define sc_inline __attribute__((always_inline)) inline

class sc_signal_sensitive_delegator : public sc_signal_access_manager {
public:
  void sensitive_add(int method_id) { the_simcontext->sensitive_add(*this, method_id); }
  void sensitive_merge(sc_sensitive_methods& other) { the_simcontext->sensitive_merge(*this, other); }
};

template <typename T> class sc_signal_data : public sc_signal_sensitive_delegator {
protected:
  T m_val[2];
};

template <typename T> class sc_signal_accessor : public sc_signal_data<T> {
public:
  sc_inline const T& read() const { return this->m_val[this->m_rix]; };
  sc_inline void write(const T& val) { this->m_val[this->m_wix] = val;  this->m_written = true; }
  sc_inline operator const T&() const { return read(); }
  sc_inline void operator=(const T& val) { write(val); }
};

template <typename T> class sc_signal : public sc_signal_accessor<T> {
public:
  sc_signal() { the_simcontext->register_signal(*this); }
  using sc_signal_accessor<T>::operator=;
  sc_inline void operator=(const sc_signal& rhs) { operator=(rhs.read()); }
};

////////////////////////////////////////////////////////////////

template <typename T> class sc_in;
template <typename T> class sc_out;

class sc_in_ports_generic {
public:
  sc_in_ports_generic();
  ~sc_in_ports_generic();
  int size();
  void push_back(void *);
  void * const operator[](int) const;
private:
  struct impl_t;
  impl_t& m;
};

template <typename T> class sc_in_ports : public sc_in_ports_generic {
public:
  typedef sc_in<T> *value_type;
  const value_type operator[](int n) const { return static_cast<const value_type>(sc_in_ports_generic::operator[](n)); }
};

template <typename T> class sc_in_data {
public:
  sc_in_data() : m_info(new dstinfo_t) {}
  void sensitive_add(int method_id) { m_info->smeths.add(method_id); }
protected:
  struct dstinfo_t {
    sc_in_ports<T>       ports;
    sc_sensitive_methods smeths;
  };
  union {
    dstinfo_t    *m_info;
    sc_signal<T> *m_sig;
  };
};

template <typename T> class sc_in_reader : public sc_in_data<T> {
public:
  sc_inline const T& read() const { return this->m_sig->read(); }
  sc_inline operator const T&() const { return read(); }
};

template <typename T> class sc_in_body : public sc_in_reader<T> {
public:
  using sc_in_reader<T>::m_info;
  using sc_in_reader<T>::m_sig;
  void bind(sc_in<T>& src)  { src.m_info->ports.push_back(this); }
  void bind(sc_out<T>& src) { src.m_info->dstports.push_back(this); }
  void bind(sc_signal<T>& src);
  template <typename S> void operator()(S& src) { bind(src); }
};

template <typename T> void sc_in_body<T>::bind(sc_signal<T>& src) {
  int n = m_info->ports.size();
  for (int i=0; i<n; ++i)  m_info->ports[i]->bind(src);
  src.sensitive_merge(m_info->smeths);
  delete m_info;
  m_sig = &src;
}

class sc_clock_edge {
#ifdef BCASYSC_MULTICLOCK
public:
  explicit sc_clock_edge(const sc_in<bool>& port) : m_port(port) {}
  void sensitive_add(int method_id) const { the_simcontext->mark_method_as_clocked(method_id, m_port); }
  const sc_in<bool>& clock_port() const { return m_port; }
private:
  const sc_in<bool>& m_port;
#else
public:
  void sensitive_add(int method_id) const { the_simcontext->mark_method_as_clocked(method_id); }
#endif
};

template <typename T> class sc_in : public sc_in_body<T> {};
template <> class sc_in<bool> : public sc_in_body<bool> {
public:
#ifdef BCASYSC_MULTICLOCK
  const sc_clock_edge pos() const { return sc_clock_edge(*this); }
#else
  const sc_clock_edge pos() const { return sc_clock_edge(); }
#endif
};

////////////////////////////////////////////////////////////////

template <typename T> class sc_out_data {
public:
  sc_out_data() : m_info(new info_t) {}
  void sensitive_add(int method_id) { m_info->smeths.add(method_id); }
protected:
  struct info_t {
    sc_out<T>            *srcport;
    sc_in_ports<T>       dstports;
    sc_sensitive_methods smeths;
    info_t() : srcport(0) {}
  };
  union {
    info_t       *m_info;
    sc_signal<T> *m_sig;
  };
};

template <typename T> class sc_out_accessor : public sc_out_data<T> {
public:
  sc_inline const T& read() const { return this->m_sig->read(); }
  sc_inline void write(const T& val) { this->m_sig->write(val); }
  sc_inline operator const T&() const { return read(); }
  sc_inline void operator=(const T& val) { write(val); }
};

template <typename T> class sc_out : public sc_out_accessor<T> {
public:
  using sc_out_accessor<T>::m_info;
  using sc_out_accessor<T>::m_sig;
  void bind(sc_out& dst) { dst.m_info->srcport = this; }
  void bind(sc_signal<T>& dst);
  template <typename D> void operator()(D& dst) { bind(dst); }
  using sc_out_accessor<T>::operator=;
  sc_inline void operator=(const sc_out& rhs) { write(rhs.read()); }
};

template <typename T> void sc_out<T>::bind(sc_signal<T>& dst) {
  if (m_info->srcport != 0)  m_info->srcport->bind(dst);
  int n = m_info->dstports.size();
  for (int i=0; i<n; ++i)  m_info->dstports[i]->bind(dst);
  dst.sensitive_merge(m_info->smeths);
  delete m_info;
  m_sig = &dst;
}

#undef sc_inline

} // namespace sc_core

#endif
