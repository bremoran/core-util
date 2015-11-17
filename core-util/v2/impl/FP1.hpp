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
#ifndef __FUNCTIONPOINTER_IMPL_FP1_HPP__
#define __FUNCTIONPOINTER_IMPL_FP1_HPP__
#ifndef __FUNCTIONPOINTER_HPP__
#error FP1.hpp is an internal file that should only be called from inside FunctionPointer.hpp
#endif

template <typename R, typename A1, size_t I>
class FunctionPointer<R(A1), I> {
public:
    FunctionPointer(const FunctionPointer &fp) {
        fp.const_ref().copy_to(_storage);
    }
    typedef R(*static_fp)(A1);
    FunctionPointer(R (*fp)(A1)) {
        new(_storage) impl::StaticPointer<R(A1), I>(fp);
    }
    template <typename C>
    FunctionPointer(C *c, R (C::*member)(A1)) {
        new(_storage) impl::MethodPointer<R(A1), C, I>(c, member);
    }
    template <typename F>
    FunctionPointer(const F & f) {
        new(_storage) impl::FunctorPointer<R(A1), F, I>(f);
    }
    operator bool() const {
        return const_ref();
    }
    bool operator ==(const impl::FunctionPointerInterface<R(A1)> & rhs) const {
        return const_ref() == rhs;
    }
    inline R operator ()(const A1&& a1) const {
        return call(impl::forward<const A1&&>(a1));
    }
    R call(const A1&& a1) const {
        return const_ref().call(impl::forward<const A1&&>(a1));
    }
    FPFunctor<R(), R(A1), I> bind(const A1 &a1) const {
        return FPFunctor<R(), R(A1), I>(*this, a1);
    }
    ~FunctionPointer() {
        (&ref())->~FunctionPointerInterface<R(A1)>();
    }
protected:
    union {
        char _storage[sizeof(impl::FunctionPointerSize<I>)];
        impl::FunctionPointerSize<I> _alignementAndSizeGuarantees;
    };
    impl::FunctionPointerInterface<R(A1), I> & ref() {
        return *reinterpret_cast<impl::FunctionPointerInterface<R(A1), I>*>(_storage);
    }
    const impl::FunctionPointerInterface<R(A1), I> & const_ref() const {
        return *reinterpret_cast<const impl::FunctionPointerInterface<R(A1), I>*>(_storage);
    }
};

template <typename R, typename A1, size_t I>
class FPFunctor<R(), R(A1), I> {
public:
    FPFunctor(const FunctionPointer<R(A1), I> & fp, const A1 & a1) : _fp(fp), _a1(a1) {}
    FPFunctor(const FPFunctor &other) : _fp(other._fp),_a1(other._a1) {}
    R operator ()() const {
        return _fp.call(impl::forward<const A1 &&>(_a1));
    }
protected:
    FunctionPointer<R(A1), I> _fp;
    A1 _a1;
};

namespace impl {
template <typename R, typename A1, size_t I>
class FunctionPointerInterface<R(A1), I> {
public:
    virtual operator bool() const = 0;
    virtual bool operator ==(const FunctionPointerInterface &) const = 0;
    virtual R call(const A1&&) const = 0;
    virtual ~FunctionPointerInterface(){};
    virtual void copy_to(void *) const = 0;
protected:
    impl::FunctionPointerStorage<I> _fp;
};


/** A class for storing and calling a pointer to a static or member void function without arguments
 */
template <typename R, typename A1>
class StaticPointer<R(A1)> : public FunctionPointerInterface<R(A1)>{
public:
    StaticPointer(R (*function)(A1) = 0)
    {
        attach(function);
    }
    StaticPointer(const StaticPointer& fp) {
        *this = fp;
    }
    void attach(R (*function)(A1)) {
        this->_fp._static_fp = (void*)function;
    }
    template <size_t I>
    FunctionPointer<R(), I> bind (A1&& a1) const {
        return FunctionPointer<R(), I>(StaticFunctor(get_function(),forward(a1)));
    }
    R call(const A1&& a1) const{
        R (*f)(A1) = reinterpret_cast<R (*)(A1)>(this->_fp._static_fp);
        return f(A1(forward<const A1 &>(a1)));
    }
    R (*get_function() const)(A1) {
        return this->_fp._static_fp;
    }
    operator bool() const {
        return (this->_fp._static_fp != NULL);
    }
    bool operator==(const FunctionPointerInterface<R(A1)> & rhs) const {
        return (this->_fp._static_fp == static_cast<const StaticPointer *>(&rhs)->_fp._static_fp);
    }
    FunctionPointerInterface<R(A1)> & operator = (const FunctionPointerInterface<R(A1)> & rhs) {
        return *this = static_cast<const StaticPointer &>(rhs);
    }
    StaticPointer & operator = (const StaticPointer & rhs) {
        this->_fp._static_fp = rhs._fp._static_fp;
        return *this;
    }
    void copy_to(void * storage) const {
        new(storage) StaticPointer(*this);
    }
};


template<typename R, typename A1, typename C>
class MethodPointer<R(A1), C> : public FunctionPointerInterface<R(A1)>{
public:
    MethodPointer(C *object, R (C::*member)(A1))
    {
        attach(object, member);
    }
    MethodPointer(const MethodPointer& fp) {
        MethodPointer::operator=(fp);
    }
    void attach(C *object, R (C::*member)(A1)) {
        this->_fp._method._object = (void *) object;
        *reinterpret_cast<R (C::**)(A1)>(this->_fp._method._member) = member;
    }
    template <size_t I>
    FunctionPointer<R(), I> bind (A1&& a1) const {
        return FunctionPointer<R(), I>();
    }
    R call(const A1&& a1) const{
        C* o = static_cast<C*>(this->_fp._method._object);
        R (C::**m)(A1) = reinterpret_cast<R (C::**)(A1)>(const_cast<char *>(this->_fp._method._member));
        return (o->**m)(A1(forward<const A1 &>(a1)));
    }
    R (*get_function() const)(){
        return NULL;
    }
    operator bool() const {
        return (this->_fp._method._object != NULL);
    }
    bool operator ==(const FunctionPointerInterface<R(A1)> & rhs) const {
        return (this->_fp._method._object == static_cast<const MethodPointer *>(&rhs)->_fp._method._object) &&
            (0 == memcmp(this->_fp._method._member, static_cast<const MethodPointer *>(&rhs)->_fp._method._member, sizeof(this->_fp._method._member)));
    }
    MethodPointer & operator = (const MethodPointer & rhs) {
        this->_fp._method._object = rhs._fp._method._object;
        memcpy(this->_fp._method._member, rhs._fp._method._member, sizeof(this->_fp._method._member));
        return *this;
    }
    FunctionPointerInterface<R(A1)> & operator = (const FunctionPointerInterface<R(A1)> & rhs) {
        return *this = *static_cast<const MethodPointer *>(&rhs);
    }
    void copy_to(void * storage) const {
        new(storage) MethodPointer(*this);
    }
};

template<typename R, typename A1, typename F, size_t I>
class FunctorPointer<R(A1), F, I> : public FunctionPointerInterface<R(A1), I>{
    constexpr static const bool Internal = sizeof(F) <= sizeof(impl::FunctionPointerStorage<I>);
public:
    FunctorPointer(const F & f)
    {
        attach(f);
    }
    FunctorPointer(const FunctorPointer & fp) {
        *this = fp;
    }
    template <typename T = F>
    typename std::enable_if<Internal && std::is_same<T,F>::value>::type
    attach(const F & f)
    {
        new(this->_fp._raw_storage)F(f);
    }
    template <typename T = F>
    typename std::enable_if<!Internal && std::is_same<T,F>::value>::type
    attach(const F & f) {
        this->_fp._external_functor = new F(f);
    }
    R call(const A1&& a1) const{
        return const_ref()(A1(forward<const A1 &>(a1)));
    }
    R (*get_function() const)(){
        return NULL;
    }
    operator bool() const {
        return true;
    }
    bool operator ==(const FunctionPointerInterface<R(A1), I> & rhs) const {
        return false;
    }
    FunctionPointerInterface<R(A1), I> & operator = (const FunctionPointerInterface<R(A1), I> & rhs) const {
        return *this = *static_cast<const FunctorPointer *>(&rhs);
    }
    FunctorPointer & operator = (const FunctorPointer & rhs) {
        deallocate();
        attach(rhs.const_ref());
        return *this;
    }
    void copy_to(void * storage) const {
        new(storage) FunctorPointer(*this);
    }
    ~FunctorPointer() {
        deallocate();
    }
    template <typename T = F>
    typename std::enable_if<Internal && std::is_same<T,F>::value, const F &>::type
    const_ref() const{
        return *reinterpret_cast<const F *>(this->_fp._raw_storage);
    }
    template <typename T = F>
    typename std::enable_if<!Internal && std::is_same<T,F>::value, const F &>::type
    const_ref() const{
        return *reinterpret_cast<const F *>(this->_fp._external_functor);
    }

protected:
    template <typename T = F>
    typename std::enable_if<Internal && std::is_same<T,F>::value, F &>::type
    ref() {
        return *reinterpret_cast<F *>(this->_fp._raw_storage);
    }
    template <typename T = F>
    typename std::enable_if<Internal && std::is_same<T,F>::value>::type
    deallocate() {
        reinterpret_cast<F*>(this->_fp._raw_storage)->~F();
    }

    template <typename T = F>
    typename std::enable_if<!Internal && std::is_same<T,F>::value, F &>::type
    ref() {
        return *reinterpret_cast<F *>(this->_fp._external_functor);
    }
    template <typename T = F>
    typename std::enable_if<!Internal && std::is_same<T,F>::value>::type
    deallocate() {
        reinterpret_cast<F*>(this->_fp._external_functor)->~F();
    }
};
}
#endif
