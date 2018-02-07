#ifndef Validation_DTRecHits_DTRecHitClients_h
#define Validation_DTRecHits_DTRecHitClients_h

/** \class DTDataIntegrityTest
 * *
 *  DQM Client to check the data integrity
 *
 *  \author S. Bolognesi - INFN TO
 *   
 */

#include "DQMServices/Core/interface/DQMEDHarvester.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class DTRecHitClients: public DQMEDHarvester {

public:
  /// Constructor
  DTRecHitClients(const edm::ParameterSet& ps);

  /// Destructor
  ~ DTRecHitClients();

protected:
  /// End Job
  void dqmEndJob(DQMStore::IBooker &, DQMStore::IGetter &) override;
};

#endif // Validation_DTRecHits_DTRecHitClients_h
