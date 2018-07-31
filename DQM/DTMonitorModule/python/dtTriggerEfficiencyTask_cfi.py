import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
dtTriggerEfficiencyMonitor = DQMEDAnalyzer('DTTriggerEfficiencyTask',
    # label for muons
    inputTagMuons = cms.untracked.InputTag('muons'),
    inputTagPVs   = cms.untracked.InputTag('offlinePrimaryVertices'),

    # labels of DDU/TM data and 4D segments
    inputTagTM       = cms.untracked.InputTag('dttfDigis'),
    inputTagDDU      = cms.untracked.InputTag('muonDTDigis'),
    inputTagSegments = cms.untracked.InputTag('dt4DSegments'),
    inputTagGMT      = cms.untracked.InputTag('gtDigis'),

    processDDU = cms.untracked.bool(True),  # Not needed any longer
                                            # Switches below for 2016 Eras and onwards
    processTM = cms.untracked.bool(True), # if true enables TM data analysis

    checkRPCtriggers = cms.untracked.bool(False), # Not needed any longer 
                                                  # Swittches below for 2016 Eras and onwards
    nMinHitsPhi  = cms.untracked.int32(4),
    correctBXDDU = cms.untracked.int32(10),
    phiAccRange  = cms.untracked.double(40.),

    detailedAnalysis = cms.untracked.bool(False), #if true enables detailed analysis plots
    probeSelection = cms.untracked.PSet(
        selection = cms.untracked.string("isTrackerMuon && numberOfMatchedStations >= 1 && " +
                                         "innerTrack.hitPattern.numberOfValidPixelHits >=1 && " +
                                         "innerTrack.hitPattern.trackerLayersWithMeasurement >= 5 && " +
                                         "isolationR03.sumPt / pt < 0.05"), # CB add here
        arbitrationType = cms.untracked.string("SegmentAndTrackArbitrationCleaned"),
        minMatchedSt = cms.untracked.int32(1),
        maxDxy = cms.untracked.double(0.2),
        maxDz  = cms.untracked.double(1.0)
        )                                   
)

#
# Modify for running in run 2 2016 data
#
from Configuration.Eras.Modifier_run2_common_cff import run2_common
run2_common.toModify( dtTriggerEfficiencyMonitor, checkRPCtriggers = cms.untracked.bool(False), 
                                                  processDDU = cms.untracked.bool(False), 
                                                  inputTagTM = cms.untracked.InputTag('bmtfDigis'))


