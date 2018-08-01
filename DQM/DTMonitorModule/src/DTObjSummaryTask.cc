/******* \class DTObjSummaryTask *******
 *
 * Description:
 *
 *  detailed description
 *
 * \author : C. Battilana
 * $date   : 01/08/2018 10:30:01 CET $
 *
 * Modification:
 *
 *********************************/

#include "DTObjSummaryTask.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "DataFormats/MuonDetId/interface/MuonSubdetId.h"
#include "CondFormats/DataRecord/interface/DTMtimeRcd.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "RecoMuon/Navigation/interface/DirectMuonNavigation.h"

#include "DataFormats/DetId/interface/DetId.h"
#include "TrackingTools/DetLayers/interface/DetLayer.h"
#include "DataFormats/Common/interface/RefToBase.h"

#include <TMath.h>
#include <cmath>

using namespace std;
using namespace edm;

DTObjSummaryTask::DTObjSummaryTask(const ParameterSet& pSet):
  // Get the debug parameter for verbose output
  dtDigisToken_(consumes<DTDigiCollection>(pSet.getParameter<InputTag>("digiTag"))),
  dt4DSegmentsToken_(consumes<DTRecSegment4DCollection>(pSet.getParameter<InputTag>("segmentTag")))
{

  LogTrace("DTDQM|DTMonitorModule|DTObjSummaryTask")
    << "DTObjSummaryTask: constructor called";

}

DTObjSummaryTask::~DTObjSummaryTask()
{

  LogTrace("DTDQM|DTMonitorModule|DTObjSummaryTask")
    << "DTObjSummaryTask: destructor called";

}

void DTObjSummaryTask::bookHistograms(DQMStore::IBooker & ibooker,
                                             edm::Run const & iRun,
                                             edm::EventSetup const & /* iSetup */) {

  LogTrace("DTDQM|DTMonitorModule|DTObjSummaryTask")
    << "DTObjSummaryTask: bookHistograms";

  string baseFolder = "DT/01-Summary/";

  ibooker.setCurrentFolder(baseFolder);

   histos["hNDigis"] = ibooker.book1D("hNDigis","Number of digis / event",100,-0.5,499.5);

   histos["hNSegments"] = ibooker.book1D("hNSegments","Number of segments / event",50,-0.5,49.5);
   histos["hSegNHits"]  = ibooker.book1D("hSegNHits","Number of hits / segment",15,-0.5,14.5);
   histos["hSegT0"]     = ibooker.book1D("hSegT0","Segment t0",250,-250., 250.);

}

void DTObjSummaryTask::dqmBeginRun(const Run& run, const EventSetup& setup)
{

}

void DTObjSummaryTask::analyze(const Event & event,
    const EventSetup& eventSetup)
{

  LogTrace("DTDQM|DTMonitorModule|DTObjSummaryTask") <<
    "--- [DTObjSummaryTask] Event analysed #Run: " <<
    event.id().run() << " #Event: " << event.id().event() << endl;

  // Digi collection
  edm::Handle<DTDigiCollection> digis;
  event.getByToken(dtDigisToken_, digis);

  int nDigis = 0;
  std::for_each(digis->begin(),digis->end(),[&](auto i) { ++nDigis; });

  histos["hNDigis"]->Fill(std::min(499,nDigis));

  // Get the segment collection from the event
  Handle<DTRecSegment4DCollection> segments;
  event.getByToken(dt4DSegmentsToken_, segments);

  // Loop over the segments

  int nSegments = 0;

  DTRecSegment4DCollection::const_iterator segIt   = segments->begin();
  DTRecSegment4DCollection::const_iterator segEnd  = segments->end();

  for(; segIt != segEnd; ++segIt)
    {

      ++nSegments;
      
      if( segIt->hasPhi() )
	{
	  
	  int nHits = segIt->phiSegment()->degreesOfFreedom() + 2; // CB Fix
	  histos["hSegNHits"]->Fill(nHits);
	  
	  if( segIt->phiSegment()->ist0Valid() )
	    {
	      double t0 = segIt->phiSegment()->t0();
	      histos["hSegT0"]->Fill(std::min(249.5,(std::max(-249.5,t0))));
	    }
	}

    } //end loop on segment

  histos["hNSegments"]->Fill(std::min(49,nSegments));

} //end analyze
