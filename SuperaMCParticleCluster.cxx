#ifndef __SUPERAMCPARTICLECLUSTER_CXX__
#define __SUPERAMCPARTICLECLUSTER_CXX__

#include "SuperaMCParticleCluster.h"
#include "SuperaMCParticleClusterData.h"
#include "larcv/core/DataFormat/EventParticle.h"
#include "larcv/core/DataFormat/EventVoxel3D.h"
#include "larcv/core/DataFormat/EventVoxel2D.h"

namespace larcv {
  
  static SuperaMCParticleClusterProcessFactory __global_SuperaMCParticleClusterProcessFactory__;
  
  SuperaMCParticleCluster::SuperaMCParticleCluster(const std::string name)
    : SuperaBase(name)
  {}
  
  void SuperaMCParticleCluster::configure(const PSet& cfg)
  {
    SuperaBase::configure(cfg);
    _output_label = cfg.get<std::string>("OutputLabel");
    _ref_meta3d_cluster3d = cfg.get<std::string>("Meta3DFromCluster3D","mcst");
    _ref_meta3d_tensor3d = cfg.get<std::string>("Meta3DFromTensor3D","");
    _delta_size = cfg.get<size_t>("DeltaSize",5);
    _eioni_size = cfg.get<size_t>("IonizationSize",5);
    _compton_size = cfg.get<size_t>("ComptonSize",10);
    _compton_energy = cfg.get<double>("ComptonEnergy",4);
    _edep_threshold = cfg.get<double>("EnergyDepositThreshold",0.05);
    _projection_id = cfg.get<int>("ProjectionID",0);
    _use_sed = cfg.get<bool>("UseSimEnergyDeposit");
    _use_sed_points = cfg.get<bool>("UseSimEnergyDepositPoints");
    _use_true_pos = cfg.get<bool>("UseTruePosition",true);
    
    auto tpc_v = cfg.get<std::vector<unsigned short> >("TPCList");
    larcv::Point3D min_pt(1.e9,1.e9,1.e9);
    larcv::Point3D max_pt(-1.e9,-1.e9,-1.e9);
    for(auto const& tpc_id : tpc_v) {
      auto geop = lar::providerFrom<geo::Geometry>();
      for(size_t c=0; c<geop->Ncryostats(); ++c) {
	auto const& cryostat = geop->Cryostat(c);
	if(!cryostat.HasTPC(tpc_id)) continue;
	auto const& tpcabox = cryostat.TPC(tpc_id).ActiveBoundingBox();
	if(min_pt.x > tpcabox.MinX()) min_pt.x = tpcabox.MinX();
	if(min_pt.y > tpcabox.MinY()) min_pt.y = tpcabox.MinY();
	if(min_pt.z > tpcabox.MinZ()) min_pt.z = tpcabox.MinZ();
	if(max_pt.x < tpcabox.MaxX()) max_pt.x = tpcabox.MaxX();
	if(max_pt.y < tpcabox.MaxY()) max_pt.y = tpcabox.MaxY();
	if(max_pt.z < tpcabox.MaxZ()) max_pt.z = tpcabox.MaxZ();
	break;
      }
    }
    _world_bounds.update(min_pt,max_pt);

    _ref_meta2d_tensor2d = cfg.get<std::string>("Meta2DFromTensor2D","");
    auto cryostat_v   = cfg.get<std::vector<unsigned short> >("CryostatList");
    //auto tpc_v        = cfg.get<std::vector<unsigned short> >("TPCList"     );
    auto plane_v      = cfg.get<std::vector<unsigned short> >("PlaneList"   );

    _scan.clear();
    auto geop = lar::providerFrom<geo::Geometry>();
    _scan.resize(geop->Ncryostats());
    for(size_t cryoid=0; cryoid<_scan.size(); ++cryoid) {
      auto const& cryostat = geop->Cryostat(cryoid);
      auto& scan_cryo = _scan[cryoid];
      scan_cryo.resize(cryostat.NTPC());
      for(size_t tpcid=0; tpcid<scan_cryo.size(); ++tpcid) {
	auto const& tpc = cryostat.TPC(tpcid);
	auto& scan_tpc = scan_cryo[tpcid];
	scan_tpc.resize(tpc.Nplanes(),-1);
      }
    }
    //for(size_t cryo_id=0; cryo_id<_scan.size(); ++cryo_id){
    _valid_nplanes = 0;
    for(auto const& cryo_id : cryostat_v) {
      auto const& cryostat = geop->Cryostat(cryo_id);
      for(auto const& tpc_id : tpc_v) {
	if(!cryostat.HasTPC(tpc_id)) continue;
	auto const& tpc = cryostat.TPC(tpc_id);
	for(auto const& plane_id : plane_v) {
	  if(!tpc.HasPlane(plane_id)) continue;
	  _scan[cryo_id][tpc_id][plane_id] = _valid_nplanes;
	  //std::cout<<cryo_id<<" "<<tpc_id<<" "<<plane_id<<" ... "<<_valid_nplanes<< std::endl;
	  ++_valid_nplanes;
	}
      }
    }
  }
  
  
  void SuperaMCParticleCluster::initialize()
  {
    SuperaBase::initialize();
  }

  int SuperaMCParticleCluster::plane_index(unsigned int cryo_id, unsigned int tpc_id, unsigned int plane_id)
  {
    if(_scan.size() <= cryo_id)
      return -1;
    if(_scan[cryo_id].size() <= tpc_id)
      return -1;
    if(_scan[cryo_id][tpc_id].size() <= plane_id)
      return -1;

    return _scan[cryo_id][tpc_id][plane_id];
  }
  
  std::vector<supera::ParticleGroup> 
  SuperaMCParticleCluster::CreateParticleGroups()
  {
    const larcv::Particle invalid_part;
    auto const& larmcp_v = LArData<supera::LArMCParticle_t>();
    auto const& parent_pdg_v = _mcpl.ParentPdgCode();
    auto const& trackid2index = _mcpl.TrackIdToIndex();
    std::vector<supera::ParticleGroup> result(trackid2index.size());
    for(size_t index=0; index<larmcp_v.size(); ++index) {

      auto const& mcpart = larmcp_v[index];
      int pdg_code       = abs(mcpart.PdgCode());
      int mother_index   = -1;
      int track_id       = mcpart.TrackId();
      if(mcpart.Mother() < ((int)(trackid2index.size())))
	mother_index = trackid2index[mcpart.Mother()];

      //if(pdg_code != -11 && pdg_code != 11 && pdg_code != 22) continue;
      if(pdg_code > 1000000) continue;

      supera::ParticleGroup grp(_valid_nplanes);
      grp.part = this->MakeParticle(mcpart);
      if(mother_index >= 0)
	grp.part.parent_pdg_code(parent_pdg_v[index]);
      grp.valid=true;

      if(pdg_code == 22 || pdg_code == 11) {
	if(pdg_code == 22) {
	  // photon ... reset first, last, and end position
	  grp.type = supera::kPhoton;
	  grp.part.first_step(invalid_part.first_step());
	  grp.part.last_step(invalid_part.last_step());
	  grp.part.end_position(invalid_part.end_position());
	}
	else if(pdg_code == 11) {
	  
	  std::string prc = mcpart.Process();
	  if( prc == "muIoni" || prc == "hIoni")
	    grp.type = supera::kDelta;
	  else if( prc == "muMinusCaptureAtRest" || prc == "muPlusCaptureAtRest" || prc == "Decay" )
	    grp.type = supera::kDecay;
	  else if( prc == "compt" )
	    grp.type = supera::kCompton;
	  else if( prc == "phot")
	    grp.type = supera::kPhotoElectron;
	  else if( prc == "eIoni" )
	    grp.type = supera::kIonization;
	  else if( prc == "conv" )
	    grp.type = supera::kConversion;
	  else if( prc == "primary")
	    grp.type = supera::kPrimary;
	}
	result[track_id] = grp;
      }
      else { 
	grp.type = supera::kTrack;
	if(grp.part.pdg_code() == 2112)
	  grp.type = supera::kNeutron;
	result[track_id] = grp;
      }
    }
    return result;
  }

  void SuperaMCParticleCluster::AnalyzeSimEnergyDeposit(const larcv::Voxel3DMeta& meta, 
							std::vector<supera::ParticleGroup>& part_grp_v)
  {
    auto const& sedep_v = LArData<supera::LArSimEnergyDeposit_t>();
    auto const& trackid2index = _mcpl.TrackIdToIndex();
    LARCV_INFO() << "Processing SimEnergyDeposit array: " << sedep_v.size() << std::endl;
    size_t bad_sedep_counter = 0;
    std::set<size_t> missing_trackid;
    for(size_t sedep_idx=0; sedep_idx<sedep_v.size(); ++sedep_idx) {
      auto const& sedep = sedep_v.at(sedep_idx);

      VoxelID_t vox_id = meta.id(sedep.X(), sedep.Y(), sedep.Z());
      if(vox_id == larcv::kINVALID_VOXELID || !_world_bounds.contains(sedep.X(),sedep.Y(),sedep.Z())) {
	LARCV_DEBUG() << "Skipping sedep from track id " << sedep.TrackID() 
		      << " E=" << sedep.Energy()
		      << " pos=(" << sedep.X() << "," << sedep.Y() << "," << sedep.Z() << ")" << std::endl;
	continue;
      }

      LARCV_DEBUG() << "Recording sedep from track id " << sedep.TrackID() 
		    << " E=" << sedep.Energy() << std::endl;

      supera::EDep pt;
      pt.x = sedep.X();
      pt.y = sedep.Y();
      pt.z = sedep.Z();
      pt.t = sedep.T();
      pt.e = sedep.Energy();

      int track_id = abs(sedep.TrackID());
      if(track_id >= ((int)(trackid2index.size()))) {
	bad_sedep_counter++;
	missing_trackid.insert(track_id);
	continue;
      }

      auto& grp = part_grp_v[track_id];
      if(!grp.valid) continue;
      grp.vs.emplace(vox_id,pt.e,true);
      grp.AddEDep(pt);
    }
    if(bad_sedep_counter) {
      LARCV_WARNING() << bad_sedep_counter << " / " << sedep_v.size() << " SimEnergyDeposits "
		      << "(from " << missing_trackid.size() << " particles) did not find corresponding MCParticle!"
		      << std::endl;
    }
  }

  void SuperaMCParticleCluster::AnalyzeFirstLastStep(const larcv::Voxel3DMeta& meta, 
						     std::vector<supera::ParticleGroup>& part_grp_v)
  {
    auto const& sedep_v = LArData<supera::LArSimEnergyDeposit_t>();
    auto const& trackid2index = _mcpl.TrackIdToIndex();
    double tmin,tmax,xmax,xmin;
    xmin=tmin=1.e20;
    xmax=tmax=-1.e20;
    for(size_t sedep_idx=0; sedep_idx<sedep_v.size(); ++sedep_idx) {
      auto const& sedep = sedep_v.at(sedep_idx);

      VoxelID_t vox_id = meta.id(sedep.X(), sedep.Y(), sedep.Z());
      if(vox_id == larcv::kINVALID_VOXELID || !_world_bounds.contains(sedep.X(),sedep.Y(),sedep.Z()))
	continue;

      int track_id = abs(sedep.TrackID());
      if(track_id >= ((int)(trackid2index.size()))) 
	continue;

      supera::EDep pt;
      pt.x = sedep.X();
      pt.y = sedep.Y();
      pt.z = sedep.Z();
      pt.t = sedep.T();
      pt.e = sedep.Energy();
      tmin = std::min(pt.t,tmin);
      tmax = std::max(pt.t,tmax);
      xmin = std::min(pt.x,xmin);
      xmax = std::max(pt.x,xmax);
      part_grp_v[track_id].AddEDep(pt);

    }
    /*
    std::cout<<"T " << tmin << " => " << tmax 
	     <<" ... X " << xmin << " => " << xmax << std::endl;
    */
  }


  void SuperaMCParticleCluster::AnalyzeSimChannel(const larcv::Voxel3DMeta& meta3d,
						  std::vector<supera::ParticleGroup>& part_grp_v,
						  larcv::IOManager& mgr)
  {
    auto geop = lar::providerFrom<geo::Geometry>();
    auto const& sch_v = LArData<supera::LArSimCh_t>();
    auto const& trackid2index = _mcpl.TrackIdToIndex();
    LARCV_INFO() << "Processing SimChannel array: " << sch_v.size() << std::endl;

    size_t ctr_missing_trackid = 0;
    size_t ctr_invalid_trackid = 0;
    size_t ctr_invalid_3d = 0;
    size_t ctr_invalid_2d = 0;
    size_t ctr_total_ide = 0;

    std::set<size_t> missing_trackid;

    bool analyze3d = false;
    bool analyze2d = false;
    bool skipped2d = false;
    bool skipped3d = false;

    std::pair<double,double> recorded_xrange2d, recorded_trange2d;
    std::pair<double,double> xrange2d,trange2d;
    
    xrange2d.first = trange2d.first = recorded_xrange2d.first = recorded_trange2d.first = 1.e20;
    xrange2d.second = trange2d.second = recorded_xrange2d.second = recorded_trange2d.second = -1.e20;

    larcv::VoxelSet kvs;

    for(auto const& sch : sch_v) {

      auto ch = sch.Channel();
      // Check if should use this channel (3d)
      analyze3d = (_projection_id == ::supera::ChannelToProjectionID(ch));
      // Check if should use this channel (2d)
      auto wid_v = geop->ChannelToWire(ch);
      assert(wid_v.size() == 1);
      auto const& wid = wid_v[0];
      auto vs2d_idx = this->plane_index(wid.Cryostat,wid.TPC,wid.Plane);
      analyze2d = (vs2d_idx >= 0 && _meta2d_v[vs2d_idx].min_x() <= wid.Wire && wid.Wire < _meta2d_v[vs2d_idx].max_x());

      //if(ch!=267) continue;
      //std::cout<<"Analyze? ch " << ch << " 2d " << analyze2d << " 3d " << analyze3d << std::endl;
      if(!analyze2d && !analyze3d) continue;

      for (auto const tick_ides : sch.TDCIDEMap()) {
	
	//x_pos = (supera::TPCTDC2Tick(tick_ides.first) + time_offset) * supera::TPCTickPeriod()  * supera::DriftVelocity();
	double x_pos = supera::TPCTDC2Tick(tick_ides.first) * supera::TPCTickPeriod()  * supera::DriftVelocity();
        //std::cout << tick_ides.first << " : " << supera::TPCTDC2Tick(tick_ides.first) << " : " << time_offset << " : " << x_pos << std::flush;
        //std::cout << (supera::TPCTDC2Tick(tick_ides.first) + time_offset) << std::endl
        //<< (supera::TPCTDC2Tick(tick_ides.first) + time_offset) * supera::TPCTickPeriod() << std::endl
        //<< (supera::TPCTDC2Tick(tick_ides.first) + time_offset) * supera::TPCTickPeriod() * supera::DriftVelocity() << std::endl
        //<< std::endl;

        for (auto const& edep : tick_ides.second) {

	  ctr_total_ide++;

	  supera::EDep pt;
	  pt.x = x_pos;
	  pt.y = edep.y;
	  pt.z = edep.z;
	  pt.e = edep.energy;

	  if(_use_true_pos)
	    pt.x = edep.x;

	  double wire_pos = larcv::kINVALID_DOUBLE;
	  double time_pos = larcv::kINVALID_DOUBLE;
	  skipped2d = skipped3d = false;
          //supera::ApplySCE(x,y,z);
          //std::cout << " ... " << x << std::end;l
	  /*
	  if(vs2d_idx==0) {
	    auto const& meta2d = _meta2d_v[vs2d_idx];
	    wire_pos = wid.Wire;
	    time_pos = supera::TPCTDC2Tick(tick_ides.first);
	    if( (meta2d.min_x() <= wire_pos && wire_pos < meta2d.max_x()) &&
		(meta2d.min_y() <= time_pos && time_pos < meta2d.max_y()) ) {
	      auto vox2d_id = meta2d.id(wire_pos,time_pos);
	      kvs.emplace(vox2d_id,pt.e,true);
	    }
	  }
	  */
	  int track_id = abs(edep.trackID);
	  if(track_id >= ((int)(trackid2index.size()))) {
	    ++ctr_missing_trackid;
	    missing_trackid.insert(track_id);
	    continue;
	  }

	  auto& grp = part_grp_v[track_id];
	  if(!grp.valid) {
	    ++ctr_invalid_trackid;
	    continue;
	  }
	  /*
	  if(vs2d_idx==0) {
	    auto const& meta2d = _meta2d_v[vs2d_idx];
	    wire_pos = wid.Wire;
	    time_pos = supera::TPCTDC2Tick(tick_ides.first);
	    if( (meta2d.min_x() <= wire_pos && wire_pos < meta2d.max_x()) &&
		(meta2d.min_y() <= time_pos && time_pos < meta2d.max_y()) ) {
	      auto vox2d_id = meta2d.id(wire_pos,time_pos);
	      kvs.emplace(vox2d_id,pt.e,true);
	    }
	  }
	  */
	  if(analyze3d) {
	    VoxelID_t vox_id = meta3d.id(pt.x, pt.y, pt.z);
	    if(vox_id != larcv::kINVALID_VOXELID) {
	      grp.vs.emplace(vox_id,pt.e,true);
	      grp.AddEDep(pt);
	    }
	    else { skipped3d = true; ++ctr_invalid_3d; }
	  }

	  if(analyze2d) {
	    VoxelID_t vox2d_id = larcv::kINVALID_VOXELID;
	    auto const& meta2d = _meta2d_v[vs2d_idx];
	    wire_pos = wid.Wire;
	    time_pos = supera::TPCTDC2Tick(tick_ides.first);

	    if(pt.x < xrange2d.first ) xrange2d.first  = pt.x;
	    if(pt.x > xrange2d.second) xrange2d.second = pt.x;
	    if(time_pos < trange2d.first ) trange2d.first  = time_pos;
	    if(time_pos > trange2d.second) trange2d.second = time_pos;

	    if( (meta2d.min_x() <= wire_pos && wire_pos < meta2d.max_x()) &&
		(meta2d.min_y() <= time_pos && time_pos < meta2d.max_y()) )
	      vox2d_id = meta2d.id(wire_pos,time_pos);

	    //if(track_id == 7305 || track_id == 7311)
	    //std::cout << "track " << track_id << " (wire,time) = (" << wire_pos << "," << time_pos << ") ... vox2d id " << vox2d_id << std::endl;

	    if(vox2d_id != larcv::kINVALID_VOXELID) {
	      if(pt.x < recorded_xrange2d.first ) recorded_xrange2d.first  = pt.x;
	      if(pt.x > recorded_xrange2d.second) recorded_xrange2d.second = pt.x;
	      if(time_pos < recorded_trange2d.first ) recorded_trange2d.first  = time_pos;
	      if(time_pos > recorded_trange2d.second) recorded_trange2d.second = time_pos;
	      grp.vs2d_v[vs2d_idx].emplace(vox2d_id,edep.numElectrons,true);
	    }
	    else {
	      skipped2d = true;
	      ++ctr_invalid_2d;
	    }
	  }

	  LARCV_DEBUG() << "IDE ... TrackID " << edep.trackID << " E " << edep.energy << " Q " << edep.numElectrons << std::endl;
	  
	  if(skipped2d || skipped3d) {

	    LARCV_INFO() << "Skipped 2d/3d " << (skipped2d ? "1" : "0") << "/" << (skipped3d ? "1" : "0") << " TrackID " << edep.trackID
			 << " pos (" << pt.x << "," << pt.y << "," << pt.z << ")"
			 << " ... TDC " << tick_ides.first << " => Tick " << time_pos 
			 << "... Wire " << wid.Wire 
			 <<std::endl;
	  }

	}
      }
      //std::cout<<" done" << std::endl;
    }

    if(ctr_missing_trackid || ctr_invalid_trackid || ctr_invalid_2d || ctr_invalid_3d) {
      LARCV_WARNING() << "Total IDE ctr " << ctr_total_ide << std::endl
		      << " ... due to missing track ID " << ctr_missing_trackid 
		      << " from " << missing_trackid.size() << " track IDs" << std::endl
		      << " ... due to invalid particle " << ctr_invalid_trackid << std::endl
		      << " ... due to invalid 2d index " << ctr_invalid_2d << std::endl
		      << " ... due to invalid 3d index " << ctr_invalid_3d << std::endl
		      << std::endl;
      if(ctr_invalid_2d) {
	LARCV_WARNING() << "  X range: " << xrange2d.first << " => " << xrange2d.second 
			<< " ... T range: " << trange2d.first << " => " << trange2d.second << std::endl
			<< " Recorded: " << recorded_xrange2d.first << " => " << recorded_xrange2d.second
			<< " ... T range: " << recorded_trange2d.first << " => " << recorded_trange2d.second << std::endl;

      }
    }

    auto out_data = (larcv::EventSparseTensor2D*)(mgr.get_data("sparse2d","kazu"));
    auto kmeta = _meta2d_v[0];
    out_data->emplace(std::move(kvs),std::move(kmeta));

  }


  void SuperaMCParticleCluster::MergeShowerIonizations(std::vector<supera::ParticleGroup>& part_grp_v)
  {

    int merge_ctr = 0;
    int invalid_ctr = 0;
    do {
      merge_ctr = 0;
      for(auto& grp : part_grp_v) {
	if(!grp.valid) continue;
	if(grp.type != supera::kIonization) continue;
	// merge to a valid "mother"
	bool parent_found = false;
	int parent_index = grp.part.parent_track_id();
	int parent_index_before = grp.part.track_id();
	while(1) {
	  //std::cout<< "Inspecting: " << grp.part.track_id() << " => " << parent_index << std::endl;
	  if(parent_index <0) {
	    LARCV_DEBUG() << "Invalid parent track id " << parent_index
			  << " Could not find a parent for " << grp.part.track_id() << " PDG " << grp.part.pdg_code()
			  << " " << grp.part.creation_process() << " E = " << grp.part.energy_init() 
			  << " (" << grp.part.energy_deposit() << ") MeV" << std::endl;
	    auto const& parent = part_grp_v[parent_index_before].part;
	    LARCV_DEBUG() << "Previous parent: " << parent.track_id() << " PDG " << parent.pdg_code() 
			  << " " << parent.creation_process()
			  << std::endl;
	    parent_found=false;
	    invalid_ctr++;
	    break;
	    //throw std::exception();
	  }
	  auto const& parent = part_grp_v[parent_index];
	  parent_found = parent.valid;
	  if(parent_found) break;
	  else{
	    int ancestor_index = parent.part.parent_track_id();
	    if(ancestor_index == parent_index) {
	      LARCV_INFO() << "Particle " << parent_index << " is root and invalid particle..." << std::endl
			   << "PDG " << parent.part.pdg_code() << " " << parent.part.creation_process() << std::endl;
	      break;
	    }
	    parent_index_before = parent_index;
	    parent_index = ancestor_index;
	  }
	}
	// if parent is found, merge
	if(parent_found) {
	  auto& parent = part_grp_v[parent_index];
	  parent.Merge(grp);
	  merge_ctr++;
	  /*
	  std::cout<<"Track " << grp.part.track_id() << " PDG " << grp.part.pdg_code()
		  <<" ... parent found " << parent.part.track_id() << " PDG " << parent.part.pdg_code() << std::endl;
	  */
	}
      }
      LARCV_INFO() << "Ionization merge counter: " << merge_ctr << " invalid counter: " << invalid_ctr << std::endl;
    }while(merge_ctr>0);
  }

  void SuperaMCParticleCluster::ApplyEnergyThreshold(std::vector<supera::ParticleGroup>& part_grp_v)
  {
    // Loop again and eliminate voxels that has energy below threshold
    for(auto& grp : part_grp_v) {
      larcv::VoxelSet vs;
      vs.reserve(grp.vs.size());
      for(auto const& vox : grp.vs.as_vector()) {
	if(vox.value() < _edep_threshold) continue;
	vs.emplace(vox.id(),vox.value(),true);
      }
      grp.vs = vs;
      // If compton, here decide whether it should be supera::kComptonHE (high energy)
      if(grp.type == supera::kCompton && grp.vs.size() > _compton_size && grp.vs.sum() > _compton_energy)
        grp.type = supera::kComptonHE;
    }
  }


  void SuperaMCParticleCluster::MergeShowerConversion(std::vector<supera::ParticleGroup>& part_grp_v)
  {
    int merge_ctr = 0;
    int invalid_ctr = 0;
    do {
      merge_ctr = 0;
      for(auto& grp : part_grp_v) {
	if(!grp.valid) continue;
	//if(grp.type != supera::kIonization && grp.type != supera::kConversion && grp.type != supera::kComptonHE) continue;
	if(grp.type != supera::kIonization && grp.type != supera::kConversion) continue;
	// merge to a valid "mother"
	bool parent_found = false;
	int parent_index = grp.part.parent_track_id();
	int parent_index_before = grp.part.track_id();
	while(1) {
	  //std::cout<< "Inspecting: " << grp.part.track_id() << " => " << parent_index << std::endl;
	  if(parent_index <0) {
	    LARCV_DEBUG() << "Invalid parent track id " << parent_index
			  << " Could not find a parent for " << grp.part.track_id() << " PDG " << grp.part.pdg_code()
			  << " " << grp.part.creation_process() << " E = " << grp.part.energy_init() 
			  << " (" << grp.part.energy_deposit() << ") MeV" << std::endl;
	    auto const& parent = part_grp_v[parent_index_before].part;
	    LARCV_DEBUG() << "Previous parent: " << parent.track_id() << " PDG " << parent.pdg_code() 
			  << " " << parent.creation_process()
			  << std::endl;
	    parent_found=false;
	    invalid_ctr++;
	    break;
	    //throw std::exception();
	  }
	  auto const& parent = part_grp_v[parent_index];
	  parent_found = parent.valid;
	  if(parent_found) break;
	  else{
	    int ancestor_index = parent.part.parent_track_id();
	    if(ancestor_index == parent_index) {
	      LARCV_INFO() << "Particle " << parent_index << " is root and invalid particle..." << std::endl
			   << "PDG " << parent.part.pdg_code() << " " << parent.part.creation_process() << std::endl;
	      break;
	    }
	    parent_index_before = parent_index;
	    parent_index = ancestor_index;
	  }
	}
	// if parent is found, merge
	if(parent_found) {
	  auto& parent = part_grp_v[parent_index];
	  parent.Merge(grp);
	  merge_ctr++;
	  /*
	  std::cout<<"Track " << grp.part.track_id() << " PDG " << grp.part.pdg_code() << " " << grp.part.creation_process()
		   <<" ... parent found " << parent.part.track_id() << " PDG " << parent.part.pdg_code() << " " << parent.part.creation_process()
		   << std::endl;
	  */
	}
      }
      LARCV_INFO() << "Merge counter: " << merge_ctr << " invalid counter: " << invalid_ctr << std::endl;
    }while(merge_ctr>0);
  }


  void SuperaMCParticleCluster::MergeShowerTouching(const larcv::Voxel3DMeta& meta,
						    std::vector<supera::ParticleGroup>& part_grp_v)
  {
    int merge_ctr = 0;
    int invalid_ctr = 0;
    do {
      merge_ctr = 0;
      for(auto& grp : part_grp_v) {
	if(!grp.valid) continue;
	//if(grp.type != supera::kIonization && grp.type != supera::kConversion && grp.type != supera::kComptonHE) continue;
	if(grp.type == supera::kTrack || grp.type == supera::kNeutron || grp.type == supera::kDelta || grp.type == supera::kDecay) continue;
	// search for a possible parent
	int parent_trackid = -1;
	// a direct parent ?
	if(part_grp_v[grp.part.parent_track_id()].valid)
	  parent_trackid = grp.part.parent_track_id();
	else {
	  for(size_t shower_trackid = 0; shower_trackid<part_grp_v.size(); ++shower_trackid) {
	    auto const& candidate_grp = part_grp_v[shower_trackid];
	    if(shower_trackid == grp.part.parent_track_id() || !candidate_grp.valid) continue;
	    for(auto const& trackid : candidate_grp.trackid_v) {
	      if(trackid != grp.part.parent_track_id()) continue;
	      parent_trackid = shower_trackid;
	      break;
	    }
	    if(parent_trackid >= 0) break;
	  }
	}
	if(parent_trackid < 0 || parent_trackid == (int)(grp.part.track_id())) continue;
	auto& parent = part_grp_v[parent_trackid];
	auto parent_type = part_grp_v[parent_trackid].type;
	if(parent_type == supera::kTrack || parent_type == supera::kNeutron) continue;
	if(this->IsTouching(meta,grp.vs,parent.vs)) {
	  // if parent is found, merge
	  parent.Merge(grp);
	  merge_ctr++;
	  /*
	  std::cout<<"Track " << grp.part.track_id() << " PDG " << grp.part.pdg_code() << " " << grp.part.creation_process()
		   <<" ... parent found " << parent.part.track_id() << " PDG " << parent.part.pdg_code() << " " << parent.part.creation_process()
		   << std::endl;
	  */	  
	}
      }
      LARCV_INFO() << "Merge counter: " << merge_ctr << " invalid counter: " << invalid_ctr << std::endl;
    }while(merge_ctr>0);
  }

  void SuperaMCParticleCluster::MergeShowerTouching2D(std::vector<supera::ParticleGroup>& part_grp_v)
  {
    std::vector<std::vector<size_t> > children_v(part_grp_v.size());
    std::vector<bool> valid_v(part_grp_v.size());
    for(size_t plane=0; plane < _valid_nplanes; ++plane) {
      // first copy the merge history and validity from 3D
      auto const& meta = _meta2d_v[plane];
      size_t vox2d_ctr=0;
      for(size_t i=0; i<part_grp_v.size(); ++i) {
	children_v[i] = part_grp_v[i].trackid_v;
	valid_v[i]    = part_grp_v[i].valid;
	if(part_grp_v[i].vs2d_v.size() > plane)
	  vox2d_ctr    += part_grp_v[i].vs2d_v.at(plane).size();
      }
      if(vox2d_ctr<1) continue;

      int merge_ctr = 0;
      int invalid_ctr = 0;
      do {
	merge_ctr = 0;
	for(size_t part_idx=0; part_idx<part_grp_v.size(); ++part_idx) {
	  auto& grp = part_grp_v[part_idx];
	  if(valid_v[part_idx]) continue;
	  if(plane >= grp.vs2d_v.size()) continue;
	  if(grp.vs2d_v[plane].size()<1) continue;
	  if(grp.type == supera::kTrack || grp.type == supera::kNeutron || grp.type == supera::kDelta || grp.type == supera::kDecay) continue;
	  // search for a possible parent
	  int parent_trackid = -1;
	  // a direct parent ?
	  if(valid_v[grp.part.parent_track_id()])
	    parent_trackid = grp.part.parent_track_id();
	  else {
	    for(size_t shower_trackid = 0; shower_trackid<part_grp_v.size(); ++shower_trackid) {
	      if(shower_trackid == grp.part.parent_track_id() || !valid_v[shower_trackid]) continue;
	      for(auto const& trackid : children_v[shower_trackid]) {
		if(trackid != grp.part.parent_track_id()) continue;
		parent_trackid = shower_trackid;
		break;
	      }
	      if(parent_trackid >= 0) break;
	    }
	  }
	  if(parent_trackid < 0 || parent_trackid == (int)(grp.part.track_id())) continue;
	  auto& parent = part_grp_v[parent_trackid];
	  auto parent_type = part_grp_v[parent_trackid].type;
	  if(parent_type == supera::kTrack || parent_type == supera::kNeutron) continue;
	  if(this->IsTouching2D(meta,grp.vs2d_v[plane],parent.vs2d_v[plane])) {
	    // if parent is found, merge 2d voxels
	    
	    valid_v[grp.part.track_id()] = false;
	    children_v[parent_trackid].push_back(grp.part.track_id());
	    for(auto const& trackid : children_v[grp.part.track_id()])
	      children_v[parent_trackid].push_back(trackid);
	    children_v[part_idx].clear();
	    // merge 2D voxels
	    for(auto const& vox : grp.vs2d_v[plane].as_vector())
	      parent.vs2d_v[plane].emplace(vox.id(),vox.value(),true);
	    grp.vs2d_v[plane].clear_data();
	    merge_ctr++;
	    /*
	    std::cout<<"Track " << grp.part.track_id() << " PDG " << grp.part.pdg_code() << " " << grp.part.creation_process()
		     <<" ... parent found " << parent.part.track_id() << " PDG " << parent.part.pdg_code() << " " << parent.part.creation_process()
		     << std::endl;
	    */
	  }
	}
	LARCV_INFO() << "Merge counter: " << merge_ctr << " invalid counter: " << invalid_ctr << std::endl;
      }while(merge_ctr>0);
    }
  }


  void SuperaMCParticleCluster::MergeShowerDeltas(std::vector<supera::ParticleGroup>& part_grp_v)
  {
    for(auto& grp : part_grp_v) {
      if(grp.type != supera::kDelta) continue;
      int parent_trackid = grp.part.parent_track_id();
      auto& parent = part_grp_v[parent_trackid];
      if(!parent.valid) continue;
      
      // if voxel count is smaller than delta ray requirement, simply merge
      if(grp.vs.size() < _delta_size) {
	// if parent is found, merge
	/*
	if(grp.vs.size()>0) {
	  std::cout<<"Track " << grp.part.track_id() << " PDG " << grp.part.pdg_code() 
		   << " " << grp.part.creation_process() << " vox count " << grp.vs.size() << std::endl
		   <<" ... parent found " << parent.part.track_id() 
		   << " PDG " << parent.part.pdg_code() << " " << parent.part.creation_process()
		   << std::endl;
	  for(auto const& vs : grp.vs2d_v)
	    std::cout<<vs.size() << " " << std::flush;
	  std::cout<<std::endl;	  
	}
	*/
	parent.Merge(grp);
      }else{
	// check unique number of voxels
	size_t unique_voxel_count = 0;
	for(auto const& vox : grp.vs.as_vector()) {
	  if(parent.vs.find(vox.id()).id() != larcv::kINVALID_VOXELID)
	    continue;
	  ++unique_voxel_count;
	}
	if(unique_voxel_count < _delta_size) {
	  // if parent is found, merge
	  /*
	  if(unique_voxel_count>0) {
	    std::cout<<"Track " << grp.part.track_id() << " PDG " << grp.part.pdg_code() 
		     << " " << grp.part.creation_process() << " vox count " << grp.vs.size() << std::endl
		     <<" ... parent found " << parent.part.track_id() 
		     << " PDG " << parent.part.pdg_code() << " " << parent.part.creation_process()
		     << std::endl;
	    for(auto const& vs : grp.vs2d_v)
	      std::cout<<vs.size() << " " << std::flush;
	    std::cout<<std::endl;
	  }
	  */
	  parent.Merge(grp);
	  /*
	  if(unique_voxel_count>0)
	    std::cout<<"Track " << grp.part.track_id() << " PDG " << grp.part.pdg_code() 
		     << " " << grp.part.creation_process() << " vox count " << grp.vs.size() << std::endl
		     <<" ... parent found " << parent.part.track_id() 
		     << " PDG " << parent.part.pdg_code() << " " << parent.part.creation_process()
		     << std::endl;
	  */
	}
	/*
	else{
	  std::cout<<"Track " << grp.part.track_id() << " PDG " << grp.part.pdg_code() 
		   << " " << grp.part.creation_process() << " vox count " << grp.vs.size() << std::endl
		   <<" ... parent found " << parent.part.track_id() 
		   << " PDG " << parent.part.pdg_code() << " " << parent.part.creation_process()
		   << std::endl;
	  for(auto const& vs : grp.vs2d_v)
	    std::cout<<vs.size() << " " << std::flush;
	  std::cout<<std::endl;
	}
	*/
      }
    }
  }

  bool SuperaMCParticleCluster::process(IOManager& mgr)
  {

    SuperaBase::process(mgr);
    larcv::Voxel3DMeta meta3d;

    if(!_ref_meta3d_cluster3d.empty()) {
      auto const& ev_cluster3d = mgr.get_data<larcv::EventClusterVoxel3D>(_ref_meta3d_cluster3d);
      meta3d = ev_cluster3d.meta();
    }
    else if(!_ref_meta3d_tensor3d.empty()) {
      auto const& ev_tensor3d = mgr.get_data<larcv::EventSparseTensor3D>(_ref_meta3d_tensor3d);
      meta3d = ev_tensor3d.meta();
    }

    // fill 2d meta if needed
    _meta2d_v.clear();
    _meta2d_v.resize(_valid_nplanes);
    for(size_t cryo_id=0; cryo_id < _scan.size(); cryo_id++) {
      for(size_t tpc_id=0; tpc_id < _scan[cryo_id].size(); tpc_id++) {
	for(size_t plane_id=0; plane_id < _scan[cryo_id][tpc_id].size(); ++plane_id) {
	  auto meta2d_idx = _scan[cryo_id][tpc_id][plane_id];
	  if(meta2d_idx < 0) continue;
	  std::string product_name = _ref_meta2d_tensor2d + "_";
	  product_name = product_name + std::to_string(cryo_id) + "_" + std::to_string(tpc_id) + "_" + std::to_string(plane_id);
	  larcv::ProducerName_t product_id("sparse2d",product_name);
	  if(mgr.producer_id(product_id) == larcv::kINVALID_PRODUCER) {
	    LARCV_CRITICAL() << "Could not find tensor2d with a name: " << product_name << std::endl;
	    throw larbys();
	  }
	  _meta2d_v[meta2d_idx] = mgr.get_data<larcv::EventSparseTensor2D>(product_name).as_vector().front().meta();
	}
      }
    }

    // Build MCParticle List
    auto const& larmcp_v = LArData<supera::LArMCParticle_t>();
    auto const *ev = GetEvent();
    _mcpl.Update(larmcp_v,ev->id().run(),ev->id().event());

    auto const& trackid2index = _mcpl.TrackIdToIndex();
    // Create ParticleGroup
    auto part_grp_v = this->CreateParticleGroups();

    // Fill Voxel Information
    if(_use_sed)
      this->AnalyzeSimEnergyDeposit(meta3d, part_grp_v);
    else
      this->AnalyzeSimChannel(meta3d, part_grp_v, mgr);

    // Define particle first/last points based on SED
    if(_use_sed_points && !_use_sed)
      this->AnalyzeFirstLastStep(meta3d, part_grp_v);

    // Merge fragments of showers
    this->MergeShowerIonizations(part_grp_v);

    // Apply energy threshold
    this->ApplyEnergyThreshold(part_grp_v);

    // Merge fragments of showers
    this->MergeShowerConversion(part_grp_v);

    // Merge touching shower fragments
    this->MergeShowerTouching(meta3d,part_grp_v);

    // Merge touching showers in 2D
    this->MergeShowerTouching2D(part_grp_v);

    // merge too small deltas into tracks
    this->MergeShowerDeltas(part_grp_v);
    /*
    for(auto const& grp : shower_grp_v) {
      if(grp.type != supera::kDelta) continue;
      std::cout << "Track ID " << grp.part.track_id() << " PDG " << grp.part.pdg_code() 
		<< " valid? " << grp.valid
		<< " 3D voxels " << grp.vs.size() << " 2D voxels " << std::flush;
      for(auto const& vs2d : grp.vs2d_v) std::cout << vs2d.size() << " " << std::flush;
      std::cout<<std::endl;
    }
    */    

    // Assign output IDs
    std::vector<int> trackid2output(trackid2index.size(),-1);
    std::vector<int> output2trackid;
    output2trackid.reserve(trackid2index.size());
    for(auto& grp : part_grp_v) {
      if(!grp.valid) continue;
      if(grp.size_all()<1) continue;
      grp.part.energy_deposit((grp.vs.size() ? grp.vs.sum() : 0.));
      if(grp.type == supera::kNeutron || grp.type == supera::kCompton || grp.type == supera::kPhotoElectron) continue;
      size_t output_counter = output2trackid.size();
      grp.part.id(output_counter);
      trackid2output[grp.part.track_id()] = output_counter;
      for(auto const& child : grp.trackid_v)
	trackid2output[child] = output_counter;
      output2trackid.push_back(grp.part.track_id());
      ++output_counter;
    }

    // Assign relationships
    for(auto const& trackid : output2trackid) {
      auto& grp = part_grp_v[trackid];
      if( abs(grp.part.pdg_code()) != 11 && abs(grp.part.pdg_code()) != 22 ) continue;
      int parent_trackid = grp.part.parent_track_id();
      if(trackid2output[parent_trackid] >= 0) {
      /*
      if(trackid2output[parent_trackid] < 0)
	grp.part.parent_id(grp.part.id());
      else {
      */
	grp.part.parent_id(trackid2output[parent_trackid]);
	int parent_output_id = trackid2output[parent_trackid];
	int parent_id = output2trackid[parent_output_id]; 
	if(part_grp_v[parent_id].valid)
	  part_grp_v[parent_id].part.children_id(grp.part.id());
      }
    }

    // At this point, count total number of voxels (will be used for x-check later)
    size_t total_vs_size = 0;
    for(auto& grp : part_grp_v) {
      if(!grp.valid) continue;
      if(grp.size_all()<1) continue;
      total_vs_size += grp.vs.size();
      // Also define particle "first step" and "last step"
      auto& part = grp.part;
      auto const& first_pt = grp.first_pt;
      auto const& last_pt  = grp.last_pt;
      //std::cout<<first_pt.x<< " " << first_pt.y << " " << first_pt.z << std::endl;
      if(first_pt.t != larcv::kINVALID_DOUBLE) 
	part.first_step(first_pt.x,first_pt.y,first_pt.z,first_pt.t);
      if(last_pt.t  != larcv::kINVALID_DOUBLE)
	part.last_step(last_pt.x,last_pt.y,last_pt.z,last_pt.t);
    }
    
    // Unique group id for output particles
    int group_counter = -1;

    // loop over MCShower to assign parent/ancestor information
    auto const& mcs_v = LArData<supera::LArMCShower_t>();
    LARCV_INFO() << "Processing MCShower array: " << mcs_v.size() << std::endl;
    for(auto const& mcs : mcs_v) {
      int track_id = mcs.TrackID();
      if(track_id >= ((int)(trackid2output.size()))) {
	LARCV_INFO() << "MCShower " << track_id << " PDG " << mcs.PdgCode() 
		     << " not found in output group..." << std::endl;
	continue;
      }
      
      int output_id = trackid2output[track_id];
      int group_id  = -1;
      if(output_id >= 0) {
	auto& grp = part_grp_v[track_id];
	if(grp.part.group_id() == larcv::kINVALID_INSTANCEID) {
	  if(group_id < 0) { 
	    group_id = group_counter + 1;
	    ++group_counter;
	  }
	  grp.part.group_id(group_id);
	}
	// see if first step is not set yet
	if(grp.first_pt.t == larcv::kINVALID_DOUBLE)
	  grp.part.first_step(mcs.DetProfile().X(),mcs.DetProfile().Y(),mcs.DetProfile().Z(),mcs.DetProfile().T());
	grp.part.parent_position(mcs.MotherStart().X(),
				 mcs.MotherStart().Y(),
				 mcs.MotherStart().Z(),
				 mcs.MotherStart().T());
	grp.part.parent_creation_process(mcs.MotherProcess());
	grp.part.ancestor_position(mcs.AncestorStart().X(),
				   mcs.AncestorStart().Y(),
				   mcs.AncestorStart().Z(),
				   mcs.AncestorStart().T());
	grp.part.ancestor_track_id(mcs.AncestorTrackID());
	grp.part.ancestor_pdg_code(mcs.AncestorPdgCode());
	grp.part.ancestor_creation_process(mcs.AncestorProcess());
      }

      for(auto const& child : mcs.DaughterTrackID()) {
	//if(child < trackid2output.size() && trackid2output[child] < 0)
	if(child < trackid2output.size() && trackid2output[child] >= 0) {
	  trackid2output[child] = output_id;
	  auto& grp = part_grp_v[child];
	  if(grp.part.group_id() == larcv::kINVALID_INSTANCEID) {
	    if(group_id < 0) {
	      group_id = group_counter + 1;
	      ++group_counter;
	    }
	    grp.part.group_id(group_id);
	  }
	  grp.part.ancestor_position(mcs.AncestorStart().X(),
				     mcs.AncestorStart().Y(),
				     mcs.AncestorStart().Z(),
				     mcs.AncestorStart().T());
	  grp.part.ancestor_track_id(mcs.AncestorTrackID());
	  grp.part.ancestor_pdg_code(mcs.AncestorPdgCode());
	  grp.part.ancestor_creation_process(mcs.AncestorProcess());
	}
      }
    }

    // loop over MCTrack to assign parent/ancestor information
    auto const& mct_v = LArData<supera::LArMCTrack_t>();
    LARCV_INFO() << "Processing MCTrack array: " << mct_v.size() << std::endl;
    for(auto const& mct : mct_v) {
      int track_id = mct.TrackID();
      int output_id = trackid2output[track_id];
      int group_id = -1;
      if(output_id >= 0) {
	auto& grp = part_grp_v[track_id];
	if(group_id < 0) {
	  group_id = group_counter + 1;
	  ++group_counter;
	}
	if(grp.first_pt.t == larcv::kINVALID_DOUBLE && mct.size())
	  grp.part.first_step(mct.front().X(),mct.front().Y(),mct.front().Z(),mct.front().T());
	if(grp.last_pt.t == larcv::kINVALID_DOUBLE && mct.size()) 
	  grp.part.last_step(mct.back().X(),mct.back().Y(),mct.back().Z(),mct.back().T());
	grp.part.group_id(group_id);
	grp.part.parent_position(mct.MotherStart().X(),
				 mct.MotherStart().Y(),
				 mct.MotherStart().Z(),
				 mct.MotherStart().T());
	grp.part.parent_creation_process(mct.MotherProcess());
	grp.part.ancestor_position(mct.AncestorStart().X(),
				   mct.AncestorStart().Y(),
				   mct.AncestorStart().Z(),
				   mct.AncestorStart().T());
	grp.part.ancestor_track_id(mct.AncestorTrackID());
	grp.part.ancestor_pdg_code(mct.AncestorPdgCode());
	grp.part.ancestor_creation_process(mct.AncestorProcess());
      }
      for(size_t output_index=0; output_index<output2trackid.size(); ++output_index) {
	int output_trackid = output2trackid[output_index];
	auto& grp = part_grp_v[output_trackid];
	if((int)(grp.part.parent_track_id()) != track_id) continue;
	/*
	if(group_id < 0) {
	  group_id = group_counter + 1;
	  ++group_counter;
	}
	grp.part.group_id(group_id);
	*/
	grp.part.ancestor_position(mct.AncestorStart().X(),
				   mct.AncestorStart().Y(),
				   mct.AncestorStart().Z(),
				   mct.AncestorStart().T());
	grp.part.ancestor_track_id(mct.AncestorTrackID());
	grp.part.ancestor_pdg_code(mct.AncestorPdgCode());
	grp.part.ancestor_creation_process(mct.AncestorProcess());
      }
    }

    // for shower particles with invalid parent ID, attempt a search
    for(size_t out_index=0; out_index<output2trackid.size(); ++out_index) {
      int trackid = output2trackid[out_index];
      auto& grp = part_grp_v[trackid];
      if(grp.part.parent_id() != larcv::kINVALID_INSTANCEID) continue;
      if(grp.type != supera::kComptonHE && grp.type != supera::kPhoton && grp.type != supera::kComptonHE && grp.type != supera::kConversion)
	continue;
      int own_partid = grp.part.id();
      // initiate a search of parent in the valid output particle
      int parent_trackid = grp.part.parent_track_id();
      int parent_partid  = -1;
      while(1) {
	if(parent_trackid >= ((int)(trackid2index.size())) || trackid2index[parent_trackid] <0)
	  break;
	if(parent_trackid < ((int)(trackid2output.size())) && 
	   trackid2output[parent_trackid] >= 0 &&
	   part_grp_v[parent_trackid].valid ) {
	  //parent_partid = trackid2output[parent_trackid];
	  parent_partid = part_grp_v[parent_trackid].part.id();
	  break;
	}
	parent_trackid = larmcp_v[trackid2index[parent_trackid]].Mother();
      }
      if(parent_partid >=0) {
	grp.part.parent_id(parent_partid);
	part_grp_v[parent_trackid].part.children_id(own_partid);
	LARCV_INFO() << "PartID " << own_partid << " (output index " << out_index << ") assigning parent " << parent_partid << std::endl;
      }else{
	grp.part.parent_id(grp.part.id());
	LARCV_INFO() << "PartID " << own_partid << " (output index " << out_index << ") assigning itself as a parent..." << std::endl;
      }
    }

    // Now loop over otuput particle list and check if any remaining group id needs to be assigned
    // Use ancestor to group...
    std::map<size_t,size_t> group_id_by_ancestor;
    for(size_t output_index=0; output_index<output2trackid.size(); ++output_index) {
      auto& grp = part_grp_v[output2trackid[output_index]];
      if(grp.part.group_id() != larcv::kINVALID_INSTANCEID) continue;
      // If delta, its own grouping
      if(grp.type == supera::kDelta) {
	grp.part.group_id(group_counter + 1);
	for(auto const& child_index : grp.part.children_id())
	  part_grp_v[output2trackid[child_index]].part.group_id(group_counter+1);
	++group_counter;
      }else{
	if(group_id_by_ancestor.find(grp.part.ancestor_track_id()) == group_id_by_ancestor.end()) {
	  grp.part.group_id(group_counter + 1);
	  group_id_by_ancestor[grp.part.ancestor_track_id()] = group_counter + 1;
	  ++group_counter;
	}else
	  grp.part.group_id(group_id_by_ancestor[grp.part.ancestor_track_id()]);
      }
    }

    // now loop over to find any particle for which first_step is not defined
    for(size_t output_index=0; output_index<output2trackid.size(); ++output_index) {
      auto& grp = part_grp_v[output2trackid[output_index]];
      auto const& fs = grp.part.first_step();
      if(fs.x()!=0. || fs.y()!=0. || fs.z()!=0. || fs.t()!=0.) continue;
      auto const vtx = grp.part.position().as_point3d();
      double min_dist = std::fabs(larcv::kINVALID_DOUBLE);
      larcv::Point3D min_pt;
      for(auto const& vox : grp.vs.as_vector()) {
	auto const pt = meta3d.position(vox.id());
	double dist = pt.squared_distance(vtx);
	if(dist > min_dist) continue;
	min_dist = dist;
	min_pt = pt;
      }
      if(min_dist > (sqrt(3.) + 1.e-3)) grp.part.first_step(larcv::kINVALID_DOUBLE,larcv::kINVALID_DOUBLE,larcv::kINVALID_DOUBLE,larcv::kINVALID_DOUBLE);
      else grp.part.first_step(min_pt.x, min_pt.y, min_pt.z, grp.part.position().t());

    }

    // now loop over to create VoxelSet for compton/photoelectron
    std::vector<larcv::Particle> part_v; part_v.resize(output2trackid.size());
    auto event_cluster    = (EventClusterVoxel3D*)(mgr.get_data("cluster3d",_output_label));
    auto event_cluster_he = (EventClusterVoxel3D*)(mgr.get_data("cluster3d",_output_label + "_highE"));
    auto event_cluster_le = (EventClusterVoxel3D*)(mgr.get_data("cluster3d",_output_label + "_lowE"));
    auto event_leftover   = (EventSparseTensor3D*)(mgr.get_data("sparse3d",_output_label + "_leftover"));
    // set meta for all
    event_cluster->meta(meta3d);
    event_cluster_he->meta(meta3d);
    event_cluster_le->meta(meta3d);
    event_leftover->meta(meta3d);

    std::vector<larcv::ClusterPixel2D> vsa2d_v;          vsa2d_v.resize(_valid_nplanes);
    std::vector<larcv::ClusterPixel2D> vsa2d_he_v;       vsa2d_he_v.resize(_valid_nplanes);
    std::vector<larcv::ClusterPixel2D> vsa2d_le_v;       vsa2d_le_v.resize(_valid_nplanes);
    for(size_t plane_idx=0; plane_idx<_valid_nplanes; ++plane_idx) {
      vsa2d_v[plane_idx].resize(output2trackid.size());
      vsa2d_he_v[plane_idx].resize(output2trackid.size());
      vsa2d_le_v[plane_idx].resize(output2trackid.size());
      vsa2d_v[plane_idx].meta(_meta2d_v[plane_idx]);
      vsa2d_he_v[plane_idx].meta(_meta2d_v[plane_idx]);
      vsa2d_le_v[plane_idx].meta(_meta2d_v[plane_idx]);
    }

    event_cluster->resize(output2trackid.size());
    event_cluster_he->resize(output2trackid.size());
    event_cluster_le->resize(output2trackid.size());
    for(size_t index=0; index<output2trackid.size(); ++index) {
      int trackid = output2trackid[index];
      auto& grp   = part_grp_v[trackid];
      // set particle
      std::swap(grp.part, part_v[index]);
      // fill 3d cluster
      event_cluster->writeable_voxel_set(index) = grp.vs;
      event_cluster_he->writeable_voxel_set(index) = grp.vs;
      grp.vs.clear_data();
      // fill 2d cluster
      for(size_t plane_idx=0; plane_idx<_valid_nplanes; ++plane_idx) {
	auto& vsa2d = vsa2d_v[plane_idx];
	auto& vsa2d_he = vsa2d_he_v[plane_idx];
	vsa2d.writeable_voxel_set(index) = grp.vs2d_v[plane_idx];
	vsa2d_he.writeable_voxel_set(index) = grp.vs2d_v[plane_idx];
	grp.vs2d_v[plane_idx].clear_data();
      }
      grp.valid=false;
    }

    // Loop to store output cluster/semantic: low energy depositions
    for(auto& grp : part_grp_v) {
      if(grp.type != supera::kCompton && grp.type != supera::kPhotoElectron && grp.type != supera::kNeutron) continue;
      if(!grp.valid) continue;
      if(grp.size_all()<1) continue;
      int trackid = grp.part.parent_track_id();
      int output_index = -1;
      if(trackid < ((int)(trackid2output.size()))) output_index = trackid2output[trackid];
      if(output_index<0) {
	// search the first direct, valid parent
	while(1) {
	  if(trackid >= ((int)(trackid2index.size())) || trackid2index[trackid] < 0) break;
	  trackid = larmcp_v[trackid2index[trackid]].Mother();
	  if(trackid < ((int)(trackid2output.size()))) {
	    output_index = trackid2output[trackid];
	    break;
	  }
	}
      }
      if(output_index<0) continue;
      // fill 3D cluster
      auto& vs_le = event_cluster_le->writeable_voxel_set(output_index);
      auto& vs    = event_cluster->writeable_voxel_set(output_index);
      for(auto const& vox : grp.vs.as_vector()) {
	vs_le.emplace(vox.id(),vox.value(),true);
	vs.emplace(vox.id(),vox.value(),true);
      }
      grp.vs.clear_data();
      // fill 2d cluster
      for(size_t plane_idx=0; plane_idx<_valid_nplanes; ++plane_idx) {
	auto& vs2d = vsa2d_v[plane_idx].writeable_voxel_set(output_index);
	auto& vs2d_le = vsa2d_le_v[plane_idx].writeable_voxel_set(output_index);
	for(auto const& vox : grp.vs2d_v[plane_idx].as_vector()) {
	  vs2d.emplace(vox.id(),vox.value(),true);
	  vs2d_le.emplace(vox.id(),vox.value(),true);
	}
	grp.vs2d_v[plane_idx].clear_data();
      }
    }

    // create particle ID vs ... overlapped voxel gets higher id number
    auto const& main_vs = event_cluster_he->as_vector();
    auto const& lowe_vs = event_cluster_le->as_vector();
    auto event_cindex = (EventSparseTensor3D*)(mgr.get_data("sparse3d",_output_label + "_index"));
    event_cindex->meta(meta3d);
    larcv::VoxelSet cid_vs;
    for(size_t index=0; index<main_vs.size(); ++index) {
      auto const& vs = main_vs[index];
      for(auto const& vox : vs.as_vector()) cid_vs.emplace(vox.id(),(float)(index),false);
    }
    event_cindex->emplace(std::move(cid_vs),meta3d);

    // Count output voxel count and x-check
    size_t output_vs_size = 0;
    for(auto const& vs : main_vs) output_vs_size += vs.size();
    for(auto const& vs : lowe_vs) output_vs_size += vs.size();
    LARCV_INFO() << "Voxel count x-check: output = " << output_vs_size << " ... total = " << total_vs_size << std::endl;

    LARCV_INFO()<<"Combined reminders..."<<std::endl;
    larcv::VoxelSet leftover_vs; leftover_vs.reserve(total_vs_size - output_vs_size);
    std::vector<larcv::VoxelSet> leftover2d_vs(_valid_nplanes);
    for(auto& vs : leftover2d_vs) vs.reserve(total_vs_size - output_vs_size);

    if(total_vs_size > output_vs_size) {
      int ctr= 0;
      for(auto& grp : part_grp_v) {
	if(grp.size_all()<1) continue;
	for(auto const& vox : grp.vs.as_vector()) leftover_vs.emplace(vox.id(),vox.value(),true);
	for(size_t plane_idx=0; plane_idx<_valid_nplanes; ++plane_idx) {
	  for(auto const& vox : grp.vs2d_v[plane_idx].as_vector()) {
	    leftover2d_vs[plane_idx].emplace(vox.id(),vox.value(),true);
	  }
	}
	ctr++;
	auto const& part = grp.part;
	LARCV_INFO() << "Particle ID " << part.id() << " Type " << grp.type << " Valid " << grp.valid << " Track ID " << part.track_id() << " PDG " << part.pdg_code() 
		     << " " << part.creation_process() << " ... " << part.energy_init() << " MeV => " << part.energy_deposit() << " MeV "
		     << grp.trackid_v.size() << " children " << grp.vs.size() << " voxels " << grp.vs.sum() << " MeV" << std::endl;
	LARCV_INFO() << "  Parent " << part.parent_track_id() << " PDG " << part.parent_pdg_code() << " " << part.parent_creation_process() 
		     << " Ancestor " << part.ancestor_track_id() << " PDG " << part.ancestor_pdg_code() << " " << part.ancestor_creation_process() << std::endl;
	LARCV_INFO() << "  Group ID: " << part.group_id() << std::endl;
	std::stringstream ss1,ss2;
	
	ss1 << "  Children particle IDs: " << std::flush;
	for(auto const& child : part.children_id()) ss1 << child << " " << std::flush;
	ss1 << std::endl;
	LARCV_INFO() << ss1.str();

	ss2 << "  Children track IDs: " << std::flush;
	for(auto const& child : grp.trackid_v) ss2 << child << " " << std::flush;
	ss2 << std::endl;
	LARCV_INFO() << ss2.str();

	if(grp.type != supera::kIonization && grp.type != supera::kConversion && grp.type != supera::kComptonHE) continue;
	LARCV_INFO() << "Above was supposed to be merged..." << std::endl;
	
      }
      LARCV_INFO() << "... " << ctr << " particles" << std::endl;
    }
    event_leftover->emplace(std::move(leftover_vs),meta3d);

    // Loop over to find any "stil valid" supera::kIonization supera::kConversion supera::kComptonHE
    LARCV_INFO() << "Particle list" << std::endl;
    for(size_t index = 0; index < part_v.size(); ++index) {
      int trackid = output2trackid[index];
      auto const& grp  = part_grp_v[trackid];
      auto const& part = part_v[index];
      auto const& vs0  = main_vs[index];
      auto const& vs1  = lowe_vs[index];
      LARCV_INFO() << "Particle ID " << part.id() << " Track ID " << part.track_id() << " PDG " << part.pdg_code() 
		   << " " << part.creation_process() << " ... " << part.energy_init() << " MeV => " << part.energy_deposit() << " MeV "
		   << grp.trackid_v.size() << " children " << vs0.size() << " (" << vs1.size() << ") voxels" << std::endl;
      LARCV_INFO() << "  Parent TrackID " << part.parent_track_id() << " PartID " << part.parent_id() 
		   << " PDG " << part.parent_pdg_code() << " " << part.parent_creation_process() 
		   << " Ancestor TrackID " << part.ancestor_track_id() 
		   << " PDG " << part.ancestor_pdg_code() << " " << part.ancestor_creation_process() << std::endl;
      LARCV_INFO() << "  Group ID: " << part.group_id() << std::endl;
      std::stringstream ss1,ss2;

      ss1 << "  Children particle IDs: " << std::flush;
      for(auto const& child : part.children_id()) ss1 << child << " " << std::flush;
      ss1 << std::endl;
      LARCV_INFO() << ss1.str();

      ss2 << "  Children track IDs: " << std::flush;
      for(auto const& child : grp.trackid_v) ss2 << child << " " << std::flush;
      ss2 << std::endl;
      LARCV_INFO() << ss2.str();

      LARCV_INFO() << "  Start: " << part.first_step().x() << " " << part.first_step().y() << " " << part.first_step().z() << std::endl;
    }
    LARCV_INFO() << "... " << part_v.size() << " particles" << std::endl;

    // Store output
    auto event_mcp = (EventParticle*)(mgr.get_data("particle",_output_label));
    event_mcp->emplace(std::move(part_v));

    // Prep semantic segmentation output
    enum SemanticType_t {
      kSemanticTrack,
      kSemanticMichel,
      kSemanticShower,
      kSemanticDelta,
      kSemanticCompton
    };
    // semantic in 3d
    auto event_segment = (EventSparseTensor3D*)(mgr.get_data("sparse3d",_output_label + "_semantics"));
    event_segment->meta(meta3d);
    larcv::VoxelSet semantic_vs; semantic_vs.reserve(total_vs_size);
    // semantic in 2d
    std::vector<larcv::VoxelSet> semantic2d_vs_v; 
    semantic2d_vs_v.resize(_valid_nplanes);
    for(auto& vs : semantic2d_vs_v) vs.reserve(total_vs_size);
    // Comptons in 3d
    for(auto const& vs : event_cluster_le->as_vector()) {
      for(auto const& vox : vs.as_vector()) semantic_vs.emplace(vox.id(),(float)(kSemanticCompton),false);
    }
    //for(auto const& vox : event_leftover->as_vector()) semantic_vs.emplace(vox.id(),(float)(kSemanticCompton),false);

    // Comptons in 2d
    for(size_t plane_idx=0; plane_idx<_valid_nplanes; ++plane_idx) {
      auto& semantic2d = semantic2d_vs_v[plane_idx];
      auto& vsa2d_le   = vsa2d_le_v[plane_idx];
      for(auto const& vs : vsa2d_le.as_vector()) {
	for(auto const& vox : vs.as_vector()) semantic2d.emplace(vox.id(),(float)(kSemanticCompton),false);
      }
    }
    
    // Loop over "high energy" depositions
    for(size_t index=0; index<output2trackid.size(); ++index) {
      int trackid = output2trackid[index];
      auto const& grp = part_grp_v[trackid];
      auto const& vs  = event_cluster_he->as_vector()[index];
      float semantic = -1;
      switch(grp.type) {
      case supera::kTrack:
	semantic=(float)kSemanticTrack; break;
      case supera::kPrimary:
      case supera::kPhoton:
      case supera::kComptonHE:
      case supera::kConversion:
	semantic=(float)kSemanticShower; break;
      case supera::kDelta:
	//std::cout<<"Delta found!"<<std::endl;
	semantic=(float)kSemanticDelta; break;
      case supera::kDecay:
	semantic=(float)kSemanticMichel; break;
	//case supera::kNeutron:
	//semantic=(float)kSemanticCompton; break;
      default:
	LARCV_CRITICAL() << "Unexpected type while assigning semantic class: " << grp.type << std::endl;
	break;
      }
      std::cout<<semantic<<" "<<std::flush;
      for(auto const& vox : vs.as_vector()) {
	auto const& prev = semantic_vs.find(vox.id());
	if(prev.id() == larcv::kINVALID_VOXELID || prev.value() > semantic) 
	  semantic_vs.emplace(vox.id(),semantic,false);
      }
      for(size_t plane_id=0; plane_id < _valid_nplanes; ++plane_id) {
	auto& semantic2d_vs = semantic2d_vs_v[plane_id];
	auto const& vsa2d_he = vsa2d_he_v[plane_id];
	auto const& vs2d = vsa2d_he.as_vector()[index];
	for(auto const& vox : vs2d.as_vector()) {
	  auto const& prev = semantic2d_vs.find(vox.id());
	  if(prev.id() == larcv::kINVALID_VOXELID || prev.value() > semantic) {
	    semantic2d_vs.emplace(vox.id(),semantic,false);
	  }
	}
	/*
	if(semantic == (float)(kSemanticDelta))
	  std::cout<<"Delta track " << trackid << " voxel " <<vs2d.as_vector().size() << " recorded " << recorded << " plane " << plane_id << std::endl;
	*/
      }
    }
    event_segment->emplace(std::move(semantic_vs),meta3d);

    for(size_t cryo_id=0; cryo_id<_scan.size(); ++cryo_id) {
      auto const& tpcs = _scan[cryo_id];
      for(size_t tpc_id=0; tpc_id<tpcs.size(); ++tpc_id) {
	auto const& planes = tpcs[tpc_id];
	for(size_t plane_id=0; plane_id<planes.size(); ++plane_id) {
	  auto const& idx = planes.at(plane_id);
	  if(idx < 0) continue;
	  if(idx >= (int)(_meta2d_v.size())) {
	    LARCV_CRITICAL() <<idx << "Unexpected: " << _meta2d_v.size() << " " << semantic2d_vs_v.size() << std::endl;
	    throw larbys();
	  }
	  auto meta2d = _meta2d_v[idx];
	  std::string suffix = "_" + std::to_string(cryo_id) + "_" + std::to_string(tpc_id) + "_" + std::to_string(plane_id);
	  std::string output_producer = _output_label + "_semantics2d" + suffix;
	  auto semantic2d_output = (larcv::EventSparseTensor2D*)(mgr.get_data("sparse2d",output_producer));
	  semantic2d_output->emplace(std::move(semantic2d_vs_v[idx]),std::move(meta2d));
	  output_producer = _output_label + suffix;
	  auto& cluster2d_output  = mgr.get_data<larcv::EventClusterPixel2D>(output_producer);
	  cluster2d_output.emplace(std::move(vsa2d_v[idx]));

	  output_producer = _output_label + "_he" + suffix;
	  auto& cluster2d_he_output  = mgr.get_data<larcv::EventClusterPixel2D>(output_producer);
	  cluster2d_he_output.emplace(std::move(vsa2d_he_v[idx]));

	  output_producer = _output_label + "_le" + suffix;
	  auto& cluster2d_le_output  = mgr.get_data<larcv::EventClusterPixel2D>(output_producer);
	  cluster2d_le_output.emplace(std::move(vsa2d_le_v[idx]));
	  
	  output_producer = _output_label + "_leftover" + suffix;
	  auto& tensor2d_leftover  = mgr.get_data<larcv::EventSparseTensor2D>(output_producer);
	  meta2d = _meta2d_v[idx];
	  tensor2d_leftover.emplace(std::move(leftover2d_vs[idx]),std::move(meta2d));
	  //std::cout<<cryo_id<<" "<<tpc_id<<" "<<plane_id<<" ... " << semantic2d_output.as_vector().size() << " " << semantic2d_output.as_vector().front().meta().id() <<std::endl;
	}
      }
    }

    return true;
  }

  larcv::Particle 
  SuperaMCParticleCluster::MakeParticle(const supera::LArMCParticle_t& larmcp)
  {
    
    larcv::Particle res;
    res.pdg_code(larmcp.PdgCode());
    res.shape(larcv::kShapeShower);
    res.track_id(larmcp.TrackId());
    res.pdg_code(larmcp.PdgCode());
    res.momentum(larmcp.Px()*1000,larmcp.Py()*1000,larmcp.Pz()*1000);
    res.position(larmcp.Vx(),larmcp.Vy(), larmcp.Vz(), larmcp.T());
    res.first_step(larmcp.Vx(),larmcp.Vy(), larmcp.Vz(), larmcp.T());
    res.end_position(larmcp.EndX(),larmcp.EndY(),larmcp.EndZ(),larmcp.EndT());
    res.last_step(larmcp.EndX(),larmcp.EndY(),larmcp.EndZ(),larmcp.EndT());
    res.energy_init(larmcp.E()*1000);
    res.creation_process(larmcp.Process());

    if(larmcp.Mother() > 0)
      res.parent_track_id(larmcp.Mother());
    else
      res.parent_track_id(res.track_id());
    // parent pdg code
    // parent position
    // ancestor track id
    // ancestor position
    return res;
  }

  bool SuperaMCParticleCluster::IsTouching(const Voxel3DMeta& meta, const VoxelSet& vs1, const VoxelSet& vs2) const
  {
    
    bool touching = false;
    size_t ix1,iy1,iz1;
    size_t ix2,iy2,iz2;
    size_t diffx, diffy, diffz;

    for(auto const& vox1 : vs1.as_vector()) {
      meta.id_to_xyz_index(vox1.id(), ix1, iy1, iz1);
      for(auto const& vox2 : vs2.as_vector()) {
	meta.id_to_xyz_index(vox2.id(), ix2, iy2, iz2);
	if(ix1>ix2) diffx = ix1-ix2; else diffx = ix2-ix1;
	if(iy1>iy2) diffy = iy1-iy2; else diffy = iy2-iy1;
	if(iz1>iz2) diffz = iz1-iz2; else diffz = iz2-iz1;
	touching = diffx<=1 && diffy<=1 && diffz <=1;
	if(touching) break;
      }
      if(touching) break;
    }

    return touching;
  }


  bool SuperaMCParticleCluster::IsTouching2D(const ImageMeta& meta, const VoxelSet& vs1, const VoxelSet& vs2) const
  {
    
    bool touching = false;
    size_t ix1,iy1;
    size_t ix2,iy2;
    size_t diffx, diffy;

    for(auto const& vox1 : vs1.as_vector()) {
      meta.index_to_rowcol(vox1.id(), iy1, ix1);
      for(auto const& vox2 : vs2.as_vector()) {
	meta.index_to_rowcol(vox2.id(), iy2, ix2);
	if(ix1>ix2) diffx = ix1-ix2; else diffx = ix2-ix1;
	if(iy1>iy2) diffy = iy1-iy2; else diffy = iy2-iy1;
	touching = diffx<=1 && diffy<=1;
	if(touching) break;
      }
      if(touching) break;
    }

    return touching;
  }
  
  void SuperaMCParticleCluster::finalize()
  {}
}

#endif