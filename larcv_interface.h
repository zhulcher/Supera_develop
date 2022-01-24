#ifndef LARCV_INTERFACE_H
#define LARCV_INTERFACE_H

#include "BBox.h"

#if __has_include("larcv/core/DataFormat/Particle.h")
#include "larcv/core/DataFormat/EventParticle.h"
#include "larcv/core/DataFormat/EventVoxel3D.h"
#include "larcv/core/DataFormat/Particle.h"
#include "larcv2core/DataFormat/Voxel3DMeta.h"
typedef larcv::Voxel3DMeta IM;
typedef larcv::EventClusterVoxel3D ECV3D;
typedef larcv::EventSparseTensor3D EST3D;
typedef larcv::EventParticle EP;
typedef larcv::EventClusterVoxel3D *ECV3Ds;
typedef larcv::EventSparseTensor3D *EST3Ds;
typedef larcv::EventParticle *EPs;
#elif __has_include("larcv3/core/dataformat/Particle.h")
#include "larcv3/core/dataformat/EventSparseCluster.h"
#include "larcv3/core/dataformat/EventSparseTensor.h"
#include "larcv3/core/dataformat/ImageMeta.h"
#include "larcv3/core/dataformat/Particle.h"
#include "larcv3/core/dataformat/EventParticle.h"
#define larcv larcv3
typedef larcv::ImageMeta<3> IM;
typedef larcv::EventSparseCluster3D ECV3D;
typedef larcv::EventSparseTensor3D EST3D;
typedef larcv::EventParticle EP;
typedef std::shared_ptr<larcv::EventSparseCluster3D> ECV3Ds;
typedef std::shared_ptr<larcv::EventSparseTensor3D> EST3Ds;
typedef std::shared_ptr<larcv::EventParticle> EPs;
#endif

EST3Ds get_tensor_pointer(larcv::IOManager &mgr, std::string str1, std::string str2);
ECV3Ds get_cluster_pointer(larcv::IOManager &mgr, std::string str1, std::string str2);
EPs get_particle_pointer(larcv::IOManager &mgr, std::string str1, std::string str2);

void newmeta_clus(ECV3Ds event_clus, IM themeta);
void newmeta_tens(EST3Ds event_tens, IM themeta);

void newmeta_clus_nostar(ECV3D *event_clus, IM themeta);
void newmeta_tens_nostar(EST3D *event_tens, IM themeta);

IM getmeta_cluster(ECV3D event_clus);
IM getmeta_cluster_2(ECV3Ds event_clus);
IM getmeta_tensor(EST3D event_tens);
IM getmeta_tensor_2(EST3Ds event_tens);
void myresize(ECV3Ds event_clus, const size_t mynum);

supera::Point3D make_sup_point(larcv::Point3D point);
supera::Point3D make_sup_point(std::vector<double> vec);
double meta_pos(IM themeta, unsigned long long myid, int dim);
void emplace_writeable_voxel(ECV3Ds event_clus, int outindex, larcv::VoxelSet myvs);
void set_writeable_voxel(ECV3Ds event_clus, int index, larcv::VoxelSet myvs);
void emplace_tens(EST3Ds event_tens, larcv::VoxelSet myvs, IM themeta);
void emplace_clus(ECV3Ds event_clus, larcv::VoxelSet myvs, IM themeta);
double meta_min(IM themeta, int dim);
double meta_vox_dim(IM themeta, int dim);
void id_to_xyz_id(IM themeta, larcv::VoxelID_t id, size_t& x, size_t& y, size_t& z);
int pos_to_xyz_id(IM themeta, double posx, double posy, double posz, size_t &x, size_t &y, size_t &z, larcv::VoxelID_t &vox_id);

#endif