/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_FUNCTIONPOINTER_H
#define MBED_FUNCTIONPOINTER_H

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <new>
#include "FunctionPointerBase.h"
#include "FunctionPointerBind.h"

namespace mbed {
namespace util {

template <typename R, typename... Args>
class FunctionPointerV : public FunctionPointerBase<R> {
protected:
    template <class... Ts> struct tuple {};

    template <class T, class... Ts>
    struct tuple<T, Ts...> : tuple<Ts...> {
        tuple(T& t, Ts... ts) : tuple<Ts...>(ts...), tail(t) {}
        tuple(const struct tuple<T, Ts...>& t) : tuple<Ts...>(t), tail(t.tail) {}

      T tail;
    };
    typedef struct tuple<Args...> ArgStruct;

public:
    typedef R(*static_fp)(Args...);
    /** Create a FunctionPointer, attaching a static function
     *
     *  @param function The void static function to attach (default is none)
     */
    FunctionPointerV(static_fp function = 0) {
        attach(function);
    }

    /** Create a FunctionPointer, attaching a member function
     *
     *  @param object The object pointer to invoke the member function on (i.e. the this pointer)
     *  @param function The address of the void member function to attach
     */
    template<typename T>
    FunctionPointerV(T *object, R (T::*member)(Args...)) {
        attach(object, member);
    }

    /** Attach a static function
     *
     *  @param function The void static function to attach (default is none)
     */
    void attach(static_fp function) {
        FunctionPointerBase<R>::_object = reinterpret_cast<void*>(function);
        FunctionPointerBase<R>::_membercaller = &FunctionPointerV::staticcaller;
    }

    /** Attach a member function
     *
     *  @param object The object pointer to invoke the member function on (i.e. the this pointer)
     *  @param function The address of the void member function to attach
     */
    template<typename T>
    void attach(T *object, R (T::*member)(Args...))
    {
        FunctionPointerBase<R>::_object = static_cast<void*>(object);
        *reinterpret_cast<R (T::**)(Args...)>(FunctionPointerBase<R>::_member) = member;
        FunctionPointerBase<R>::_membercaller = &FunctionPointerV::membercaller<T>;
    }

    FunctionPointerBind<R> bind(Args... args) {
        FunctionPointerBind<R> fp;
        fp.bind(&_fpv_ops, (ArgStruct *) NULL, this, std::forward<Args>(args)...);
        return fp;
    }


    /** Call the attached static or member function
     */
    R call(Args... args)
    {
        ArgStruct as(args...);
        // return (R)0;
        return FunctionPointerBase<R>::call(&as);
    }
//
//     static_fp get_function()const
//     {
//         return reinterpret_cast<static_fp>(FunctionPointerBase<R>::_object);
//     }
//
    R operator ()(Args... args) {
        return call(std::forward<Args>(args)...);
    }

private:
    template <typename T, class... FArgs>
    static R membercaller(T* o, R (T::**m)(Args...), struct tuple<>* t, FArgs... args) {
        return (o->**m)(std::forward<FArgs>(args)...);
    }
    template <typename T, class TA, class... TArgs, class... FArgs>
    static R membercaller(T* o, R (T::**m)(Args...), struct tuple<TA, TArgs...>* t, FArgs... args) {
        return membercaller(o, m, static_cast<struct tuple<TArgs...>* >(t), std::forward<FArgs>(args)..., t->tail);
    }

    template<typename T>
    static R membercaller(void *object, uintptr_t *member, void *arg) {
        ArgStruct *as = static_cast<ArgStruct *>(arg);
        T* o = static_cast<T*>(object);
        R (T::**m)(Args...) = reinterpret_cast<R (T::**)(Args...)>(member);
        return membercaller(o,m,as);
    }

    template <class... FArgs>
    static R staticcaller(static_fp f, struct tuple<>* t, FArgs... args) {
        return f(std::forward<FArgs>(args)...);
    }
    template <class TA, class... TArgs, class... FArgs>
    static R staticcaller(static_fp f, struct tuple<TA, TArgs...>* t, FArgs... args) {
        return staticcaller(f, static_cast<struct tuple<TArgs...>* >(t), std::forward<FArgs>(args)..., t->tail);
    }

    static R staticcaller(void *object, uintptr_t *member, void *arg) {
        ArgStruct *as = static_cast<ArgStruct *>(arg);
        (void) member;
        static_fp f = reinterpret_cast<static_fp>(object);
        return staticcaller(f,as);
    }


    template <class... FArgs>
    static void constructor(void *dest, va_list vargs, struct tuple<>* t, FArgs... args) {
        new(dest) ArgStruct(args...);
    }
    template <class TA, class... TArgs, class... FArgs>
    static void constructor(void *dest, va_list vargs, struct tuple<TA, TArgs...>* t, FArgs... args) {
        constructor(dest, vargs, static_cast<struct tuple<TArgs...>* >(t), std::forward<FArgs>(args)..., va_arg(vargs, TA));
    }

    static void constructor(void * dest, va_list args) {
        constructor(dest, args, (ArgStruct *)NULL);
    }
    static void copy_constructor(void *dest , void* src) {
        ArgStruct *src_args = static_cast<ArgStruct *>(src);
        new(dest) ArgStruct(*src_args);
    }
    static void destructor(void *args) {
        ArgStruct *argstruct = static_cast<ArgStruct *>(args);
        argstruct->~ArgStruct();
    }

protected:
    static const struct FunctionPointerBase<R>::ArgOps _fpv_ops;
};
//
template <typename R, typename... Args>
const struct FunctionPointerBase<R>::ArgOps FunctionPointerV<R,Args...>::_fpv_ops = {
    FunctionPointerV<R,Args...>::constructor,
    FunctionPointerV<R,Args...>::copy_constructor,
    FunctionPointerV<R,Args...>::destructor
};

template <typename R>
using FunctionPointer0 = FunctionPointerV<R>;
template <typename R, typename A1>
using FunctionPointer1 = FunctionPointerV<R,A1>;
template <typename R, typename A1, typename A2>
using FunctionPointer2 = FunctionPointerV<R,A1,A2>;
template <typename R, typename A1, typename A2, typename A3>
using FunctionPointer3 = FunctionPointerV<R,A1,A2,A3>;
template <typename R, typename A1, typename A2, typename A3, typename A4>
using FunctionPointer4 = FunctionPointerV<R,A1,A2,A3,A4>;

typedef FunctionPointer0<void> FunctionPointer;
//typedef FunctionPointer1<void, int> event_callback_t;

} // namespace util
} // namespace mbed

#endif
