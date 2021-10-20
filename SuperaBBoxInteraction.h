/**
 * \file SuperaBBoxInteraction.h
 *
 * \ingroup Package_Name
 *
 * \brief Class def header for a class SuperaBBoxInteraction
 *
 * @author kazuhiro
 */

/** \addtogroup Package_Name

    @{*/
#ifndef __SUPERABBOXINTERACTION_H__
#define __SUPERABBOXINTERACTION_H__
//#ifndef __CINT__
//#ifndef __CLING__
#include "SuperaBase.h"
#include "BBox.h"

#ifdef __has_include
#if __has_include("larcv3/core/dataformat/Particle.h")
#include "larcv3/core/dataformat/EventBBox.h"
#include "larcv3/core/dataformat/EventParticle.h"
#define larcv larcv3
#endif
#endif

    namespace larcv {

  /**
     \class SuperaBBoxInteraction
     Responsible for defining a rectangular volume boundary and voxelization
  */
  class SuperaBBoxInteraction : public SuperaBase {

  public:

    /// Default constructor
    SuperaBBoxInteraction(const std::string name = "SuperaBBoxInteraction");

    /// Default destructor
    ~SuperaBBoxInteraction() {}

    void configure(const PSet&);

    void initialize();

    bool process(IOManager& mgr);

    void finalize();

  private:

    unsigned short _origin;
    double _xlen, _ylen, _zlen;
    double _xvox, _yvox, _zvox;
    supera::BBox3D _world_bounds;
    supera::BBox3D _bbox;
    std::vector<std::string> _cluster3d_labels;
    std::vector<std::string> _tensor3d_labels;

    bool _use_fixed_bbox;
    std::vector<double> _bbox_bottom;

    bool update_bbox(supera::BBox3D& bbox, const supera::Point3D& pt);
    void randomize_bbox_center(supera::BBox3D& bbox);
  };

  /**
     \class larcv::SuperaBBoxInteractionFactory
     \brief A concrete factory class for larcv::SuperaBBoxInteraction
  */
  class SuperaBBoxInteractionProcessFactory : public ProcessFactoryBase {
  public:
    /// ctor
    SuperaBBoxInteractionProcessFactory() { ProcessFactory::get().add_factory("SuperaBBoxInteraction", this); }
    /// dtor
    ~SuperaBBoxInteractionProcessFactory() {}
    /// creation method
    ProcessBase* create(const std::string instance_name) { return new SuperaBBoxInteraction(instance_name); }
  };

}
//#endif
//#endif
#endif
/** @} */ // end of doxygen group

