#pragma once

#include <coroutine>
#include <optional>
#include <variant>
#include <exception>


namespace NNet {

template<typename T> struct TFinalSuspendContinuation;

template<typename T> struct TValueTask;

template<typename T>
struct TValuePromiseBase {
    std::suspend_never initial_suspend() { return {}; }
    TFinalSuspendContinuation<T> final_suspend() noexcept;
    std::coroutine_handle<> Caller = std::noop_coroutine();
};

template<typename T>
struct TValuePromise: public TValuePromiseBase<T> {
    TValueTask<T> get_return_object();

    void return_value(const T& t) {
        ErrorOr = t;
    }

    void return_value(T&& t) {
        ErrorOr = std::move(t);
    }

    void unhandled_exception() {
        ErrorOr = std::current_exception();
    }

    std::optional<std::variant<T, std::exception_ptr>> ErrorOr;
};

template<typename T>
struct TValueTaskBase : std::coroutine_handle<TValuePromise<T>> {
    ~TValueTaskBase() { this->destroy(); }

    bool await_ready() {
        return !!this->promise().ErrorOr;
    }

    void await_suspend(std::coroutine_handle<> caller) {
        this->promise().Caller = caller;
    }

    using promise_type = TValuePromise<T>;
};

template<typename T>
struct TValueTask : public TValueTaskBase<T> {
    T await_resume() {
        auto& errorOr = *this->promise().ErrorOr;
        if (auto* res = std::get_if<T>(&errorOr)) {
            return std::move(*res);
        } else {
            std::rethrow_exception(std::get<std::exception_ptr>(errorOr));
        }
    }
};

template<> struct TValueTask<void>;

template<>
struct TValuePromise<void>: public TValuePromiseBase<void> {
    TValueTask<void> get_return_object();

    void return_void() {
        ErrorOr = nullptr;
    }

    void unhandled_exception() {
        ErrorOr = std::current_exception();
    }

    std::optional<std::exception_ptr> ErrorOr;
};

template<>
struct TValueTask<void> : public TValueTaskBase<void> {
    void await_resume() {
        auto& errorOr = *this->promise().ErrorOr;
        if (errorOr) {
            std::rethrow_exception(errorOr);
        }
    }
};

template<typename T>
struct TFinalSuspendContinuation {
    bool await_ready() noexcept { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<TValuePromise<T>> h) noexcept {
        return h.promise().Caller;
    }
    void await_resume() noexcept { }
};

inline TValueTask<void> TValuePromise<void>::get_return_object() { return { TValueTask<void>::from_promise(*this) }; }
template<typename T>
TValueTask<T> TValuePromise<T>::get_return_object() { return { TValueTask<T>::from_promise(*this) }; }


template<typename T>
TFinalSuspendContinuation<T> TValuePromiseBase<T>::final_suspend() noexcept { return {}; }

} // namespace NNet
