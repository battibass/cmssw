import FWCore.ParameterSet.Config as cms

from Configuration.StandardSequences.GeometryDB_cff import *
from Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff import *
from Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff import *
from CondCore.CondDB.CondDB_cfi import *

from RecoLocalMuon.Configuration.RecoLocalMuonCosmics_cff import *

import EventFilter.DTRawToDigi.dturosunpacker_cfi # legacy unpacker
import EventFilter.DTRawToDigi.dtab7unpacker_cfi # phase II unpacker
muonDTDigis = EventFilter.DTRawToDigi.dturosunpacker_cfi.dturosunpacker.clone()
muonDTDigisPhase2 = EventFilter.DTRawToDigi.dtab7unpacker_cfi.dtAB7unpacker.clone()

dtCalibOfflineReco = cms.Sequence(dt1DRecHits + dt2DSegments + dt4DSegments)
dtCalibOfflineRecoRAW = cms.Sequence(muonDTDigis + dt1DRecHits + dt2DSegments + dt4DSegments)
