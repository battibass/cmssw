#include "PhysicsTools/TagAndProbe/interface/TagProbeFitter.h"
#include <stdexcept>
//#include "TagProbeFitter.h"

#include "TROOT.h"
#include "TFile.h"
#include "TPad.h"
#include "TText.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TH2F.h"
#include "TStyle.h"
#include "Math/QuantFuncMathCore.h"
#include "Roo1DTable.h"
#include "RooAbsDataStore.h"
#include "RooAbsReal.h"
#include "RooAddPdf.h"
#include "RooBinning.h"
#include "RooBinningCategory.h"
#include "RooCategory.h"
#include "RooChebychev.h"
#include "RooDataHist.h"
#include "RooDataSet.h"
#include "RooEfficiency.h"
#include "RooExtendPdf.h"
#include "RooFitResult.h"
#include "RooFormulaVar.h"
#include "RooGaussian.h"
#include "RooGenericPdf.h"
#include "RooGlobalFunc.h"
#include "RooLinkedListIter.h"
#include "RooMappedCategory.h"
#include "RooMinimizer.h"
#include "RooMinuit.h"
#include "RooMsgService.h"
#include "RooMultiCategory.h"
#include "RooNLLVar.h"
#include "RooNumIntConfig.h"
#include "RooPlot.h"
#include "RooProdPdf.h"
#include "RooProfileLL.h"
#include "RooRealVar.h"
#include "RooSimultaneous.h"
#include "RooThresholdCategory.h"
#include "RooTrace.h"
#include "RooWorkspace.h"

using namespace std;
using namespace RooFit;

TagProbeFitter::TagProbeFitter(const std::vector<std::string>& inputFileNames, string inputDirectoryName, string inputTreeName, string outputFileName, int numCPU_, bool saveWorkspace_, bool floatShapeParameters_, const std::vector<std::string>& fixVars_){
  inputTree = new TChain((inputDirectoryName+"/"+inputTreeName).c_str());
  for(size_t i=0; i<inputFileNames.size(); i++){
    inputTree->Add(inputFileNames[i].c_str());
  }
  outputFile = new TFile(outputFileName.c_str(),"recreate");
  outputDirectory = outputFile->mkdir(inputDirectoryName.c_str());
  numCPU = numCPU_;
  saveWorkspace = saveWorkspace_;
  massBins = 0; // automatic default
  floatShapeParameters = floatShapeParameters_;
  fixVars = fixVars_;
  weightVar = "";
  if(!floatShapeParameters && fixVars.empty()) std::cout << "TagProbeFitter: " << "You wnat to fix some variables but do not specify them!";

  gROOT->SetStyle("Plain");
  gStyle->SetTitleFillColor(0);
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat(".2f");

  quiet = false;

  binnedFit = false;

  doSaveDistributionsPlot = true;
  
  // make integration very precise
  RooAbsReal::defaultIntegratorConfig()->setEpsAbs(1e-13);
  RooAbsReal::defaultIntegratorConfig()->setEpsRel(1e-13);
}

TagProbeFitter::~TagProbeFitter(){
  if(inputTree)
    delete inputTree;
  if(outputFile)
    outputFile->Close();
}

void TagProbeFitter::setQuiet(bool quiet_) { 
    quiet = quiet_; 
    if (quiet) {
        RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    } else {
        RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);
    }
}
bool TagProbeFitter::addVariable(string name, string title, double low, double hi, string units){
  RooRealVar temp(name.c_str(), title.c_str(), low, hi, units.c_str());
  temp.setBins(5000,"cache");
  variables.addClone(temp);
  return true;
}

bool TagProbeFitter::addCategory(string name, string title, string expression){
  RooCategory* c = (RooCategory*) parameterParser.factory(expression.c_str());
  if(!c)
    return false;
  //set the name of the category to the passed name instead of the one in the expression
  c->SetName(name.c_str());
  c->SetTitle(title.c_str());
  variables.addClone(*c);
  return true;
}

bool TagProbeFitter::addExpression(string expressionName, string title, string expression, const std::vector<string>& arguments) {
  expressionVars.push_back(make_pair(make_pair(expressionName,title), make_pair(expression,arguments)));
  return true;
}


bool TagProbeFitter::addThresholdCategory(string categoryName, string title, string varName, double cutValue){
  thresholdCategories.push_back(make_pair(make_pair(categoryName,title), make_pair(varName,cutValue)));
  return true;
}


void TagProbeFitter::addPdf(string name, vector<string>& pdfCommands){
  pdfs[name] = pdfCommands;
}

void TagProbeFitter::setBinsForMassPlots(int bins){
  massBins = bins;
}

void TagProbeFitter::setWeightVar(const std::string &var) {
  weightVar = var;
}

string TagProbeFitter::calculateEfficiency(string dirName,const std::vector<string>& effCats,const std::vector<string>& effStates, vector<string>& unbinnedVariables, map<string, vector<double> >& binnedReals, map<string, std::vector<string> >& binnedCategories, vector<string>& binToPDFmap){
  // cout << "MCBS: in TagProbeFitter::calculateEfficiency" << endl;
  //go to home directory
  outputDirectory->cd();
  //make a directory corresponding to this efficiency binning
  gDirectory->mkdir(dirName.c_str())->cd();

  //MCBS: Change #10, Keep a RooArgSet of the unbinnedVars in order to use
  //MCBS: it in the creation of the RooDataHist, together with the efficiency category
  //
  RooArgSet dataVars;
  RooArgSet unbinnedVars;
  //collect unbinned variables
  for(vector<string>::iterator v=unbinnedVariables.begin(); v!=unbinnedVariables.end(); v++){
    // cout << "MCBS: unbinnedVariable : " << v->c_str() << endl;
    dataVars.addClone(variables[v->c_str()], true);
    unbinnedVars.addClone(variables[v->c_str()],true);
    if (binnedFit && (v == unbinnedVariables.begin())) {
        ((RooRealVar&)dataVars[v->c_str()]).setBins(massBins);
	// cout << "MCBS: massBins = " << massBins << endl;
    }
  }
  //collect the binned variables and the corresponding bin categories
  RooArgSet binnedVariables;
  RooArgSet binCategories;
  char cutBuffer[100];
  for(map<string, vector<double> >::iterator v=binnedReals.begin(); v!=binnedReals.end(); v++){
    TString name = v->first;
    vector<double>& binLimits = v->second;
    if (variables.find(name) == 0) { cerr << "Binned variable '"<<name<<"' not found." << endl; return "Error"; }
    binnedVariables.addClone(variables[name]);
   ((RooRealVar&)binnedVariables[name]).setBinning( RooBinning(v->second.size()-1, &v->second[0]) );
    binCategories.addClone( RooBinningCategory(name+"_bins", name+"_bins", (RooRealVar&)binnedVariables[name]) );
    // cout << "MCBS: Binned variable " << name << " has " << binLimits.size()-1 << " bins" << endl;
    for (size_t cutIdx = 0; cutIdx<binLimits.size()-1; ++cutIdx) {
      sprintf(cutBuffer,"%4.2f < %s && %s < %4.2f",binLimits[cutIdx],name.Data(),name.Data(),binLimits[cutIdx+1]);
      // cout << "MCBS: " << cutBuffer << endl;
    }
      
    //((RooBinningCategory&)binCategories[name+"_bins"]).Print(); // Print this RooBinnningCategory. MCBS
  }
  dataVars.addClone(binnedVariables, true);
  
  
  //collect the category variables and the corresponding mapped categories
  RooArgSet categories;
  RooArgSet mappedCategories;
  for(map<string, vector<string> >::iterator v=binnedCategories.begin(); v!=binnedCategories.end(); v++){
    TString name = v->first;
    if (variables.find(name) == 0) { cerr << "Binned category '"<<name<<"' not found." << endl; return "Error"; }
    categories.addClone(variables[name]);
    mappedCategories.addClone(RooMappedCategory(name+"_bins", name+"_bins", (RooCategory&)categories[name]));
    // cout << "MCBS: Binned category " << name << ", RooMappedCategory " << name << "_bins" << endl;
    // ((RooCategory&)categories[name]).Print(); // MCBS


    for(unsigned int i = 0; i<v->second.size(); i++){
      ((RooMappedCategory&)mappedCategories[name+"_bins"]).map(v->second[i].c_str(), name+"_"+TString(v->second[i].c_str()).ReplaceAll(",","_"));
      // cout << "   MCBS: mapping " << v->second[i] << " to " << name+"_"+v->second[i] << endl;
    }
    
    // ((RooMappedCategory&)mappedCategories[name+"_bins"]).Print(); // MCBS
    //((RooCategory&)mappedCategories[name+"_bins"]).Print(); // MCBS
  }
  dataVars.addClone(categories, true);


  // add the efficiency category if it's not a dynamic one
  for (vector<string>::const_iterator effCat = effCats.begin(); effCat != effCats.end(); ++effCat) {
     if (variables.find(effCat->c_str()) != 0) {
        dataVars.addClone(variables[effCat->c_str()], true);
	// cout << "MCBS: Adding efficiency category: " << *effCat << endl;
     }
  }

  //  add all variables used in expressions
   for(vector<pair<pair<string,string>, pair<string, vector<string> > > >::const_iterator ev = expressionVars.begin(), eve = expressionVars.end(); ev != eve; ++ev){
     for (vector<string>::const_iterator it = ev->second.second.begin(), ed = ev->second.second.end(); it != ed; ++it) {
       // provided that they are real variables themselves
       if (variables.find(it->c_str())) dataVars.addClone(variables[it->c_str()], true);
       // cout << "MCBS: Variable used in expression: " << *it << endl; 
     }
   }
   // add all real variables used in cuts
   for(vector<pair<pair<string,string>, pair<string, double> > >::const_iterator tc = thresholdCategories.begin(), tce = thresholdCategories.end(); tc != tce; ++tc){
     if (variables.find(tc->second.first.c_str())) {
       dataVars.addClone(variables[tc->second.first.c_str()], true);
       // cout << "MCBS: Variable used in cut: " << tc->second.first << ", < " << tc->first.first << "," << tc->first.second << " >" << endl;
     }
   }

   // MCBS: dataVars is a RooArgSet, which at this point contains the
   // subset of variables in the inputTree that are used in the RooDataSet.
   // The RooArgSet dataVars contains at this point the 
   // unbinned variables, 
   // binned variables,
   // categories, 
   // variables used in expressions,
   // variables used in cuts
   //
   // cout << "MCBS: 1) dataVars = " << dataVars.contentsString() << endl;

  //now add the necessary mass and passing variables to make the unbinned RooDataSet
  RooDataSet data("data", "data", inputTree, 
                  dataVars,
                  /*selExpr=*/"", /*wgtVarName=*/(weightVar.empty() ? 0 : weightVar.c_str()));
							       
  
  // cout << "MCBS: Finished building RooDataSet data with entries " << data.numEntries() << endl;
   // Now add all expressions that are computed dynamically
   for(vector<pair<pair<string,string>, pair<string, vector<string> > > >::const_iterator ev = expressionVars.begin(), eve = expressionVars.end(); ev != eve; ++ev){
     RooArgList args;
     for (vector<string>::const_iterator it = ev->second.second.begin(), ed = ev->second.second.end(); it != ed; ++it) {
         args.add(dataVars[it->c_str()]);
     }
     RooFormulaVar expr(ev->first.first.c_str(), ev->first.second.c_str(), ev->second.first.c_str(), args);
     // cout << "MCBS: Adding RooFormulaVar " << expr.GetName() << ", " << expr.GetTitle() << endl;
     // cout << "MCBS:"; expr.printMetaArgs(cout); cout << endl;
     
     RooRealVar *col = (RooRealVar *) data.addColumn(expr);
     dataVars.addClone(*col);
   }
 
   // And add all dynamic categories from thresholds
   for(vector<pair<pair<string,string>, pair<string, double> > >::const_iterator tc = thresholdCategories.begin(), tce = thresholdCategories.end(); tc != tce; ++tc){
     RooThresholdCategory tmp(tc->first.first.c_str(), tc->first.second.c_str(), (RooAbsReal &)dataVars[tc->second.first.c_str()], "above", 1);
     tmp.addThreshold(tc->second.second, "below",0);
     RooCategory *cat = (RooCategory *) data.addColumn(tmp);
     dataVars.addClone(*cat);			
     // cout << "MCBS: RooThresholdCategory <" << tc->first.first << ", " << tc->first.second << ">  and <" << tc->second.first << ", " << tc->second.second << ">" << endl;
   }
   
  //merge the bin categories to a MultiCategory for convenience
  RooMultiCategory allCats("allCats", "allCats", RooArgSet(binCategories, mappedCategories));
  data.addColumn(allCats);

   //MCBS: at this point, dataVars has some new expressions and categories, even after the construction
   //MCBS: of the RooDataSet. Checking:
   // cout << "MCBS: 2) dataVars = " << dataVars.contentsString() << endl;



  string effName;
  //setup the efficiency category
  // cout << "MCBS: effCats.size() " << effCats.size() << endl; // MCBS
  // cout << "MCBS: effCats[0]     " << effCats[0] << endl;     // MCBS
  // cout << "MCBS: effStates[0]   " << effStates[0] << endl;   // MCBS
  if (effCats.size() == 1) {
      effName = effCats.front() + "::" + effStates.front();
      RooMappedCategory efficiencyCategory("_efficiencyCategory_", "_efficiencyCategory_", (RooCategory&)dataVars[effCats.front().c_str()], "Failed");
      efficiencyCategory.map(effStates.front().c_str(), "Passed");
      data.addColumn( efficiencyCategory );
      
  } else {
      RooArgSet rooEffCats; 
      string multiState = "{";
      for (size_t i = 0; i < effCats.size(); ++i) {
        if (i) { multiState += ";"; effName += " && "; }
        rooEffCats.add((RooCategory &) dataVars[effCats[i].c_str()]);
        multiState += effStates[i];
        effName = effCats[i] + "::" + effStates[i];
      }
      multiState += "}";
      RooMultiCategory efficiencyMultiCategory("_efficiencyMultiCategory_", "_efficiencyMultiCategory_", rooEffCats);
      RooMappedCategory efficiencyCategory("_efficiencyCategory_", "_efficiencyCategory_", efficiencyMultiCategory, "Failed");
      efficiencyCategory.map(multiState.c_str(), "Passed");
      data.addColumn( efficiencyCategory );
  }
  // cout << "MCBS: binToPDFmap[0] " << binToPDFmap[0] << endl; // set to "shape" in .py,
  //setup the pdf category
  RooMappedCategory pdfCategory("_pdfCategory_", "_pdfCategory_", allCats, (binToPDFmap.size()>0)?binToPDFmap[0].c_str():"all");
  for(unsigned int i = 1; i<binToPDFmap.size(); i+=2){
    pdfCategory.map(binToPDFmap[i].c_str(), binToPDFmap[i+1].c_str());
    // cout << "MCBS: Mapping " << binToPDFmap[i] << " to " << binToPDFmap[i+1] << endl;
  }
  data.addColumn( pdfCategory );
  
  //create the empty efficiency datasets from the binned variables
  RooRealVar efficiency("efficiency", "Efficiency", 0, 1);

  RooDataSet fitEfficiency("fit_eff", "Efficiency from unbinned ML fit", RooArgSet(RooArgSet(binnedVariables, categories), efficiency), StoreAsymError(RooArgSet(binnedVariables, efficiency)));
//  RooDataSet sbsEfficiency("sbs_eff", "Efficiency from side band substraction", RooArgSet(RooArgSet(binnedVariables, categories), efficiency), StoreAsymError(RooArgSet(binnedVariables, efficiency)));
  RooDataSet cntEfficiency("cnt_eff", "Efficiency from counting", RooArgSet(RooArgSet(binnedVariables, categories), efficiency), StoreAsymError(RooArgSet(binnedVariables, efficiency)));

  // MCBS: At this point, the RooDataSet data is completely made, including not just dataVars,
  // MCBS: but also the pdfCategory, efficiencyCategory, the RooFormulaVar from the expressions
  // MCBS: Computed dynamically (e.g. the (Tight2012==1 && abs(dzPV)<0.5) condition)
  cout << "Finished adding columns to the RooDataSet, print it" << endl;
  // cout << "MCBS: data.numEntries() " << data.numEntries() << endl;
  // cout << "MCBS: sizeof(data) " << sizeof(data) << endl;
  // data.Print();

  if(!floatShapeParameters){
    // cout << "MCBS: Performing first fit over whole dataset" << endl;
    //fitting whole dataset to get initial values for some parameters
    RooWorkspace* w = new RooWorkspace();
    w->import(data);
    efficiency.setVal(0);//reset
    efficiency.setAsymError(0,0);
    std::cout << "ALL dataset: calling doFitEfficiency with pdf: " << pdfCategory.getLabel() << std::endl;
    doFitEfficiency(w, pdfCategory.getLabel(), efficiency);
    delete w;
  }

  // cout << "MCBS: Begin loop over all bins requested in Py file, using allCats" << endl;
  //loop over all bins with the help of allCats
  TIterator* it = allCats.typeIterator();
  for(RooCatType* t = (RooCatType*)it->Next(); t!=0; t = (RooCatType*)it->Next() ){
    //name of the multicategory
    TString catName = t->GetName();
    // MCBS: Print out the category names
    // cout << "MCBS: doing catName : " << catName <<  " t->getVal() : " << t->getVal() << endl;
    //skip unmapped states
    if(catName.Contains("NotMapped")) continue;
    //create the dataset
    // MCBS: this call to create the dataset is one of the biggest increases in the
    // memory footprint of the program.
    // Can this be changed to use TH1s? or vectors of TH1s?
    // Here is where we will probably need to make the appropriate histograms

    // cout << "MCBS: This RooCatType will be fitted. Using the cut: TString::Format( allCats==%d ,t->getVal()) : " << TString::Format("allCats==%d",t->getVal()) << endl;


    // cout << "MCBS: Use original dataset for setting category variables" << endl;
    // const RooArgSet* row = data_bin->get(); MCBS Commented
    
    // MCBS Change #1: Use original dataset
    // MCBS: But to get the right pdfName, one needs to ensure that the row used
    // is one that satisfies the allCats condition for this bin.

    size_t rowIndex(0);
    const RooArgSet* row = data.get(rowIndex); 
    while (((RooMultiCategory*)row->find("allCats"))->getIndex()!=t->getVal()) {
      row = data.get(++rowIndex);
    }
    // cout << "MCBS: Found row " << rowIndex << " with allCats=" << ((RooMultiCategory*)row->find("allCats"))->getIndex() << endl;
    //get PDF name
    TString pdfName(((RooCategory*)row->find("_pdfCategory_"))->getLabel());
    
    
    //make directory name
    TString dirName = catName;
    dirName.ReplaceAll("{","").ReplaceAll("}","").ReplaceAll(";","__");
    if(pdfName.Length() > 0){
      dirName.Append("__").Append(pdfName);
    }
    
    cout<<"MCBS: Fitting bin, this is also a lowest-level directory in output file:  " << endl
	<<"MCBS: " << dirName << endl;
    //make a directory for each bin
    gDirectory->mkdir(dirName)->cd();

    //create a workspace
    RooWorkspace* w = new RooWorkspace();
    //import the data

    // MCBS: Change #4, use binnedFit to decide whether to import the reduced RooDataSet
    // MCBS: or the smaller RooDataHist
    // MCBS: Change #5 Use the binnedFit bool to decide whether to call 
    // MCBS: reduce and create a reduced dataset
    // MCBS: OR to create the RooDataHist from here

    RooAbsData* data_bin;
    RooDataHist* data_binHist;
    size_t DataNumEntries(0);
    // MCBS: When using binnedFit, need the cut corresponding to current binning
    // MCBS: for making the RooDatHist and for getting the mean of variables.
    char currentCut[100];
    sprintf(currentCut,"allCats==%d",t->getVal());

    if (!binnedFit) { 
      data_bin = (RooDataSet*) data.reduce(Cut(TString::Format("allCats==%d",t->getVal())));
      DataNumEntries = data_bin->numEntries();
      // cout << "MCBS: data_bin has been created. How different is it from the RooDataSet from where it came?" << endl;
      // cout << "MCBS: data_bin->numEntries() " << DataNumEntries << endl;
      // cout << "MCBS: sizeof(*data_bin) " << sizeof(*data_bin) << endl;
      // data_bin->Print();
      w->import(*data_bin);
      delete data_bin; // clean up earlier
      data_bin = w->data("data"); // point to the data that's in the workspace now (saves memory)

    }

    // MCBS: Change #2 Need to create the RooDataHist at this point
    // MCBS: And Import RooDataHist into workspace
    // MCBS: 
    
    // MCBS: Need the RooArgSet that determines the dimensions of the RooDataHist
    // MCBS: Originally, the RooArgSet was obtained via w->pdf("simPdf")->getObservables(dataObs)
    // MCBS: This returns the variables present in dataObs that are used in simPdf
    // MCBS: Do this change at the same time as Change #3: remove the RooDataHist that was created in doFitEfficiency.

    else {
      RooArgSet* obs = new RooArgSet;
      obs->add((*data.get(rowIndex))["_efficiencyCategory_"]);
      obs->add(*unbinnedVars.first());


      data_binHist = new RooDataHist("data_binned", "data_binned", *obs);
      data_binHist->add(data, currentCut );
      // cout << "MCBS: Printing MY RooDataHist" << endl;
      data_binHist->printMultiline(cout,0,kTRUE,"MCBS:");
      
      DataNumEntries = data_binHist->sumEntries(); 
      w->import(*data_binHist); // The import command will clone and own the clone.
      delete data_binHist; // clean up the original one
      data_binHist = (RooDataHist*) w->data("data_binned"); // point to the RooDataHist that is in the workspace now

    }
    
    // cout << "MCBS: created workspace with RooDataHist called 'data_binned' Print it:" << endl;
    // w->Print("t");

    //save the distribution of variables
    if (doSaveDistributionsPlot) saveDistributionsPlot(w);
    //do the fitting only if there is sufficient number of events
    // if(data_bin->numEntries()>0){ // MCBS: Change #6, change to not use data_bin to avoid using pointer (compiler error)
    if(DataNumEntries>0){
      //set the values of binnedVariables to the mean value in this data bin
      // cout << "MCBS: Setting means of variables for " << currentCut << endl;
      RooArgSet meanOfVariables;
      RooLinkedListIter vit = binnedVariables.iterator();
      for(RooRealVar* v = (RooRealVar*)vit.Next(); v!=0; v = (RooRealVar*)vit.Next() ){
        meanOfVariables.addClone(*v);
        //double mean = w->data("data")->mean(*v); // MCBS: Change #7, this line needs w->data("data"), change to use if
	
	double mean(0);
	if (binnedFit) {
	  //	  mean = data.mean(*v,currentCut); // MCBS: Needs to be done only in the region consistent with allCats bin.
	  mean = calculateMeanOfVariable(&data,v,currentCut); // MCBS: Use a helper function to do the mean calculation correctly

	}
	else {
	  mean = w->data("data")->mean(*v);
	}

	// cout << "MCBS: Mean " << v->GetName() << " = " << mean << endl;
	  
        RooBinning binning((RooBinning&)v->getBinning());
        int ind = binning.binNumber(mean);
        RooRealVar& newVar = (RooRealVar&)meanOfVariables[v->GetName()];
        newVar.setVal(mean);
        newVar.setAsymError(binning.binLow(ind)-mean, binning.binHigh(ind)-mean);
      }
      // cout << "MCBS: End of setting values of binnedVariables" << endl;
  
      //put an entry in the efficiency dataset
      //note that the category values are coming from data_bin->get(0)
      // meanOfVariables.addClone(*data_bin->get(0), true); // MCBS: Avoid using data_bin 
      meanOfVariables.addClone(*(data.get(rowIndex)), true); 
      
      // cout << "MCBS: Calling doFitEfficiency" << endl;
      efficiency.setVal(0);//reset
      efficiency.setAsymError(0,0);
      doFitEfficiency(w, pdfName.Data(), efficiency);
      fitEfficiency.add( RooArgSet(meanOfVariables, efficiency) );

      // cout << "MCBS: After doFitEfficiency, fitEfficiency.add" << endl;
/*      efficiency.setVal(0);//reset
      doSBSEfficiency(w, efficiency);
      sbsEfficiency.add( RooArgSet(meanOfVariables, efficiency) );*/

      // MCBS: CNT efficiency, next 3 lines of code
      efficiency.setVal(0);//reset
      doCntEfficiency(w, efficiency);
      cntEfficiency.add( RooArgSet(meanOfVariables, efficiency) );
    }
    // MCBS: delete data_bin, since this is the last time it is used is in the previous code block
    // // cout << "MCBS: Deleting data_bin" << endl; // MCBS
    // delete data_bin; // MCBS

    //save the workspace if requested
    if(saveWorkspace){
      w->Write("w");
    }
    //clean up
    delete w;
    //get back to the initial directory
    gDirectory->cd("..");

    
  } // end of loop over bins using allCats
  
  //save the efficiency data
  fitEfficiency.Write();
  gDirectory->mkdir("fit_eff_plots")->cd();
  saveEfficiencyPlots(fitEfficiency, effName, binnedVariables, mappedCategories);
  gDirectory->cd("..");

/*  sbsEfficiency.Write();
  gDirectory->mkdir("sbs_eff_plots")->cd();
  saveEfficiencyPlots(sbsEfficiency, effCat+"::"+effState, binnedVariables, mappedCategories);
  gDirectory->cd("..");*/

  //MCBS: Comment out cntEfficiency
  cntEfficiency.Write();
  gDirectory->mkdir("cnt_eff_plots")->cd();
  saveEfficiencyPlots(cntEfficiency, effName, binnedVariables, mappedCategories);
  gDirectory->cd("..");
  //empty string means no error
  return "";
}

double TagProbeFitter::calculateMeanOfVariable(RooDataSet* ds, RooRealVar* var, char* cut) {

  // The cut specification is such that it can be understood by the dataset using a RooFormula
  // It is similar to a cut applied to a TTree.  For example: "-2.4 < eta && eta < 0",
  // or for our case, since we have added allCats to the dataset, we can use "allCats==0", "allCats==1", etc.
  RooRealVar *varPtr = (RooRealVar*) ds->get()->find(var->GetName());
  RooFormula select("select",cut,*(ds->get()));
  double theSum(0);
  double sumEntries(0);
  
  for (size_t iRow=0; iRow<(size_t)ds->numEntries(); ++iRow) {
    ds->get(iRow); 
    // apply the cut using the RooFormula
    if (select.eval() == 0) continue; 
    theSum += ds->weight() * varPtr->getVal();
    sumEntries++;
  }// loop over the rows/entries of the dataset
  // // cout << "MCBS:     theSum = " << theSum << endl;
  // // cout << "MCBS: sumEntries = " << sumEntries << endl;
  
  return (sumEntries>0) ? theSum/sumEntries : 0;
}
void TagProbeFitter::doFitEfficiency(RooWorkspace* w, string pdfName, RooRealVar& efficiency){
  // cout << "MCBS: Inside doFitEfficiency" << endl;
  //if pdfName is empty skip the fit
  if(pdfName == "all"){
    return;
  }
  
  //create the simultaneous pdf of name pdfName
  createPdf(w, pdfs[pdfName]);
  

  //set the initial values for the yields of signal and background

  //RooAbsData *data = w->data("data");
  
  setInitialValues(w);  
  std::auto_ptr<RooFitResult> res(0);
  

  RooAbsData *data = w->data("data");
  std::auto_ptr<RooDataHist> bdata; 
  // cout << "MCBS: Check binnedFit " << binnedFit << endl;
  if (binnedFit) { 
    // cout << "MCBS: Doing binned fit!" << endl;
    // get variables from data, which contain also other binning or expression variables
    
    // MCBS: Change #3, Do not create the RooDataHist here.
    // MCBS: This is created in Change #2.  So here all that is needed is to
    // MCBS: have the pointer come from the RooDataHist that has already been imported to the workspace
    //

    // MCBS: Begin commenting out blockCreateHistogram:

    // const RooArgSet *dataObs = data->get(0); 
    // // cout << "MCBS: print out dataObs, obtained from data->get(0)" << endl;
    // dataObs->Print();
    // // remove everything which is not a dependency of the pdf
    // RooArgSet *obs = w->pdf("simPdf")->getObservables(dataObs);
    // // cout << "MCBS: obs is used for creating the RooDataHist, print it" << endl;
    // obs->Print();
    // bdata.reset(new RooDataHist("data_binned", "data_binned", *obs, *data)); 
    // //binnedW->import(*bdata); //MCBS: changed from w to binnedW, use workspace with only RooDataHist
    // w->import(*bdata); // MCBS: Insert RooDataHist into RooWorskpace
    // data = w->data("data_binned");  // Set pointer to point to RooDataHist 
    // delete obs;

    // MCBS: End commenting out blockCreateHistogram
    
    // MCBS: Replacement code to set the pointer to the RooDataHist:
    data = w->data("data_binned");
    
    // cout << "MCBS: check created RooDataHist by using printMultiline" << endl;
    data->printMultiline(cout,0,kTRUE,"MCBS:");
  }
  // MCBS: End of original code block.
  
  double totPassing = data->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Passed");
  double totFailing = data->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Failed");

  // cout << "MCBS: After the workspace has imported the RooDataHist for this binned fit, Print workspace" << endl;
  // w->Print("t");// MCBS


  // cout << "MCBS: totPassing " << totPassing << ", totFailing " << totFailing << endl;
  
  // cout << "MCBS: Creating simNLL as RooAbsReal, and passing it to RooMinimizer and RooMiniut" << endl;
  // cout << "MCBS: simNLL is created based on *data, which will be a RooDataHist if binnedFit is true" << endl;
  // cout << "MCBS: Print out data object, which has name " << data->GetName() << endl;
  // data->Print();

  RooAbsReal* simNLL = w->pdf("simPdf")->createNLL(*data,Extended(true),NumCPU(numCPU));

  RooMinimizer minimizer(*simNLL); // we are going to use this for 'scan'
  RooMinuit minuit(*simNLL);
  minuit.setStrategy(1);
  minuit.setProfile(true);
  RooProfileLL profileLL("simPdfNLL","",*simNLL,*w->var("efficiency"));

  // cout << "MCBS:After constructing RooMinuit" << endl;  

  //******* The block of code below is to make the fit converge faster.
  // ****** This part is OPTIONAL, i.e., off be default. User can activate this
  // ****** by setting the following parameters: "fixVars" and "floatShapeParameters"
  // ****** Here is the full logic:
  /////   ---> if "floatShapeParameters==true" && "fixVars is empty" : 
  ////             Just perform the fit without any of these optimizations (this is the default logic)
  /////   ---> if "floatShapeParameters==true" && "fixVars is NOT empty" : 
  ////             In each bin, fix the desired parameters and perform the fit to obtain a good starting point.
  /////            Then float these parameters and fit again.
  ////    ---> if "floatShapeParameters==false" && "fixVars is empty" : 
  ////             Do not perform any fit at all. Just print error message.
  ////    ---> if "floatShapeParameters==false" && "fixVars is NOT empty" : 
  ///              Perform a global fit to the whole sample, save the fitted values of the 
  ///              user specified parameters, and fix them for bin-by-bin fit. 

  // cout << "MCBS: fixVars.empty() " << fixVars.empty() << endl;

  if(!fixVars.empty()){
    // calculate initial values for parameters user want to fix
    if(!floatShapeParameters && fixVarValues.empty()){
      // we want to fix these parameters for each bin.
      // the following sequence fixes them, fits, releases and fits again
      // to get reasonable values. 
      // ----------------------------------------------------------------------
      // This procedure works only once with a whole dataset (without binning)
      // ----------------------------------------------------------------------

      // fix them
      varFixer(w,true);
      //do fit 
      minimizer.minimize("Minuit2","Scan");
      minuit.migrad();
      minuit.hesse();
      //minuit.minos();
      //w->pdf("simPdf")->fitTo(*data, Save(true), Extended(true), NumCPU(numCPU), Strategy(2),
      //PrintLevel(quiet?-1:1), PrintEvalErrors(quiet?-1:1), Warnings(!quiet));
      //release vars
      varFixer(w,false);
      //do fit 
      minimizer.minimize("Minuit2","Scan");      
      minuit.migrad();
      minuit.hesse();
      //minuit.minos();
      //w->pdf("simPdf")->fitTo(*data, Save(true), Extended(true), NumCPU(numCPU), Strategy(2),
      //PrintLevel(quiet?-1:1), PrintEvalErrors(quiet?-1:1), Warnings(!quiet));
      //save vars
      varSaver(w);
      // now we have a starting point. Fit will converge faster.
    }
    
    // here we can use initial values if we want (this works for each bin)
    if(!floatShapeParameters) varRestorer(w);  //restore vars

    // cout << "MCBS: Calling minimize, minuit.migrad, and minuit.hesse. 2nd time (fixvars not empty() )" << endl;
    //do fit
    minimizer.minimize("Minuit2","Scan");
    minuit.migrad();
    minuit.hesse();
    // initialize the profile likelihood
    profileLL.getVal();
    RooMinimizer* profMinuit = profileLL.minimizer();
    profMinuit->setProfile(true);
    profMinuit->setStrategy(2);     
    profMinuit->setPrintLevel(1);
    profMinuit->minos(*w->var("efficiency"));
    res.reset( profMinuit->save() );
    //res = w->pdf("simPdf")->fitTo(*data, Save(true), Extended(true), NumCPU(numCPU), Strategy(2),
    //Minos(*w->var("efficiency")), PrintLevel(quiet?-1:1), 
    //PrintEvalErrors(quiet?-1:1), Warnings(!quiet));
  }//if(!fixVars.empty())
      
  // (default = true) if we don't want to fix any parameters or want to fit each bin with all parameters floating
  if(floatShapeParameters){
    //release vars
    varFixer(w,false);
    
    // cout << "MCBS: Calling minimize, minuit.migrad, and minuit.hesse. 3d time (floatShapeParameters == true) " << endl;
    //do fit
    minimizer.minimize("Minuit2","Scan");
    minuit.migrad();
    minuit.hesse();
    res.reset( w->pdf("simPdf")->fitTo(*data, Save(true), Extended(true), NumCPU(numCPU), Strategy(2),
                                    Minos(*w->var("efficiency")), PrintLevel(quiet?-1:1), 
      				  PrintEvalErrors(quiet?-1:1), Warnings(!quiet)) );
   }



  // save everything
  res->Write("fitresults");
  w->saveSnapshot("finalState",w->components());
  saveFitPlot(w);
  //extract the efficiency parameter from the results
  RooRealVar* e = (RooRealVar*) res->floatParsFinal().find("efficiency");
  //What's wrong with this? and why don't they copy the errors!
  //efficiency = *e;
  efficiency.setVal(e->getVal());
  Double_t errLo = e->getErrorLo(), errHi = e->getErrorHi();
  if (errLo == 0 && e->getVal() < 0.5) errLo = e->getMin()-e->getVal();
  if (errHi == 0 && e->getVal() > 0.5) errHi = e->getMax()-e->getVal();
  efficiency.setAsymError(errLo, errHi);

  if (totPassing * totFailing == 0) {
   RooRealVar* nTot = (RooRealVar*) res->floatParsFinal().find("numTot");
   RooRealVar* fSig = (RooRealVar*) res->floatParsFinal().find("fSigAll");
   double cerr = ROOT::Math::beta_quantile( 1-(1.0-.68540158589942957)/2, 1, nTot->getVal() * fSig->getVal() ); 
    
    std::cout << "======================================================================================" << std::endl;
    std::cout << "======= totPassing "  << totPassing << ", totFailing " << totFailing << std::endl;
    std::cout << "======= FIT: e  "  <<  e->getVal() << ",  e Lo " << e->getErrorLo()  << ",  e Hi " <<  e->getErrorHi() << std::endl;
    // std::cout << "======= FIT:nS  "  << nS->getVal() << ", nS Lo " << nS->getErrorLo() << ", nS Hi " << nS->getErrorHi() << std::endl;
    // std::cout << "======= FIT:nB  "  << nB->getVal() << ", nB Lo " << nB->getErrorLo() << ", nB Hi " << nB->getErrorHi() << std::endl;
    // std::cout << "======= CNT:    "  << cerr << std::endl;
    std::cout << "======================================================================================" << std::endl;
    // MCBS: Uncommented the cout statements above.
    if (totPassing == 0) {
      efficiency.setVal(0);
      efficiency.setAsymError(0,cerr);
    } else {
      efficiency.setVal(1);
      efficiency.setAsymError(-cerr,0);
    }
  }

  delete simNLL;
}

void TagProbeFitter::createPdf(RooWorkspace* w, vector<string>& pdfCommands){
  // create the signal and background pdfs defined by the user
  // cout << "MCBS: inside createPdf" <<  endl;
  for(unsigned int i=0; i<pdfCommands.size(); i++){
    // cout << "MCBS: pdfCommands[" << i << "] = " << pdfCommands[i] << endl;
    const std::string & command = pdfCommands[i];
    if (command.find("#import ") == 0) {
        TDirectory *here = gDirectory;
        w->import(command.substr(8).c_str());
        here->cd();
    } else {
      TDirectory *here = gDirectory;
      w->factory(command.c_str());
      here->cd();
    }
  }
  // cout << "MCBS: end of loop of pdfCommands in createPdf" << endl;
  // setup the simultaneous extended pdf

  w->factory("expr::nSignalPass('efficiency*fSigAll*numTot', efficiency, fSigAll[.9,0,1],numTot[1,0,1e10])");
  w->factory("expr::nSignalFail('(1-efficiency)*fSigAll*numTot', efficiency, fSigAll,numTot)");  
  w->factory("expr::nBkgPass('effBkg*(1-fSigAll)*numTot', effBkg[.9,0,1],fSigAll,numTot)");
  w->factory("expr::nBkgFail('(1-effBkg)*(1-fSigAll)*numTot', effBkg,fSigAll,numTot)");  
  TString sPass = "signal", sFail = "signal";
  if (w->pdf("signalPass") != 0 && w->pdf("signalFail") != 0) {
    if (w->pdf("signal") != 0) throw std::logic_error("You must either define one 'signal' PDF or two PDFs ('signalPass', 'signalFail'), not both!"); 
    sPass = "signalPass"; sFail = "signalFail";
  } else if (w->pdf("signal") != 0) {
    if (w->pdf("signalPass") != 0 || w->pdf("signalFail") != 0) {
        throw std::logic_error("You must either define one 'signal' PDF or two PDFs ('signalPass', 'signalFail'), not both!"); 
    }
  } else {
    throw std::logic_error("You must either define one 'signal' PDF or two PDFs ('signalPass', 'signalFail')");
  }
  w->factory("SUM::pdfPass(nSignalPass*"+sPass+", nBkgPass*backgroundPass)"); //fBkgPass*
  w->factory("SUM::pdfFail(nSignalFail*"+sFail+", nBkgFail*backgroundFail)"); //fBkgFail*

  w->factory("SIMUL::simPdf(_efficiencyCategory_, Passed=pdfPass, Failed=pdfFail)");
  // signalFractionInPassing is not used in the fit just to set the initial values
  if (w->pdf("simPdf") == 0) throw std::runtime_error("Could not create simultaneous fit pdf.");
  if(w->var("signalFractionInPassing") == 0)
    w->factory("signalFractionInPassing[0.9]");
}

void TagProbeFitter::setInitialValues(RooWorkspace* w){
  // calculate initial values
  double signalEfficiency = w->var("efficiency")->getVal();
  double signalFractionInPassing = w->var("signalFractionInPassing")->getVal();
  // MCBS: Change #8
  // Enclose in if, for binnedFit or not
  // double totPassing = w->data("data")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Passed");
  // double totFailinging = w->data("data")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Failed");
  double totPassing = 0;
  double totFailing = 0;
 
  if (!binnedFit) {
    totPassing = w->data("data")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Passed");
    totFailing = w->data("data")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Failed");
  }
  else {
    totPassing = w->data("data_binned")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Passed");
    totFailing = w->data("data_binned")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Failed"); 
  }
  double numSignalAll = totPassing*signalFractionInPassing/signalEfficiency;

  //std::cout << "Number of probes: " << totPassing+totFailing << std::endl;

  // check if this value is inconsistent on the failing side
  if(numSignalAll*(1-signalEfficiency) > totFailing)
    numSignalAll = totFailing;
  // now set the values
  w->var("numTot")->setVal(totPassing+totFailing);
  w->var("numTot")->setMax(2.0*(totPassing+totFailing)+10); //wiggle room in case of 0 events in bin

  if (totPassing == 0) {
    w->var("efficiency")->setVal(0.0);
    w->var("efficiency")->setAsymError(0,1);
    w->var("efficiency")->setConstant(false);
  } else if (totFailing == 0) {
    w->var("efficiency")->setVal(1.0);
    w->var("efficiency")->setAsymError(-1,0);
    w->var("efficiency")->setConstant(false);
  } else {
    w->var("efficiency")->setConstant(false);
  }

  // if signal fraction is 1 then set the number of background events to 0.
  //RooRealVar* fBkgPass = w->var("numBackgroundPass");
  //if(signalFractionInPassing==1.0) { fBkgPass->setVal(0.0); fBkgPass->setConstant(true); }

  // save initial state for reference
  w->saveSnapshot("initialState",w->components());
}

void TagProbeFitter::saveFitPlot(RooWorkspace* w){
  // save refferences for convenience
  RooCategory& efficiencyCategory = *w->cat("_efficiencyCategory_");
  RooAbsData* dataAll = (binnedFit ? w->data("data_binned") : w->data("data") );
  RooAbsData* dataPass = dataAll->reduce(Cut("_efficiencyCategory_==_efficiencyCategory_::Passed")); 
  RooAbsData* dataFail = dataAll->reduce(Cut("_efficiencyCategory_==_efficiencyCategory_::Failed")); 
  RooAbsPdf& pdf = *w->pdf("simPdf");
  std::auto_ptr<RooArgSet> obs(pdf.getObservables(*dataAll));
  RooRealVar* mass = 0;
  RooLinkedListIter it = obs->iterator();
  for(RooAbsArg* v = (RooAbsArg*)it.Next(); v!=0; v = (RooAbsArg*)it.Next() ){
    if(!v->InheritsFrom("RooRealVar")) continue;
    mass = (RooRealVar*)v;
    break;
  }
  if(!mass) return;
  // make a 2x2 canvas
  TCanvas canvas("fit_canvas");
  canvas.Divide(2,2);
  vector<RooPlot*> frames;
  // plot the Passing Probes
  canvas.cd(1);
  if (massBins == 0) {
      frames.push_back(mass->frame(Name("Passing"), Title("Passing Probes")));
      frames.push_back(mass->frame(Name("Failing"), Title("Failing Probes")));
      frames.push_back(mass->frame(Name("All"),     Title("All Probes")));
  } else {
      frames.push_back(mass->frame(Name("Passing"), Title("Passing Probes"), Bins(massBins)));
      frames.push_back(mass->frame(Name("Failing"), Title("Failing Probes"), Bins(massBins)));
      frames.push_back(mass->frame(Name("All"),     Title("All Probes"), Bins(massBins)));
  }
  dataPass->plotOn(frames[0]);
  pdf.plotOn(frames[0], Slice(efficiencyCategory, "Passed"), ProjWData(*dataPass), LineColor(kGreen));
  pdf.plotOn(frames[0], Slice(efficiencyCategory, "Passed"), ProjWData(*dataPass), LineColor(kGreen), Components("backgroundPass"), LineStyle(kDashed));
  frames[0]->Draw();
  // plot the Failing Probes
  canvas.cd(2);
  dataFail->plotOn(frames[1]);
  pdf.plotOn(frames[1], Slice(efficiencyCategory, "Failed"), ProjWData(*dataFail), LineColor(kRed));
  pdf.plotOn(frames[1], Slice(efficiencyCategory, "Failed"), ProjWData(*dataFail), LineColor(kRed), Components("backgroundFail"), LineStyle(kDashed));
  frames[1]->Draw();
  // plot the All Probes
  canvas.cd(3);
  dataAll->plotOn(frames.back());
  pdf.plotOn(frames.back(), ProjWData(*dataAll), LineColor(kBlue));
  pdf.plotOn(frames.back(), ProjWData(*dataAll), LineColor(kBlue), Components("backgroundPass,backgroundFail"), LineStyle(kDashed));
  frames.back()->Draw();
  // plot the Fit Results
  canvas.cd(4);
  frames.push_back(mass->frame(Name("Fit Results"), Title("Fit Results")));
  pdf.paramOn(frames.back(), dataAll, "", 0, "NELU", 0.1, 0.9, 0.9);
  // draw only the parameter box not the whole frame
  frames.back()->findObject(Form("%s_paramBox",pdf.GetName()))->Draw();
  //save and clean up
  canvas.Draw();
  canvas.Write();
  for (size_t i=0; i<frames.size(); i++) {
    delete frames[i];
  }
  delete dataPass;
  delete dataFail;
}

void TagProbeFitter::saveDistributionsPlot(RooWorkspace* w){
  // save pointers to datasets to manage memory
  RooAbsData* dataAll = w->data("data");
  RooAbsData* dataPass = dataAll->reduce(Cut("_efficiencyCategory_==_efficiencyCategory_::Passed")); 
  RooAbsData* dataFail = dataAll->reduce(Cut("_efficiencyCategory_==_efficiencyCategory_::Failed")); 

  const RooArgSet* vars = dataAll->get();
  vector<RooRealVar*> reals;
  RooLinkedListIter it = vars->iterator();
  for(RooAbsArg* v = (RooAbsArg*)it.Next(); v!=0; v = (RooAbsArg*)it.Next() ){
    if(!v->InheritsFrom("RooRealVar")) continue;
    reals.push_back((RooRealVar*)v);
  }
  TCanvas canvas("distributions_canvas");
  canvas.Divide(3,reals.size());
  vector<RooPlot*> frames;
  for(unsigned int i=0; i<reals.size(); i++){
    // plot the Passing Probes
    canvas.cd(3*i+1);
    frames.push_back(reals[i]->frame(Name("Passing"), Title("Passing Probes"), Bins(100)));
    dataPass->plotOn(frames.back(), LineColor(kGreen));
    dataPass->statOn(frames.back());
    frames.back()->Draw();
    // plot the Failing Probes
    canvas.cd(3*i+2);
    frames.push_back(reals[i]->frame(Name("Failing"), Title("Failing Probes"), Bins(100)));
    dataFail->plotOn(frames.back(), LineColor(kRed));
    dataFail->statOn(frames.back());
    frames.back()->Draw();
    // plot the All Probes
    canvas.cd(3*i+3);
    frames.push_back(reals[i]->frame(Name("All"), Title("All Probes"), Bins(100)));
    dataAll->plotOn(frames.back(), LineColor(kBlue));
    dataAll->statOn(frames.back());
    frames.back()->Draw();
  }
  canvas.Draw();
  canvas.Write();
  for (size_t i=0; i<frames.size(); i++) {
    delete frames[i];
  }
  delete dataPass;
  delete dataFail;
}

void TagProbeFitter::saveEfficiencyPlots(RooDataSet& eff, const TString& effName, RooArgSet& binnedVariables,RooArgSet& mappedCategories){
  RooLinkedListIter v1it = binnedVariables.iterator();
  bool isOnePoint = (eff.numEntries() == 1); // for datasets with > 1 entry, we don't make plots for variables with just one bin
  for(RooRealVar* v1 = (RooRealVar*)v1it.Next(); v1!=0; v1 = (RooRealVar*)v1it.Next() ){
    RooArgSet binCategories1D;
    if (v1->numBins() == 1 && !isOnePoint) continue;
    RooLinkedListIter v2it = binnedVariables.iterator();
    for(RooRealVar* v2 = (RooRealVar*)v2it.Next(); v2!=0; v2 = (RooRealVar*)v2it.Next() ){
      if(v2 == v1) continue;
      if (v2->numBins() == 1 && !isOnePoint) continue;
      binCategories1D.addClone( RooBinningCategory(TString(v2->GetName())+"_bins", TString(v2->GetName())+"_bins", *v2) );

      RooArgSet binCategories2D;
      RooLinkedListIter v3it = binnedVariables.iterator();
      for(RooRealVar* v3 = (RooRealVar*)v3it.Next(); v3!=0; v3 = (RooRealVar*)v3it.Next() ){
        if(v3 == v1 || v3 == v2) continue;
        binCategories2D.addClone( RooBinningCategory(TString(v3->GetName())+"_bins", TString(v3->GetName())+"_bins", *v3) );
      }  
      RooMultiCategory allCats2D("allCats2D", "allCats2D", RooArgSet(binCategories2D, mappedCategories));
      if(allCats2D.numTypes()==0){
        makeEfficiencyPlot2D(eff, *v1, *v2, TString::Format("%s_%s_PLOT", v1->GetName(), v2->GetName()), "", effName);
      }else{
        RooDataSet myEff(eff);
        myEff.addColumn(allCats2D);
        std::auto_ptr<TIterator> catIt(allCats2D.typeIterator());
        for(RooCatType* t = (RooCatType*)catIt->Next(); t!=0; t = (RooCatType*)catIt->Next() ){
          TString catName = t->GetName();
          if(catName.Contains("NotMapped")) continue;
          catName.ReplaceAll("{","").ReplaceAll("}","").ReplaceAll(";","_&_");
          makeEfficiencyPlot2D(myEff, *v1, *v2, TString::Format("%s_%s_PLOT_%s",v1->GetName(), v2->GetName(), catName.Data()), catName, effName, "allCats1D", t->getVal());
        }        
      }
    }
    RooMultiCategory allCats1D("allCats1D", "allCats1D", RooArgSet(binCategories1D, mappedCategories));
    if(allCats1D.numTypes()==0){
      makeEfficiencyPlot1D(eff, *v1, TString::Format("%s_PLOT", v1->GetName()), "", effName);
    }else{
      RooDataSet myEff(eff);
      myEff.addColumn(allCats1D);
      std::auto_ptr<TIterator> catIt(allCats1D.typeIterator());
      for(RooCatType* t = (RooCatType*)catIt->Next(); t!=0; t = (RooCatType*)catIt->Next() ){
        TString catName = t->GetName();
        if(catName.Contains("NotMapped")) continue;
        catName.ReplaceAll("{","").ReplaceAll("}","").ReplaceAll(";","_&_");
        makeEfficiencyPlot1D(myEff, *v1, TString::Format("%s_PLOT_%s", v1->GetName(), catName.Data()), catName, effName, "allCats1D", t->getVal());
      }
    }
  }
}

void TagProbeFitter::makeEfficiencyPlot1D(RooDataSet& eff, RooRealVar& v, const TString& plotName, const TString& plotTitle, const TString& effName, const char *catName, int catIndex){
 TGraphAsymmErrors *p = new TGraphAsymmErrors();
  const RooArgSet *entry = eff.get();
  const RooRealVar &vi = dynamic_cast<const RooRealVar &>(*entry->find(v.GetName()));
  const RooRealVar &ei = dynamic_cast<const RooRealVar &>(*entry->find("efficiency"));
  for (unsigned int i = 0, n = eff.numEntries(); i < n; ++i) {
    entry = eff.get(i); 
    if (catName != 0 && entry->getCatIndex(catName) != catIndex) continue;
    int j = p->GetN(); p->Set(j+1);
    p->SetPoint(j, vi.getVal(), ei.getVal() );
    p->SetPointError(j, -vi.getAsymErrorLo(), vi.getAsymErrorHi(), -ei.getAsymErrorLo(), ei.getAsymErrorHi() );
  }
  TCanvas canvas(plotName);
  TH1F *frame = new TH1F("frame", "Efficiency of "+effName, 1, v.getMin(), v.getMax()); frame->SetDirectory(0);
  p->SetNameTitle(Form("hxy_%s", eff.GetName()), "Efficiency of "+effName);
  p->GetXaxis()->SetTitle(strlen(v.getUnit()) ? Form("%s (%s)", v.GetName(), v.getUnit()) : v.GetName());
  p->GetYaxis()->SetTitle("Efficiency of "+effName);
  frame->GetXaxis()->SetTitle(strlen(v.getUnit()) ? Form("%s (%s)", v.GetName(), v.getUnit()) : v.GetName());
  frame->GetYaxis()->SetTitle("Efficiency of "+effName);
  frame->GetYaxis()->SetRangeUser(0,1);
  frame->Draw();
  p->SetLineWidth(2); p->SetMarkerStyle(kFullCircle); p->SetMarkerSize(1.2);
  p->Draw("P SAME");
  canvas.Write();
  delete frame;
  delete p;  
}

void TagProbeFitter::makeEfficiencyPlot2D(RooDataSet& eff, RooRealVar& v1, RooRealVar& v2, const TString& plotName, const TString& plotTitle, const TString& effName, const char *catName, int catIndex){
 TCanvas canvas(plotName);
  canvas.SetRightMargin(0.15);
  TH2F* h = new TH2F(plotName, plotName, v1.getBinning().numBins(), v1.getBinning().array(), v2.getBinning().numBins(), v2.getBinning().array());
  const RooArgSet* set = eff.get();
  RooRealVar* e = (RooRealVar*) set->find("efficiency");
  RooRealVar* v1_ = (RooRealVar*) set->find(v1.GetName());
  RooRealVar* v2_ = (RooRealVar*) set->find(v2.GetName());
  h->SetTitle(TString::Format("%s;%s%s;%s%s;Efficiency of %s", plotTitle.Data(),
    v1.GetTitle(), TString(v1.getUnit()).Length()==0?"":TString::Format(" (%s)", v1.getUnit()).Data(),
    v2.GetTitle(), TString(v2.getUnit()).Length()==0?"":TString::Format(" (%s)", v2.getUnit()).Data(), effName.Data()));
  h->SetOption("colztexte");
  h->GetZaxis()->SetRangeUser(-0.001,1.001);
  h->SetStats(kFALSE);
  for(int i=0; i<eff.numEntries(); i++){
    const RooArgSet *entry = eff.get(i); 
    if (catName != 0 && entry->getCatIndex(catName) != catIndex) continue;
    h->SetBinContent(h->FindBin(v1_->getVal(), v2_->getVal()), e->getVal());
    h->SetBinError(h->FindBin(v1_->getVal(), v2_->getVal()), (e->getErrorHi()-e->getErrorLo())/2.);
  }
  h->Draw();
  canvas.Draw();
  canvas.Write();
  delete h;
}

void TagProbeFitter::doSBSEfficiency(RooWorkspace* w, RooRealVar& efficiency){
}

void TagProbeFitter::doCntEfficiency(RooWorkspace* w, RooRealVar& efficiency){
  // MCBS: Change #9. Encapsulate calls to reduced RooDataSet in if statement
  // MCBS: with the alternative being a call to the RooDataHist  
  // int pass = w->data("data")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Passed");
  // int fail = w->data("data")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Failed");

  int pass(-1), fail(-1); //   
  if (!binnedFit) {
    pass = w->data("data")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Passed");
    fail = w->data("data")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Failed");
  }
  else {
    pass = w->data("data_binned")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Passed");
    fail = w->data("data_binned")->sumEntries("_efficiencyCategory_==_efficiencyCategory_::Failed");
  
  }
  double e = (pass+fail == 0) ? 0 : pass/double(pass+fail);
  // Use Clopper-Pearson
  double alpha = (1.0 - .68540158589942957)/2;
  double lo = (pass == 0) ? 0.0 : ROOT::Math::beta_quantile(   alpha, pass,   fail+1 );
  double hi = (fail == 0) ? 1.0 : ROOT::Math::beta_quantile( 1-alpha, pass+1, fail   );
  ////from TGraphAsymmErrors
  //double lob, hib;
  //Efficiency( pass, pass+fail, .68540158589942957, e, lob, hib );
  //std::cerr << "CNT " << pass << "/" << fail << ":  Clopper Pearson [" << lo << ", "  << hi << "], Bayes [" << lob << ", " << hib << "]" << std::endl;
  efficiency.setVal(e);
  efficiency.setAsymError(lo-e, hi-e);
}

void TagProbeFitter::varFixer(RooWorkspace* w, bool fix){
  std::vector<std::string>::const_iterator it;
  for(it=fixVars.begin(); it<fixVars.end(); it++){    
    if(w->var((*it).c_str()))
      w->var((*it).c_str())->setAttribute("Constant",fix);
    else{
      std::cout << "TagProbeFitter: " << "Can't find a variable to fix: " << *it;
    }
  }
}

void TagProbeFitter::varSaver(RooWorkspace* w){
  if(!fixVarValues.empty()){
    std::cout << "attempt to save variables more than once!" << std::endl;
    return;
  }
  std::vector<std::string>::const_iterator it;
  for(it=fixVars.begin(); it<fixVars.end(); it++){
    fixVarValues.push_back(w->var((*it).c_str())->getVal());
  }
  
}

void TagProbeFitter::varRestorer(RooWorkspace* w){
  if(fixVarValues.size()==fixVars.size())
    for(unsigned int i=0; i< fixVars.size(); i++){
      std::cout << "setting variable " << fixVars[i].c_str() << std::endl;
      w->var(fixVars[i].c_str())->setVal(fixVarValues[i]);
    }
  else{
    std::cout << "fixVars and fixVarValues are not of the same size!" << std::endl; 
  }
}

