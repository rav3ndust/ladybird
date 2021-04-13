/*
 * Copyright (c) 2021, the SerenityOS developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <AK/Optional.h>
#include <AK/StdLibExtras.h>
#include <LibJS/Runtime/VM.h>
#include <LibWeb/Bindings/DOMExceptionWrapper.h>
#include <LibWeb/DOM/ExceptionOr.h>

namespace Web::Bindings {

template<typename>
constexpr bool IsExceptionOr = false;

template<typename T>
constexpr bool IsExceptionOr<DOM::ExceptionOr<T>> = true;

template<typename T>
ALWAYS_INLINE bool throw_dom_exception(JS::VM& vm, JS::GlobalObject& global_object, DOM::ExceptionOr<T>& result)
{
    if (result.is_exception()) {
        vm.throw_exception(global_object, DOMExceptionWrapper::create(global_object, const_cast<DOM::DOMException&>(result.exception())));
        return true;
    }
    return false;
}

namespace Detail {

template<typename T>
struct ExtractExceptionOrValueType {
    using Type = T;
};

template<typename T>
struct ExtractExceptionOrValueType<DOM::ExceptionOr<T>> {
    using Type = T;
};

template<>
struct ExtractExceptionOrValueType<void> {
    using Type = JS::Value;
};

template<>
struct ExtractExceptionOrValueType<DOM::ExceptionOr<void>> {
    using Type = JS::Value;
};

}

template<typename T>
using ExtractExceptionOrValueType = typename Detail::ExtractExceptionOrValueType<T>::Type;

// Return type depends on the return type of 'fn' (when invoked with no args):
// void or ExceptionOr<void>: Optional<JS::Value>, always returns JS::js_undefined()
// ExceptionOr<T>: Optional<T>
// T: Optional<T>
template<typename F, typename T = decltype(declval<F>()()), typename Ret = Conditional<!IsExceptionOr<T> && !IsVoid<T>, T, ExtractExceptionOrValueType<T>>>
Optional<Ret> throw_dom_exception_if_needed(auto&& vm, auto&& global_object, F&& fn)
{
    if constexpr (IsExceptionOr<T>) {
        auto&& result = fn();
        if (throw_dom_exception(vm, global_object, result))
            return {};
        if constexpr (requires(T v) { v.value(); })
            return result.value();
        else
            return JS::js_undefined();
    } else if constexpr (IsVoid<T>) {
        fn();
        return JS::js_undefined();
    } else {
        return fn();
    }
}

template<typename T>
bool should_return_empty(const Optional<T>& value)
{
    if constexpr (IsSame<JS::Value, T>)
        return !value.has_value() || value.value().is_empty();
    return !value.has_value();
}

}
