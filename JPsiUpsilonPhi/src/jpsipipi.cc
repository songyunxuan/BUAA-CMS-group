// -*- C++ -*-
//
// Package:    jpsipipi
// Class:      jpsipipi
// 

//=================================================
// original author:  Jhovanny Andres Mejia        |
//         created:  Monday Aug 28 (2017)         |
//         <jhovanny.andres.mejia.guisao@cern.ch> | 
// modified by    :   Hanwen Wang
//         <allenwang@buaa.edu.cn>
//=================================================

// system include files
#include <memory>


#include "jpsipipi.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Common/interface/TriggerResults.h"

#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "MagneticField/Engine/interface/MagneticField.h"            
#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"

#include "RecoVertex/KinematicFit/interface/KinematicParticleVertexFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticleFactoryFromTransientTrack.h"
#include "RecoVertex/KinematicFit/interface/MassKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicParticleFitter.h"
#include "RecoVertex/KinematicFitPrimitives/interface/MultiTrackKinematicConstraint.h"
#include "RecoVertex/KinematicFit/interface/KinematicConstrainedVertexFitter.h"
#include "RecoVertex/KinematicFit/interface/TwoTrackMassKinematicConstraint.h"
#include "RecoVertex/KinematicFitPrimitives/interface/KinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/RefCountedKinematicParticle.h"
#include "RecoVertex/KinematicFitPrimitives/interface/TransientTrackKinematicParticle.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TLorentzVector.h"
#include "TTree.h"
#include "TH2F.h"

//
// constants, enums and typedefs
//

  typedef math::Error<3>::type CovarianceMatrix;

//
// static data member definitions
//

//
// constructors and destructor
//

jpsipipi::jpsipipi(const edm::ParameterSet& iConfig)
  :
  dimuon_Label(consumes<edm::View<pat::Muon>>(iConfig.getParameter<edm::InputTag>("dimuons"))),
  trakCollection_label(consumes<edm::View<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("Trak"))),
  primaryVertices_Label(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("primaryVertices"))),
   
  isMC_(iConfig.getParameter<bool>("isMC")),


  tree_(0), 

  mumC2(0), mumNHits(0), mumNPHits(0),
  mupC2(0), mupNHits(0), mupNPHits(0),
  mumdxy(0), mupdxy(0), mumdz(0), mupdz(0),
  muon_dca(0),

  mu1soft(0), mu2soft(0), mu1tight(0), mu2tight(0), 
  mu1PF(0), mu2PF(0), mu1loose(0), mu2loose(0),
 
  nJ(0),

  J_mass(0), J_px(0), J_py(0), J_pz(0),J_energy(0),

  J_px1(0), J_py1(0), J_pz1(0),
  J_px2(0), J_py2(0), J_pz2(0), 
  J_charge1(0), J_charge2(0),

 


{
   //now do what ever initialization is needed
}


jpsipipi::~jpsipipi()
{

}


//
// member functions
//

// ------------ method called to for each event  ------------
void jpsipipi::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using std::vector;
  using namespace edm;
  using namespace reco;
  using namespace std;
  
  //*********************************
  // Get event content information
  //*********************************  

  // Kinematic fit
  edm::ESHandle<TransientTrackBuilder> theB; 
  iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder",theB); 

  edm::Handle<View<pat::PackedCandidate>> thePATTrackHandle;
  iEvent.getByToken(trakCollection_label,thePATTrackHandle);


  edm::Handle< View<pat::Muon> > thePATMuonHandle;
  iEvent.getByToken(dimuon_Label,thePATMuonHandle);
  
 
  //*********************************
  //Now we get the primary vertex 
  //*********************************

  reco::Vertex bestVtx;
  edm::Handle<reco::VertexCollection> primaryVertices_handle;
  iEvent.getByToken(primaryVertices_Label, primaryVertices_handle);

  bestVtx = *(primaryVertices_handle->begin());

  //nVtx = primaryVertices_handle->size(); 
  
  //*****************************************
  //Let's begin by looking for J/psi

  //unsigned int nMu_tmp = thePATMuonHandle->size();
 
 for(View<pat::Muon>::const_iterator iMuon1 = thePATMuonHandle->begin(); iMuon1 != thePATMuonHandle->end(); ++iMuon1) 
    {
      
      for(View<pat::Muon>::const_iterator iMuon2 = iMuon1+1; iMuon2 != thePATMuonHandle->end(); ++iMuon2) 
	{
	  if(iMuon1==iMuon2) continue;
	  
	  //opposite charge 
	  if( (iMuon1->charge())*(iMuon2->charge()) == 1) continue;

	  TrackRef glbTrackP;	  
	  TrackRef glbTrackM;	  
	  
	  if(iMuon1->charge() == 1){ glbTrackP = iMuon1->track();}
	  if(iMuon1->charge() == -1){ glbTrackM = iMuon1->track();}
	  
	  if(iMuon2->charge() == 1) { glbTrackP = iMuon2->track();}
	  if(iMuon2->charge() == -1){ glbTrackM = iMuon2->track();}
	  
	  if( glbTrackP.isNull() || glbTrackM.isNull() ) 
	    {
	      //std::cout << "continue due to no track ref" << endl;
	      continue;
	    }

	  if(iMuon1->track()->pt()<4.0) continue;
	  if(iMuon2->track()->pt()<4.0) continue;

	  if(!(glbTrackM->quality(reco::TrackBase::highPurity))) continue;
	  if(!(glbTrackP->quality(reco::TrackBase::highPurity))) continue;	 
	  
	  reco::TransientTrack muon1TT((*theB).build(glbTrackP));
	  reco::TransientTrack muon2TT((*theB).build(glbTrackM));

	 // *****  Trajectory states to calculate DCA for the 2 muons *********************
	  FreeTrajectoryState mu1State = muon1TT.impactPointTSCP().theState();
	  FreeTrajectoryState mu2State = muon2TT.impactPointTSCP().theState();

	  if( !muon1TT.impactPointTSCP().isValid() || !muon2TT.impactPointTSCP().isValid() ) continue;

	  // Measure distance between tracks at their closest approach
	  ClosestApproachInRPhi cApp;
	  cApp.calculate(mu1State, mu2State);
	  if( !cApp.status() ) continue;
	  float dca = fabs( cApp.distance() );	  
	  if (dca < 0. || dca > 0.5) continue;
	  //cout<<" closest approach  "<<dca<<endl;


	  // ******  Methods to check to which category of muon candidates a given pat::Muon object belongs ****

	  /*
	  //if (iMuon1->isTrackerMuon() || iMuon2->isTrackerMuon())
	  //if (muon::isHighPtMuon(*iMuon1,bestVtx) || muon::isHighPtMuon(*iMuon2,bestVtx))
	  if (muon::isGoodMuon(*iMuon1,muon::TMLastStationAngTight) || muon::isGoodMuon(*iMuon2,muon::TMLastStationAngTight))
	    {
	      cout<<" is category muon  "<<endl;
	    }
	  else
	    {
	      cout<<" it is not category muon  "<<endl;
	    }
	  */
	  
	  // ******   Let's check the vertex and mass ****

	  
	  //The mass of a muon and the insignificant mass sigma 
	  //to avoid singularities in the covariance matrix.
	  ParticleMass muon_mass = 0.10565837; //pdg mass
	  //ParticleMass psi_mass = 3.096916;
	  float muon_sigma = muon_mass*1.e-6;
	  //float psi_sigma = psi_mass*1.e-6;
	  
	  //Creating a KinematicParticleFactory
	  KinematicParticleFactoryFromTransientTrack pFactory;
	  
	  //initial chi2 and ndf before kinematic fits.
	  float chi = 0.;
	  float ndf = 0.;
	  vector<RefCountedKinematicParticle> muonParticles;
	  try {
	    muonParticles.push_back(pFactory.particle(muon1TT,muon_mass,chi,ndf,muon_sigma));
	    muonParticles.push_back(pFactory.particle(muon2TT,muon_mass,chi,ndf,muon_sigma));
	  }
	  catch(...) { 
	    std::cout<<" Exception caught ... continuing 1 "<<std::endl; 
	    continue;
	  }
	  
	  KinematicParticleVertexFitter fitter;   
	  
	  RefCountedKinematicTree psiVertexFitTree;
	  try {
	    psiVertexFitTree = fitter.fit(muonParticles); 
	  }
	  catch (...) { 
	    std::cout<<" Exception caught ... continuing 2 "<<std::endl; 
	    continue;
	  }
	  
	  if (!psiVertexFitTree->isValid()) 
	    {
	      //std::cout << "caught an exception in the psi vertex fit" << std::endl;
	      continue; 
	    }
	  
	  psiVertexFitTree->movePointerToTheTop();
	  
	  RefCountedKinematicParticle psi_vFit_noMC = psiVertexFitTree->currentParticle();
	  RefCountedKinematicVertex psi_vFit_vertex_noMC = psiVertexFitTree->currentDecayVertex();
	  
	  if( psi_vFit_vertex_noMC->chiSquared() < 0 )
	    {
	      //std::cout << "negative chisq from psi fit" << endl;
	      continue;
	    }
	  
	  //some loose cuts go here
	  
	  if(psi_vFit_vertex_noMC->chiSquared()>50.) continue;
	  if(psi_vFit_noMC->currentState().mass()<2.9 || psi_vFit_noMC->currentState().mass()>3.3) continue;
	  
	  //fill variables?iMuon1->track()->pt()
	  
	  J_mass->push_back( psi_vFit_noMC->currentState().mass() );
	  J_px->push_back( psi_vFit_noMC->currentState().globalMomentum().x() );
	  J_py->push_back( psi_vFit_noMC->currentState().globalMomentum().y() );
	  J_pz->push_back( psi_vFit_noMC->currentState().globalMomentum().z() );
	  
	  J_px1->push_back(iMuon1->track()->px());
	  J_py1->push_back(iMuon1->track()->py());
	  J_pz1->push_back(iMuon1->track()->pz());
	  J_charge1->push_back(iMuon1->charge());
	  
	  J_px2->push_back(iMuon2->track()->px());
	  J_py2->push_back(iMuon2->track()->py());
	  J_pz2->push_back(iMuon2->track()->pz());
	  J_charge2->push_back(iMuon2->charge());
	  
	   // ************
	  
	  mu1soft->push_back(iMuon1->isSoftMuon(bestVtx) );
	  mu2soft->push_back(iMuon2->isSoftMuon(bestVtx) );
	  mu1tight->push_back(iMuon1->isTightMuon(bestVtx) );
	  mu2tight->push_back(iMuon2->isTightMuon(bestVtx) );
	  mu1PF->push_back(iMuon1->isPFMuon());
	  mu2PF->push_back(iMuon2->isPFMuon());
	  mu1loose->push_back(muon::isLooseMuon(*iMuon1));
	  mu2loose->push_back(muon::isLooseMuon(*iMuon2));
	  
	  mumC2->push_back( glbTrackP->normalizedChi2() );
	  //mumAngT->push_back( muon::isGoodMuon(*iMuon1,muon::TMLastStationAngTight) ); // 
	  mumNHits->push_back( glbTrackP->numberOfValidHits() );
	  mumNPHits->push_back( glbTrackP->hitPattern().numberOfValidPixelHits() );	       
	  mupC2->push_back( glbTrackM->normalizedChi2() );
	  //mupAngT->push_back( muon::isGoodMuon(*iMuon2,muon::TMLastStationAngTight) );  // 
	  mupNHits->push_back( glbTrackM->numberOfValidHits() );
	  mupNPHits->push_back( glbTrackM->hitPattern().numberOfValidPixelHits() );
	  mumdxy->push_back(glbTrackP->dxy(bestVtx.position()) );
	  mupdxy->push_back(glbTrackM->dxy(bestVtx.position()) );
	  mumdz->push_back(glbTrackP->dz(bestVtx.position()) );
	  mupdz->push_back(glbTrackM->dz(bestVtx.position()) );
	  muon_dca->push_back(dca);          	  
	  
	  nJ++;	       
	  muonParticles.clear();
	  
	}
    }



  //for (reco::PackedCandidate::const_iterator iTrack1= thePATTrackHandle->begin();  iTrack1 != thePATTrackHandle->end(); ++iTrack1){
  	
  
  
    

    //std::cout << "filling tree" << endl;
   tree_->Fill();
    

   
   nJ = 0; 
   

   J_mass->clear(); J_px->clear();   J_py->clear();  J_pz->clear();  
   J_px1->clear();  J_py1->clear();  J_pz1->clear(), J_charge1->clear();
   J_px2->clear();  J_py2->clear();  J_pz2->clear(), J_charge2->clear();

   
   mupC2->clear();
   mupNHits->clear(); mupNPHits->clear();
   mumdxy->clear(); mupdxy->clear(); mumdz->clear(); mupdz->clear(); muon_dca->clear();

   mu1soft->clear(); mu2soft->clear(); mu1tight->clear(); mu2tight->clear();
   mu1PF->clear(); mu2PF->clear(); mu1loose->clear(); mu2loose->clear(); 


}


// ------------ method called once each job just before starting event loop  ------------

void 
jpsipipi::beginJob()
{

  std::cout << "Beginning analyzer job with value of isMC= " << isMC_ << std::endl;

  edm::Service<TFileService> fs;
  tree_ = fs->make<TTree>("ntuple"," J/psi ntuple");

  tree_->Branch("nJ",&nJ,"nJ/i"); 
  tree_->Branch("J_mass", &J_mass);
  tree_->Branch("J_px", &J_px);
  tree_->Branch("J_py", &J_py);
  tree_->Branch("J_pz", &J_pz);

  tree_->Branch("J_px1", &J_px1);
  tree_->Branch("J_py1", &J_py1);
  tree_->Branch("J_pz1", &J_pz1);
  tree_->Branch("J_charge1", &J_charge1);

  tree_->Branch("J_px2", &J_px2);
  tree_->Branch("J_py2", &J_py2);
  tree_->Branch("J_pz2", &J_pz2);
  tree_->Branch("J_charge2", &J_charge2);

 
  tree_->Branch("mumC2",&mumC2);  
  tree_->Branch("mumNHits",&mumNHits);
  tree_->Branch("mumNPHits",&mumNPHits);
  tree_->Branch("mupC2",&mupC2);  
  tree_->Branch("mupNHits",&mupNHits);
  tree_->Branch("mupNPHits",&mupNPHits);
  tree_->Branch("mumdxy",&mumdxy);
  tree_->Branch("mupdxy",&mupdxy);
  tree_->Branch("mumdz",&mumdz);
  tree_->Branch("mupdz",&mupdz);
  tree_->Branch("muon_dca",&muon_dca);

  tree_->Branch("mu1soft",&mu1soft);
  tree_->Branch("mu2soft",&mu2soft);
  tree_->Branch("mu1tight",&mu1tight);
  tree_->Branch("mu2tight",&mu2tight);
  tree_->Branch("mu1PF",&mu1PF);
  tree_->Branch("mu2PF",&mu2PF);
  tree_->Branch("mu1loose",&mu1loose);
  tree_->Branch("mu2loose",&mu2loose);


}




// ------------ method called once each job just after ending the event loop  ------------
void jpsipipi::endJob() {
  tree_->GetDirectory()->cd();
  tree_->Write();
}

//define this as a plug-in
DEFINE_FWK_MODULE(jpsipipi);