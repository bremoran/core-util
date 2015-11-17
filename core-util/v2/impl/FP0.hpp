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
#ifndef __FUNCTIONPOINTER_IMPL_FP0_HPP__
#define __FUNCTIONPOINTER_IMPL_FP0_HPP__
#ifndef __FUNCTIONPOINTER_HPP__
#error FP0.hpp is an internal file that should only be called from inside FunctionPointer.hpp
#endif

template<typename R, size_t I>
class FunctionPointer<R(), I> {
public:
    typedef struct arg_struct{
    } ArgStruct;
    typedef R(*static_fp)(void);

    FunctionPointer(R (*fp)()) {
        new(_storage) impl::StaticPointer<R(), I>(fp);
    }
    template <typename C>
    FunctionPointer(C *c, R (C::*member)()) {
        new(_storage) impl::MethodPointer<R(), C, I>(c, member);
    }
    template <typename F>
    FunctionPointer(const F & f) {
        new(_storage) impl::FunctorPointer<R(), F, I>(f);
    }
    operator bool() const {
        return *reinterpret_cast<impl::FunctionPointerInterface<R()> *>(&_storage);
    }
    bool operator ==(const FunctionPointer<R()> & rhs) const {
        return *reinterpret_cast<impl::FunctionPointerInterface<R()> *>(&_storage) ==
            *reinterpret_cast<impl::FunctionPointerInterface<R()> *>(rhs.storage);
    }
    FunctionPointer<R()> & operator=(const FunctionPointer<R()> & rhs) {
        reinterpret_cast<impl::FunctionPointerInterface<R()>*>(_storage)->~FunctionPointerInterface<R()>();
        rhs.copy_to(_storage);
        return *this;
    }
    inline R operator ()() const {
        return call();
    }
    R call() const {
        return reinterpret_cast<impl::FunctionPointerInterface<R()>*>(_storage)->call();
    }
    ~FunctionPointer() {
        reinterpret_cast<impl::FunctionPointerInterface<R()>*>(_storage)->~FunctionPointerInterface<R()>();
    }
protected:
    union {
        char _storage[sizeof(impl::FunctionPointerSize<I>)];
        impl::FunctionPointerSize<I> _alignementAndSizeGuarantees;
    };
};

namespace impl {
template <typename R, size_t I>
class FunctionPointerInterface<R(),I>{
public:
    virtual operator bool() const = 0;
    virtual bool operator ==(const FunctionPointerInterface &) const = 0;
    virtual R call(void) const = 0;
    virtual ~FunctionPointerInterface(){};
    virtual void copy_to(void *) const = 0;
protected:
    FunctionPointerStorage<I> _fp;
};

/** A class for storing and calling a pointer to a static or member void function without arguments
 */
template<typename R, size_t I>
class StaticPointer<R(), I> : public FunctionPointerInterface<R(), I>{
public:
    StaticPointer(R (*function)() = 0)
    {
        attach(function);
    }
    StaticPointer(const StaticPointer& fp) {
        *this = fp;
    }
    void attach(R (*function)()) {
        this->_fp._static_fp = (void*)function;
    }
    R call() const{
        return reinterpret_cast<R (*)()>(this->_fp._static_fp)();
    }
    R (*get_function() const)() {
        return this->_fp._static_fp;
    }
    operator bool() const {
        return (this->_fp._static_fp != NULL);
    }
    bool operator==(const FunctionPointerInterface<R()> & rhs) const {
        return (this->_fp._static_fp == static_cast<const StaticPointer *>(&rhs)->_fp._static_fp);
    }
    StaticPointer & operator = (const StaticPointer & rhs) {
        this->_fp._static_fp = rhs._fp._static_fp;
        return *this;
    }
    void copy_to(void * storage) const {
        new(storage) StaticPointer(*this);
    }
};

template<typename R, typename C, size_t I>
class MethodPointer<R(), C, I> : public FunctionPointerInterface<R(), I>{
public:
    MethodPointer(C *object, R (C::*member)(void))
    {
        attach(object, member);
    }
    MethodPointer(const MethodPointer& fp) {
        *this = fp;
    }
    void attach(C *object, R (C::*member)(void)) {
        this->_fp._method._object = (void *) object;
        *reinterpret_cast<R (C::**)(void)>(this->_fp._method._member) = member;
    }
    R call() const{
        C* o = static_cast<C*>(this->_fp._method._object);
        R (C::**m)(void) = reinterpret_cast<R (C::**)(void)>(const_cast<char *>(this->_fp._method._member));
        return (o->**m)();
    }
    R (*get_function() const)(){
        return NULL;
    }
    operator bool() const {
        return (this->_fp._method._object != NULL);
    }
    bool operator ==(const FunctionPointerInterface<R()> & rhs) const {
        return (this->_fp._method._object == static_cast<const MethodPointer *>(&rhs)->_fp._method._object) &&
            (0 == memcmp(this->_fp._method._member, static_cast<const MethodPointer *>(&rhs)->_fp._method._member, sizeof(this->_fp._method._member)));
    }
    MethodPointer & operator = (const MethodPointer & rhs) {
        this->_fp._method._object = rhs._fp._method._object;
        memcpy(this->_fp._method._member, rhs._fp._method._member, sizeof(this->_fp._method._member));
        return *this;
    }
    void copy_to(void * storage) const {
        new(storage) MethodPointer(*this);
    }
};



template<typename R, typename F, size_t I>
class FunctorPointer<R(), F, I> : public FunctionPointerInterface<R(), I>{
    constexpr static const bool Internal = sizeof(F) <= sizeof(impl::FunctionPointerStorage<I>);
public:
    FunctorPointer(const F & f)
    {
        attach(f);
    }
    FunctorPointer(const FunctorPointer & fp)
    {
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
    R call() const{
        return const_ref()();
    }
    R (*get_function() const)(){
        return NULL;
    }
    operator bool() const {
        return true;
    }
    bool operator ==(const FunctionPointerInterface<R(), I> & rhs) const {
        return false;
    }
    FunctorPointer & operator = (const FunctorPointer & rhs) {
        deallocate();
        attach(*reinterpret_cast<const F *>(rhs._fp._raw_storage));
        return *this;
    }
    ~FunctorPointer() {
        deallocate();
    }
    void copy_to(void * storage) const {
        new(storage) FunctorPointer(*this);
    }
protected:
    template <typename T = F>
    typename std::enable_if<Internal && std::is_same<T,F>::value, const F &>::type
    const_ref() const{
        return *reinterpret_cast<const F *>(this->_fp._raw_storage);
    }
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
    typename std::enable_if<!Internal && std::is_same<T,F>::value, const F &>::type
    const_ref() const{
        return *reinterpret_cast<const F *>(this->_fp._external_functor);
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
