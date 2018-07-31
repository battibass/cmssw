/*
 * \file DTTriggerEfficiencyTask.cc
 *
 * \author C.Battilana - INFN BO
 *
 */

#include "DQM/DTMonitorModule/src/ProbeMuonProvider.h"
#include <bitset>

using namespace std;

int ProbeMuonCandidate::nOtherMatchedStations( int targetSt ) const 
{

  std::bitset<4> nOtherMatchedSt("0000");
  
  for( const auto & chambMatch : m_muon->matches() ) 
    {

      int matchSt = chambMatch.station();
      
      for( const auto & segmMatch : chambMatch.segmentMatches ) 
	{
	  if ( !hasChamberMatch(segmMatch) )
	    continue;
	  
	  if (matchSt != targetSt) 
	    nOtherMatchedSt.set(matchSt - 1); 
	}
  }

  return nOtherMatchedSt.count();

}

std::vector<const reco::MuonSegmentMatch *> ProbeMuonCandidate::matchedSegments() const 
{

  std::vector<const reco::MuonSegmentMatch *> matchedSegments;
  
  for( int iCh = 1 ; iCh < 5; ++ iCh ) 
    {
      for( const auto segmMatch : matchedSegmentsInCh(iCh) ) 
	{
	  matchedSegments.push_back(segmMatch);
	}
    }

  return matchedSegments;

}

std::vector<const reco::MuonSegmentMatch *> ProbeMuonCandidate::matchedSegmentsInCh( int targetSt ) const 
{

  std::vector<const reco::MuonSegmentMatch *> matchedSegments;

  if ( nOtherMatchedStations(targetSt) < m_minMatchedSt)
    return matchedSegments;
  
  for( const auto & chambMatch : m_muon->matches() ) 
    {

      if( chambMatch.detector() != MuonSubdetId::DT ) {continue;}

      int matchSt = chambMatch.station();
    
      for( const auto & segmMatch : chambMatch.segmentMatches ) 
	{
	  if ( !hasChamberMatch(segmMatch) ||
	       matchSt != targetSt )
	    continue;
	  
	  matchedSegments.push_back(&segmMatch);
	}
    }
  
  return matchedSegments;

}

bool ProbeMuonCandidate::hasChamberMatch( reco::MuonSegmentMatch segmMatch ) const 
{
  
  if( m_arbitration == "SegmentArbitration"
      && !(segmMatch.isMask(reco::MuonSegmentMatch::BestInChamberByDR)) ) 
    return false;
  
  if( m_arbitration == "SegmentAndTrackArbitration"
      && (!(segmMatch.isMask(reco::MuonSegmentMatch::BestInChamberByDR)) ||
	  !(segmMatch.isMask(reco::MuonSegmentMatch::BelongsToTrackByDR))) ) 
    return false;

  if( m_arbitration == "SegmentAndTrackArbitrationCleaned"
      && (!(segmMatch.isMask(reco::MuonSegmentMatch::BestInChamberByDR))  ||
	  !(segmMatch.isMask(reco::MuonSegmentMatch::BelongsToTrackByDR)) ||
	  !(segmMatch.isMask(reco::MuonSegmentMatch::BelongsToTrackByCleaning))) ) 
    return false;

  return true;

}



ProbeMuonProvider::ProbeMuonProvider( const edm::ParameterSet& ps ) :
  m_selector(ps.getUntrackedParameter<std::string>("selection")),
  m_dxy(ps.getUntrackedParameter<double>("maxDxy")),
  m_dz(ps.getUntrackedParameter<double>("maxDz")),
  m_arbitration(ps.getUntrackedParameter<std::string>("arbitrationType")),
  m_minMatchedSt(ps.getUntrackedParameter<int>("minMatchedSt")) 
{ 

}

std::vector<ProbeMuonCandidate> ProbeMuonProvider::filteredMuons( reco::MuonCollection muons,
								  reco::VertexCollection pvs ) const 
{

  std::vector<ProbeMuonCandidate> probeMuons;
 
  reco::Vertex firstPV;
  bool useFirstPV = false;

  for ( const auto & pv : pvs )
    {
      double pvChi2    = pv.normalizedChi2();
      double pvNdof    = pv.ndof();
      bool   fakeVtx   = pv.isFake();
      double pvZ  = pv.z();
      
      if (  !fakeVtx
	    && pvNdof   > 4
	    && pvChi2   < 999
	    && std::abs(pvZ)< 24. )
	{
	  firstPV = pv;
	  useFirstPV = true;
	}
    }

  for ( const auto & muon : muons )
    {
      if ( m_selector(muon)               &&
	   ( !useFirstPV ||
	     ( std::abs(muon.muonBestTrack()->dxy(firstPV.position())) < m_dxy &&
	       std::abs(muon.muonBestTrack()->dz(firstPV.position()))  < m_dz ) ) )
	{
	  probeMuons.push_back(ProbeMuonCandidate(&muon,m_arbitration,m_minMatchedSt));
	  
	}
    }
  
  return probeMuons;
  
}

std::set<std::size_t> ProbeMuonProvider::segmentCandidatesIdx( reco::MuonCollection muons,
							       reco::VertexCollection pvs ) const 
{
  
  const auto probeMuons = filteredMuons(muons,pvs);
  std::set<std::size_t> segmentCandidatesIdx;
  
  for ( const auto & probeMuon : probeMuons )
    {
      for (const auto & segment : probeMuon.matchedSegments())
	{
	  segmentCandidatesIdx.insert(segment->dtSegmentRef.key());
	}
    }
  
  return segmentCandidatesIdx;
  
}

