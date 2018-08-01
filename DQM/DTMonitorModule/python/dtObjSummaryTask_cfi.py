import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
dtObjSummaryTask = DQMEDAnalyzer('DTObjSummaryTask',
    digiTag    = cms.InputTag("muonDTDigis"),                                 
    segmentTag = cms.InputTag("dt4DSegments"),                                 
)

