#ifndef FUNCTION_REF_FUNCTION_REF_HPP_EZASYJ8JS
#define FUNCTION_REF_FUNCTION_REF_HPP_EZASYJ8JS

#include <type_traits>
#include <cstdlib>

#define FUNCTION_REF_DECLVAL(T) (*static_cast<T (*)()>(nullptr))()

namespace fnref {

template <typename T>
struct compact_function_ref;
template <typename T>
struct function_ref;

namespace detail {

struct dummy {};

union sstate_t {
  void* ptr;
  void (*fn)();
};
union state_t {
  void* ptr;
  void (*fn)();
  void (dummy::*mem_fn)();
};

template <bool Cond, typename T>
struct enable_if {
  using type = T;
};
template <typename T>
struct enable_if<false, T> {};
template <bool Cond, typename T = void>
using enable_if_t = typename enable_if<Cond, T>::type;
template <typename T>
using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
template <typename T>
using decay_t = typename std::decay<T>::type;
template <typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;
template <typename T>
struct is_fn_ptr {
  static constexpr bool value = false;
};
template <typename Ret, typename... Args>
struct is_fn_ptr<Ret (*)(Args...)> {
  static constexpr bool value = true;
};

template <typename T, typename Enable = void>
struct has_unary_plus {
  static constexpr bool value = false;
};
template <typename T>
struct has_unary_plus<T, decltype(static_cast<void>(+FUNCTION_REF_DECLVAL(T &&)))> {
  static constexpr bool value = is_fn_ptr<decay_t<decltype(+FUNCTION_REF_DECLVAL(T &&))>>::value;
};

template <typename Enable, typename Fn, typename To, typename... Args>
struct call_r_impl {
  static constexpr bool value = false;
};
template <typename To, typename Fn, typename... Args>
struct call_r_impl<
    enable_if_t<
        std::is_same<To, void>::value ||
        std::is_convertible<
            decltype(FUNCTION_REF_DECLVAL(Fn &&)(FUNCTION_REF_DECLVAL(Args)...)),
            To>::value>,
    Fn,
    To,
    Args...> {
  static constexpr bool value = true;

  static auto apply(Fn&& fn, Args&&... args)
      -> decltype(static_cast<Fn&&>(fn)(static_cast<Args&&>(args)...)) {
    return static_cast<Fn&&>(fn)(static_cast<Args&&>(args)...);
  }
};

template <typename Enable, typename Fn, typename To, typename... Args>
struct invoke_r_impl : call_r_impl<Enable, Fn, To, Args...> {};

template <typename To, typename Mem_Fn, typename First, typename... Args>
struct invoke_r_impl<
    enable_if_t<
        std::is_same<To, void>::value ||
        std::is_convertible<
            decltype((FUNCTION_REF_DECLVAL(First &&).*FUNCTION_REF_DECLVAL(Mem_Fn))(
                FUNCTION_REF_DECLVAL(Args &&)...)),
            To>::value>,
    Mem_Fn,
    To,
    First,
    Args...> {

  static constexpr bool value = true;

  static auto apply(Mem_Fn fn, First&& first, Args&&... args)
      -> decltype((static_cast<First&&>(first).*fn)(static_cast<Args&&>(args)...)) {
    return (static_cast<First&&>(first).*fn)(static_cast<Args&&>(args)...);
  }
};

template <typename Fn, typename To, typename... Args>
struct is_invoke_convertible : invoke_r_impl<void, Fn, To, Args...> {};
template <typename Fn, typename To, typename... Args>
struct is_call_convertible : call_r_impl<void, Fn, To, Args...> {};

template <typename Ret>
struct discard_void {
  // non void case
  template <typename Fn, typename... Args>
  static auto apply(Fn&& fn, Args&&... args) -> Ret {
    return is_invoke_convertible<Fn, Ret, Args...>::apply(
        static_cast<Fn&&>(fn), static_cast<Args&&>(args)...);
  }
};
template <>
struct discard_void<void> {
  template <typename Fn, typename... Args>
  static auto apply(Fn&& fn, Args&&... args) -> void {
    is_invoke_convertible<Fn, void, Args...>::apply(
        static_cast<Fn&&>(fn), static_cast<Args&&>(args)...);
  }
};

enum struct fn_kind_e {
  fn_ptr,
  mem_fn_ptr,
  fn_obj,
};

template <typename T>
struct fn_kind {
  static constexpr fn_kind_e value =
      has_unary_plus<T>::value ? fn_kind_e::fn_ptr : fn_kind_e::fn_obj;
};

template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...)> {
  static constexpr fn_kind_e value = fn_kind_e::fn_ptr;
};

// member functions were a mistake
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) &&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const&&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) volatile> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const volatile> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) volatile&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const volatile&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) volatile&&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const volatile&&> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};

#if __cplusplus >= 201703L

template <typename Ret, typename... Args>
struct fn_kind<Ret (*)(Args...) noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...)&& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const&& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) volatile noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const volatile noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) volatile& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const volatile& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) volatile&& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};
template <typename Ret, typename Self, typename... Args>
struct fn_kind<Ret (Self::*)(Args...) const volatile&& noexcept> {
  static constexpr fn_kind_e value = fn_kind_e::mem_fn_ptr;
};

#endif

template <fn_kind_e Kind>
struct fn_ref_impl;

template <>
struct fn_ref_impl<fn_kind_e::fn_obj> {
  template <typename State, typename Fn>
  static auto address(Fn& arg) -> State {
    return {&const_cast<char&>(reinterpret_cast<const volatile char&>(arg))};
  }
  template <typename State, typename Fn, typename Ret, typename... Args>
  static auto call(State state, Args... args) -> Ret {
    return discard_void<Ret>::apply(
        static_cast<Fn&&>(*static_cast<remove_cvref_t<Fn>*>(state.ptr)),
        static_cast<Args&&>(args)...);
  }
};
template <>
struct fn_ref_impl<fn_kind_e::fn_ptr> {
  template <typename State, typename Fn>
  static auto address(Fn& arg) -> State {
    State rv;
    rv.fn = reinterpret_cast<void (*)()>(+arg);
    return rv;
  }
  template <typename State, typename Fn, typename Ret, typename... Args>
  static auto call(State state, Args... args) -> Ret {
    return discard_void<Ret>::apply(
        reinterpret_cast<decltype(+FUNCTION_REF_DECLVAL(Fn&))>(state.fn),
        static_cast<Args&&>(args)...);
  }
};

template <>
struct fn_ref_impl<fn_kind_e::mem_fn_ptr> {
  template <typename State, typename Fn>
  static auto address(Fn& arg) -> state_t {
    state_t rv;
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

    rv.mem_fn = reinterpret_cast<void (dummy::*)()>(arg);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    return rv;
  }
  template <typename State, typename Fn, typename Ret, typename... Args>
  static auto call(State state, Args... args) -> Ret {
    return discard_void<Ret>::apply(
        reinterpret_cast<decltype(FUNCTION_REF_DECLVAL(Fn&))>(state.mem_fn),
        static_cast<Args&&>(args)...);
  }
};
template <
    typename State,
    template <typename Ret, typename Fn, typename... Args>
    class Concept,
    bool No_Except,
    typename Ret,
    typename... Args>
struct function_ref_impl {

  template <
      typename Fn,
      typename =                           //
      detail::enable_if_t<                 //
                                           //
          !std::is_same<                   //
              detail::remove_cvref_t<Fn>,  //
              function_ref_impl            //
              >::value &&                  //
                                           //
          Concept<Fn, Ret, Args...>::value //
          >                                //
      >
  function_ref_impl(Fn&& fn) noexcept
      : m_state(detail::fn_ref_impl<detail::fn_kind<detail::decay_t<Fn>>::value>::template address<
                State>(fn)),
        m_call(detail::fn_ref_impl<detail::fn_kind<detail::decay_t<Fn>>::value>::
                   template call<State, Fn, Ret, Args...>) {}

  auto operator()(Args... args) const noexcept(No_Except) -> Ret {
    return this->m_call(this->m_state, static_cast<Args&&>(args)...);
  }

private:
  template <detail::fn_kind_e>
  friend struct detail::fn_ref_impl;

  State m_state;
  auto (*m_call)(State, Args...) -> Ret;
};

} // namespace detail

template <typename Ret, typename... Args>
struct compact_function_ref<Ret(Args...)>
    : private detail::
          function_ref_impl<detail::sstate_t, detail::is_call_convertible, false, Ret, Args...> {
private:
  using base =
      detail::function_ref_impl<detail::sstate_t, detail::is_call_convertible, false, Ret, Args...>;

public:
  using base ::base;
  using base::operator();
};

template <typename Ret, typename... Args>
struct function_ref<Ret(Args...)>
    : private detail::
          function_ref_impl<detail::state_t, detail::is_invoke_convertible, false, Ret, Args...> {
private:
  using base = detail::
      function_ref_impl<detail::state_t, detail::is_invoke_convertible, false, Ret, Args...>;

public:
  using base ::base;
  using base::operator();
};

#if __cplusplus >= 201703L

template <typename Ret, typename... Args>
struct compact_function_ref<Ret(Args...) noexcept>
    : private detail::
          function_ref_impl<detail::sstate_t, detail::is_call_convertible, true, Ret, Args...> {
private:
  using base =
      detail::function_ref_impl<detail::sstate_t, detail::is_call_convertible, true, Ret, Args...>;

public:
  using base ::base;
  using base::operator();
};

template <typename Ret, typename... Args>
struct function_ref<Ret(Args...) noexcept>
    : private detail::
          function_ref_impl<detail::state_t, detail::is_invoke_convertible, true, Ret, Args...> {
private:
  using base =
      detail::function_ref_impl<detail::state_t, detail::is_invoke_convertible, true, Ret, Args...>;

public:
  using base ::base;
  using base::operator();
};

#endif

} // namespace fnref

#undef FUNCTION_REF_DECLVAL

#endif /* end of include guard FUNCTION_REF_FUNCTION_REF_HPP_EZASYJ8JS */
