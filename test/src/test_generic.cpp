#include <function_ref.hpp>
#include "doctest.h"

DOCTEST_TEST_CASE("no args") {
  static int global = 0;
  int i = 0;
  auto inc_lambda = [&i] { ++i; };
  auto inc2_lambda = [&i] { i += 2; };
  auto returns_lambda = [&i] {
    i += 3;
    return i;
  };

  void (*inc_fn_ptr)() = +[] { ++global; };
  auto inc2_global_lambda = [] { global += 2; };
  auto returns_fn_ptr = +[] {
    global += 3;
    return global;
  };

  veg::compact_function_ref<void()> fn_ref{inc_lambda};
  DOCTEST_CHECK(i == 0);
  fn_ref();
  DOCTEST_CHECK(i == 1);
  fn_ref();
  DOCTEST_CHECK(i == 2);

  fn_ref = inc2_lambda;
  fn_ref();
  DOCTEST_CHECK(i == 4);

  fn_ref = returns_lambda;
  fn_ref();
  DOCTEST_CHECK(i == 7);

  fn_ref = inc_fn_ptr;
  DOCTEST_CHECK(global == 0);
  fn_ref();
  DOCTEST_CHECK(global == 1);

  fn_ref = inc2_global_lambda;
  fn_ref();
  DOCTEST_CHECK(global == 3);

  fn_ref = +returns_fn_ptr;
  fn_ref();
  DOCTEST_CHECK(global == 6);
  static_assert(!noexcept(fn_ref()));
}

static void const* global = nullptr;

struct foo {
  auto bar(foo /*unused*/, int /*unused*/) & noexcept -> foo {
    global = this;
    return {};
  }
  auto bar2(foo /*unused*/, int /*unused*/) noexcept -> foo {
    global = this + 1;
    return {};
  }
};

auto baz(foo const& /*unused*/, foo /*unused*/, int /*unused*/) -> foo {
  global = nullptr;
  return {};
}

DOCTEST_TEST_CASE("member functions") {
  foo a;
  foo b;
  veg::function_ref<foo(foo&, foo, int) noexcept> fn(&foo::bar);
  static_assert(noexcept(fn(b, {}, 1)));

  foo _ = fn(a, {}, 1);
  DOCTEST_CHECK(global == &a);
  fn = &foo::bar2;
  _ = fn(b, {}, 1);
  DOCTEST_CHECK(global == (&b + 1));

  fn = baz;
  _ = fn(b, {}, 1);
  DOCTEST_CHECK(global == nullptr);
}

DOCTEST_TEST_CASE("null") {
  veg::function_ref<void()> f;
  DOCTEST_CHECK(!f);
  f = [] {};
  DOCTEST_CHECK(f);
}
