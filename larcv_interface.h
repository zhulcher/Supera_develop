#ifndef LARCV_INTERFACE_H
#define LARCV_INTERFACE_H
#if __has_include("larcv/core/DataFormat/Particle.h")
#include "larcv/core/DataFormat/EventParticle.h"
#include "larcv/core/DataFormat/EventVoxel3D.h"
#include "larcv/core/DataFormat/Particle.h"
#include "larcv2core/DataFormat/Voxel3DMeta.h"
#elif __has_include("larcv3/core/dataformat/Particle.h")
#include "larcv3/core/dataformat/EventSparseCluster.h"
#include "larcv3/core/dataformat/EventSparseTensor.h"
#include "larcv3/core/dataformat/ImageMeta.h"
#include "larcv3/core/dataformat/Particle.h"
#include "larcv3/core/dataformat/EventParticle.h"
#define larcv larcv3
#endif
#if __has_include("larcv/core/DataFormat/Particle.h")
typedef larcv3::Voxel3DMeta IM;
typedef larcv::EventClusterVoxel3D ECV3D;
typedef larcv::EventSparseTensor3D EST3D;
typedef larcv::EventClusterVoxel3D *ECV3Ds;
typedef larcv::EventSparseTensor3D *EST3Ds;
EST3D *get_tensor_pointer(larcv::IOManager &mgr, std::string str1, std::string str2) ;
ECV3D *get_cluster_pointer(larcv::IOManager &mgr, std::string str1, std::string str2) ;
void newmeta_clus(ECV3D *event_clus,IM themeta) ;
void newmeta_tens(EST3D *event_tens, IM themeta) ;
//void newmeta_clus_nostar(*ECV3D *event_clus, IM themeta) { *event_clus->meta(themeta); }
//void newmeta_tens_nostar(*EST3D *event_tens, IM themeta) { *event_tens->meta(themeta); }
IM getmeta_cluster(ECV3D event_clus) ;
IM getmeta_cluster_2(ECV3D *event_clus) ;
void myresize(ECV3D *event_clus, const size_t mynum) ;
void emplace_writeable_voxel(ECV3Ds event_clus, int outindex, larcv::VoxelSet myvs);
void set_writeable_voxel(ECV3Ds event_clus, int index, larcv::VoxelSet myvs);
void emplace_tens(EST3Ds event_tens, larcv::VoxelSet myvs, larcv::Voxel3DMeta themeta);
IM getmeta_tensor_2(EST3Ds event_tens);
#elif __has_include("larcv3/core/dataformat/Particle.h")
typedef larcv3::ImageMeta<3> IM;
typedef larcv3::EventSparseCluster3D ECV3D;
typedef larcv3::EventSparseTensor3D EST3D;
typedef std::shared_ptr<larcv3::EventSparseCluster3D> ECV3Ds;
typedef std::shared_ptr<larcv3::EventSparseTensor3D> EST3Ds;
EST3Ds get_tensor_pointer(larcv3::IOManager &mgr, std::string str1, std::string str2) ;
ECV3Ds get_cluster_pointer(larcv3::IOManager &mgr, std::string str1, std::string str2) ;
void newmeta_clus(ECV3Ds event_clus, IM themeta);
void newmeta_tens(EST3Ds event_tens, IM themeta);
//void newmeta_clus_nostar(*ECV3D *event_clus, IM themeta)
//{
//    for (size_t i = 0; i < (*event_clus)->size(); i++)
//    {
//        *event_clus->at(i).meta(themeta);
//    }
//}
//void newmeta_tens_nostar(*EST3D *event_tens, IM themeta)
//{
//    for (size_t i = 0; i < (*event_tens)->size(); i++)
//    {
//        *event_tens->at(i).meta(themeta);
//    }
//}
IM getmeta_cluster(ECV3D event_clus) ;
IM getmeta_cluster_2(ECV3Ds event_clus);
void myresize(ECV3Ds event_clus, const size_t mynum) ;
void emplace_writeable_voxel(ECV3Ds event_clus, int outindex, larcv3::VoxelSet myvs);
void set_writeable_voxel(ECV3Ds event_clus, int index, larcv::VoxelSet myvs);
void emplace_tens(EST3Ds event_tens, larcv3::VoxelSet myvs, IM themeta);

IM getmeta_tensor_2(EST3Ds event_tens);
#endif

#endif