#ifndef DTObjSummaryTask_H
#define DTObjSummaryTask_H

/** \class DTObjSummaryTask
 *
 * Description:
 *
 *
 * \author : C. Battilana
 * $date   : 01/08/2018 10:31:01 CET $
 *
 * Modification:
 *
 */

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"

#include "DataFormats/Common/interface/Handle.h"

#include "DataFormats/DTDigi/interface/DTDigiCollection.h"
#include "DataFormats/DTRecHit/interface/DTRecSegment4DCollection.h"

#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"

#include <vector>
#include <string>

class DQMStore;
class MonitorElement;

class DTObjSummaryTask : public DQMEDAnalyzer
{

  public:
    //Constructor
    DTObjSummaryTask(const edm::ParameterSet& pset) ;

    //Destructor
    ~DTObjSummaryTask() override ;

    //BookHistograms
    void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;

    //Operations
    void analyze(const edm::Event & event, const edm::EventSetup& eventSetup) override;
    void dqmBeginRun(const edm::Run& , const edm::EventSetup&) override;

  private:

    void bookChamberHistos(DQMStore::IBooker &,const DTChamberId& dtCh, std::string histoType, int , float , float);

    edm::EDGetTokenT<DTDigiCollection> dtDigisToken_;
    edm::EDGetTokenT<DTRecSegment4DCollection> dt4DSegmentsToken_;

    std::map<std::string, MonitorElement*> histos;

  protected:


};

#endif
