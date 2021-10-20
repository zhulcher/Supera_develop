/**
 * \file SuperMCParticleCluster.h
 *
 * \brief Algorithm to do clustering of MC particles in groups like EM showers
 *        Ported from DeepLearnPhysics/Supera
 *
 * @author J. Wolcott <jwolcott@fnal.gov>
 */

#ifndef LARCV2_SUPERAMCPARTICLECLUSTER_H
#define LARCV2_SUPERAMCPARTICLECLUSTER_H

#include <string>

#ifdef __has_include
#if __has_include("larcv/core/DataFormat/Particle.h")
#include "larcv/core/Base/PSet.h"
#elif __has_include("larcv3/core/dataformat/Particle.h")
#include "larcv3/core/base/PSet.h"
#define larcv larcv3
#endif
#endif





#include "MCParticleList.h"
#include "SuperaBase.h"
#include "SuperaMCParticleClusterData.h"

class TG4Trajectory;

namespace larcv
{
  class EventClusterVoxel3D;
  class EventSparseTensor3D;

  class IOManager;

  class Particle;

  #ifdef __has_include
  #if __has_include("larcv/core/DataFormat/Particle.h")
  class Voxel3DMeta;
  typedef EventClusterVoxel3D ECV3D;
  typedef EventSparseTensor3D EST3D;
  #elif __has_include("larcv3/core/dataformat/Particle.h")
  #include "larcv3/core/dataformat/Voxel.h"
  typedef ImageMeta<3> Voxel3DMeta;
  typedef SparseCluster<3> ECV3D;
  typedef SparseTensor<3> EST3D;
  #define larcv larcv3
  #endif
  #endif
  
  class VoxelSet;

  /**
   * \class SuperaMCParticleCluster
   *  Clusters together true MC particles that belong together (e.g., EM showers)
   */
  class SuperaMCParticleCluster : public SuperaBase
  {
    public:
      /// Default constructor
      SuperaMCParticleCluster(const std::string name = "SuperaMCParticleCluster");

      /// Default destructor
      ~SuperaMCParticleCluster() override = default;

      void configure(const PSet&) override;
      void initialize() override;
      bool process(IOManager& mgr) override;

    private:
      std::vector<supera::ParticleGroup> CreateParticleGroups(const std::vector<larcv::Particle> & particles);

      void AssignParticleGroupIDs(const std::vector<int> &trackid2index, std::vector<int> &output2trackid,
                                  std::vector<supera::ParticleGroup> &part_grp_v, std::vector<int> &trackid2output) const;

      void AnalyzeSimEnergyDeposit(const larcv::Voxel3DMeta& meta,
                                   std::vector<supera::ParticleGroup>& part_grp_v);

      void ApplyEnergyThreshold(std::vector<supera::ParticleGroup>& part_grp_v) const;


      #if __has_include("larcv/core/DataFormat/Particle.h")
    void BuildLEScatterClusters(const std::vector<larcv::Particle> &particles,
                                  const std::vector<int> &trackid2index,
                                  std::vector<supera::ParticleGroup> &part_grp_v,
                                  const std::vector<int> &trackid2output,
                                  larcv::ECV3D *event_cluster,
                                  larcv::ECV3D *event_cluster_le,
                                  VoxelSet &cid_vs) const;
    #elif __has_include("larcv3/core/dataformat/Particle.h")
        void BuildLEScatterClusters(const std::vector<larcv::Particle> &particles,
                                  const std::vector<int> &trackid2index,
                                  std::vector<supera::ParticleGroup> &part_grp_v,
                                  const std::vector<int> &trackid2output,
                                  std::shared_ptr<larcv3::SparseCluster<3> >event_cluster,
                                  std::shared_ptr<larcv3::SparseCluster<3> >event_cluster_le,
                                  VoxelSet &cid_vs) const;
    #endif

      
      #if __has_include("larcv/core/DataFormat/Particle.h")
      void BuildOutputClusters(const std::vector<int> &output2trackid,
                               larcv::ECV3D *event_cluster,
                               larcv::ECV3D *event_cluster_he,
                               larcv::ECV3D *event_cluster_le,
                               VoxelSet &cid_vs,
                               std::vector<supera::ParticleGroup> &part_grp_v,
                               std::vector<larcv::Particle> &part_v) const;
      #elif __has_include("larcv3/core/dataformat/Particle.h")
      void BuildOutputClusters(const std::vector<int> &output2trackid,
                               std::shared_ptr<larcv3::SparseCluster<3> > event_cluster,
                               std::shared_ptr<larcv3::SparseCluster<3> > event_cluster_he,
                               std::shared_ptr<larcv3::SparseCluster<3> > event_cluster_le,
                               VoxelSet &cid_vs,
                               std::vector<supera::ParticleGroup> &part_grp_v,
                               std::vector<larcv::Particle> &part_v) const;
      #endif
      

      void CheckParticleValidity(const std::vector<supera::ParticleGroup> &part_grp_v,
                                 std::vector<larcv::Particle> &part_v) const;
      #if __has_include("larcv/core/DataFormat/Particle.h")
      void ConsolidateLeftoverVoxels(const Voxel3DMeta &meta3d,
                                     const std::vector<supera::ParticleGroup> &part_grp_v,
                                     size_t total_vs_size,
                                     larcv::EST3D *event_leftover,
                                     size_t output_vs_size) const;
      #elif __has_include("larcv3/core/dataformat/Particle.h")
      void ConsolidateLeftoverVoxels(const Voxel3DMeta &meta3d,
                                     const std::vector<supera::ParticleGroup> &part_grp_v,
                                     size_t total_vs_size,
                                     std::shared_ptr<larcv3::SparseTensor<3> > event_leftover,
                                     size_t output_vs_size) const;
      #endif
      

      void DumpHierarchy(size_t trackid,
                         const std::vector<supera::ParticleGroup>& part_grp_v) const;

      void FixFirstStepInfo(std::vector<supera::ParticleGroup> &part_grp_v,
                            const Voxel3DMeta &meta3d,
                            const std::vector<int> &output2trackid) const;

      void FixInvalidParentShowerGroups(const std::vector<larcv::Particle> &particles,
                                        std::vector<supera::ParticleGroup> &part_grp_v,
                                        std::vector<int> &trackid2output,
                                        std::vector<int> &output2trackid) const;

      void FixOrphanNonShowerGroups(const std::vector<larcv::Particle> &particles,
                                    const std::vector<int> &output2trackid,
                                    std::vector<supera::ParticleGroup> &part_grp_v,
                                    std::vector<int> &trackid2output) const;

      void FixOrphanShowerGroups(const std::vector<larcv::Particle> &particles,
                                 std::vector<int> &output2trackid,
                                 std::vector<supera::ParticleGroup> &part_grp_v,
                                 std::vector<int> &trackid2output) const;

      void FixUnassignedGroups(std::vector<supera::ParticleGroup> &part_grp_v, std::vector<int> &output2trackid) const;

      void FixUnassignedLEScatterGroups(std::vector<supera::ParticleGroup> &part_grp_v,
                                        const std::vector<int> &output2trackid) const;

      void FixUnassignedParentGroups(std::vector<supera::ParticleGroup> &part_grp_v,
                                     std::vector<int> &trackid2output,
                                     std::vector<int> &output2trackid) const;

      bool IsTouching(const Voxel3DMeta& meta, const VoxelSet& vs1, const VoxelSet& vs2) const;

      void MergeShowerConversion(std::vector<supera::ParticleGroup>& part_grp_v);

      void MergeShowerIonizations(std::vector<supera::ParticleGroup>& part_grp_v);

      void MergeShowerFamilyTouching(const larcv::Voxel3DMeta& meta,
                                     std::vector<supera::ParticleGroup>& part_grp_v);

      void MergeShowerTouching(const larcv::Voxel3DMeta& meta,
                               std::vector<supera::ParticleGroup>& part_grp_v,
                               const std::vector<larcv::Particle> & particles);

      void MergeShowerTouchingLEScatter(const larcv::Voxel3DMeta& meta,
                                        std::vector<supera::ParticleGroup>& part_grp_v,
                                        const std::vector<larcv::Particle> & particles);

      void MergeShowerDeltas(std::vector<supera::ParticleGroup>& part_grp_v) const;

      std::vector<unsigned int> ParentShowerTrackIDs(size_t trackid,
                                                     const std::vector<supera::ParticleGroup>& part_grp_v,
                                                     const std::vector<larcv::Particle> & particles,
                                                     bool include_lescatter=false) const;

      std::vector<unsigned int> ParentTrackIDs(size_t trackid,
                                               const std::vector<larcv::Particle> & particles) const;

      size_t SemanticPriority(size_t a, size_t b) const;

      supera::MCParticleList _mc_part_list;
      std::string _ref_meta3d_cluster3d;
      std::string _ref_meta3d_tensor3d;
      std::string _ref_meta2d_tensor2d;
      std::string _input_particle_label;
      std::string _output_label;
      std::string _masked_true2reco_cluster3d;
      std::string _masked_true_tensor3d;
      size_t _eioni_size;
      size_t _delta_size;
      size_t _compton_size;
      double _compton_energy;
      double _edep_threshold;
      bool _check_particle_validity;
      int  _projection_id;

      std::vector<size_t> _semantic_priority;

      void
      CheckForUnassignedGroups(const std::vector<supera::ParticleGroup> &part_grp_v,
                               const std::vector<int> &output2trackid,
                               const std::vector<larcv::Particle> &part_v, const std::vector<larcv::VoxelSet> &main_vs,
                               const std::vector<larcv::VoxelSet> &lowe_vs) const;
  };

  // ------------------------------------------------------

  /**
   \class larcv::SuperaBBoxInteractionFactory
   \brief A concrete factory class for larcv::SuperaBBoxInteraction
*/
  class SuperaMCParticleClusterProcessFactory : public ProcessFactoryBase
  {
    public:
      /// ctor
      SuperaMCParticleClusterProcessFactory()
      {
        ProcessFactory::get().add_factory("SuperaMCParticleCluster", this);
      }

      /// dtor
      ~SuperaMCParticleClusterProcessFactory() override = default;

      /// creation method
      ProcessBase* create(const std::string instance_name) override
      {
        return new SuperaMCParticleCluster(instance_name);
      }
  };

}
#endif //LARCV2_SUPERAMCPARTICLECLUSTER_H
