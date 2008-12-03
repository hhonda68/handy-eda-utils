// -*- C++ -*-

//
// Lightweight functional digital logic simulator library.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on October 2005.

#ifndef CYCLEC_INCLUDED
#define CYCLEC_INCLUDED

#include	<string>
#include	<stdexcept>
#include	<vector>
#include	<stack>

//#define CYCLEC_MULTICLOCK

namespace CycleC {

class Object {
public:
  Object() throw();
  virtual ~Object() /*throw()*/;
  std::string name() const;
  const Object& parent() const throw();
protected:
  void set_myname(const std::string& name);
  void set_parent(const Object& parent);
  virtual const std::string& myname() const;
private:
  mutable std::string m_myname;
  const Object* m_parent;
  //disabled
  Object(const Object&);
  Object& operator=(const Object&);
};

class ClockedObject : public Object {
public:
  ClockedObject();
  virtual void tick() = 0;
  virtual void reset() = 0;
};

template <typename T>
class Port : public ClockedObject {
public:
  Port();
  T get() const;
  virtual void tick();
  virtual void reset();
  T operator()() const;
protected:
  virtual T realget() const = 0;
  virtual void realreset();
private:
  mutable bool m_cached;
  mutable T m_cachedvalue;
};
 
template <typename T>
class OutPort : public Port<T> {
  // FW team may add somthing here.
};

class Circuit : public Object {
public:
  class Name {
  public:
    Name(const char* name);
    Name(const Name&);
    ~Name();
  private:
    friend class Circuit;
    const char* m_str;
    Circuit* m_circuit;
    bool m_pushed;
    //disabled
    Name& operator=(const Name&);
  };

  template <typename T>
  class Register : public ClockedObject {
  public:
    class NxtProxy {
    public:
      NxtProxy(Register& ref);
      void operator=(const T& val);
    private:
      Register& m_ref;
      //disabled
      NxtProxy& operator=(const NxtProxy&);
    };
    class CkeProxy {
    public:
      CkeProxy(Register& ref);
      void operator=(bool val);
    private:
      Register& m_ref;
      //disabled
      CkeProxy& operator=(const CkeProxy&);
    };
    Register(const T& initial_value = T());
    T cur() const;
    void nxt(const T& val);
    NxtProxy nxt();
    void cke(bool val);
    CkeProxy cke();
    virtual void tick();
    virtual void reset();
  private:
    T m_cur, m_nxt, m_init;
    bool m_cke;
  };

  template <typename T>
  class InPort : public Port<T> {
  public:
    class Proxy {
    public:
      Proxy(InPort& ref);
      void operator=(const Port<T>& src);
    private:
      InPort& m_ref;
      //disabled
      Proxy& operator=(const Proxy&);
    };
    InPort();
    void connect(const Port<T>& src);
    Proxy src();
  protected:
    virtual T realget() const;
  private:
    const Port<T>* m_src;
  };

  template <typename T>
  class FwdOutPort : public OutPort<T> {
  public:
    class Proxy {
    public:
      Proxy(FwdOutPort& ref);
      void operator=(const Port<T>& src);
    private:
      FwdOutPort& m_ref;
      //disabled
      Proxy& operator=(const Proxy&);
    };
    FwdOutPort();
    void connect(const Port<T>& src);
    Proxy src();
  protected:
    virtual T realget() const;
  private:
    const Port<T>* m_src;
  };

  template <typename T>
  class RegOutPort : public OutPort<T> {
  public:
    RegOutPort(const T& initial_value = T());
    T cur() const;
    void nxt(const T& val);
    typename Register<T>::NxtProxy nxt();
    void cke(bool val);
    typename Register<T>::CkeProxy cke();
#ifdef CYCLEC_MULTICLOCK
    virtual void tick();
#endif
  protected:
    virtual T realget() const;
    virtual void realreset();
  private:
    Register<T> m_reg;
  };

  template <typename T>
  class RegRefOutPort : public OutPort<T> {
  public:
    RegRefOutPort(const Register<T>& ref);
  protected:
    virtual T realget() const;
  private:
    const Register<T>& m_ref;
  };

  template <typename C, typename T>
  class CombOutPortBase : public OutPort<T> {
  public:
    CombOutPortBase(const C& circuit, T (C::*getfunc)() const);
  protected:
    virtual T realget() const;
  private:
    const C& m_circuit;
    T (C::*m_getfunc)() const;
  };

  template <typename T>
  class VarOutPort : public OutPort<T> {
  public:
    VarOutPort(const T& initial_value = T());
    void set(const T& val);
    void operator=(const T& val);
  protected:
    virtual T realget() const;
    virtual void realreset();
  private:
    T m_var, m_init;
  };

  template <typename T>
  class VarRefOutPort : public OutPort<T> {
  public:
    VarRefOutPort(const T& ref);
  protected:
    virtual T realget() const;
  private:
    const T& m_ref;
  };

  Circuit();
  void evaluate();
  void update();
  void reset();
  void tick();
protected:
  virtual void evaluate_self();
  virtual void update_self();
  virtual void reset_self();
private:
  friend class Name;
  friend class ClockedObject;
  static std::stack<Circuit*>		the_circuit_hierarchy;
  static std::stack<Name*>		the_name_hierarchy;
  std::vector<ClockedObject*>		m_clockedobjects;
  std::vector<Circuit*>			m_subcircuits;

#ifdef CYCLEC_MULTICLOCK
public:
  typedef unsigned int clkmap_t;
  static clkmap_t the_active_clock_map;
  void assign_clock(int clknr);
protected:
  clkmap_t m_clockmap;
#endif

};

#define	HAS_COMBOUTPORT(name)						\
public:									\
  typedef name circuit_type;						\
  template <typename T> struct CombOutPort : CombOutPortBase<name,T> {	\
    CombOutPort(const name& circuit, T (name::*getfunc)() const)	\
      : CombOutPortBase<name,T>(circuit, getfunc) {}			\
  }

#define CombOut(port, func)	port(*this, &circuit_type::func)
#define	RegRefOut(port, reg)	port(reg)
#define	VarRefOut(port, var)	port(var)

#ifdef CYCLEC_MULTICLOCK
class TimeKeeper {
public:
  TimeKeeper();
  void define_clock(int id, unsigned int period, unsigned int start_time = 0);
  void reset();
  void advance();
  unsigned int time() const;
  unsigned int cycle(int clkid) const;
  Circuit::clkmap_t clockmap() const;
private:
  struct ClockInfo{
    int id;
    unsigned int period;
    unsigned int start_time;
    unsigned int rest;
    unsigned int cycle;
  };
  std::vector<ClockInfo>  m_clockinfo;
  unsigned int            m_time;
  Circuit::clkmap_t       m_clockmap;
};
#endif

//================================================================

inline Object::Object() throw() : m_parent(NULL) {}
inline Object::~Object() /*throw()*/ {}
inline const Object& Object::parent() const throw() { return *m_parent; }
inline void Object::set_myname(const std::string& name) { m_myname = name; }

template <typename T>
inline Circuit::Register<T>::Register(const T& initial_value)
  : m_cur(initial_value), m_nxt(initial_value),
    m_init(initial_value), m_cke(true) {}
template <typename T>
inline T Circuit::Register<T>::cur() const { return m_cur; }
template <typename T>
inline void Circuit::Register<T>::nxt(const T& val) { m_nxt = val; }
template <typename T>
inline typename Circuit::Register<T>::NxtProxy Circuit::Register<T>::nxt() {
  return NxtProxy(*this);
}
template <typename T>
inline void Circuit::Register<T>::cke(bool val) { m_cke = val; }
template <typename T>
inline typename Circuit::Register<T>::CkeProxy Circuit::Register<T>::cke() {
  return CkeProxy(*this);
}
template <typename T>
inline void Circuit::Register<T>::tick() {
  if (m_cke)  m_cur = m_nxt;
  m_cke = true;
}
template <typename T>
inline void Circuit::Register<T>::reset() {
  m_cur = m_init;
  m_nxt = m_init;
  m_cke = true;
}

template <typename T>
inline Circuit::Register<T>::NxtProxy::NxtProxy(Register& ref) : m_ref(ref) {}
template <typename T>
inline void Circuit::Register<T>::NxtProxy::operator=(const T& val) {
  m_ref.nxt(val);
}

template <typename T>
inline Circuit::Register<T>::CkeProxy::CkeProxy(Register& ref) : m_ref(ref) {}
template <typename T>
inline void Circuit::Register<T>::CkeProxy::operator=(bool val) {
  m_ref.cke(val);
}

template <typename T>
inline Port<T>::Port() : m_cached(false) {}
template <typename T>
inline void Port<T>::tick() { m_cached = false; }
template <typename T>
inline void Port<T>::reset() { m_cached = false;  realreset(); }
template <typename T>
inline void Port<T>::realreset() {}
template <typename T>
T Port<T>::get() const {
  if (! m_cached) {
    m_cachedvalue = realget();
    m_cached = true;
  }
  return m_cachedvalue;
}
template <typename T>
inline T Port<T>::operator()() const { return get(); }

template <typename T>
inline Circuit::InPort<T>::InPort() : m_src(NULL) {}
template <typename T>
inline void Circuit::InPort<T>::connect(const Port<T>& src) { m_src = &src; }
template <typename T>
inline T Circuit::InPort<T>::realget() const { return m_src->get(); }
template <typename T>
inline typename Circuit::InPort<T>::Proxy Circuit::InPort<T>::src() {
  return Proxy(*this);
}

template <typename T>
inline Circuit::InPort<T>::Proxy::Proxy(InPort<T>& ref) : m_ref(ref) {}
template <typename T>
inline void Circuit::InPort<T>::Proxy::operator=(const Port<T>& src) {
  m_ref.connect(src);
}

template <typename T>
inline Circuit::FwdOutPort<T>::FwdOutPort() : m_src(NULL) {}
template <typename T>
inline void Circuit::FwdOutPort<T>::connect(const Port<T>& src) {
  m_src = &src;
}
template <typename T>
inline T Circuit::FwdOutPort<T>::realget() const { return m_src->get(); }
template <typename T>
inline typename Circuit::FwdOutPort<T>::Proxy Circuit::FwdOutPort<T>::src() {
  return Proxy(*this);
}

template <typename T>
inline Circuit::FwdOutPort<T>::Proxy::Proxy(FwdOutPort<T>& ref) : m_ref(ref) {}
template <typename T>
inline void Circuit::FwdOutPort<T>::Proxy::operator=(const Port<T>& src) {
  m_ref.connect(src);
}

template <typename T>
inline Circuit::RegOutPort<T>::RegOutPort(const T& initial_value)
  : m_reg(initial_value) {}
template <typename T>
inline T Circuit::RegOutPort<T>::cur() const { return m_reg.cur(); }
template <typename T>
inline void Circuit::RegOutPort<T>::nxt(const T& val) { m_reg.nxt(val); }
template <typename T>
inline typename Circuit::Register<T>::NxtProxy Circuit::RegOutPort<T>::nxt() {
  return m_reg.nxt();
}
template <typename T>
inline void Circuit::RegOutPort<T>::cke(bool val) { m_reg.cke(val); }
template <typename T>
inline typename Circuit::Register<T>::CkeProxy Circuit::RegOutPort<T>::cke() {
  return m_reg.cke();
}
template <typename T>
inline T Circuit::RegOutPort<T>::realget() const { return m_reg.cur(); }
template <typename T>
inline void Circuit::RegOutPort<T>::realreset() { m_reg.reset(); }
#ifdef CYCLEC_MULTICLOCK
template <typename T>
inline void Circuit::RegOutPort<T>::tick() {
  OutPort<T>::tick();
  m_reg.tick();
}
#endif

template <typename T>
inline Circuit::RegRefOutPort<T>::RegRefOutPort(const Register<T>& ref)
  : m_ref(ref) {}
template <typename T>
inline T Circuit::RegRefOutPort<T>::realget() const { return m_ref.cur(); }

template <typename T>
inline Circuit::VarOutPort<T>::VarOutPort(const T& initial_value)
  : m_var(initial_value), m_init(initial_value) {}
template <typename T>
inline T Circuit::VarOutPort<T>::realget() const { return m_var; }
template <typename T>
inline void Circuit::VarOutPort<T>::realreset() { m_var = m_init; }

template <typename T>
inline void Circuit::VarOutPort<T>::set(const T& val) { m_var = val; }
template <typename T>
inline void Circuit::VarOutPort<T>::operator=(const T& val) { m_var = val; }

template <typename T>
inline Circuit::VarRefOutPort<T>::VarRefOutPort(const T& ref) : m_ref(ref) {}
template <typename T>
inline T Circuit::VarRefOutPort<T>::realget() const { return m_ref; }

template <typename C, typename T>
Circuit::CombOutPortBase<C,T>::CombOutPortBase(const C& circuit,
					       T (C::*getfunc)() const)
  : m_circuit(circuit), m_getfunc(getfunc) {}
template <typename C, typename T>
T Circuit::CombOutPortBase<C,T>::realget() const {
  return (m_circuit.*m_getfunc)();
}

inline void Circuit::tick() { evaluate(); update(); }
inline void Circuit::evaluate_self() {}
inline void Circuit::update_self() {}
inline void Circuit::reset_self() {}

} // namespace CycleC

#endif

