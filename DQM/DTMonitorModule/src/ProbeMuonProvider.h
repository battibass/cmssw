#ifndef ProbeMuonProvider_H
#define ProbeMuonProvider_H

/*
 * \file ProbeMuonProvider.h
 *
 * \author C. Battilana - INFN BO
 *
*/

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "CommonTools/Utils/interface/AnySelector.h"

#include <string>
#include <vector>
#include <set>

class ProbeMuonCandidate
{

 public:

  /// Constructor
 ProbeMuonCandidate( const reco::Muon * muon, 
		     std::string arbitration, 
		     int minMatchedSt ) : m_muon(muon) , 
                                          m_arbitration(arbitration), 
                                          m_minMatchedSt(minMatchedSt) { };

  /// Destructor
  ~ProbeMuonCandidate() { };

  /// Check matches in stations others in given one
  int nOtherMatchedStations( int target ) const;

  /// List of matched segments in a given chamber 
  /// if nOtherMatchedStations is above a threshold
  std::vector<const reco::MuonSegmentMatch *> matchedSegmentsInCh( int target ) const;
  
  /// List of matched segments for the whole muon chamber matches 
  /// if nOtherMatchedStations is above a threshold
  std::vector<const reco::MuonSegmentMatch *> matchedSegments() const;
  
  // Match in a given chamber by arbitration type
  bool hasChamberMatch( reco::MuonSegmentMatch segmMatch ) const;
  
  /// The reco muon
  const reco::Muon & mu() const { return (*m_muon); };
  
 private:

  
  const reco::Muon * m_muon; // link to the RECO muon
  std::string m_arbitration; // selector of arbitration type
  int m_minMatchedSt; // minimal number of matches to identify a good probe

};

class ProbeMuonProvider
{
  
 public:
  
  /// Constructor
  ProbeMuonProvider( const edm::ParameterSet& ps );
  
  /// Destructor
  ~ProbeMuonProvider() { };
  
  /// Return a vector of muons passing quality / kinematics cuts
  std::vector<ProbeMuonCandidate> filteredMuons( reco::MuonCollection muons,
						 reco::VertexCollection pvs ) const;
  
  /// Return a vector of segments from muons passing quality / kinematics cuts
  std::set<std::size_t> segmentCandidatesIdx( reco::MuonCollection muons,
					      reco::VertexCollection pvs ) const;

 private:
  
  /// String selector cut
  StringCutObjectSelector<reco::Candidate,true> m_selector; 
  
  /// Vertex
  double m_dxy;
  double m_dz;
  
  std::string m_arbitration; // selector of arbitration type
  int m_minMatchedSt; // minimal number of matches to identify a good probe
  
};

#endif
