#pragma once

#include "stat_assert.h"

#include <cassert>
#include <cstring>

template <unsigned N, typename T>
unsigned copy_on_buffer(T const &value, char (&buffer)[N]) {
  static const unsigned valueSize = sizeof(value);
  stat_assert<valueSize <= N>("Not enough memory in the buffer");

  typedef char BufferForValue[valueSize];
  BufferForValue const &valueBuffer = reinterpret_cast<BufferForValue const &>(value);

  static const unsigned bytesToCopy = valueSize < N ? valueSize : N;
  std::memcpy(buffer, valueBuffer, bytesToCopy);
  return bytesToCopy;
}

/**
 *
 * Since I have no variadic template in C++98, I factor out as much as the
 * common functionality of Callback in this base class. If I could expand a
 * parameter pack, I could do all of this in Callback<>.
 */
class CallbackBase {
private:
  typedef void(DummySignature)();

  /**
   * \brief A function pointer type that I'll use as a reference.
   * \remark I am assuming that all the function pointers have the same length!
   */
  // typedef void (*FuncPtr)();
  typedef DummySignature *FuncPtr;
  typedef char FuncPtrBuffer[sizeof(FuncPtr)];

  /**
   * \brief A membef function pointer type that I'll use as a reference.
   *
   * \remark I am assuming that all the member function pointers have the same
   * length! This doesn't appear to be true on MSVC.
   */
  typedef DummySignature CallbackBase::*MemFuncPtr;
  typedef char MemFuncPtrBuffer[sizeof(MemFuncPtr)];

protected:
  explicit CallbackBase() //
      : _fAlign_(0), _callee(0) {}

  template <typename FPtr>
  explicit CallbackBase(FPtr *fptr) //
      : _fAlign_(0), _callee(0) {
    // Initialize the free-function pointer.
    copy_on_buffer(fptr, _f);
  }

  template <typename C, typename U, typename FPtr>
  explicit CallbackBase(C &callee, FPtr U::*memFptr) //
      : _mfAlign_(0), _callee(static_cast<void *>(&callee)) {
    // Initialize the mem-function pointer.
    copy_on_buffer(memFptr, _mf);
  }

  bool isSet() const {
    if (_callee) {
      assert(_fAlign_);
      return true;
    }

    return _fAlign_;
  }

  union {
    /** \brief The buffer for the function pointer. */
    FuncPtrBuffer _f;

    /** \brief A function pointer insure the union alignment. */
    FuncPtr _fAlign_;

    /** \brief The buffer for the member function pointer. */
    MemFuncPtrBuffer _mf;

    /** \brief A member function pointer insure the union alignment. */
    MemFuncPtr _mfAlign_ /* unused */;
  };

  /** \brief The target callee object (if any). */
  void *_callee;
};

template <typename> //
class Callback;

// template <typename> //
// struct CallbackTraits;
//
// template <typename SignatureT> //
// struct CallbackTraits<Callback<SignatureT> > {
//   /** \brief The type of the target signature. */
//   typedef SignatureT Signature;
//
//   /** \brief The type of The Callback<> object. */
//   typedef Callback<Signature> Self;
//
//   /**
//    * \brief The type of a member function pointer with the same signature as
//    * SignatureT as a member of this class.
//    *
//    * I'll use this type to reserve a buffer to store a member function pointer
//    * address, assuming that _every_ member function pointer has the same size.
//    */
//   typedef Signature Self::*MyMemPtrType;
//
//   /**
//    * \brief The type of a pointer to a free (or static member) function with
//    * the same signature as SignatureT.
//    */
//   typedef Signature *FunctionPtr;
//
//   /**
//    * \brief The type of the buffer for the member function pointer.
//    */
//   typedef char MemberPtrBuffer[sizeof(MyMemPtrType)];
// };

template <typename> //
struct Tag {
  /**\brief Ctor to avoid the most vexing parse. */
  Tag(int) {}
};

template <typename R, typename Arg0> //
class Callback<R(Arg0)> : protected CallbackBase {

  /** \brief The type of this Callback<> object. */
  typedef Callback<R(Arg0)> Self;

  template <typename CBReturnT, typename CalleeSignatureT>
  static CBReturnT call_free_func_impl(Self const *self, Arg0 arg, //
                                       Tag<CalleeSignatureT>, Tag<CBReturnT>) {
    CalleeSignatureT *const callee =
        reinterpret_cast<CalleeSignatureT *const &>(self->_f);
    return callee(arg);
  }

  template <typename CalleeSignatureT>
  static void call_free_func_impl(Self const *self, Arg0 arg, //
                                  Tag<CalleeSignatureT>, Tag<void>) {
    CalleeSignatureT *const callee =
        reinterpret_cast<CalleeSignatureT *const &>(self->_f);
    callee(arg);
  }

  template <typename CalleeSignatureT>
  static R call_free_func(Self const *self, Arg0 arg) {
    Tag<CalleeSignatureT> const calleeT(0);
    Tag<R> const returnT(0);
    return call_free_func_impl(self, arg, calleeT, returnT);
  }

  template <typename CalleeT, typename CBReturnT, typename CalleeMemFunT>
  static CBReturnT call_mem_func_impl(Self const *self, Arg0 arg, //
                                      Tag<CalleeT>,               //
                                      Tag<CalleeMemFunT>,         //
                                      Tag<CBReturnT>) {
    CalleeMemFunT const calleeMemFun =
        reinterpret_cast<CalleeMemFunT const &>(self->_mf);
    CalleeT *const callee = reinterpret_cast<CalleeT *>(self->_callee);
    return (callee->*calleeMemFun)(arg);
  }

  template <typename CalleeT, typename CalleeMemFunT>
  static void call_mem_func_impl(Self const *self, Arg0 arg, //
                                 Tag<CalleeT>,               //
                                 Tag<CalleeMemFunT>,         //
                                 Tag<void>) {
    CalleeMemFunT const calleeMemFun =
        reinterpret_cast<CalleeMemFunT const &>(self->_mf);
    CalleeT *const callee = reinterpret_cast<CalleeT *>(self->_callee);
    (callee->*calleeMemFun)(arg);
  }

  template <typename C, typename CalleeMemFunT>
  static R call_mem_func(Self const *self, Arg0 arg) {
    Tag<C> const calleeT(0);
    Tag<CalleeMemFunT> const calleeMemFunT(0);
    Tag<R> const returnT(0);
    return call_mem_func_impl(self, arg, calleeT, calleeMemFunT, returnT);
  }

public:
  explicit Callback() //
      : CallbackBase(), _thunk(0) {}

  template <typename RR, typename AArgs>
  explicit Callback(RR (*f)(AArgs)) //
      : CallbackBase(f), _thunk(call_free_func<RR(AArgs)>) {}

  template <typename C, typename RR, typename U, typename AArgs>
  explicit Callback(C &obj, RR (U::*mf)(AArgs)) //
      : CallbackBase(obj, mf), _thunk(call_mem_func<C, RR (U::*)(AArgs)>) {}

  template <typename C, typename RR, typename U, typename AArgs>
  explicit Callback(C &obj, RR (U::*mf)(AArgs) const) //
      : CallbackBase(obj, mf),
        _thunk(call_mem_func<C const, RR (U::*)(AArgs) const>) {}

  R operator()(Arg0 args) const { return _thunk(this, args); }
  using CallbackBase::isSet;

private:
  R (*_thunk)(Self const *, Arg0);
};

// template <typename Arg0> //
// class Callback<void(Arg0)> : protected CallbackBase {
//
//   /** \brief The type of this Callback<> object. */
//   typedef Callback<void(Arg0)> Self;
//
//   template <typename RR, typename AArgs>
//   static void call_free_func(Self const *self, Arg0 arg) {
//     typedef RR(ActualFuncType)(AArgs);
//     reinterpret_cast<ActualFuncType *const &>(self->_f)(arg);
//   }
//
//   template <typename C, typename RR, typename U, typename AArgs>
//   static void call_mem_func(Self const *self, Arg0 arg) {
//     C &callee = *static_cast<C *>(self->_callee);
//
//     typedef RR(ActualFuncType)(AArgs);
//     ActualFuncType U::*memFun =
//         reinterpret_cast<ActualFuncType U::*const &>(self->_mf);
//
//     (callee.*memFun)(arg);
//   }
//
//   template <typename C, typename RR, typename U, typename AArgs>
//   static void call_const_mem_func(Self const *self, Arg0 args) {
//     C const &callee = *static_cast<C const *>(self->_callee);
//
//     typedef void(ActualFuncType)(AArgs) const;
//     ActualFuncType U::*memFun =
//         reinterpret_cast<ActualFuncType U::*const &>(self->_mf);
//
//     (callee.*memFun)(args);
//   }
//
// public:
//   explicit Callback() //
//       : CallbackBase(), _thunk(0) {}
//
//   template <typename RR, typename AArgs>
//   explicit Callback(RR (*f)(AArgs)) //
//       : CallbackBase(f), _thunk(call_free_func<RR, AArgs>) {}
//
//   template <typename C, typename RR, typename U, typename AArgs>
//   explicit Callback(C &obj, RR (U::*mf)(AArgs)) //
//       : CallbackBase(obj, mf), _thunk(call_mem_func<C, RR, U, AArgs>) {}
//
//   template <typename C, typename RR, typename U, typename AArgs>
//   explicit Callback(C const &obj, RR (U::*mf)(AArgs)) //
//       : CallbackBase(obj, mf), _thunk(call_const_mem_func<C, RR, U, AArgs>) {}
//
//   void operator()(Arg0 args) const { return _thunk(this, args); }
//   using CallbackBase::isSet;
//
// private:
//   void (*_thunk)(Self const *, Arg0);
// };

// template <typename R, typename Arg0> //
// class Callback<R(Arg0)> {
//
//   /** \brief The type traits of this class */
//   typedef CallbackTraits<Callback<R(Arg0)> > Traits;
//
//   template <typename RR, typename AArgs>
//   static R call_free_func(typename Traits::Self const *self, Arg0 arg) {
//     typedef RR(ActualFuncType)(AArgs);
//     return static_cast<ActualFuncType *>(self->_f)(arg);
//   }
//
//   template <typename C, typename RR, typename U, typename AArgs>
//   static R call_mem_func(typename Traits::Self const *self, Arg0 arg) {
//     C &callee = *static_cast<C *>(self->_callee);
//
//     typedef RR(ActualFuncType)(AArgs);
//     ActualFuncType U::*memFun =
//         reinterpret_cast<ActualFuncType U::*const &>(self->_mf);
//
//     return (callee.*memFun)(arg);
//   }
//
//   template <typename C, typename RR, typename U, typename AArgs>
//   static R call_const_mem_func(typename Traits::Self const *self, Arg0 args) {
//     C const &callee = *static_cast<C *>(self->_callee);
//
//     typedef RR(ActualFuncType)(AArgs) const;
//     ActualFuncType U::*memFun =
//         reinterpret_cast<ActualFuncType U::*const &>(self->_mf);
//
//     return (callee.*memFun)(args);
//   }
//
// public:
//   template <typename RR, typename AArgs>
//   explicit Callback(RR (*f)(AArgs)) //
//       : _thunk(call_free_func<RR, AArgs>), _f(0), _callee(0) {
//     copy_on_buffer(f, _f);
//   }
//
//   template <typename C, typename RR, typename U, typename AArgs>
//   explicit Callback(C &obj, RR (U::*mf)(AArgs)) //
//       : _thunk(call_mem_func<C, RR, U, AArgs>), _callee(&obj) {
//     copy_on_buffer(mf, _mf);
//   }
//
//   // template <typename C, typename RR, typename U, typename  AArgs>
//   // explicit Callback(C const &obj, RR (U::*mf)(AArgs ) const) //
//   //     : _thunk(call_const_mem_func<C, RR, U, AArgs >), _callee(&obj)
//   //     {
//   //   stat_assert<sizeof(mf) <= sizeof(_mf)>("Not enough memory in _mf");
//   //   std::memcpy(_mf, (char const *)mf, sizeof(mf));
//   // }
//
//   R operator()(Arg0 args) const { return _thunk(this, args); }
//
// private:
//   R (*_thunk)(typename Traits::Self const *, Arg0);
//
//   union {
//     /** \brief The function pointer. */
//     typename Traits::FunctionPtr _f;
//
//     /** \brief The member function pointer. */
//     typename Traits::MemberPtrBuffer _mf;
//
//     // For alignment
//     typename Traits::MemberPtrBuffer _;
//   };
//
//   /** \brief The target callee object (if any). */
//   void *_callee;
// };
