#include "polymorphic_caster.h"

#include <nanobind/stl/bind_vector.h>
#include <nanobind/stl/string.h>
#include <vector>

namespace nb = nanobind;

namespace myext {

struct X;
using Poly_X = xyz::polymorphic<X>;
using XVec = std::vector<Poly_X>;

struct X {
  X() { printf("called X ctor\n"); }
  virtual std::string hello() const { return "Hello, I'm base class X!"; }
};

struct Y : X {
  Y() { printf("called Y ctor!\n"); }
  std::string hello() const override { return "Hello, I'm derived class Y!"; }
};

void echoX(const X &x) { printf("X says: %s\n", x.hello().c_str()); }

Poly_X getY() { return Poly_X{Y()}; }

struct Xstore {
  Xstore(const Poly_X &s) : store(s) {}
  Poly_X store;
};

void echoX_list(const XVec &xs) {
  for (const auto &x : xs) {
    echoX(*x);
  }
}

NB_MODULE(myext, m) {

  nb::class_<X>(m, "X").def(nb::init<>()).def("hello", &X::hello);

  nb::class_<Y, X>(m, "Y").def(nb::init<>());

  myext::register_implicit_conversion_polymorphic<X, Poly_X>();
  myext::register_implicit_conversion_polymorphic<Y, Poly_X>();

  nb::class_<Xstore>(m, "Xstore")
      .def(nb::init<const Poly_X &>())
      .def_rw("store", &Xstore::store)
      .def("__repr__", [](Xstore *self) -> std::string {
        printf("Attempting a conversion...");
        auto s = nb::cast(self->store);
        std::string inner_repr = nb::repr(s).c_str();
        return "Xstore(Poly containing " + inner_repr + ")";
      });

  m.def("getY", getY);

  m.def("echoX", echoX, nb::arg("x"));
  m.def("echoX_list", echoX_list, nb::arg("xs"));

  nb::bind_vector<XVec>(m, "XVec");
}

} // namespace myext
