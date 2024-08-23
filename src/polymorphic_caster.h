#pragma once

#include <nanobind/nanobind.h>
#include <polymorphic_cxx14.h>
#include <tsl/robin_map.h>
#include <typeindex>

NAMESPACE_BEGIN(NB_NAMESPACE)
NAMESPACE_BEGIN(detail)
template <typename T, typename A> struct poly_caster {
  using PolyType = xyz::polymorphic<T, A>;

  static constexpr auto Name = make_caster<T>::Name;
  static constexpr bool IsClass = true;
  template <typename T_> using Cast = movable_cast_t<T_>;
  template <typename T_> static constexpr bool can_cast() { return true; }

  PolyType value;

  operator PolyType *() { return &value; }
  operator PolyType &() { return (PolyType &)value; }
  operator PolyType &&() { return (PolyType &&)value; }

  typedef bool (*conv_t)(handle, uint8_t, cleanup_list *, PolyType &) noexcept;
  typedef tsl::robin_map<std::type_index, conv_t> ImplicitConversionMap;

  // Maps runtime type info to a function to which from_python args are
  // forwarded, converting the handle to to type U, and creates polymorphic<T,
  // U> using the U&& move-ctor. This effectively type-erases the templated
  // constructor of the polymorphic value type.
  inline static ImplicitConversionMap conversions;

  static T *get_ptr(const PolyType &p) {
    return const_cast<PolyType &>(p).operator->();
  }

  bool from_python(handle src, uint8_t flags, cleanup_list *cleanup) noexcept {
    using std::type_index;
    if (src.is_none())
      return true;

    // obtain info on C++ type of input PyObject
    const std::type_info *type = nb_type_info(src.type().ptr());

    if (!type)
      return false;

    // lookup conversion from runtime C++ type to Poly
    if (const auto found = conversions.find(type_index(*type));
        found == conversions.end())
      return false;
    else {
      return found->second(src, flags, cleanup, value);
    }
  }

  template <typename U>
  static handle from_cpp(U *value, rv_policy policy,
                         cleanup_list *cleanup) noexcept {
    if (!value)
      return none().release();
    return from_cpp(*value, policy, cleanup);
  }

  template <typename U>
  static handle from_cpp(U &&value, rv_policy policy,
                         cleanup_list *cleanup) noexcept {
    if (value.valueless_after_move())
      return none().release();

    auto *ptr = get_ptr(value);
    assert(ptr != nullptr);
    const std::type_info *type = &typeid(T);

    handle result;
    if constexpr (!std::is_polymorphic_v<T>) {
      result = nb_type_put(type, ptr, policy, cleanup);
    } else {
      const std::type_info *type_p = &typeid(*ptr);
      result = nb_type_put_p(type, type_p, ptr, policy, cleanup);
    }
    return result;
  }
};

template <typename T, typename A>
struct type_caster<xyz::polymorphic<T, A>> : poly_caster<T, A> {};

NAMESPACE_END(detail)
NAMESPACE_END(NB_NAMESPACE)

namespace myext {

namespace nb = nanobind;

template <typename U, typename PolyType>
void register_implicit_conversion_polymorphic() {
  using T = typename PolyType::value_type;
  using Alloc = typename PolyType::allocator_type;
  using Caster = nb::detail::make_caster<U>;
  using nb::handle;
  using nb::rv_policy;
  using nb::detail::cast_t;
  using nb::detail::cleanup_list;
  const std::type_info &ti = typeid(U);
  auto &conversions = nb::detail::poly_caster<T, Alloc>::conversions;
  conversions[std::type_index(ti)] =
      +[](handle src, uint8_t flags, cleanup_list *cleanup,
          PolyType &value) noexcept -> bool {
    Caster caster;
    if (!caster.from_python(src, flags, cleanup))
      return false;

    new (&value) PolyType{caster.operator cast_t<U>()};

    return true;
  };
}

} // namespace myext
