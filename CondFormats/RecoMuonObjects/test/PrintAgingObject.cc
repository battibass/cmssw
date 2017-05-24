// -*- C++ -*-
//
// Class:      PrintAgingObject
// 
//
// Original Author:  Sunil Bansal
//         Created:  Wed, 29 Jun 2016 16:27:31 GMT
//
//

// system include files
#include <string>
#include <type_traits>

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
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"

#include "CondFormats/RecoMuonObjects/interface/MuonSystemAging.h"
#include "CondFormats/DataRecord/interface/MuonSystemAgingRcd.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

//
// Class declaration
//

class PrintAgingObject : public edm::one::EDAnalyzer<edm::one::WatchRuns>  
{

public:
  explicit PrintAgingObject(const edm::ParameterSet&);
  ~PrintAgingObject();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


private:

  virtual void beginJob() override { };
  virtual void beginRun(const edm::Run&, const edm::EventSetup&) override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endRun(const edm::Run&, const edm::EventSetup&) override { };
  virtual void endJob() override { };

  template<class T> 
  void printAgingMap(const std::map<uint32_t,T> & map, 
		     const std::string & type) const;

  template<class T> 
  std::string getPayload(T const & obj) const { return "" ; };

  // -- member data --

};


//
// Constructors and destructor
//

PrintAgingObject::PrintAgingObject(const edm::ParameterSet& iConfig)

{  

}


PrintAgingObject::~PrintAgingObject()
{ 

}


//
// Member Functions
//

// -- Called for each event --
void
PrintAgingObject::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

}

// -- Called at the beginning of each run --
void
PrintAgingObject::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{

  edm::ESHandle<DTGeometry> dtGeom;
  iSetup.get<MuonGeometryRecord>().get(dtGeom);

  edm::ESHandle<CSCGeometry> cscGeom;
  iSetup.get<MuonGeometryRecord>().get(cscGeom);

  edm::ESHandle<RPCGeometry> rpcGeom;
  iSetup.get<MuonGeometryRecord>().get(rpcGeom);

  edm::ESHandle<MuonSystemAging> aging;
  iSetup.get<MuonSystemAgingRcd>().get(aging);

  printAgingMap<float>(aging->m_DTChambEffs,"DT");
  printAgingMap<std::pair<unsigned int, float>>(aging->m_CSCChambEffs,"CSC");
  printAgingMap<float>(aging->m_RPCChambEffs,"RPC");

  printAgingMap<float>(aging->m_GEMChambEffs,"GEM");
  printAgingMap<float>(aging->m_ME0ChambEffs,"ME0");
  
}

template<> 
std::string PrintAgingObject::getPayload(const float & obj) const 
{ 

  return std::to_string(obj); 

}

template<> 
std::string PrintAgingObject::getPayload(const std::pair<unsigned int, float> & obj) const 
{ 

  return std::to_string(obj.first) + " , " + std::to_string(obj.second); 

}

template<class T> 
void PrintAgingObject::printAgingMap(const std::map<uint32_t,T> & map, 
				     const std::string & type) const
{
 
  std::cout << "[PrintAgingObject] List of aged " << type 
	    << ( std::is_floating_point<T>::value ? 
		 " objects (ID, efficiency)" :
		 " objects (ID, type, efficiency)" 
	       ) 
	    << std::endl;

  std::cout << "[PrintAgingObject] " << type 
	    << " aging map size : " << map.size()
	    << std::endl;



  for ( const auto & mapObj : map)
    {
      std::string payload = getPayload<T>(mapObj.second);		    
      std::cout << "\t( " << mapObj.first << " , "  <<  payload<< " )" << std::endl;
    }

}




/// -- Fill 'descriptions' with the allowed parameters for the module  --
void
PrintAgingObject::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{

  edm::ParameterSetDescription desc;
  //desc.add<std::vector<std::string> >("dtRegEx",    { } );
  //desc.add<std::vector<std::string> >("rpcRegEx",   { } );
  //desc.add<std::vector<std::string> >("cscRegEx",   { } );
  //desc.add<std::vector<int> >("maskedGEMIDs", { } );
  //desc.add<std::vector<int> >("maskedME0IDs", { } );

  descriptions.addDefault(desc);

}

//define this as a plug-in
DEFINE_FWK_MODULE(PrintAgingObject);
