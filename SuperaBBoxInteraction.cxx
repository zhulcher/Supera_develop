#ifndef __SUPERABBOXINTERACTION_CXX__
#define __SUPERABBOXINTERACTION_CXX__

#include "SuperaBBoxInteraction.h"
#include "BBox.h"
#include <random>

std::mt19937 gen(12349876); // Standard mersenne_twister_engine seeded with rd()
std::uniform_real_distribution<> dis(0.0, 1.0);

namespace larcv
{

  static SuperaBBoxInteractionProcessFactory __global_SuperaBBoxInteractionProcessFactory__;

  SuperaBBoxInteraction::SuperaBBoxInteraction(const std::string name) : SuperaBase(name)
  {
  }

  void SuperaBBoxInteraction::configure(const PSet &cfg)
  {
    SuperaBase::configure(cfg);

    _origin = cfg.get<unsigned short>("Origin", 0);

    _cluster3d_labels = cfg.get<std::vector<std::string> >("Cluster3DLabels");
    _tensor3d_labels = cfg.get<std::vector<std::string> >("Tensor3DLabels");

    auto bbox_size = cfg.get<std::vector<double> >("BBoxSize");
    assert(bbox_size.size() == 3);
    _xlen = bbox_size.at(0);
    _ylen = bbox_size.at(1);
    _zlen = bbox_size.at(2);

    auto voxel_size = cfg.get<std::vector<double> >("VoxelSize");
    assert(voxel_size.size() == 3);
    _xvox = voxel_size.at(0);
    _yvox = voxel_size.at(1);
    _zvox = voxel_size.at(2);

    auto bbox_top = cfg.get<std::vector<double> >("BBoxTop");
    assert(bbox_top.size() == 3);
    _bbox_bottom = cfg.get<std::vector<double> >("BBoxBottom");
    assert(_bbox_bottom.size() == 3);
    assert((bbox_top[0] - _bbox_bottom[0]) >= bbox_size[0]);
    assert((bbox_top[1] - _bbox_bottom[1]) >= bbox_size[1]);
    assert((bbox_top[2] - _bbox_bottom[2]) >= bbox_size[2]);

    supera::Point3D pmin, pmax;
    pmin.x = _bbox_bottom[0];
    pmin.y = _bbox_bottom[1];
    pmin.z = _bbox_bottom[2];
    pmax.x = bbox_top[0];
    pmax.y = bbox_top[1];
    pmax.z = bbox_top[2];

    _use_fixed_bbox = ((bbox_top[0] - (_bbox_bottom[0] + bbox_size[0])) < voxel_size[0]);
    _use_fixed_bbox = _use_fixed_bbox && ((bbox_top[1] - (_bbox_bottom[1] + bbox_size[1])) < voxel_size[1]);
    _use_fixed_bbox = _use_fixed_bbox && ((bbox_top[2] - (_bbox_bottom[2] + bbox_size[2])) < voxel_size[2]);

    _world_bounds.update(pmin, pmax);
  }

  void SuperaBBoxInteraction::initialize()
  {
    SuperaBase::initialize();
  }

  bool SuperaBBoxInteraction::process(IOManager &mgr)
  {
    SuperaBase::process(mgr);
    supera::BBox3D bbox(0, 0, 0, 0, 0, 0);

    if (_use_fixed_bbox)
    {
      double x0 = _bbox_bottom[0];
      double y0 = _bbox_bottom[1];
      double z0 = _bbox_bottom[2];

      double x1 = x0 + _xlen;
      double y1 = y0 + _ylen;
      double z1 = z0 + _zlen;

      supera::Point3D p0(x0, y0, z0);
      supera::Point3D p1(x1, y1, z1);
      bbox.update(p0, p1);
    }
    else
    {
      auto const &trajectories = GetEvent()->Trajectories;
      supera::Point3D pt;
      // Register primary vertex points
      for (auto const &traj : trajectories)
      {
        if (traj.GetParentId() >= 0)
          continue;
        auto const &pos = traj.Points.at(0).GetPosition();
        pt.x = pos.X();
        pt.y = pos.Y();
        pt.z = pos.Z();
        LARCV_INFO() << "Registering Primary particle vertex: ("
                     << pt.x << "," << pt.y << "," << pt.z << ")"
                     << " ... PDG " << traj.GetPDGCode() << std::endl;
        this->update_bbox(bbox, pt);
      }

      // Register particle energy deposition coordinates
      LARCV_INFO() << "Processing G4Trajectory array: " << trajectories.size() << std::endl;
      bool stop = false;
      for (auto const &traj : trajectories)
      {
        for (auto const &point : traj.Points)
        {
          auto const &pos = point.GetPosition();
          pt.x = pos.X();
          pt.y = pos.Y();
          pt.z = pos.Z();
          stop = !update_bbox(bbox, pt);
          if (stop)
            break;
        }
        if (stop)
          break;
      }

      // Randomize BBox location
      randomize_bbox_center(bbox);
    }

    // Create 3D meta
    
    auto const &min_pt = bbox.bottom_left();
    auto const &max_pt = bbox.top_right();
    size_t xnum = _xlen / _xvox;
    size_t ynum = _ylen / _yvox;
    size_t znum = _zlen / _zvox;
    IM meta;
    #if __has_include("larcv/core/DataFormat/Particle.h")
    meta.set(min_pt.x, min_pt.y, min_pt.z, max_pt.x, max_pt.y, max_pt.z, xnum, ynum, znum);
    #elif __has_include("larcv3/core/dataformat/Particle.h")
    if(min_pt.x > max_pt.x) throw larbys("xmin > xmax not allowed for BBox3D::Update!");
    if(min_pt.y > max_pt.y) throw larbys("ymin > ymax not allowed for BBox3D::Update!");
    if(min_pt.z > max_pt.z) throw larbys("zmin > zmax not allowed for BBox3D::Update!");
    meta.set_dimension(0, _xlen, xnum, min_pt.x);
    meta.set_dimension(1, _ylen, ynum, min_pt.y);
    meta.set_dimension(2, _zlen, znum, min_pt.z);
    #endif
    LARCV_INFO() << "3D Meta:" << meta.dump() << std::endl;

    // Create Cluster3D
    for (auto const &name : _cluster3d_labels)
    {
      auto &cluster3d = mgr.get_data<ECV3D>(name);
      #if __has_include("larcv/core/DataFormat/Particle.h")
      newmeta_clus(&cluster3d, meta);
      #elif __has_include("larcv3/core/dataformat/Particle.h")
      {
        for (size_t i = 0; i < cluster3d.size(); i++)
        {
          cluster3d.at(i).meta(meta);
        }
      }
      #endif
      
    }
    // Create Tensor3D
    for (auto const &name : _tensor3d_labels)
    {
      auto &tensor3d = mgr.get_data<EST3D>(name);
      #if __has_include("larcv/core/DataFormat/Particle.h")
      newmeta_tens(&tensor3d, meta);
      #elif __has_include("larcv3/core/dataformat/Particle.h")
      {
        for (size_t i = 0; i < tensor3d.size(); i++)
        {
          tensor3d.at(i).meta(meta);
        }
      }
      #endif
      
    }
    return true;
  }

  void SuperaBBoxInteraction::finalize()
  {
  }

  bool SuperaBBoxInteraction::update_bbox(supera::BBox3D &bbox, const supera::Point3D &pt)
  {
    supera::Point3D min_pt, max_pt;
    if (bbox.empty())
    {
      min_pt = max_pt = pt;
      max_pt.x += 1.e-9;
      max_pt.y += 1.e-9;
      max_pt.z += 1.e-9;
      bbox.update(min_pt, max_pt);
      LARCV_INFO() << "Defining minimal BBox:" << bbox.dump();
      return true;
    }
    else
    {
      min_pt = bbox.bottom_left();
      max_pt = bbox.top_right();
    }
    if (_world_bounds.contains(pt))
    {
      if (!bbox.contains(pt))
      {
        LARCV_DEBUG() << "Updating BBox:" << bbox.dump();
        if ((max_pt.x - min_pt.x) < _xlen)
        {
          if (pt.x < min_pt.x)
          {
            if ((max_pt.x - pt.x) < _xlen)
              min_pt.x = pt.x;
            else
              min_pt.x = max_pt.x - _xlen;
          }
          else if (pt.x > max_pt.x)
          {
            if ((pt.x - min_pt.x) < _xlen)
              max_pt.x = pt.x;
            else
              max_pt.x = min_pt.x + _xlen;
          }
        }
        if ((max_pt.y - min_pt.y) < _ylen)
        {
          if (pt.y < min_pt.y)
          {
            if ((max_pt.y - pt.y) < _ylen)
              min_pt.y = pt.y;
            else
              min_pt.y = max_pt.y - _ylen;
          }
          else if (pt.y > max_pt.y)
          {
            if ((pt.y - min_pt.y) < _ylen)
              max_pt.y = pt.y;
            else
              max_pt.y = min_pt.y + _ylen;
          }
        }
        if ((max_pt.z - min_pt.z) < _zlen)
        {
          if (pt.z < min_pt.z)
          {
            if ((max_pt.z - pt.z) < _zlen)
              min_pt.z = pt.z;
            else
              min_pt.z = max_pt.z - _zlen;
          }
          else if (pt.z > max_pt.z)
          {
            if ((pt.z - min_pt.z) < _zlen)
              max_pt.z = pt.z;
            else
              max_pt.z = min_pt.z + _zlen;
          }
        }
        // update bbox
        bbox.update(min_pt, max_pt);
        LARCV_DEBUG() << " ... to:" << bbox.dump();
      }
      else
      {
        LARCV_DEBUG() << "No update in BBox: point already contained!" << std::endl;
      }
    }
    else
    {
      LARCV_DEBUG() << "No update in BBox: point outside the world boundary" << std::endl;
    }
    return ((max_pt.x - min_pt.x) < _xlen ||
            (max_pt.y - min_pt.y) < _ylen ||
            (max_pt.z - min_pt.z) < _zlen);
  }

  void SuperaBBoxInteraction::randomize_bbox_center(supera::BBox3D &bbox)
  {

    supera::Point3D min_pt = bbox.bottom_left();
    supera::Point3D max_pt = bbox.top_right();
    LARCV_INFO() << "Randomize before:" << bbox.dump() << std::endl;
    // see if box location can be randomized
    if ((max_pt.x - min_pt.x) < _xlen)
    {
      double xshift = dis(gen)*(_xlen - (max_pt.x - min_pt.x));
      xshift *= (((dis(gen)*2)-1) > 0. ? 1. : -1.);
      if (xshift > 0)
      {
        max_pt.x += xshift;
        min_pt.x = max_pt.x - _xlen;
      }
      else
      {
        min_pt.x += xshift;
        max_pt.x = min_pt.x + _xlen;
      }
    }
    if ((max_pt.y - min_pt.y) < _ylen)
    {
      double yshift = dis(gen)*(_ylen - (max_pt.y - min_pt.y));
      yshift *= (((dis(gen)*2)-1) > 0. ? 1. : -1.);
      if (yshift > 0)
      {
        max_pt.y += yshift;
        min_pt.y = max_pt.y - _xlen;
      }
      else
      {
        min_pt.y += yshift;
        max_pt.y = min_pt.y + _xlen;
      }
    }
    if ((max_pt.z - min_pt.z) < _zlen)
    {
      double zshift = dis(gen)*(_zlen - (max_pt.z - min_pt.z));
      zshift *= (((dis(gen)*2)-1)> 0. ? 1. : -1.);
      if (zshift > 0)
      {
        max_pt.z += zshift;
        min_pt.z = max_pt.z - _xlen;
      }
      else
      {
        min_pt.z += zshift;
        max_pt.z = min_pt.z + _xlen;
      }
    }
    bbox.update(min_pt, max_pt);
    LARCV_INFO() << "Randomize after:" << bbox.dump() << std::endl;
  }
}

#endif