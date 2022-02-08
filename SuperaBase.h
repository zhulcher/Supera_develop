/**
 * \file SuperaBase.h
 *
 * \ingroup Package_Name
 *
 * \brief Class def header for a class SuperaBase
 *
 * @author kazuhiro
 */

/** \addtogroup Package_Name

@{*/
#ifndef __SUPERABASE_H__
#define __SUPERABASE_H__


#if __has_include("larcv/core/DataFormat/Particle.h")
#include "larcv/core/Processor/ProcessBase.h"
#include "larcv/core/Processor/ProcessFactory.h"
#include "larcv/core/DataFormat/ImageMeta.h"
#include "EDepSim/TG4Event.h"
#elif __has_include("larcv3/core/dataformat/Particle.h")
#include "larcv3/core/processor/ProcessBase.h"
#include "larcv3/core/processor/ProcessFactory.h"
#include "larcv3/core/dataformat/ImageMeta.h"
#include "TG4Event.h"
#define larcv larcv3
#include <pybind11/pybind11.h>
void init_superabase(pybind11::module m);
#endif

//#include "FMWKInterface.h"
//#include "SuperaTypes.h"
//#include "ImageMetaMakerBase.h"

// FIXME(kvtsang) Temporary solution to access associations
//#include "art/Framework/Principal/Event.h"


namespace larcv
{

  /**
  \class ProcessBase
  User defined class SuperaBase ... these comments are used to generate
  doxygen documentation!
  */
  class SuperaBase : public ProcessBase
  {

  public:
    /// Default constructor
    SuperaBase(const std::string name = "SuperaBase");

    /// Default destructor
    ~SuperaBase() {}

    virtual void configure(const PSet &);

    virtual void initialize();

    virtual bool process(IOManager &mgr);

    virtual void finalize();

    virtual bool is(const std::string question) const;

    inline void SetCSV(const std::string &fname)
    {
      _csv_fname = fname;
    }

    void SetEvent(const TG4Event *ev) { _event = ev; };

    void ClearEventData();

    //
    // Getter
    //

    const std::string &CSV() const { return _csv_fname; }

    const TG4Event *GetEvent();

  private:
    std::string _empty_string;
    std::string _csv_fname;
    const TG4Event *_event;
  };

  /**
  \class larcv::SuperaBaseFactory
  \brief A concrete factory class for larcv::SuperaBase
  */
  class SuperaBaseProcessFactory : public ProcessFactoryBase
  {
  public:
    /// ctor
    SuperaBaseProcessFactory() { ProcessFactory::get().add_factory("SuperaBase", this); }
    /// dtor
    ~SuperaBaseProcessFactory() {}
    /// creation method
    ProcessBase *create(const std::string instance_name) { return new SuperaBase(instance_name); }
  };

}


#endif
/** @} */ // end of doxygen group
