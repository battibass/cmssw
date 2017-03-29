// -*- C++ -*-
//
// Class:      ProduceAgingObject
// 
//
// Original Author:  Sunil Bansal
//         Created:  Wed, 29 Jun 2016 16:27:31 GMT
//
//

// system include files
#include <memory>
#include <regex>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/IOVSyncValue.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"

#include "CondFormats/MuonSystemAging/interface/MuonSystemAging.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

//
// Class declaration
//

class ProduceAgingObject : public edm::one::EDAnalyzer<edm::one::WatchRuns>  
{

public:
  explicit ProduceAgingObject(const edm::ParameterSet&);
  ~ProduceAgingObject();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


private:

  virtual void beginJob() override { };
  virtual void beginRun(const edm::Run&, const edm::EventSetup&) override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endRun(const edm::Run&, const edm::EventSetup&) override { };
  virtual void endJob() override { };

  void createRpcAgingMap();
  void createDtAgingMap(edm::ESHandle<DTGeometry> & dtGeom);
  void createCscAgingMap(edm::ESHandle<CSCGeometry> & cscGeom);


  // -- member data --

  std::vector<std::string> m_RPCRegEx;
  std::map<uint32_t, float> m_RPCChambEffs;

  std::vector<std::string> m_DTRegEx;
  std::map<uint32_t, float> m_DTChambEffs;

  std::vector<std::string> m_CSCRegEx;
  std::map<uint32_t, std::pair<uint32_t, float>> m_CSCChambEffs;

  std::vector<int> m_maskedGE11PlusIDs;
  std::vector<int> m_maskedGE11MinusIDs;
  std::vector<int> m_maskedGE21PlusIDs;
  std::vector<int> m_maskedGE21MinusIDs;
  std::vector<int> m_maskedME0PlusIDs;
  std::vector<int> m_maskedME0MinusIDs;

};


//
// Constructors and destructor
//

ProduceAgingObject::ProduceAgingObject(const edm::ParameterSet& iConfig)

{  

  m_DTRegEx  = iConfig.getParameter<std::vector<std::string>>("dtRegEx"); 
  m_RPCRegEx = iConfig.getParameter<std::vector<std::string>>("rpcRegEx");   
  m_CSCRegEx = iConfig.getParameter<std::vector<std::string>>("cscRegEx");   

  for ( auto ge11plus_ids : iConfig.getParameter<std::vector<int>>("maskedGE11PlusIDs"))
    {
      m_maskedGE11PlusIDs.push_back(ge11plus_ids);
    }
  
  for ( auto ge11minus_ids : iConfig.getParameter<std::vector<int>>("maskedGE11MinusIDs"))
    {
      m_maskedGE11MinusIDs.push_back(ge11minus_ids);
    }
  
  for ( auto ge21plus_ids : iConfig.getParameter<std::vector<int>>("maskedGE21PlusIDs"))
    {
      m_maskedGE21PlusIDs.push_back(ge21plus_ids);
    }
  
  for ( auto ge21minus_ids : iConfig.getParameter<std::vector<int>>("maskedGE21MinusIDs"))
    {
      m_maskedGE21MinusIDs.push_back(ge21minus_ids);
    }
  
  for ( auto me0plus_ids : iConfig.getParameter<std::vector<int>>("maskedME0PlusIDs"))
    {
      m_maskedME0PlusIDs.push_back(me0plus_ids);
    }
  
  for ( auto me0minus_ids : iConfig.getParameter<std::vector<int>>("maskedME0MinusIDs"))
    {
      m_maskedME0MinusIDs.push_back(me0minus_ids);
    }
  
}


ProduceAgingObject::~ProduceAgingObject()
{ 

}


//
// Member Functions
//

// -- Called for each event --
void
ProduceAgingObject::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  MuonSystemAging* muonAgingObject = new MuonSystemAging();
  
  muonAgingObject->m_DTChambEffs  = m_DTChambEffs;
  muonAgingObject->m_RPCChambEffs = m_RPCChambEffs;
  muonAgingObject->m_CSCChambEffs = m_CSCChambEffs;

  for(unsigned int i = 0; i < m_maskedGE11PlusIDs.size();++i){
    muonAgingObject->m_GE11Pluschambers.push_back(m_maskedGE11PlusIDs.at(i));
  }
  
  for(unsigned int i = 0; i < m_maskedGE11MinusIDs.size();++i){
    muonAgingObject->m_GE11Minuschambers.push_back(m_maskedGE11MinusIDs.at(i));
  }
  
  for(unsigned int i = 0; i < m_maskedGE21PlusIDs.size();++i){
    muonAgingObject->m_GE21Pluschambers.push_back(m_maskedGE21PlusIDs.at(i));
  }
  
  for(unsigned int i = 0; i < m_maskedGE21MinusIDs.size();++i){
    muonAgingObject->m_GE21Minuschambers.push_back(m_maskedGE21MinusIDs.at(i));
  }
  
  for(unsigned int i = 0; i < m_maskedME0PlusIDs.size();++i){
    muonAgingObject->m_ME0Pluschambers.push_back(m_maskedME0PlusIDs.at(i));
  }
  
  for(unsigned int i = 0; i < m_maskedME0MinusIDs.size();++i){
    muonAgingObject->m_ME0Minuschambers.push_back(m_maskedME0MinusIDs.at(i));
  }
  
  edm::Service<cond::service::PoolDBOutputService> poolDbService;
  if( poolDbService.isAvailable() ) 
    poolDbService->writeOne( muonAgingObject, 
			     poolDbService->currentTime(),
			     "MuonSystemAgingRcd" );
  
}

// -- Called at the beginning of each run --
void
ProduceAgingObject::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{

  edm::ESHandle<DTGeometry> dtGeom;
  iSetup.get<MuonGeometryRecord>().get(dtGeom);

  edm::ESHandle<CSCGeometry> cscGeom;
  iSetup.get<MuonGeometryRecord>().get(cscGeom);

  createDtAgingMap(dtGeom);
  createCscAgingMap(cscGeom);
  createRpcAgingMap();
  
}

/// -- Create RPC aging map --
void
ProduceAgingObject::createRpcAgingMap()
{
 
  std::cout << "[ProduceAgingObject] List of aged RPC objects (ID, efficiency)" 
	    << std::endl;
  for (auto & chRegExStr : m_RPCRegEx )
    {

      std::string id = chRegExStr.substr(0, chRegExStr.find(":"));
      std::string eff = chRegExStr.substr(id.size()+1, chRegExStr.find(":"));

      std::cout << "\t(" << id << "," << eff << ")" << std::endl;
      m_RPCChambEffs[std::atoi(id.c_str())] = std::atof(eff.c_str());
      
    }
  
}

/// -- Create DT aging map ------------
void
ProduceAgingObject::createDtAgingMap(edm::ESHandle<DTGeometry> & dtGeom)
{

  const std::vector<const DTChamber*> chambers = dtGeom->chambers();

  std::cout << "[ProduceAgingObject] List of aged DT chambers (ChamberID, efficiency)" 
	    << std::endl;
  for ( const DTChamber *ch : chambers)
   {

     DTChamberId chId = ch->id();

     std::string chTag = "WH" + std::to_string(chId.wheel())
                       + "_ST" + std::to_string(chId.station())
                       + "_SEC" + std::to_string(chId.sector());

     float eff = 1.;

     for (auto & chRegExStr : m_DTRegEx)
       {

	 std::string effTag(chRegExStr.substr(chRegExStr.find(":")));

	 const std::regex chRegEx(chRegExStr.substr(0,chRegExStr.find(":")));
	 const std::regex effRegEx("(\\d*\\.\\d*)");

	 std::smatch effMatch;

	 if ( std::regex_search(chTag, chRegEx) &&
	      std::regex_search(effTag, effMatch, effRegEx))
	   {
	     std::string effStr = effMatch.str();
	     eff = std::atof(effStr.c_str());
	   }

       } 

     if (eff < 1.)
       {
	 std::cout << "\t(" << chId << "," << eff << ")" << std::endl;
	 m_DTChambEffs[chId.rawId()] = eff;
       }
         
   }
  
}

/// -- Create CSC aging map ------------
void
ProduceAgingObject::createCscAgingMap(edm::ESHandle<CSCGeometry> & cscGeom)
{

  const auto chambers = cscGeom->chambers();

  std::cout << "[ProduceAgingObject] List of aged CSC chambers (ChamberID, efficiency, type)" 
	    << std::endl;

  for ( const auto *ch : chambers) {
    
    CSCDetId chId = ch->id();
    
    
    std::string chTag = (chId.zendcap() == 1 ? "ME+" : "ME-")
      + std::to_string(chId.station())
      + "/" + std::to_string(chId.ring())
      + "/" + std::to_string(chId.chamber());
    
    int type = 0;
    float eff = 1.;

    for (auto & chRegExStr : m_CSCRegEx) {
      
      int loc = chRegExStr.find(":");
      // if there's no :, then we don't have to correct format
      if (loc < 0) continue;

      std::string effTag(chRegExStr.substr(loc));

      const std::regex chRegEx(chRegExStr.substr(0,chRegExStr.find(":")));
      const std::regex predicateRegEx("(\\d*,\\d*\\.\\d*)");

      std::smatch predicate;

      if ( std::regex_search(chTag, chRegEx) && std::regex_search(effTag, predicate, predicateRegEx)) {
	std::string predicateStr = predicate.str();
	std::string typeStr = predicateStr.substr(0,predicateStr.find(","));
	std::string effStr = predicateStr.substr(predicateStr.find(",")+1);
	type = std::atoi(typeStr.c_str());
	eff = std::atof(effStr.c_str());
	
	std::cout << "\t(" << chTag << "," << eff << "," << type << ")" << std::endl;
      }

    } 

    // Note, layer 0 for chamber specification
    int rawId = chId.rawIdMaker(chId.endcap(), chId.station(), chId.ring(), chId.chamber(), 0);
    m_CSCChambEffs[rawId] = std::make_pair(type, eff);
    
  }

}



/// -- Fill 'descriptions' with the allowed parameters for the module  --
void
ProduceAgingObject::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{

  edm::ParameterSetDescription desc;
  desc.add<std::vector<std::string> >("dtRegEx",    { } );
  desc.add<std::vector<std::string> >("rpcRegEx",   { } );
  desc.add<std::vector<std::string> >("cscRegEx",   { } );
  desc.add<std::vector<int> >("maskedGE11PlusIDs",  { } );
  desc.add<std::vector<int> >("maskedGE11MinusIDs", { } );
  desc.add<std::vector<int> >("maskedGE21PlusIDs",  { } );
  desc.add<std::vector<int> >("maskedGE21MinusIDs", { } );
  desc.add<std::vector<int> >("maskedME0PlusIDs",   { } );
  desc.add<std::vector<int> >("maskedME0MinusIDs",  { } );

  descriptions.addDefault(desc);

}

//define this as a plug-in
DEFINE_FWK_MODULE(ProduceAgingObject);
