import FWCore.ParameterSet.Config as cms

from DPGAnalysis.MuonSysAging.DTChamberMasker_cfi import DTChamberMasker

def appendDTChamberMaskerAtUnpacking(process):

    if hasattr(process,'muonDTDigis') and hasattr(process,'RawToDigi'):
        
        print "[appendDTChamberMasker] : Found muonDTDigis, applying filter"

        process.preDtDigis = process.muonDTDigis.clone()
        process.muonDTDigis = DTChamberMasker.clone()

        process.muonDTDigis.digiTag = cms.InputTag('preDtDigis') 

        process.filteredDigiSequence = cms.Sequence(process.preDtDigis + process.muonDTDigis)
        process.RawToDigi.replace(process.muonDTDigis, process.filteredDigiSequence)

        if hasattr(process,"RandomNumberGeneratorService") :
            process.RandomNumberGeneratorService.muonDTDigis = cms.PSet(
                initialSeed = cms.untracked.uint32(789342)
                )
        else :
            process.RandomNumberGeneratorService = cms.Service(
                "RandomNumberGeneratorService",
                muonDTDigis = cms.PSet(initialSeed = cms.untracked.uint32(789342))
                )

    return process

def appendDTChamberMaskerAtHLT(process):

    if hasattr(process,'hltMuonDTDigis') and \
       ( hasattr(process,'HLTMuonLocalRecoSequence') or \
         hasattr(process,'HLTMuonLocalRecoMeanTimerSequence')) :

        print "[appendDTChamberMasker] : Found hltMuonDTDigis, applying filter"

        process.preHltDtDigis = process.hltMuonDTDigis.clone()
        process.hltMuonDTDigis = DTChamberMasker.clone()

        process.hltMuonDTDigis.digiTag = "preHltDtDigis"

        process.filteredHltDtDigiSequence = cms.Sequence(process.preHltDtDigis + process.hltMuonDTDigis)
        if hasattr(process,'HLTMuonLocalRecoSequence') :
            print "1"
            process.HLTMuonLocalRecoSequence.replace(process.hltMuonDTDigis, process.filteredHltDtDigiSequence)
        if hasattr(process,'HLTMuonLocalRecoMeanTimerSequence') :
            print "2"
            process.HLTMuonLocalRecoMeanTimerSequence.replace(process.hltMuonDTDigis, process.filteredHltDtDigiSequence)

        if hasattr(process,"RandomNumberGeneratorService") :
            process.RandomNumberGeneratorService.hltMuonDTDigis = cms.PSet(
                initialSeed = cms.untracked.uint32(789342)
                )
        else :
            process.RandomNumberGeneratorService = cms.Service(
                "RandomNumberGeneratorService",
                hltMuonDTDigis = cms.PSet(initialSeed = cms.untracked.uint32(789342))
                )
            
    return process

def appendDTChamberMaskerBeforeL1Trigger(process):

    if hasattr(process,'simDtTriggerPrimitiveDigis') and hasattr(process,'SimL1TMuonCommon') :

        print "[appendDTChamberMasker] : Found simMuonDtTriggerPrimitivesDigis, applying filter"

        process.preSimDtTriggerDigis = DTChamberMasker.clone()

        process.simDtTriggerPrimitiveDigis.digiTag = "preSimDtDigis"

        process.filteredSimDtTriggerSequence = cms.Sequence(process.preSimDtTriggerDigis + process.simDtTriggerPrimitiveDigis)
        process.SimL1TMuonCommon.replace(process.simMuonDTDigis, process.filteredSimDtTriggerSequence)

        if hasattr(process,"RandomNumberGeneratorService") :
            process.RandomNumberGeneratorService.simMuonDTDigis = cms.PSet(
                initialSeed = cms.untracked.uint32(789342)
                )
        else :
            process.RandomNumberGeneratorService = cms.Service(
                "RandomNumberGeneratorService",
                simMuonDTDigis = cms.PSet(initialSeed = cms.untracked.uint32(789342))
                )
            
    return process

# All this must be framed into the stage-2 L1 sequence
# also considering that it won't start from primitives anymore

#def reRunDttf( process ):
#
#    #process.dttfDigisMasked = process.valDttfDigis.clone()
#    #process.dttfDigisMasked.DTDigi_Source = cms.InputTag("muonDTDigis", "DT")
#    #from L1Trigger.GlobalMuonTrigger.gmtDigis_cfi import gmtDigis
#    from L1Trigger.CSCTrackFinder.csctfTrackDigis_cfi import csctfTrackDigis
#    process.load ( "L1Trigger.GlobalMuonTrigger.gmtDigis_cfi")
#    process.load ( "L1Trigger.CSCTrackFinder.csctfTrackDigis_cfi")
#    process.load("L1Trigger.CSCTriggerPrimitives.cscTriggerPrimitiveDigis_cfi")
#    process.RandomNumberGeneratorService = cms.Service("RandomNumberGeneratorService",
#        cscTriggerPrimitiveDigis = cms.PSet(
#            initialSeed = cms.untracked.uint32(789342)
#
#        ),
#        csc2DRecHitsOverload = cms.PSet(
#            initialSeed = cms.untracked.uint32(81)
#        ),
#    )
#    from L1Trigger.DTTrackFinder.dttfDigis_cfi import dttfDigis as dttfTrackFinder
#    process.dttfTrackDigis = dttfTrackFinder.clone()
#    process.dttfTrackDigis.DTDigi_Source = cms.InputTag("dttfDigis")
#    process.dttfTrackDigis.CSCStub_Source = cms.InputTag("csctfTrackDigis")
#
#    if hasattr(process.dttfTrackDigis, 'DTDigi_Theta_Source'):
#        process.dttfTrackDigis.DTDigi_Theta_Source = cms.InputTag("dttfDigis") 
#    #process.csctfTrackDigis.SectorReceiverInput = cms.untracked.InputTag("muonCSCDigis")
#    process.csctfTrackDigis.SectorReceiverInput = cms.untracked.InputTag("cscTriggerPrimitiveDigis")
#    process.csctfTrackDigis.DTproducer = cms.untracked.InputTag("dttfDigis")
#    process.gmtDigis.DTCandidates = cms.InputTag("dttfTrackDigis","DT")
#    process.gmtDigis.RPCbCandidates = cms.InputTag("gtDigis","RPCb")
#    process.gmtDigis.RPCfCandidates = cms.InputTag("gtDigis","RPCf")
#    process.gmtDigis.CSCCandidates  = cms.InputTag("gtDigis","CSC")
#    process.RawToDigiL1MuonEmulator = cms.Sequence( process.RawToDigi
#                                                    * process.cscTriggerPrimitiveDigis
#                                                    * process.csctfTrackDigis 
#                                                    * process.dttfTrackDigis
#                                                    * process.gmtDigis )
#    process.raw2digi_step.replace( process.RawToDigi, process.RawToDigiL1MuonEmulator )
#
#    # process.ValL1MuTrackFinders.replace(process.valGmtDigis, process.gmtDigis)
#    # process.ValL1Emulator.replace(process.valGmtDigis, process.gmtDigis)
#    # process.gmtDigis.DTCandidates = cms.InputTag("dttfDigis", "DT")
#    # process.l1compare.GMTsourceEmul = cms.InputTag("gmtDigis")
#    # process.deGmt.replace(process.valGmtDigis, process.gmtDigis)
#    if hasattr(process,'RECOSIMoutput'):
#        process.RECOSIMoutput.outputCommands.extend( ['keep *_dttfDigis_*_*', 
#                                                      'keep *_dttfTrackDigis_*_*', 
#                                                      'keep *_gmtDigis_*_*'] )
#    if hasattr(process,'AODSIMoutput'):
#        process.AODSIMoutput.outputCommands.extend( ['keep *_dttfDigis_*_*',
#                                                     'keep *_dttfTrackDigis_*_*',
#                                                     'keep *_gmtDigis_*_*'] )
#    return process
