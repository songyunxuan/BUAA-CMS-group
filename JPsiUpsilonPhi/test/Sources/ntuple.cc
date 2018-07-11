#define ntuple_cxx
#include "../Includes/ntuple.h"
#include "../Includes/SmartSelectionMonitor.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLorentzVector.h>

void ntuple::Loop()
{
//   In a ROOT session, you can do:
//      Root > .L ntuple.C
//      Root > ntuple t
//      Root > t.GetEntry(12); // Fill t data members with entry number 12
//      Root > t.Show();       // Show values of entry 12
//      Root > t.Show(16);     // Read and show values of entry 16
//      Root > t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;
   TFile *outFile = new TFile(outputFile_,"RECREATE");
   SmartSelectionMonitor mon;
   /*TH1F *h =(TH1F*) mon.addHistogram(new TH1F("eventflow",";;Events",6,0,6));
   h->GetXaxis()->SetBinLabel(1,"skimmed");
   h->GetXaxis()->SetBinLabel(2,"muonIDsoft");
   h->GetXaxis()->SetBinLabel(3,"J/PsiLxy");
   h->GetXaxis()->SetBinLabel(4,"J+PfitChi2");
   h->GetXaxis()->SetBinLabel(5,"J+2PfitChi2");
   h->GetXaxis()->SetBinLabel(6,"cosine<P,r>");*/

   TH1F *h1 =mon.addHistogram(new TH1F("Num_J/Psi",";N_{J/#psi};Events",5,0,5));
   TH1F *h2 =mon.addHistogram(new TH1F("M_J/Psi",";M_{J/#psi};Events",50,2.8,3.3));
   TH1F *h3 =mon.addHistogram(new TH1F("pT_J/Psi",";p_{T,J/Psi};Events",120,0,120)); 
   TH1F *h4 =mon.addHistogram(new TH1F("M_J/PsiPicut4.2",";m_{J/psi,Pi};Events",40,3.5,4.3)); 
   TH1F *h5 =mon.addHistogram(new TH1F("M_J/PsiPicut4.25",";m_{J/psi,Pi};Events",40,3.5,4.3)); 
   TH1F *h6 =mon.addHistogram(new TH1F("M_J/PsiPicut4.3",";m_{J/psi,Pi};Events",40,3.5,4.3)); 
   TH1F *h7 =mon.addHistogram(new TH1F("M_J/PsiPicut4.4",";m_{J/psi,Pi};Events",40,3.5,4.3)); 
   TH1F *h8 =mon.addHistogram(new TH1F("M_J/PsiPicut4.7",";m_{J/psi,Pi};Events",40,3.5,4.3)); 
   TH1F *h9 =mon.addHistogram(new TH1F("M_J/PsiPicut5.0",";m_{J/psi,Pi};Events",40,3.5,4.3)); 




   Long64_t nentries = fChain->GetEntries();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      if(jentry % 10000 ==0) cout << jentry << " of " << nentries << endl;
      //double weight = 1.;
      h1->Fill(nJ);

      auto smallestchi2 = std::min_element(Pi_vertexchisq2->begin(), Pi_vertexchisq2->end());
      int piN =std::distance(Pi_vertexchisq2->begin(), smallestchi2);
      auto largestlxy = std::max_element(J_lxy->begin(), J_lxy->end());
      int jpsiN =std::distance(J_lxy->begin(), largestlxy);
      TLorentzVector jpsi, pion1,pion2;
      jpsi.SetXYZM(J_px->at(jpsiN),J_py->at(jpsiN),J_pz->at(jpsiN),J_mass->at(jpsiN)); 
      pion1.SetPtEtaPhiE(Pi_pt1->at(piN),Pi_eta1->at(piN),Pi_phi1->at(piN),Pi_e1->at(piN));
      pion2.SetPtEtaPhiE(Pi_pt2->at(piN),Pi_eta2->at(piN),Pi_phi2->at(piN),Pi_e2->at(piN));
      h2->Fill(jpsi.M());
      h3->Fill(jpsi.Pt());
      float jpipi_mass = (jpsi+pion1+pion2).M();
      if (jpipi_mass>4.1 &&jpipi_mass<4.2)        h4->Fill((jpsi+pion1).M());
      else if (jpipi_mass>4.2 &&jpipi_mass<4.25)  h5->Fill((jpsi+pion1).M());
      else if (jpipi_mass>4.25 &&jpipi_mass<4.3)  h6->Fill((jpsi+pion1).M());
      else if (jpipi_mass>4.3 &&jpipi_mass<4.4)   h7->Fill((jpsi+pion1).M());
      else if (jpipi_mass>4.4 &&jpipi_mass<4.7)   h8->Fill((jpsi+pion1).M());
      else if (jpipi_mass>4.7 &&jpipi_mass<5.0)   h9->Fill((jpsi+pion1).M());
      // if (Cut(ientry) < 0) continue;
   }
   outFile->Write();
   outFile->Close();
}
