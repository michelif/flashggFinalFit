// ROOT includes
#include "TROOT.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TAxis.h"
#include "TString.h"
#include "TMath.h"
#include "TMatrixDSym.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TKey.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TChain.h"
#include "TList.h"
#include "TObject.h"
#include "TTree.h"
#include "TROOT.h"
#include "TSystem.h"

// RooFit includes
#include "RooDataHist.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooConstVar.h"
#include "RooFormulaVar.h"
#include "RooPlot.h"
#include "RooAddPdf.h"
#include "RooFitResult.h"
#include "RooArgSet.h"
#include "RooArgList.h"
#include "RooAddPdf.h"
#include "RooGlobalFunc.h"
#include "RooCmdArg.h"

// RooStats includes
#include "RooWorkspace.h"

// standard includes
#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>
#include <vector>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

//needed for option handling
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>


using namespace std;
using namespace RooFit;

using namespace boost;
using namespace boost::program_options;
using namespace boost::filesystem;


int main(int argc, char* argv[]){

  string inputFile="/mnt/t3nfs01/data01/shome/micheli/HHbbgg_ETH_devel/outfiles/20170828_optimizedCuts_noDR_minDRGJet/Total_preselection_diffNaming_transformedMVA.root";
  string outputdir = "testWSOut"; 
  string sigName = "HHbbgg";
  
options_description desc("Allowed options");
 desc.add_options()
    ("help,h", "print usage message")
   ("inputFile,i", value<string>(&inputFile), "input file")
   ("outputdir,o", value<string>(&outputdir), "output dir")
   ;

  TFile* infile = TFile::Open(inputFile.c_str(),"READ");

  variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);
  notify(vm);
  if (vm.count("help") ) {
    cout << desc << "\n";
    return 1;
  }


  std::system(Form("mkdir -p %s", outputdir.c_str()));


  string outFileName( Form("%s/ws_%s", outputdir.c_str(), sigName.c_str() ));
  TFile* dataFile = new TFile(  Form("%s.root",outFileName.c_str()) , "RECREATE" );

  TH1::AddDirectory(kTRUE); // stupid ROOT memory allocation needs this

  RooRealVar h_mass_var("Mgg","Invariant mass", 100,180); 
  RooRealVar weight("weight", "weight", -99,99 );
  //dummy variables to be pushed through
  RooRealVar IntLumi("IntLumi", "IntLumi", 1. );
  RooRealVar dZ("dZ", "dZ", 0. );
   
  RooArgSet vars;
  vars.add(h_mass_var);
  vars.add(weight);
  vars.add(IntLumi);
  vars.add(dZ);
  RooFormulaVar hgg_mass( "CMS_hgg_mass", "Mgg", vars );

  //INITIAL SELECTION
  string treeSel = "MX >200. && Mjj> 60 && Mjj<200";
  //  TFile* dataFile = new TFile(  Form("%s.root",outFileName.c_str()) , "RECREATE" );
  RooWorkspace ws_sig ("ws_sig");
  RooWorkspace ws_bkg  ("ws_bkg");
  RooWorkspace ws_data("ws_data");


  TTree* tree_data_ini =(TTree*) infile->Get("reducedTree_bkg");
  TTree* tree_sig_ini =(TTree*) infile->Get("reducedTree_sig");
  //legend for backgrounds:
//  processes["reducedTree_bkg_0"]= str("Diphoton")
//    processes["reducedTree_bkg_2"]= str("GJets")
//    processes["reducedTree_bkg_3"]= str("ggH")
//    processes["reducedTree_bkg_4"]= str("VH")
//    processes["reducedTree_bkg_5"]= str("VBF")
//    processes["reducedTree_bkg_6"]= str("bbH")
//    processes["reducedTree_bkg_7"]= str("ttH")
//    processes["reducedTree_bkg"]= str("Data")
  map<string,TTree* > trees_hig_ini;
  map<string,TTree* > trees_hig;
  map<std::string, std::string> mapOfProcesses;
  mapOfProcesses["tth"]="reducedTree_bkg_7";
  mapOfProcesses["bbh"]="reducedTree_bkg_6";
  mapOfProcesses["vbf"]="reducedTree_bkg_5";
  mapOfProcesses["vh"]="reducedTree_bkg_4";
  mapOfProcesses["ggh"]="reducedTree_bkg_3";

  std::map<std::string, std::string>::iterator it = mapOfProcesses.begin();
  while(it != mapOfProcesses.end()){
    trees_hig_ini[it->first]=((TTree*) infile->Get(it->second.c_str()));
    it++;
  }

  TTree* tree_data =(TTree*) tree_data_ini->CopyTree(treeSel.c_str() );

  TTree* tree_sig =(TTree*) tree_sig_ini->CopyTree(treeSel.c_str() );

  it = mapOfProcesses.begin();
  while(it != mapOfProcesses.end()){
    trees_hig[it->first]=((TTree*) trees_hig_ini[it->first]->CopyTree(treeSel.c_str()));
    it++;
  }


  //  TTree* tree_vbf =(TTree*) tree_vbf_ini->CopyTree(treeSel.c_str() );
  //  TTree* tree_vh =(TTree*) tree_vh_ini->CopyTree(treeSel.c_str() );
  //  TTree* tree_ggh =(TTree*) tree_ggh_ini->CopyTree(treeSel.c_str() );
  //  tree_sig->Print();

  string regionSaveName = "test";
  RooDataSet data_ds(Form("data_125_13TeV_%s", regionSaveName.c_str() ), "Data", vars, WeightVar(weight), Import(*tree_data) );
  ((RooRealVar*)data_ds.addColumn( hgg_mass))->setRange(100.,180.);
  RooDataSet sig_ds(Form("HHbbgg_125_13TeV_%s", regionSaveName.c_str() ), "HHbbgg", vars, WeightVar(weight), Import(*tree_sig) );
  ((RooRealVar*)sig_ds.addColumn( hgg_mass))->setRange(100.,180.);

  vector<RooDataSet > datasets_hig;
  it = mapOfProcesses.begin();
  while(it != mapOfProcesses.end()){
    RooDataSet dataset = RooDataSet(Form("bg_%s_125_13TeV_%s", it->first.c_str(),regionSaveName.c_str() ),it->first.c_str(),vars,WeightVar(weight), Import(*trees_hig[it->first]));
    ((RooRealVar*)dataset.addColumn( hgg_mass))->setRange(100.,180.); 
    datasets_hig.push_back(dataset);
    it++;
  }



  dataFile->cd();
  ws_data.import(data_ds);
  ws_data.Write();


  ws_sig.import(sig_ds);
  ws_sig.Write();
  
  for (unsigned int i =0; i <datasets_hig.size();++i){
    ws_bkg.import(datasets_hig[i]);
  }
  ws_bkg.Write();

  dataFile->Write();
  dataFile->Close();


  return 0;
}
