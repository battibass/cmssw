import FWCore.ParameterSet.Config as cms

process = cms.Process("PAT")


process.MessageLogger = cms.Service(
        "MessageLogger",
            categories = cms.untracked.vstring('info', 'debug','cout')
            )

process.options = cms.untracked.PSet(
    Rethrow = cms.untracked.vstring('ProductNotFound')
)
# source
process.source = cms.Source("PoolSource", 
     fileNames = cms.untracked.vstring(
    #'file:rfio:/castor/cern.ch/user/r/rompotis/DATA_STUDIES/Spring10/sample_WenuSpring10START3X_V26_S09-v1_AODSIM.root',
    "dcap://gfe02:22128/pnfs/hep.ph.ic.ac.uk/data/cms/store/data/Commissioning10/MinimumBias/RECO/May6thPDSkim2_SD_EG-v1/0135/FCC2FA5A-BB5D-DF11-8246-002618943978.root"
    )
)
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

## Load additional processes
process.load("Configuration.StandardSequences.Geometry_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
## global tags:
process.GlobalTag.globaltag = cms.string('GR_R_35X_V8B::All')
process.load("Configuration.StandardSequences.MagneticField_cff")


################################################################################################
###    P r e p a r a t i o n      o f    t h e    P A T    O b j e c t s   f r o m    A O D  ###
################################################################################################

## pat sequences to be loaded:
#process.load("PhysicsTools.PFCandProducer.PF2PAT_cff")
process.load("PhysicsTools.PatAlgos.patSequences_cff")
#process.load("PhysicsTools.PatAlgos.triggerLayer1.triggerProducer_cff")
##
#
## %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
## MET creation     <=== WARNING: YOU MAY WANT TO MODIFY THIS PART OF THE CODE       %%%%%%%%%%%%%
##                                specify the names of the MET collections that you need here %%%%
##                                                                                             #%%
## if you don't specify anything the default MET is the raw Calo MET                           #%%
process.layer1RawCaloMETs = process.patMETs.clone(                                          #%%
    metSource = cms.InputTag("met","","RECO"),
    addTrigMatch = cms.bool(False),
    addMuonCorrections = cms.bool(False),
    addGenMET = cms.bool(False),
    )
## specify here what you want to have on the plots! <===== MET THAT YOU WANT ON THE PLOTS  %%%%%%%
myDesiredMetCollection = 'layer1RawCaloMETs'
## modify the sequence of the MET creation:                                                    #%%
process.makePatMETs = cms.Sequence(process.layer1RawCaloMETs)
## %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
## modify the final pat sequence: keep only electrons + METS (muons are needed for met corrections)
process.load("RecoEgamma.EgammaIsolationAlgos.egammaIsolationSequence_cff")
#process.patElectronIsolation = cms.Sequence(process.egammaIsolationSequence)

process.patElectrons.isoDeposits = cms.PSet()
process.patElectrons.userIsolation = cms.PSet()
process.patElectrons.addElectronID = cms.bool(True)
process.patElectrons.electronIDSources = cms.PSet(
    simpleEleId95relIso= cms.InputTag("simpleEleId95relIso"),
    simpleEleId90relIso= cms.InputTag("simpleEleId90relIso"),
    simpleEleId85relIso= cms.InputTag("simpleEleId85relIso"),
    simpleEleId80relIso= cms.InputTag("simpleEleId80relIso"),
    simpleEleId70relIso= cms.InputTag("simpleEleId70relIso"),
    simpleEleId60relIso= cms.InputTag("simpleEleId60relIso"),
    simpleEleId95cIso= cms.InputTag("simpleEleId95cIso"),
    simpleEleId90cIso= cms.InputTag("simpleEleId90cIso"),
    simpleEleId85cIso= cms.InputTag("simpleEleId85cIso"),
    simpleEleId80cIso= cms.InputTag("simpleEleId80cIso"),
    simpleEleId70cIso= cms.InputTag("simpleEleId70cIso"),
    simpleEleId60cIso= cms.InputTag("simpleEleId60cIso"),    
    )
##
process.patElectrons.addGenMatch = cms.bool(False)
process.patElectrons.embedGenMatch = cms.bool(False)
##
process.load("ElectroWeakAnalysis.WENu.simpleEleIdSequence_cff")
process.patElectronIDs = cms.Sequence(process.simpleEleIdSequence)
process.makePatElectrons = cms.Sequence(process.patElectronIDs*process.patElectrons)
# process.makePatMuons may be needed depending on how you calculate the MET
process.makePatCandidates = cms.Sequence(process.makePatElectrons+process.makePatMETs)
process.patDefaultSequence = cms.Sequence(process.makePatCandidates)
##
##  ################################################################################
##
##  the filter to select the candidates from the data samples
##
##
## WARNING: you may want to modify this item:
HLT_process_name = "HLT"   # 
# trigger path selection
HLT_path_name    = "HLT_Ele15_LW_L1R"
# trigger filter name
HLT_filter_name  =  "hltL1NonIsoHLTNonIsoSingleElectronLWEt15PixelMatchFilter"
#
process.wenuFilter = cms.EDFilter('WenuCandidateFilter',
                                  # cfg for data
                                  dataMagneticFieldSetUp = cms.untracked.bool(True),
                                  dcsTag = cms.untracked.InputTag("scalersRawToDigi"),
                                  # cuts
                                  ETCut = cms.untracked.double(5.),
                                  METCut = cms.untracked.double(0.),
                                  # 2nd electron in W events
                                  vetoSecondElectronEvents = cms.untracked.bool(False),
                                  ETCut2ndEle = cms.untracked.double(20.),
                                  vetoSecondElectronIDType = cms.untracked.string("simpleEleId80relIso"),
                                  vetoSecondElectronIDSign = cms.untracked.string("="),
                                  vetoSecondElectronIDValue = cms.untracked.double(7.),
                                  # trigger 
                                  useTriggerInfo = cms.untracked.bool(False),
                                  triggerCollectionTag = cms.untracked.InputTag("TriggerResults","",HLT_process_name),
                                  triggerEventTag = cms.untracked.InputTag("hltTriggerSummaryAOD","",HLT_process_name),
                                  hltpath = cms.untracked.string(HLT_path_name), 
                                  hltpathFilter = cms.untracked.InputTag(HLT_filter_name,"",HLT_process_name),
                                  electronMatched2HLT = cms.untracked.bool(False),
                                  electronMatched2HLT_DR = cms.untracked.double(0.2),
                                  # additional preselection cuts
                                  useValidFirstPXBHit = cms.untracked.bool(False),
                                  useConversionRejection = cms.untracked.bool(False),
                                  useExpectedMissingHits = cms.untracked.bool(False),
                                  maxNumberOfExpectedMissingHits = cms.untracked.int32(1),
                                  # calculate some new cuts
                                  calculateValidFirstPXBHit = cms.untracked.bool(True),
                                  calculateConversionRejection = cms.untracked.bool(True),
                                  calculateExpectedMissingHits = cms.untracked.bool(True),
                                  # electrons and MET
                                  electronCollectionTag = cms.untracked.InputTag("patElectrons","","PAT"),
                                  metCollectionTag = cms.untracked.InputTag(myDesiredMetCollection,"","PAT"),

                                  )
####################################################################################
process.wenuPath = cms.Path(process.patDefaultSequence*process.wenuFilter)


process.wenuOutputModule = cms.OutputModule( "PoolOutputModule",
                                             fileName = cms.untracked.string("wenuCandidates.root"),
                                             outputCommands = cms.untracked.vstring(
                                                    'drop *',
                                                    'keep *_*_selectedWenuCandidates_*', 
                                                    ),
                                             SelectEvents = cms.untracked.PSet(
                                                     SelectEvents = cms.vstring('wenuPath',),
                                                                              )
                                             )

process.outpath = cms.EndPath(process.wenuOutputModule)





