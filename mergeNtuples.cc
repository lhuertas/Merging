#include <TFile.h>

#include "MyEvtId.h"      // UA tree event desctiption
#include "MyL1Trig.h"  	 // UA tree event desctiption (arrays)
#include "MyL1TrigOld.h" // UA tree event desctiption (arrays)
#include "MyHLTrig.h"
#include "MyTracks.h"
#include "MyVertex.h"
#include "MyPFJet.h"
#include "MyCaloJet.h"
#include "MyMuon.h"
#include "MyElectron.h"
#include "MyPFCand.h"
#include "MyCaloTower.h"
#include "MyMet.h"
#include "MyFSCInfo.h"
#include "MyFSCHit.h"
#include "MyFSCDigi.h"

#include "MyBeamSpot.h"
#include "MyKshorts.h"
#include "MyLambdas.h"

#include "utilities.h"
#include <map>
#include <limits>
#include <time.h>
#include <stdio.h>
#include <sstream>

using namespace std;

TBranch *trigDataBranch;

//it would be nice to move next three procedures (checkAndGetBranch, prepareRomanPotBranches, prepareTotemBranches)
//to a place common for both merge12 and combine programs but there are some memory problems that I
//encountered and currently I don't have time to investigate it

/**
 * This function checks if branch of a given name exists.
 * if branch exists - the pointer is returned
 * if not - the program is terminated with error message
 */
TBranch* checkAndGetBranch(TTree* tree, string branchName) {
  TBranch *branch = tree->GetBranch(branchName.c_str());
  if (!branch) {
    string dotBranchName = branchName + ".";
    branch = tree->GetBranch(dotBranchName.c_str());
  }
  if (!branch) {
    tree->Print();
    error(" No data branch " + branchName + " found in input file!");
  }
  return branch;
}

/**
 * This function prepares roman pot branches for merging.
 * First it connects roman pot variables with proper branches in totem ntuple.
 * Then it creates appropriate branches in output ntuple (with this variables).
 */
void prepareRomanPotBranches(TTree *outputTree, TTree *tree_totem) {
  checkAndGetBranch(tree_totem, "rec_prot_left")->SetAddress(&recProtLeft);
  checkAndGetBranch(tree_totem, "rec_prot_right")->SetAddress(&recProtRight);
  checkAndGetBranch(tree_totem, "rec_prot_pair")->SetAddress(&recProtonPair);

  outputTree->Branch("rec_prot_left.", &recProtLeft);
  outputTree->Branch("rec_prot_right.", &recProtRight);
  outputTree->Branch("rec_prot_pair.", &recProtonPair);

  char br_name[200];
  for (unsigned int a = 0; a < 2; ++a) { //the digis, patterns and tracks are stored separately
    //for every RP, this loops iterate over RP ids
    int s = 2;
    for (unsigned int r = 0; r < 6; r++) {
      unsigned int id = 100 * a + 10 * s + r;
      if (includeDigi) {
        sprintf(br_name, "digi_rp_%u.", id);
        checkAndGetBranch(tree_totem, br_name)->SetAddress(
            &digi_info_[id]);
        outputTree->Branch(br_name, &digi_info_[id]);
      }
      if (includePatterns) {
        sprintf(br_name, "par_patterns_rp_%u.", id);
        checkAndGetBranch(tree_totem, br_name)->SetAddress(
            &par_patterns_info_[id]);
        outputTree->Branch(br_name, &par_patterns_info_[id]);

        sprintf(br_name, "nonpar_patterns_rp_%u.", id);
        checkAndGetBranch(tree_totem, br_name)->SetAddress(
            &nonpar_patterns_info_[id]);
        outputTree->Branch(br_name, &nonpar_patterns_info_[id]);
      }

      sprintf(br_name, "track_rp_%u.", id);
      checkAndGetBranch(tree_totem, br_name)->SetAddress(
          &track_info_[id]);
      outputTree->Branch(br_name, &track_info_[id]);
      if (includeMultiTracks) {
        sprintf(br_name, "multi_track_rp_%u", id);
        checkAndGetBranch(tree_totem, br_name)->SetAddress(
            &multi_track_info_[id]);
        outputTree->Branch(br_name, &multi_track_info_[id]);
      }
    }
  }
}

/**
 * This function prepares totem branches in output tree.
 */
void prepareTotemBranches(const string& totemFileName, TTree *outputTree,
    TTree *tree_totem) {
  checkAndGetBranch(tree_totem, "branchT2EV")->SetAddress(&t2Event);
  checkAndGetBranch(tree_totem, "branchT1EV")->SetAddress(&t1Event);
  trigDataBranch = checkAndGetBranch(tree_totem, "trigger_data");
  trigDataBranch->SetAddress(&trigData);
  checkAndGetBranch(tree_totem, "event_info")->SetAddress(&totemEventData);
  prepareRomanPotBranches(outputTree, tree_totem);
  outputTree->Branch("trigger_data.", &trigData);
  outputTree->Branch("event_info.", &totemEventData);
  outputTree->Branch("branchT1EV.", &t1Event);
  outputTree->Branch("branchT2EV.", &t2Event);
}

/**
 * This function merges the totem and cms ntuples. The result is saved in outputFileName.
 * dOrbit is previously found (using merge12/findOrbitOffset program) orbit offset (as far as we tested
 * it was always 0)
 * cmsRunNo - cms un
 */
void mergeNtuples(const int dOrbit, const string& totemFileName,
    const string& cmsFileName, const string &outputFileName, ULong_t cmsRunNo) {
  cout << "Opening Totem File: " << totemFileName << endl; //opening input ntuples
  TFile* totemFile = TFile::Open(totemFileName.c_str());
  if (!totemFile || totemFile->IsZombie())
    error("Error opening file");
  TTree* totemTree = (TTree*) totemFile->Get("TotemNtuple");
  if (!totemTree)
    error("No data tree found");
  cout << "Entries: " << totemTree->GetEntries() << endl;
  cout << "Opening CMS File: " << cmsFileName << endl;
  TFile* cmsFile = TFile::Open(cmsFileName.c_str());
  if (!cmsFile || cmsFile->IsZombie())
    error("Error opening file");
  TTree * cmsTree = (TTree*) cmsFile->Get("evt");
  unsigned int cmsSize = cmsTree->GetEntries();
  cout << "CMS entries: " << cmsTree->GetEntries() << endl;

  MyEvtId *evtcmsUA = 0; //assigns cms variables to input ntuple
  TBranch *evtId = checkAndGetBranch(cmsTree, "evtId");
  evtId->SetAddress(&evtcmsUA);
  MyL1TrigOld * trigcmsUA = 0;
  checkAndGetBranch(cmsTree, "L1TrigOld")->SetAddress(&trigcmsUA);
  MyHLTrig * hltTrig_cmsUA = 0;
  checkAndGetBranch(cmsTree, "HLTrig")->SetAddress(&hltTrig_cmsUA);
  // Tracks, vertices
  vector<MyTracks> *tracks_cmsUA = 0;
  checkAndGetBranch(cmsTree, "generalTracks")->SetAddress(&tracks_cmsUA);
  vector<MyVertex> *vertices_cmsUA = 0;
  checkAndGetBranch(cmsTree, "offlinePrimaryVertices")->SetAddress(&vertices_cmsUA);
  // Jets
  vector<MyPFJet> *ak5PFJets_cmsUA = 0;
  checkAndGetBranch(cmsTree, "ak5PFJets")->SetAddress(&ak5PFJets_cmsUA);
  vector<MyPFJet> *ak7PFJets_cmsUA = 0;
  checkAndGetBranch(cmsTree, "ak7PFJets")->SetAddress(&ak7PFJets_cmsUA);
  vector<MyCaloJet> *ak5CaloJets_cmsUA = 0;
  checkAndGetBranch(cmsTree, "ak5CaloJets")->SetAddress(&ak5CaloJets_cmsUA);
  vector<MyCaloJet> *ak7CaloJets_cmsUA = 0;
  checkAndGetBranch(cmsTree, "ak7CaloJets")->SetAddress(&ak7CaloJets_cmsUA);
  // Muons, electrons
  vector<MyMuon> *muons_cmsUA = 0;
  checkAndGetBranch(cmsTree, "muons")->SetAddress(&muons_cmsUA);
  vector<MyElectron> *electrons_cmsUA = 0;
  checkAndGetBranch(cmsTree, "gsfElectrons")->SetAddress(&electrons_cmsUA);
  // Particle-flow, calo towers
  vector<MyPFCand> *pFlow_cmsUA = 0;
  checkAndGetBranch(cmsTree, "particleFlow")->SetAddress(&pFlow_cmsUA);
  vector<MyCaloTower> *caloTowers_cmsUA = 0;
  checkAndGetBranch(cmsTree, "caloTowers")->SetAddress(&caloTowers_cmsUA);
  // MET
  vector<MyMet> *met_cmsUA = 0;
  checkAndGetBranch(cmsTree, "met")->SetAddress(&met_cmsUA);
  vector<MyMet> *pfMet_cmsUA = 0;
  checkAndGetBranch(cmsTree, "pfMet")->SetAddress(&pfMet_cmsUA);
  vector<MyMet> *tcMet_cmsUA = 0;
  checkAndGetBranch(cmsTree, "tcMet")->SetAddress(&tcMet_cmsUA);
  // FSC
  MyFSCInfo *fscInfo_cmsUA = 0;
  checkAndGetBranch(cmsTree, "fscInfo")->SetAddress(&fscInfo_cmsUA);

  vector<MyFSCHit> *fscHits_cmsUA = 0;
  checkAndGetBranch(cmsTree, "fscHits")->SetAddress(&fscHits_cmsUA);
  vector<MyFSCDigi> *fscDigis_cmsUA = 0;
  checkAndGetBranch(cmsTree, "fscDigis")->SetAddress(&fscDigis_cmsUA);

  /*
  //added
  MyBeamSpot *offlinebeamspot_cmsUA = 0;
  checkAndGetBranch(cmsTree, "offlineBeamSpot")->SetAddress(&offlinebeamspot_cmsUA);

  vector<MyKshorts> *kshort_cmsUA = 0;
  checkAndGetBranch(cmsTree, "Kshort")->SetAddress(&kshort_cmsUA);

  vector<MyLambdas> *lambda_cmsUA = 0;
  checkAndGetBranch(cmsTree, "Lambda")->SetAddress(&lambda_cmsUA);
  */


  TFile* output = TFile::Open(outputFileName.c_str(), "RECREATE"); //output ntuple
  TTree* outputTree = new TTree("cms_totem", "merged");
  outputTree->Branch("cmsEvtUA", &evtcmsUA); //preparing branches in output ntuple
  outputTree->Branch("cmsTrigUA", &trigcmsUA);
  outputTree->Branch("cmsHLTTrigUA", &hltTrig_cmsUA);
  // Tracks, vertices
  outputTree->Branch("cmsTracksUA", &tracks_cmsUA);
  outputTree->Branch("cmsVerticesUA", &vertices_cmsUA);
  // Jets
  outputTree->Branch("cmsak5PFJetsUA", &ak5PFJets_cmsUA);
  outputTree->Branch("cmsak7PFJetsUA", &ak7PFJets_cmsUA);
  outputTree->Branch("cmsak5CaloJetsUA", &ak5CaloJets_cmsUA);
  outputTree->Branch("cmsak7CaloJetsUA", &ak7CaloJets_cmsUA);
  // Muons, electrons
  outputTree->Branch("cmsMuonsUA", &muons_cmsUA);
  outputTree->Branch("cmsElectronsUA", &electrons_cmsUA);
  // Particle-flow, calo towers
  outputTree->Branch("cmsParticleFlowUA", &pFlow_cmsUA);
  outputTree->Branch("cmsCaloTowersUA", &caloTowers_cmsUA);
  // MET
  outputTree->Branch("cmsMETUA", &met_cmsUA);
  outputTree->Branch("cmsPFMETUA", &pfMet_cmsUA);
  outputTree->Branch("cmsTCMETUA", &tcMet_cmsUA);
  // FSC
  outputTree->Branch("cmsFSCInfoUA", &fscInfo_cmsUA);
  outputTree->Branch("cmsFSCHitsUA", &fscHits_cmsUA);
  outputTree->Branch("cmsFSCDigisUA", &fscDigis_cmsUA);

  /*
  //neutral identification
  outputTree->Branch("offlineBeamSpot", &offlinebeamspot_cmsUA);
  outputTree->Branch("Kshort", &kshort_cmsUA);
  outputTree->Branch("Lambda", &lambda_cmsUA);
  */  

  // TOTEM
  prepareTotemBranches(totemFileName, outputTree, totemTree);

  vector<info> totem; //this vector contains only basic information about totem events
  //it will speed up merging procedure
  map<unsigned int, vector<int> > totemOrbits; //this map helps in searching for totem event with given orbit number
  //key - orbit number, value - vector of indexes of totem events that have this orbit number
  map<unsigned int, vector<int> >::iterator it;
  const int totemSize = totemTree->GetEntries(); //here you can set how many totem events you would like to merge
  for (int i = 0; i < totemSize; ++i)
  {
    trigDataBranch->GetEntry(i);

    const info totemInfo(i, trigData->event_num, trigData->orbit_num,
        trigData->bunch_num);
    totem.push_back(totemInfo); //get event from totem tree and add it to the vector
    unsigned int orbitNumber = trigData->orbit_num;
    it = totemOrbits.find(orbitNumber);
    if (it != totemOrbits.end())
    { //if keys exist add to the existing vector
      it->second.push_back(i);
    }
    else
    { //if not create a vector for given orbit number
      vector<int> vect;
      vect.push_back(i);
      totemOrbits.insert(pair<int, vector<int> >(orbitNumber, vect));
    }
  }
  int matchingEventsCounter = 0;
  unsigned int firstCMSorbit = (totem[0]).orbit + dOrbit;
  unsigned int lastCMSorbit = (totem[totemSize - 1]).orbit + dOrbit;
  unsigned int countCms = 0;
  unsigned int countCmsNoMatch = 0;
  for (unsigned int cms_i = 0; cms_i < cmsSize; cms_i++)
  { //iterate over cms events, cms ntuples is not sorted
    evtId->GetEntry(cms_i);
    if(evtcmsUA->Run != cmsRunNo)
    {
//      cout << "!!!!!!!!!!!!!!!!!!!!!!!!! " << evtcmsUA->Run<< " != " << cmsRunNo << endl;
      continue;
    }
    cout << cms_i << "\t\t" << evtcmsUA->Orbit << "\t\t"
        << evtcmsUA->Bunch << endl;
    if (cleanBX && !collidingBunch(evtcmsUA->Bunch))
    { //if bunch is not colliding
      cout << "Non colliding bunch.. skipping." << endl;
      continue;
    }
    if (((unsigned int) evtcmsUA->Orbit >= firstCMSorbit)
        && ((unsigned int) evtcmsUA->Orbit <= lastCMSorbit))
    { //in this we should be able to find corresponding event in totem vector
      countCms++;
    }
    else
    {
      continue;
    }
    unsigned int totemOrbitCorrected = evtcmsUA->Orbit - dOrbit;
    bool match = false;
    it = totemOrbits.find(totemOrbitCorrected); //we are looking for totem events with totem_orbit_corrected orbit number
    if (it != totemOrbits.end())
    {
      vector<int> totemEvents = it->second;
      for (unsigned int i = 0; (i < totemEvents.size()) && !match; i++)
      { //iterate over totem events with given orbit number and find one that has a proper bunch crossing
        int index = totemEvents[i];
        if ((evtcmsUA->Bunch - totem[index].bcn) == 1)
        {
          matchingEventsCounter++;
          totemTree->GetEntry(index);
          cmsTree->GetEntry(cms_i);
          outputTree->Fill(); //fill the output ntuple
          match = true;
          cout << trigData->orbit_num << "\t\t" << evtcmsUA->Orbit
              << "\t\t" << evtcmsUA->Orbit - trigData->orbit_num
              << "\t\t" << trigData->bunch_num << "\t\t"
              << evtcmsUA->Bunch << "\t\t"
              << evtcmsUA->Bunch - trigData->bunch_num << endl;
        }
      }
    }
    if (!match && ((unsigned int) evtcmsUA->Orbit >= firstCMSorbit)
        && ((unsigned int) evtcmsUA->Orbit <= lastCMSorbit))
    {
      countCmsNoMatch++;
    }
  } // end loop cms events

  outputTree->Write();
  output->Close();
  if (matchingEventsCounter == 0) { //we don't need to store empty ntuples
    if (remove(outputFileName.c_str()) != 0)
      error("Error deleting file");
  }
  //print some basic information to STDOUT
  cout << "cleanBX (0/1)    = " << cleanBX << endl;
  cout << "match xcheck     = " << matchingEventsCounter << endl;
  cout << "countCMS         = " << countCms << endl;
  cout << "countCMSnomatch  = " << countCmsNoMatch << endl;
}

/**
 * You can use this program by typing:
 * ./mergeNtuples totemNtuplePath cmsNtuplePath outputNtuple
 */
int main(int argc, char** argv) {
  if (argc < 6)
  {
//    error("Please specify TOTEM and CMS data files, output file, totem run number, cms run number and offset difference.");
    error("mergeNtuples <TOTEM data file> <CMS data file> <output file> <CMS run number> <offset>");
  }
  const string totem(argv[1]);
  const string cms(argv[2]);
  const string outputFileName(argv[3]);

  ULong_t cmsRunNo;
  stringstream cmsRunNoStream;
  cmsRunNoStream  << argv[4];
  cmsRunNoStream >> cmsRunNo;

  int offset;
  stringstream offsetStream;
  offsetStream  << argv[5];
  offsetStream >> offset;

  cout << "CMS counter-part run number: " << cmsRunNo << endl;
  cout << "CMS-TOTEM orbit offset: " << offset << endl;

  mergeNtuples(offset, totem, cms, outputFileName, cmsRunNo);
  return 0;
}
