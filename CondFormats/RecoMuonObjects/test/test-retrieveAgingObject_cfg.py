import FWCore.ParameterSet.Config as cms

process = cms.Process("PRINT")
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

from Configuration.AlCa.autoCond import autoCond
process.GlobalTag.globaltag = autoCond['run2_design']

process.GlobalTag.toGet = cms.VPSet( cms.PSet(
        record = cms.string('MuonSystemAgingRcd'),
        tag = cms.string('MuonSystemAging_Phase2-3000fbm1_v1_mc'),
        connect = cms.string('frontier://FrontierProd/CMS_CONDITIONS')
        ))

process.source = cms.Source("EmptyIOVSource",
    lastValue = cms.uint64(1),
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    interval = cms.uint64(1)
)

process.get = cms.EDAnalyzer("EventSetupRecordDataGetter",
    toGet = cms.VPSet(cms.PSet(
        record = cms.string('MuonSystemAgingRcd'),
        data = cms.vstring('MuonSystemAging')
    )),
    verbose = cms.untracked.bool(True)
)

process.printAgingObject = cms.EDAnalyzer("PrintAgingObject")

process.p = cms.Path(process.get + process.printAgingObject)
