#include "DQM/DTMonitorClient/src/DTTnPEfficiencyClient.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

DTTnPEfficiencyClient::DTTnPEfficiencyClient (const edm::ParameterSet &pSet) {
  edm::LogVerbatim("DQM|DTMonitorModule|DTTnPEfficiencyClient") << "DTTnPEfficiencyClient: Constructor called";
};

DTTnPEfficiencyClient::~DTTnPEfficiencyClient () {
  edm::LogVerbatim("DQM|DTMonitorModule|DTTnPEfficiencyClient") << "DTTnPEfficiencyClient: Constructor called";
};

void DTTnPEfficiencyClient::beginRun(const edm::Run& run, const edm::EventSetup& setup) {
}

void DTTnPEfficiencyClient::dqmEndLuminosityBlock(DQMStore::IBooker& ibooker,
                                                      DQMStore::IGetter& igetter,
                                                      edm::LuminosityBlock const& lumiSeg,
                                                      edm::EventSetup const& setup) {
  edm::LogVerbatim("DTDQM|DTMonitorClient|DTChamberEfficiencyClient") << "DTChamberEfficiencyClient: endluminosityBlock";
}

void DTTnPEfficiencyClient::dqmEndJob(DQMStore::IBooker& ibooker, DQMStore::IGetter& igetter) {
  edm::LogVerbatim("DTDQM|DTMonitorClient|DTChamberEfficiencyClient") << "DTChamberEfficiencyClient: endRun";

  std::string outFolder = "Task";

  bookHistos(ibooker,outFolder);

  std::string baseFolder = topFolder() + outFolder + "/";

  std::string histoName_pass_allCh = baseFolder + "nPassingProbe_allCh";
  std::string histoName_fail_allCh = baseFolder + "nFailingProbe_allCh";
  
  MonitorElement* me_pass_allCh = igetter.get(histoName_pass_allCh);
  MonitorElement* me_fail_allCh = igetter.get(histoName_fail_allCh);

  TH1F* h_pass_allCh = me_pass_allCh->getTH1F();
  TH1F* h_fail_allCh = me_fail_allCh->getTH1F();
  const int nBin = effHistos["chamberEff_allCh"]->getNbinsX();
  for(int ch_idx=1; ch_idx<=nBin; ++ch_idx){
    const float nPass = h_pass_allCh->GetBinContent(ch_idx);
    const float nFail = h_fail_allCh->GetBinContent(ch_idx);
    
    const float num = nPass;
    const float den = nPass + nFail;
    
    if (den != 0.) {
      const float eff = num / den;
      const float eff_error = ((TMath::Sqrt(num)/num) + (TMath::Sqrt(den)/den))*eff;
    
      effHistos["chamberEff_allCh"]->setBinContent(ch_idx, eff);
      effHistos["chamberEff_allCh"]->setBinError(ch_idx, eff_error);
    }
  }

  //Loop over the wheels
  for (int wheel = -2; wheel <= 2; wheel++) {
    std::string histoName_pass = baseFolder + "nPassingProbePerCh_W" + std::to_string(wheel);
    std::string histoName_fail = baseFolder + "nFailingProbePerCh_W" + std::to_string(wheel);

    MonitorElement* me_pass = igetter.get(histoName_pass);
    MonitorElement* me_fail = igetter.get(histoName_fail);

    //get the TH2F
    if (!me_pass || !(me_pass->getTH2F()) || !me_fail || !(me_fail->getTH2F())) {
      edm::LogWarning("DTTnPEfficiencyClient") << "Monitor Element not available" << std::endl;
      return;
    }

    TH2F* h_pass = me_pass->getTH2F();
    TH2F* h_fail = me_fail->getTH2F();

    const int nBinX = effHistos[std::string("chamberEff_W") + std::to_string(wheel)]->getNbinsX();
    const int nBinY = effHistos[std::string("chamberEff_W") + std::to_string(wheel)]->getNbinsY();

    for (int sec_idx = 1; sec_idx <= nBinX; sec_idx++) {
      for (int sta_idx = 1; sta_idx <= nBinY; sta_idx++) {

        const float nPass = h_pass->GetBinContent(sec_idx, sta_idx);
        const float nFail = h_fail->GetBinContent(sec_idx, sta_idx);

        const float num = nPass;
        const float den = nPass + nFail;

        if (den != 0.) {
          const float eff = num / den;
          const float eff_error = ((TMath::Sqrt(num)/num) + (TMath::Sqrt(den)/den))*eff;
          //const float eff_error_All = sqrt((effAll + effAll * effAll) / denom);

          //std::cout<<"---------------NUM = "<<num<<"---------------"<<std::endl;
          //std::cout<<"---------------DEN = "<<den<<"---------------"<<std::endl;
          //std::cout<<"---------------EFFICIENCY ERROR = "<<eff_error<<"---------------"<<std::endl;


          effHistos[std::string("chamberEff_W") + std::to_string(wheel)]->setBinContent(sec_idx, sta_idx, eff);
          effHistos[std::string("chamberEff_W") + std::to_string(wheel)]->setBinError(sec_idx, sta_idx, eff_error);
        }
      }
    }
  }

  return;
}

void DTTnPEfficiencyClient::bookHistos(DQMStore::IBooker& ibooker, std::string folder) {
  ibooker.setCurrentFolder(topFolder() + folder + "/");
  
  effHistos["chamberEff_allCh"] = ibooker.book1D("chamberEff_allCh","chamberEff_allCh", 20, 1., 15.);
  effHistos["chamberEff_allCh"]->setBinLabel(1 , "MB1/YB-2", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(2 , "MB2/YB-2", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(3 , "MB3/YB-2", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(4 , "MB4/YB-2", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(5 , "MB1/YB-1", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(6 , "MB2/YB-1", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(7 , "MB3/YB-1", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(8 , "MB4/YB-1", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(9 , "MB1/YB0", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(10, "MB2/YB0", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(11, "MB3/YB0", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(12, "MB4/YB0", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(13, "MB1/YB1", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(14, "MB2/YB1", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(15, "MB3/YB1", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(16, "MB4/YB1", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(17, "MB1/YB2", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(18, "MB2/YB2", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(19, "MB3/YB2", 1);
  effHistos["chamberEff_allCh"]->setBinLabel(20, "MB4/YB2", 1);
  effHistos["chamberEff_allCh"]->setAxisTitle("Efficiency", 2);

  for (int wh = -2; wh <= 2; wh++) {
    std::string histoName = std::string("chamberEff_W") + std::to_string(wh);

    effHistos[histoName] = ibooker.book2D(histoName.c_str(),histoName.c_str(), 14, 1., 15., 4, 0., 5.);
    effHistos[histoName]->setAxisTitle("Sector", 1);
    effHistos[histoName]->setBinLabel(1, "MB1", 2);
    effHistos[histoName]->setBinLabel(2, "MB2", 2);
    effHistos[histoName]->setBinLabel(3, "MB3", 2);
    effHistos[histoName]->setBinLabel(4, "MB4", 2);
  }

  return;
}
