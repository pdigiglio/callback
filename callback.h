#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <bitset>
#include <iostream>

#define Ellipsis

template <bool> //
struct stat_assert;

template <> //
struct stat_assert<true> {
  explicit stat_assert(char const *) {}
};



template <typename> //
class Callback;

template <typename R, typename Ellipsis Args> //
class Callback<R(Args Ellipsis)> {

  /** \brief The type of the target signature. */
  typedef R(SignatureT)(Args Ellipsis);

  /** \brief The type of this Callback<> object. */
  typedef Callback<SignatureT> SelfT;

  /**
   * \brief The type of a member function pointer with the same signature as
   * SignatureT as a member of this class.
   *
   * I'll use this type to reserve a buffer to store a member function pointer
   * address, assuming that _every_ member function pointer has the same size.
   */
  typedef SignatureT SelfT::*MyMemPtrType;

  /**
   * \brief The type of a pointer to a free (or static member) function with
   * the same signature as SignatureT.
   */
  typedef SignatureT *FunctionPtrT;

  /**
   * \brief The type of the buffer for the member function pointer.
   */
  typedef char MemberPtrBufferT[sizeof(MyMemPtrType)];

  template <typename RR, typename Ellipsis AArgs>
  static R call_free_func(SelfT const *self, Args Ellipsis args) {
    typedef RR(ActualFuncType)(AArgs Ellipsis);
    return static_cast<ActualFuncType *>(self->_f)(args Ellipsis);
  }

  template <typename C, typename RR, typename U, typename Ellipsis AArgs>
  static R call_mem_func(SelfT const *self, Args Ellipsis args) {
    std::puts("mem");
    C &callee = *static_cast<C *>(self->_callee);

    typedef RR(ActualFuncType)(AArgs Ellipsis);
    ActualFuncType U::*memFun =
        reinterpret_cast<ActualFuncType U::*const &>(self->_mf);

    return (callee.*memFun)(args Ellipsis);
  }

  template <typename C, typename RR, typename U, typename Ellipsis AArgs>
  static R call_const_mem_func(SelfT const *self, Args Ellipsis args) {
    C const &callee = *static_cast<C *>(self->_callee);

    typedef RR(ActualFuncType)(AArgs Ellipsis) const;
    ActualFuncType U::*memFun =
        reinterpret_cast<ActualFuncType U::*const &>(self->_mf);

    return (callee.*memFun)(args Ellipsis);
  }

public:
  template <typename RR, typename Ellipsis AArgs>
  explicit Callback(RR (*f)(AArgs Ellipsis)) //
      : _thunk(call_free_func<RR, AArgs Ellipsis>), _f(f), _callee(0) {}

  template <typename C, typename RR, typename U, typename Ellipsis AArgs>
  explicit Callback(C &obj, RR (U::*mf)(AArgs Ellipsis)) //
      : _thunk(call_mem_func<C, RR, U, AArgs Ellipsis>), _callee(&obj) {
    static const int mf_size = sizeof(mf);
    stat_assert<mf_size <= sizeof(_mf)>("Not enough memory in _mf");
    std::memcpy(_mf, reinterpret_cast<char const(&)[mf_size]>(mf), mf_size);
  }

  // template <typename C, typename RR, typename U, typename Ellipsis AArgs>
  // explicit Callback(C const &obj, RR (U::*mf)(AArgs Ellipsis) const) //
  //     : _thunk(call_const_mem_func<C, RR, U, AArgs Ellipsis>), _callee(&obj)
  //     {
  //   stat_assert<sizeof(mf) <= sizeof(_mf)>("Not enough memory in _mf");
  //   std::memcpy(_mf, (char const *)mf, sizeof(mf));
  // }

  R operator()(Args Ellipsis args) const { return _thunk(this, args Ellipsis); }

private:
  R (*_thunk)(SelfT const *, Args Ellipsis);

  union {
    /** \brief The function pointer. */
    FunctionPtrT _f;
    /** \brief The member function pointer. */
    MemberPtrBufferT _mf;

    // For alignment
    MemberPtrBufferT _;
  };

  /** \brief The target callee object (if any). */
  void *_callee;
};
