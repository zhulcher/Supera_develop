#include "SuperaLorentz.h"
#include <pybind11/operators.h>

void init_SuperaLorentz(pybind11::module m)
{
    pybind11::class_<larcv3::SLorentzvector>(m, "SLorentzVector")
        .def(pybind11::init<>())
        .def(pybind11::init<double_t, double_t, double_t, double_t>())
        //.def(pybind11::init<pybind11::self>())
        .def("X", &larcv3::SLorentzvector::X)
        .def("Y", &larcv3::SLorentzvector::Y)
        .def("Z", &larcv3::SLorentzvector::Z)
        .def("T", &larcv3::SLorentzvector::T)
        .def("Px", &larcv3::SLorentzvector::Px)
        .def("Py", &larcv3::SLorentzvector::Py)
        .def("Pz", &larcv3::SLorentzvector::Pz)
        .def("P", &larcv3::SLorentzvector::P)
        .def("E", &larcv3::SLorentzvector::E)
        .def("Vect", &larcv3::SLorentzvector::Vect)
        .def("SetVect", &larcv3::SLorentzvector::SetVect)
        .def(pybind11::self + pybind11::self)
        .def(pybind11::self += pybind11::self)
        .def(pybind11::self - pybind11::self)
        .def(pybind11::self -= pybind11::self)
        .def(pybind11::self *= double_t())
        .def(pybind11::self * double_t())
        .def(pybind11::self == pybind11::self)
        .def(pybind11::self != pybind11::self)
        .def("Mag2", &larcv3::SLorentzvector::Mag2)
        .def("Mag", &larcv3::SLorentzvector::Mag)
        ;
}
