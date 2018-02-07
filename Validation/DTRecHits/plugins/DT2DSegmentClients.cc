#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "Validation/DTRecHits/interface/utils.h"

#include "DT2DSegmentClients.h"

using namespace std;
using namespace edm;

DT2DSegmentClients::DT2DSegmentClients(edm::ParameterSet const& config)
{
}

DT2DSegmentClients::~DT2DSegmentClients()
{
}

void DT2DSegmentClients::dqmEndJob(DQMStore::IBooker & booker, DQMStore::IGetter & getter)
{
  MonitorElement * hResPos    = getter.get("DT/2DSegments/Res/2D_SuperPhi_hResPos");
  MonitorElement * hResAngle  = getter.get("DT/2DSegments/Res/2D_SuperPhi_hResAngle");
  MonitorElement * hPullPos   = getter.get("DT/2DSegments/Pull/2D_SuperPhi_hPullPos");
  MonitorElement * hPullAngle = getter.get("DT/2DSegments/Pull/2D_SuperPhi_hPullAngle");

  Tutils util;
  util.drawGFit(hResPos->getTH1(), -0.1, 0.1, -0.1, 0.1);
  util.drawGFit(hResAngle->getTH1(), -0.1, 0.1, -0.1, 0.1);
  util.drawGFit(hPullPos->getTH1(), -5, 5, -5, 5);
  util.drawGFit(hPullAngle->getTH1(), -5, 5, -5, 5);
}

// declare this as a framework plugin
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(DT2DSegmentClients);
