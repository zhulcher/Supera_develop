#ifndef __SUPERABASE_CXX__
#define __SUPERABASE_CXX__

#include "SuperaBase.h"
#include "TG4Event.h"

namespace larcv
{

  static SuperaBaseProcessFactory __global_SuperaBaseProcessFactory__;

  SuperaBase::SuperaBase(const std::string name)
      : ProcessBase(name), _empty_string()
  {
    ClearEventData();
  }

  void SuperaBase::configure(const PSet &cfg)
  {
  }

  void SuperaBase::initialize()
  {
    ClearEventData();
  }

  bool SuperaBase::process(IOManager &mgr)
  {
    return true;
  }

  void SuperaBase::finalize()
  {
    ClearEventData();
  }

  bool SuperaBase::is(const std::string question) const
  {
    if (question == "Supera")
      return true;
    return false;
  }

  void SuperaBase::ClearEventData()
  {
    _event = nullptr;
  }

  // FIXME(kvtsang) Temporary solution to access associations
  const TG4Event *SuperaBase::GetEvent()
  {
    if (!_event)
      throw larbys("EdepSim TG4Event not set!");
    return _event;
  }

}

void init_superabase(pybind11::module m)
{
  using Class = larcv3::SuperaBase;
  pybind11::class_<Class> superabase(m, "SuperaBase");
  superabase.def("is", &Class::is);
  superabase.def("SetEvent", &Class::SetEvent);

  // We don't actually want to construct any instances of ProcessBase
  // processbase.def(pybind11::init<const std::string>(),
  // pybind11::arg("name")="ProcessBase");
}

#endif
