import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
dtTnPEfficiencyMonitor = DQMEDAnalyzer('DTTnPEfficiencyTask',
                                       # The muon object input tag
                                       inputTagMuons = cms.untracked.InputTag('muons'),
                                       inputTagPrimaryVertices = cms.untracked.InputTag('offlinePrimaryVertices'),
                                       trigResultsTag = cms.untracked.InputTag("TriggerResults"),
                                       trigEventTag = cms.untracked.InputTag("TriggerEvent"),
                                       # A string-based cut on muon variables
                                       probeCut = cms.untracked.string('isTrackerMuon && (innerTrack.normalizedChi2 < 10) && (innerTrack.hitPattern.numberOfValidPixelHits > 0) && (innerTrack.hitPattern.trackerLayersWithMeasurement) && ((isolationR03.sumPt)/(pt) < 0.1)'),
                                       probeDxyCut = cms.untracked.double(0.2),
                                       probeDzCut = cms.untracked.double(0.5),
                                       #Cut on muon ID:
				       #  CutBasedIdLoose = 1UL << 0
                                       #  CutBasedIdMedium = 1UL << 1
                                       #  CutBasedIdMediumPrompt = 1UL << 2 
				       #  CutBasedIdTight = 1UL << 3
                                       tagCut = cms.untracked.string('(selectors & 8) && ((isolationR03.sumPt)/(pt) < 0.05)'),
                                       trigName = cms.untracked.string("HLT_Mu50_v*"),
                                       isoTrigName = cms.untracked.string("HLT_IsoMu24_v*"),
                                       # If true, enables detailed analysis plots
                                       detailedAnalysis = cms.untracked.bool(True)
)
