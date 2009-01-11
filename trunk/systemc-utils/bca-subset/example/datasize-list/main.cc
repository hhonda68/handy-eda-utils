#include <iostream>
#include <systemc>
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_core::sc_module;
using sc_dt::sc_int;
using sc_dt::sc_uint;

#define P(x)  do { ::std::cout << "sizeof(" #x ") = " << sizeof(x) << '\n'; } while (0)

int main()
{
  typedef sc_int<32>  s32_t;
  typedef sc_int<64>  s64_t;
  typedef sc_uint<32> u32_t;
  typedef sc_uint<64> u64_t;
  P(s32_t);
  P(s64_t);
  P(u32_t);
  P(u64_t);
  P(sc_module);  // Note: BCA-subset uses pimpl idiom which makes sizeof(sc_module) somewhat meaningless.
  P(sc_signal<bool>);
  P(sc_signal<int>);
  P(sc_signal<long long>);
  P(sc_signal<u32_t>);
  P(sc_signal<u64_t>);
  P(sc_in<bool>);
  P(sc_in<int>);
  P(sc_in<long long>);
  P(sc_in<u32_t>);
  P(sc_in<u64_t>);
  P(sc_out<bool>);
  P(sc_out<int>);
  P(sc_out<long long>);
  P(sc_out<u32_t>);
  P(sc_out<u64_t>);
  return 0;
}
