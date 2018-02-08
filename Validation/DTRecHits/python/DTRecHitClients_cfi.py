import FWCore.ParameterSet.Config as cms

dtrechitclients = cms.EDAnalyzer("DTRecHitClients",
    doStep2 = cms.untracked.bool(False),
    # Switches for analysis at various steps
    doStep1 = cms.untracked.bool(False),
    # Lable to retrieve RecHits from the event
    doStep3 = cms.untracked.bool(True),
    doall   = cms.untracked.bool(False),
    local   = cms.untracked.bool(False)
)

dt2dsegmentclients = cms.EDAnalyzer("DT2DSegmentClients",
    doSLPhi = cms.untracked.bool(False)
)
dt4dsegmentclients = cms.EDAnalyzer("DT4DSegmentClients",
    local = cms.untracked.bool(False)
)
                               
##dtLocalRecoValidationClients = cms.Sequence(dt2dsegmentclients*dt4dsegmentclients)
dtLocalRecoValidationClients = cms.Sequence(dtrechitclients*dt2dsegmentclients*dt4dsegmentclients)


