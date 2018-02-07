#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "Validation/DTRecHits/interface/utils.h"

#include "DTRecHitClients.h"

using namespace std;
using namespace edm;

DTRecHitClients::DTRecHitClients(edm::ParameterSet const& config)
{
}

DTRecHitClients::~DTRecHitClients()
{
}

void DTRecHitClients::dqmEndJob(DQMStore::IBooker & booker, DQMStore::IGetter & getter)
{

  MonitorElement * hRes_S3RPhi = getter.get("DT/1DRecHits/Res/1D_S3RPhi_hRes");
  MonitorElement * hRes_S3RZ = getter.get("DT/1DRecHits/Res/1D_S3RZ_hRes");
  MonitorElement * hRes_S3RZ_W0 = getter.get("DT/1DRecHits/Res/1D_S3RZ_W0_hRes");
  MonitorElement * hRes_S3RZ_W1 = getter.get("DT/1DRecHits/Res/1D_S3RZ_W1_hRes");
  MonitorElement * hRes_S3RZ_W2 = getter.get("DT/1DRecHits/Res/1D_S3RZ_W2_hRes");

  MonitorElement * hPull_S3RPhi = getter.get("DT/1DRecHits/Pull/1D_S3RPhi_hPull");
  MonitorElement * hPull_S3RZ = getter.get("DT/1DRecHits/Pull/1D_S3RZ_hPull");
  MonitorElement * hPull_S3RZ_W0 = getter.get("DT/1DRecHits/Pull/1D_S3RZ_W0_hPull");
  MonitorElement * hPull_S3RZ_W1 = getter.get("DT/1DRecHits/Pull/1D_S3RZ_W1_hPull");
  MonitorElement * hPull_S3RZ_W2 = getter.get("DT/1DRecHits/Pull/1D_S3RZ_W2_hPull");

  Tutils util;
  util.drawGFit(hRes_S3RPhi->getTH1(),-0.2,0.2,-0.1,0.1);
  util.drawGFit(hRes_S3RZ->getTH1(),-0.2,0.2,-0.1,0.1);
  util.drawGFit(hRes_S3RZ_W0->getTH1(),-0.2,0.2,-0.1,0.1);
  util.drawGFit(hRes_S3RZ_W1->getTH1(),-0.2,0.2,-0.1,0.1);
  util.drawGFit(hRes_S3RZ_W2->getTH1(),-0.2,0.2,-0.1,0.1);

  util.drawGFit(hPull_S3RPhi->getTH1(),-5,5,-5,5);
  util.drawGFit(hPull_S3RZ->getTH1(),-5,5,-5,5);
  util.drawGFit(hPull_S3RZ_W0->getTH1(),-5,5,-5,5);
  util.drawGFit(hPull_S3RZ_W1->getTH1(),-5,5,-5,5);
  util.drawGFit(hPull_S3RZ_W2->getTH1(),-5,5,-5,5);
}

// declare this as a framework plugin
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(DTRecHitClients);
