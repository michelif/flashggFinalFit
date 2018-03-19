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


using namespace std;
using namespace RooFit;


int main(int argc, char* argv[]){

  string inputFile="/mnt/t3nfs01/data01/shome/micheli/HHbbgg_ETH_devel/outfiles/20170828_optimizedCuts_noDR_minDRGJet/Total_preselection_diffNaming_transformedMVA.root";
  TFile* infile = TFile::Open(inputFile.c_str(),"READ");


  std::string outputdir = "testWSOut"; 
  system(Form("mkdir -p %s", outputdir.c_str()));

  string sigName = "HHbbgg";
  std::string outFileName( Form("%s/ws_%s", outputdir.c_str(), sigName.c_str() ));
  TFile* dataFile = new TFile(  Form("%s.root",outFileName.c_str()) , "RECREATE" );

  TH1::AddDirectory(kTRUE); // stupid ROOT memory allocation needs this

  RooRealVar h_mass_var("Mgg","Invariant mass", 100,180); 
  RooRealVar weight("weight", "weight", -99,99 );
  //dummy variables to be pushed through
  RooRealVar IntLumi("IntLumi", "IntLumi", 1. );
  //  RooRealVar dZ("dZ", "dZ", 0. );
   
  RooArgSet vars;
  vars.add(h_mass_var);
  vars.add(weight);
  vars.add(IntLumi);
  //  vars.add(dZ);

  //INITIAL SELECTION
  string treeSel = "MX >250. && MX<294 && Mjj> 85 && Mjj<141";
  //  TFile* dataFile = new TFile(  Form("%s.root",outFileName.c_str()) , "RECREATE" );
  RooWorkspace ws_sig ("ws_sig");
  RooWorkspace ws_bg  ("ws_bg");
  RooWorkspace ws_data("ws_data");


  TTree* tree_data_ini =(TTree*) infile->Get("reduced_Tree_bkg");
  TTree* tree_bkg_ini =(TTree*) infile->Get("reduced_Tree_bkg_7");
  TTree* tree_sig_ini =(TTree*) infile->Get("reducedTree_sig");

  TTree* tree_sig =(TTree*) tree_sig_ini->CopyTree(treeSel.c_str() );
  tree_sig->Print();

  string regionSaveName = "test";
  RooDataSet sig_ds(Form("HHbbgg_125_13TeV_%s", regionSaveName.c_str() ), "HHbbgg", vars, WeightVar(weight), Import(*tree_sig) );

  dataFile->cd();
  ws_sig.import(sig_ds);
  
  ws_sig.Write();
  dataFile->Write();
  dataFile->Close();


  return 0;
}
