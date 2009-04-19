// -*- c++ -*-

//
// StreamFlowC : A C++ coding style to describe hardware structure for stream processing
//

// Copyright (c) 2009 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#ifndef STREAMFLOWC_H
#define STREAMFLOWC_H

namespace streamflowc {

template <typename T, bool BIG> struct fastarg_traits;
template <typename T> struct fastarg_traits<T,false> { typedef T        type; };
template <typename T> struct fastarg_traits<T,true > { typedef const T& type; };
template <typename T> struct fastarg : fastarg_traits<T,(sizeof(T)>8)> {};

template <typename T> struct port {
  typedef typename fastarg<T>::type arg_type;
  void (*m_proc)(void *, arg_type);
  void operator()(arg_type x) { return m_proc(this, x); }
};

struct portref_base {
  __SIZE_TYPE__ m_ptr;
  portref_base() : m_ptr(0) {}
  void chain(portref_base *src);
  void assign(void *dst);
  void *get() const { return reinterpret_cast<void *>(m_ptr); }
};

template <typename T> struct portref : private portref_base {
  void operator=(port<T>& dst) { assign(&dst); }
  void operator=(portref<T>& src) { chain(&src); }
  void operator()(typename port<T>::arg_type x) { (*static_cast<port<T>*>(get()))(x); }
};

template <typename T0, typename T1> struct issame      { static const int val = 0; };
template <typename T>               struct issame<T,T> { static const int val = 1; };

template <int N> struct garbage { template <typename T> garbage(const T&) {} };

struct nil {};
template <typename T00=nil, typename T01=nil, typename T02=nil, typename T03=nil, typename T04=nil,
	  typename T05=nil, typename T06=nil, typename T07=nil, typename T08=nil, typename T09=nil,
	  typename T10=nil, typename T11=nil, typename T12=nil, typename T13=nil, typename T14=nil,
	  typename T15=nil, typename T16=nil, typename T17=nil, typename T18=nil, typename T19=nil,
	  typename T20=nil, typename T21=nil, typename T22=nil, typename T23=nil, typename T24=nil,
	  typename T25=nil, typename T26=nil, typename T27=nil, typename T28=nil, typename T29=nil>
struct outtypes {
  template <typename T> struct dupcnt {
    static const int val = (issame<T,T00>::val+issame<T,T01>::val+issame<T,T02>::val+issame<T,T03>::val+issame<T,T04>::val
			    +issame<T,T05>::val+issame<T,T06>::val+issame<T,T07>::val+issame<T,T08>::val+issame<T,T09>::val
			    +issame<T,T10>::val+issame<T,T11>::val+issame<T,T12>::val+issame<T,T13>::val+issame<T,T14>::val
			    +issame<T,T15>::val+issame<T,T16>::val+issame<T,T17>::val+issame<T,T18>::val+issame<T,T19>::val
			    +issame<T,T20>::val+issame<T,T21>::val+issame<T,T22>::val+issame<T,T23>::val+issame<T,T24>::val
			    +issame<T,T25>::val+issame<T,T26>::val+issame<T,T27>::val+issame<T,T28>::val+issame<T,T29>::val);
  };
  template <typename T, int N, bool DUP> struct nth_judge;
  template <typename T, int N> struct nth_judge<T,N,false> { typedef portref<T>&      type; };
  template <typename T, int N> struct nth_judge<T,N,true>  { typedef const garbage<N> type; };
  template <typename T, int N> struct nth : nth_judge<T,N,(dupcnt<T>::val>1)> {};
};

} // namespace streamflowc

#endif
