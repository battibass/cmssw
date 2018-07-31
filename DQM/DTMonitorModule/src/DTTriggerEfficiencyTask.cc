/*
 * \file DTTriggerEfficiencyTask.cc
 *
 * \author C.Battilana - CIEMAT
 *
 */

#include "DQM/DTMonitorModule/src/DTTriggerEfficiencyTask.h"

// Framework
#include "FWCore/Framework/interface/EventSetup.h"

// DT trigger
#include "DQM/DTMonitorModule/interface/DTTrigGeomUtils.h"

// Geometry
#include "DataFormats/GeometryVector/interface/Pi.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/DTGeometry/interface/DTLayer.h"
#include "Geometry/DTGeometry/interface/DTSuperLayer.h"
#include "Geometry/DTGeometry/interface/DTTopology.h"

// DT Digi
#include "DataFormats/DTDigi/interface/DTDigi.h"
#include "DataFormats/DTDigi/interface/DTDigiCollection.h"

// Muons and Vertexes tracks
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

//Root
#include"TH1.h"
#include"TAxis.h"

#include <sstream>
#include <iostream>
#include <fstream>


using namespace edm;
using namespace std;

DTTriggerEfficiencyTask::DTTriggerEfficiencyTask(const edm::ParameterSet& ps) : 
  trigGeomUtils(nullptr),
  probeMuonProvider(ProbeMuonProvider(ps.getUntrackedParameter<ParameterSet>("probeSelection")))
{
  
  LogTrace ("DTDQM|DTMonitorModule|DTTriggerEfficiencyTask")  
    << "[DTTriggerEfficiencyTask]: Constructor" << endl;

  muons_Token_ = consumes<reco::MuonCollection>(
      ps.getUntrackedParameter<edm::InputTag>("inputTagMuons"));
  pvs_Token_ = consumes<reco::VertexCollection>(
      ps.getUntrackedParameter<edm::InputTag>("inputTagPVs"));
  seg_Token_ = consumes<DTRecSegment4DCollection>(
      ps.getUntrackedParameter<edm::InputTag>("inputTagSegments"));

  detailedPlots = ps.getUntrackedParameter<bool>("detailedAnalysis");
  processTM = ps.getUntrackedParameter<bool>("processTM");
  processDDU = ps.getUntrackedParameter<bool>("processDDU");

  checkRPCtriggers = ps.getUntrackedParameter<bool>("checkRPCtriggers");
  nMinHitsPhi  = ps.getUntrackedParameter<int>("nMinHitsPhi");
  phiAccRange  = ps.getUntrackedParameter<double>("phiAccRange");
  correctBXDDU = ps.getUntrackedParameter<int>("correctBXDDU");

  if (processTM) 
    {
      processTags.push_back("TM");
      tm_Token_ = consumes<L1MuDTChambPhContainer>(
           ps.getUntrackedParameter<edm::InputTag>("inputTagTM"));
    }

  if (processDDU) 
    {
      processTags.push_back("DDU");
      ddu_Token_   = consumes<DTLocalTriggerCollection>(
	   ps.getUntrackedParameter<edm::InputTag>("inputTagDDU"));
    }

  if (checkRPCtriggers) 
    {
      gmt_Token_   = consumes<L1MuGMTReadoutCollection>(
           ps.getUntrackedParameter<edm::InputTag>("inputTagGMT"));
    }

  if (!processTM && !processDDU) 
    LogError ("DTDQM|DTMonitorModule|DTTriggerEfficiencyTask")  
      << "[DTTriggerEfficiencyTask]: Error, no trigger source (DDU or TM) has been selected!!" 
      << endl;

}


DTTriggerEfficiencyTask::~DTTriggerEfficiencyTask() 
{

  LogTrace ("DTDQM|DTMonitorModule|DTTriggerEfficiencyTask")  
    << "[DTTriggerEfficiencyTask]: analyzed " << nEvents << " events" << endl;

}

void DTTriggerEfficiencyTask::dqmBeginRun(const edm::Run& run, const edm::EventSetup& context) 
{
  
  // Get the geometry
  context.get<MuonGeometryRecord>().get(muonGeom);
  trigGeomUtils = new DTTrigGeomUtils(muonGeom);

}

void DTTriggerEfficiencyTask::bookHistograms(DQMStore::IBooker & ibooker,
                                             edm::Run const & run,
                                             edm::EventSetup const & context) 
{

  LogTrace ("DTDQM|DTMonitorModule|DTTriggerEfficiencyTask") 
    << "[DTTriggerEfficiencyTask]: bookHistograms" << endl;

  nEvents = 0;
  for (int wh=-2;wh<=2;++wh){
    vector<string>::const_iterator tagIt  = processTags.begin();
    vector<string>::const_iterator tagEnd = processTags.end();
    for (; tagIt!=tagEnd; ++tagIt) {

      bookWheelHistos(ibooker,wh,(*tagIt),"Task");
      if (detailedPlots) {
        for (int stat=1;stat<=4;++stat){
          for (int sect=1;sect<=12;++sect){
            bookChamberHistos(ibooker,DTChamberId(wh,stat,sect),(*tagIt),"Segment");
          }
        }
      }
    }
  }

}


void DTTriggerEfficiencyTask::analyze(const edm::Event& e, const edm::EventSetup& c)
{

  nEvents++;

  if (checkRPCtriggers)
    { //For pre-2016 Era compatibility
      if (!hasRPCTriggers(e)) { return; }
    }

  map<DTChamberId,const L1MuDTChambPhDigi*> phBestTM;
  map<DTChamberId,const DTLocalTrigger*>    phBestDDU;

  // Getting best TM Stuff
  if (processTM)
    {
      edm::Handle<L1MuDTChambPhContainer> l1DTTPGPh;
      e.getByToken(tm_Token_, l1DTTPGPh);
      vector<L1MuDTChambPhDigi> const*  phTrigs = l1DTTPGPh->getContainer();
      
      for(const auto & iPh : (*phTrigs) ) 
	{
	  
	  int phWheel = iPh.whNum();
	  int phSec   = iPh.scNum() + 1; // DTTF numbering [0:11] -> DT numbering [1:12]
	  int phSt    = iPh.stNum();
	  int phBX    = iPh.bxNum();
	  int phCode  = iPh.code();
	  
	  DTChamberId chId(phWheel,phSt,phSec);
	  
	  if( phBX == 0  &&
	      phCode < 7 && 
	      ( phBestTM.find(chId) == phBestTM.end() ||
		phCode>phBestTM[chId]->code()
		) 
	      ) 
	    phBestTM[chId] = &iPh;
	}
    }

  //Getting Best DDU Stuff
  if (processDDU)
    {
      Handle<DTLocalTriggerCollection> trigsDDU;
      e.getByToken(ddu_Token_, trigsDDU);
      DTLocalTriggerCollection::DigiRangeIterator detUnitIt;

      // CB for (detUnitIt=trigsDDU->begin();detUnitIt!=trigsDDU->end();++detUnitIt){
      for (const auto & detUnit : (*trigsDDU) )
	{
	  
	  const DTChamberId& id = detUnit.first;
	  const DTLocalTriggerCollection::Range& range = detUnit.second;
	  
	  DTLocalTriggerCollection::const_iterator trigIt  = range.first;
	  DTLocalTriggerCollection::const_iterator trigEnd = range.second;
	  
	  for (; trigIt!= trigEnd;++trigIt)
	    {
	      int bx      = trigIt->bx();
	      int quality = trigIt->quality();

	      if( bx == correctBXDDU &&
		  quality>-1 && quality<7 &&
		  ( phBestDDU.find(id) == phBestDDU.end() ||
		    quality>phBestDDU[id]->quality()
		  )  
		) 
		phBestDDU[id] = &(*trigIt);
	    }
	}
    }
  
  //Getting Matched Segments
  vector<const DTRecSegment4D*> matched4DSegments;

  Handle<reco::MuonCollection> muons;
  e.getByToken(muons_Token_, muons);

  // Get the PV collection
  Handle<reco::VertexCollection> pvs;
  e.getByToken(pvs_Token_, pvs);

  // Get a vector of segments matched to muons
  set<size_t> matchedSegmentsIdx;

  matchedSegmentsIdx = probeMuonProvider.segmentCandidatesIdx((*muons),(*pvs));

  // Get the segment collection from the event
  Handle<DTRecSegment4DCollection> segments;
  e.getByToken(seg_Token_, segments);

  DTRecSegment4DCollection::const_iterator segmentIt  = segments->begin();
  DTRecSegment4DCollection::const_iterator segmentEnd = segments->end();

  for(std::size_t iSegment = 0;
      segmentIt != segmentEnd; ++segmentIt, ++ iSegment)
    {
    
      for( const auto & matchedSegIdx : matchedSegmentsIdx )
	{
	  if( matchedSegIdx == iSegment &&
	      segmentIt->hasPhi() && 
	      (segmentIt->chamberId().station() == 4 || segmentIt->hasZed()) )
	    {
	      matched4DSegments.push_back(&(*segmentIt));
	    }
	}

    }

  // Plot filling

  for ( const auto & segment : matched4DSegments )
    {
      int wheel    = segment->chamberId().wheel();
      int station  = segment->chamberId().station();
      
      int trigSector = 0;
      float x, xDir, y, yDir;
      trigGeomUtils->computeSCCoordinates(segment,trigSector,x,xDir,y,yDir);
      
      int nHitsPhi = segment->phiSegment()->degreesOfFreedom()+2;
      
      DTChamberId dtChId(wheel,station,trigSector);
      uint32_t indexCh = dtChId.rawId();
      
      map<string, MonitorElement*> &innerChME = chamberHistos[indexCh];
      map<string, MonitorElement*> &innerWhME = wheelHistos[wheel];
      
      if ( fabs(xDir) < phiAccRange && 
	   nHitsPhi>=nMinHitsPhi )
	{

	  for (const auto & tag : processTags ) 
	    {

	      int qual = tag == "TM" ?
		phBestTM.find(dtChId) != phBestTM.end() ? phBestTM[dtChId]->code() : -1 :
		phBestDDU.find(dtChId) != phBestDDU.end() ? phBestDDU[dtChId]->quality() : -1;
	      
	      innerWhME.find(tag + "_TrigEffDenum")->second->Fill(trigSector,station);
	      
	      if ( qual>=0 && qual<7 ) 
		{
		  innerWhME.find(tag + "_TrigEffNum")->second->Fill(trigSector,station);
		  if ( qual>=4 ) 
		    {
		      innerWhME.find(tag + "_TrigEffCorrNum")->second->Fill(trigSector,station);
		    }
		}
	      
	      if (detailedPlots) 
		{
		  innerChME.find(tag + "_TrackPosvsAngle")->second->Fill(xDir,x);
		  
		  if ( qual>=0 && qual<7 ) 
		    {
		      innerChME.find(tag + "_TrackPosvsAngleAnyQual")->second->Fill(xDir,x);
		      if ( qual>=4 ) 
			{
			  innerChME.find(tag + "_TrackPosvsAngleCorr")->second->Fill(xDir,x);
			}
		    }
		}
	    }
	}
    }
  
}

bool DTTriggerEfficiencyTask::hasRPCTriggers(const edm::Event& e) {

  edm::Handle<L1MuGMTReadoutCollection> gmtrc;
  e.getByToken(gmt_Token_, gmtrc);

  std::vector<L1MuGMTReadoutRecord> gmt_records = gmtrc->getRecords();
  std::vector<L1MuGMTReadoutRecord>::const_iterator igmtrr = gmt_records.begin();
  std::vector<L1MuGMTReadoutRecord>::const_iterator egmtrr = gmt_records.end();
  for(; igmtrr!=egmtrr; igmtrr++) {

    std::vector<L1MuGMTExtendedCand> candsGMT = igmtrr->getGMTCands();
    std::vector<L1MuGMTExtendedCand>::const_iterator candGMTIt   = candsGMT.begin();
    std::vector<L1MuGMTExtendedCand>::const_iterator candGMTEnd  = candsGMT.end();

    for(; candGMTIt!=candGMTEnd; ++candGMTIt){
      if(!candGMTIt->empty()) {
        int quality = candGMTIt->quality();
        if(candGMTIt->bx()==0 &&
            (quality == 5 || quality == 7)){
          return true;
        }
      }
    }
  }

  return false;

}

void DTTriggerEfficiencyTask::bookChamberHistos(DQMStore::IBooker& ibooker,const DTChamberId& dtCh, 
                                                  string histoType, string folder) {

  int wh = dtCh.wheel();
  int sc = dtCh.sector();
  int st = dtCh.station();
  stringstream wheel; wheel << wh;
  stringstream station; station << st;
  stringstream sector; sector << sc;

  string hwFolder      = topFolder(histoType);
  string bookingFolder = hwFolder + "Wheel" + wheel.str() + "/Sector" + sector.str() + "/Station" + station.str() + "/" + folder;
  string histoTag      = "_W" + wheel.str() + "_Sec" + sector.str() + "_St" + station.str();

  ibooker.setCurrentFolder(bookingFolder);

  LogTrace ("DTDQM|DTMonitorModule|DTTriggerEfficiencyTask")
    << "[DTTriggerEfficiencyTask]: booking histos in " << bookingFolder << endl;

  float min, max;
  int nbins;
  trigGeomUtils->phiRange(dtCh,min,max,nbins,20);

  string histoName = histoType + "_TrackPosvsAngle" +  histoTag;
  string histoLabel = "Position vs Angle (phi)";

  (chamberHistos[dtCh.rawId()])[histoType + "_TrackPosvsAngle"] =
    ibooker.book2D(histoName,histoLabel,12,-30.,30.,nbins,min,max);

  histoName = histoType + "_TrackPosvsAngleAnyQual" +  histoTag;
  histoLabel = "Position vs Angle (phi) for any qual triggers";

  (chamberHistos[dtCh.rawId()])[histoType + "_TrackPosvsAngleAnyQual"] =
    ibooker.book2D(histoName,histoLabel,12,-30.,30.,nbins,min,max);

  histoName = histoType + "_TrackPosvsAngleCorr" +  histoTag;
  histoLabel = "Position vs Angle (phi) for correlated triggers";

  (chamberHistos[dtCh.rawId()])[histoType + "_TrackPosvsAngleCorr"] =
    ibooker.book2D(histoName,histoLabel,12,-30.,30.,nbins,min,max);

}

void DTTriggerEfficiencyTask::bookWheelHistos(DQMStore::IBooker& ibooker,int wheel,string hTag,
                                                string folder) {

  stringstream wh; wh << wheel;
  string basedir;
  if (hTag.find("Summary") != string::npos ) {
    basedir = topFolder(hTag);   //Book summary histo outside folder directory
  } else {
    basedir = topFolder(hTag) + folder + "/" ;
  }

  ibooker.setCurrentFolder(basedir);

  string hTagName = "_W" + wh.str();

  LogTrace("DTDQM|DTMonitorModule|DTTriggerEfficiencyTask")
    << "[DTTriggerEfficiencyTask]: booking histos in "<< basedir << endl;

  string hName = hTag + "_TrigEffDenum" + hTagName;

  MonitorElement* me = ibooker.book2D(hName.c_str(),hName.c_str(),12,1,13,4,1,5);

  me->setBinLabel(1,"MB1",2);
  me->setBinLabel(2,"MB2",2);
  me->setBinLabel(3,"MB3",2);
  me->setBinLabel(4,"MB4",2);
  me->setAxisTitle("Sector",1);

  wheelHistos[wheel][hTag + "_TrigEffDenum"] = me;

  hName = hTag + "_TrigEffNum" + hTagName;
  me = ibooker.book2D(hName.c_str(),hName.c_str(),12,1,13,4,1,5);

  me->setBinLabel(1,"MB1",2);
  me->setBinLabel(2,"MB2",2);
  me->setBinLabel(3,"MB3",2);
  me->setBinLabel(4,"MB4",2);
  me->setAxisTitle("Sector",1);

  wheelHistos[wheel][hTag + "_TrigEffNum"] = me;

  hName = hTag + "_TrigEffCorrNum" + hTagName;
  me = ibooker.book2D(hName.c_str(),hName.c_str(),12,1,13,4,1,5);

  me->setBinLabel(1,"MB1",2);
  me->setBinLabel(2,"MB2",2);
  me->setBinLabel(3,"MB3",2);
  me->setBinLabel(4,"MB4",2);
  me->setAxisTitle("Sector",1);

  wheelHistos[wheel][hTag + "_TrigEffCorrNum"] = me;

  return;
}


