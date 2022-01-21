#ifndef LARCV_INTERFACE_CXX
#define LARCV_INTERFACE_CXX
#include "larcv_interface.h"

#if __has_include("larcv/core/DataFormat/Particle.h")

EST3D *get_tensor_pointer(larcv::IOManager &mgr, std::string str1, std::string str2) { return (EST3D *)(mgr.get_data(str1, str2)); }
ECV3D *get_cluster_pointer(larcv::IOManager &mgr, std::string str1, std::string str2) { return (ECV3D *)(mgr.get_data(str1, str2)); }
EPs *get_particle_pointer(larcv::IOManager &mgr, std::string str1, std::string str2) { return (EP *)(mgr.get_data(str1, str2)); }
void newmeta_clus(ECV3D *event_clus, IM themeta) { *event_clus->meta(themeta); }
void newmeta_tens(EST3D *event_tens, IM themeta) { *event_tens->meta(themeta); }

IM getmeta_cluster(ECV3D event_clus) { return event_clus.meta(); }
IM getmeta_cluster_2(ECV3D *event_clus) { return event_clus->meta(); }
IM getmeta_tensor(EST3D event_tens) { return event_tens.meta(); }
IM getmeta_tensor_2(EST3D *event_tens) { return event_tens->meta(); }
void myresize(ECV3D *event_clus, const size_t mynum) { event_clus->resize(mynum); }
void emplace_writeable_voxel(ECV3Ds event_clus, int outindex, larcv::VoxelSet myvs)
{
    auto &new_vs = event_clus->writeable_voxel_set(outindex);
    for (auto const &vox : myvs.as_vector())
    {
        new_vs.emplace(vox.id(), vox.value(), true);
    }
}
void set_writeable_voxel(ECV3Ds event_clus, int index, larcv::VoxelSet myvs)
{
    event_clus->writeable_voxel_set(index) = myvs;
}
void emplace_tens(EST3Ds event_tens, larcv::VoxelSet myvs, larcv::Voxel3DMeta themeta)
{
    event_tens->emplace(std::move(myvs), themeta);
}


double meta_min(IM themeta,int dim) 
{
    if (dim == 0) return themeta.min_x();
    if (dim == 1) return themeta.min_y();
    if (dim == 2) return themeta.min_z();
}
double meta_vox_dim(IM themeta, int dim) 
{ 
    if (dim == 0) return themeta.size_voxel_x();
    if (dim == 1) return themeta.size_voxel_y();
    if (dim == 2) return themeta.size_voxel_z();
}

double meta_pos(IM themeta, unsigned long long myid, int dim)
{
    if (dim == 0) return themeta.pos_x(myid);
    if (dim == 1) return themeta.pos_y(myid);
    if (dim == 2) return themeta.pos_z(myid);
}
void id_to_xyz_id(IM themeta,larcv::VoxelID_t id, size_t &x, size_t &y, size_t &z)
{
    themeta.id_to_xyz_index( id, x, y, z)
}
int pos_to_xyz_id(IM themeta,double posx, double posy, double posz, size_t& x, size_t& y, size_t& z,larcv::VoxelID_t &vox_id)
{
    vox_id = themeta.id(posx, posy, posy);
    if(vox_id==larcv::kINVALID_VOXELID) return 1;
    themeta.id_to_xyz_index(vox_id, x, y, z);
    return 0
}
#elif __has_include("larcv3/core/dataformat/Particle.h")
EST3Ds get_tensor_pointer(larcv3::IOManager &mgr, std::string str1, std::string str2) { return std::dynamic_pointer_cast<EST3D>(mgr.get_data(str1, str2)); }
ECV3Ds get_cluster_pointer(larcv3::IOManager &mgr, std::string str1, std::string str2) { return std::dynamic_pointer_cast<ECV3D>(mgr.get_data(str1, str2)); }
EPs get_particle_pointer(larcv3::IOManager &mgr, std::string str1, std::string str2) { return std::dynamic_pointer_cast<EP>(mgr.get_data(str1, str2)); }
void newmeta_clus(ECV3Ds event_clus, IM themeta)
{
    for (size_t i = 0; i < event_clus->size(); i++)
    {
        event_clus->at(i).meta(themeta);
    }
}
void newmeta_tens(EST3Ds event_tens, IM themeta)
{
    for (size_t i = 0; i < event_tens->size(); i++)
    {
        event_tens->at(i).meta(themeta);
    }
}
// void newmeta_clus_nostar(*ECV3D *event_clus, IM themeta)
//{
//     for (size_t i = 0; i < (*event_clus)->size(); i++)
//     {
//         *event_clus->at(i).meta(themeta);
//     }
// }
// void newmeta_tens_nostar(*EST3D *event_tens, IM themeta)
//{
//     for (size_t i = 0; i < (*event_tens)->size(); i++)
//     {
//         *event_tens->at(i).meta(themeta);
//     }
// }
IM getmeta_cluster(ECV3D event_clus) { return event_clus.sparse_cluster(0).meta(); }
IM getmeta_cluster_2(ECV3Ds event_clus) { return event_clus->sparse_cluster(0).meta(); }
IM getmeta_tensor(EST3D event_tens) { return event_tens.sparse_tensor(0).meta(); }
IM getmeta_tensor_2(EST3Ds event_tens) { return event_tens->sparse_tensor(0).meta(); }
void myresize(ECV3Ds event_clus, const size_t mynum)
{
    for (size_t i = 0; i < event_clus->size(); i++)
    {
        event_clus->at(i).resize(mynum);
    }
}
void emplace_writeable_voxel(ECV3Ds event_clus, int outindex, larcv3::VoxelSet myvs)
{
    for (size_t i = 0; i < event_clus->size(); i++)
    {
        auto &new_vs = event_clus->at(i).writeable_voxel_set(outindex);
        for (auto const &vox : myvs.as_vector())
        {
            new_vs.emplace(vox.id(), vox.value(), true);
        }
    }
}
void set_writeable_voxel(ECV3Ds event_clus, int index, larcv::VoxelSet myvs)
{
    for (size_t i = 0; i < event_clus->size(); i++)
    {
        event_clus->at(i).writeable_voxel_set(index) = myvs;
    }
}
void emplace_tens(EST3Ds event_tens, larcv3::VoxelSet myvs, IM themeta)
{
    for (size_t i = 0; i < event_tens->size(); i++)
    {
        event_tens->at(i).emplace(std::move(myvs), themeta);
    }
}


double meta_min(IM themeta,int dim) {return themeta.min(dim);}
double meta_vox_dim(IM themeta, int dim) { return themeta.voxel_dimensions(dim); }
double meta_pos(IM themeta, unsigned long long myid, int dim) { return themeta.position(myid).at(dim); }
void id_to_xyz_id(IM themeta, larcv::VoxelID_t id, size_t &x, size_t &y, size_t &z)
{
    std::vector<long unsigned int> vect = themeta.coordinates(id);
      x=vect[0];y=vect[1];z=vect[2];
}
int pos_to_xyz_id(IM themeta, double posx, double posy, double posz, size_t &x, size_t &y, size_t &z, larcv::VoxelID_t &vox_id)
{
    std::vector<double> vect{ posx, posy, posz};
    vox_id = themeta.position_to_index(vect);
    std::vector<long unsigned int> vect2 = themeta.coordinates(vox_id);
    if(vect2[0]==larcv::kINVALID_INDEX) return 1;
    if(vect2[1]==larcv::kINVALID_INDEX) return 1;
    if(vect2[2]==larcv::kINVALID_INDEX) return 1;
    x=vect2[0];y=vect2[1];z=vect2[2];
    return 0
}
#endif

#endif