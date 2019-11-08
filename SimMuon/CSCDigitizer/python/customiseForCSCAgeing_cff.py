import FWCore.ParameterSet.Config as cms

def customiseForAgeing(process, l1tEmuPath) :

    if hasattr(process,"GlobalTag") :
            print "[customiseForAgeing]: appending ageing to the GlobalTag"
            process.GlobalTag.toGet.append(cms.PSet(record  = cms.string("MuonSystemAgingRcd"),
                                                    connect = cms.string("sqlite_file:./MuonAgeingAndFailures_3000fbm1_DT-RPC-CSC_L1TTDR_v3_mc.db"),
                                                    tag     = cms.string("MuonAgeingAndFailures_3000fbm1_DT-RPC-CSC_L1TTDR")
                                                )
                                       )

    if hasattr(process,"simCscTriggerPrimitiveDigis") :
            print "[customiseForAgeing]: prepending ageing before simCscTriggerPrimitiveDigis and adding ageing to RandomNumberGeneratorService"

            from SimMuon.CSCDigitizer.cscChamberMasker_cfi import cscChamberMasker as _cscChamberMasker

            process.agedCscDigis = _cscChamberMasker.clone()
            process.agedCscDigis.runAtRaw = False

            getattr(process,l1tEmuPath).insert(0,process.agedCscDigis)

            print getattr(process,l1tEmuPath)

            if hasattr(process,"RandomNumberGeneratorService") :
                process.RandomNumberGeneratorService.agedCscDigis = cms.PSet( initialSeed = cms.untracked.uint32(789342),
                                                                             engineName = cms.untracked.string('TRandom3') )
            else :
                process.RandomNumberGeneratorService = cms.Service("RandomNumberGeneratorService",
                                                                   agedCscDigiss = cms.PSet( initialSeed = cms.untracked.uint32(789342),
                                                                                             engineName = cms.untracked.string('TRandom3') )
                )

            process.simCscTriggerPrimitiveDigis.CSCComparatorDigiProducer = cms.InputTag("agedCscDigis","MuonCSCComparatorDigi")
            process.simCscTriggerPrimitiveDigis.CSCWireDigiProducer = cms.InputTag("agedCscDigis","MuonCSCWireDigi")

    return process
