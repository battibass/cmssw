import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
dtTriggerBaseMonitor = DQMEDAnalyzer('DTLocalTriggerBaseTask',
    testPulseMode = cms.untracked.bool(False),
    detailedAnalysis = cms.untracked.bool(False),
    targetBXTM = cms.untracked.int32(0), 
    bestTrigAccRange = cms.untracked.int32(3),
    processTM = cms.untracked.bool(True),
    processAB7 = cms.untracked.bool(False),
    nTimeBins = cms.untracked.int32(100),
    nLSTimeBin = cms.untracked.int32(15),    
    ResetCycle = cms.untracked.int32(9999),
    inputTagTM = cms.untracked.InputTag('twinMuxStage2Digis:PhIn'),
    inputTagTMphIn = cms.untracked.InputTag('twinMuxStage2Digis:PhIn'),
    inputTagTMphOut = cms.untracked.InputTag('twinMuxStage2Digis:PhOut'),
    inputTagTMth = cms.untracked.InputTag('twinMuxStage2Digis:ThIn'),
    inputTagAB7 = cms.untracked.InputTag('dtAB7Unpacker'),
    minBXTM = cms.untracked.int32(-2), 
    maxBXTM = cms.untracked.int32(2),
    minBXAB7 = cms.untracked.int32(3355), 
    maxBXAB7 = cms.untracked.int32(3380)
)


