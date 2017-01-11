import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")
process.load("CondCore.CondDB.CondDB_cfi")
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
from Configuration.AlCa.autoCond import autoCond
process.GlobalTag.globaltag = autoCond['run2_design']

process.CondDB.connect = 'sqlite_file:MuonSystemAging.db'

process.source = cms.Source("EmptyIOVSource",
    lastValue = cms.uint64(1),
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    interval = cms.uint64(1)
)

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDB,
    timetype = cms.untracked.string('runnumber'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('MuonSystemAgingRcd'),
        tag = cms.string('MuonSystemAging_test')
    ))
)

process.mytest = cms.EDAnalyzer("ChamberMasker",
           CSCineff = cms.double(0.15), 
           maskedRPCIDs = cms.vint32([637570221,637602989,637635757,637569201,637634737,637571545,637571677,637637213,637567793,637600561,637633329,637566989,637632525,637566993,637632529,637569037,637634573,637571565,637637101,637571449,637636985,637572317,637637853,637568057,637600825,637633593,637568025,637633561,637567381,637632917,637571805,637637341,637569325,637602093,637634861,637572569,637638105,637571569,637637105,637567065,637632601,637570229,637635765,637567285,637600053,637632821,637572589,637638125,637571157,637636693,637570485,637636021,637571181,637636717,637567069,637632605,637570365,637603133,637635901,637570481,637636017,637572445,637637981,637567005,637632541,637567373,637632909,637568509,637634045,637569425,637634961,637572561,637638097,637568409,637633945,637571281,637636817,637569973,637635509,637567157,637599925,637632693,637571197,637636733,637571321,637636857,637570237,637603005,637635773,637569453,637602221,637634989,637569977,637635513,637568305,637601073,637633841,637570097,637635633,637570197,637635733,637569709,637602477,637635245,637571161,637636697,637572429,637637965,637567669,637600437,637633205,637572045,637637581,637568301,637633837,637567213,637632749,637567801,637600569,637633337,637568053,637600821,637633589,637570109,637602877,637635645,637567965,637633501,637567993,637633529,637637081,637571193,637636729,637567281,637600049,637632817,637567469,637633005,637567953,637633489,637568177,637600945,637633713,637568469,637634005,637567021,637632557,637571317,637636853,637567885,637633421,637572085,637637621,637568213,637633749,637567441,637632977,637569305,637634841,637567341,637632877,637575669,637641205,637567665,637600433,637633201,637567805,637633341,637570233,637635769,637570321,637635857,637567033,637599801,637632569,637568505,637634041,637571453,637636989,637579769,637645305,637572573,637638109,637571277,637636813,637568237,637633773,637571925,637637461,637568089,637633625,637569457,637634993,637572437,637637973,637571961,637637497,637568345,637633881,637572345,637637881,637567189,637632725,637571413,637575620,637608388,637641156,637588100,637620868,637653636,637571816,637604584,637637352,637575462,637608230,637640998,637579430,637612198,637644966,637566980,637599748,637632516,637567242,637600010,637632778,637587818,637620586,637653354,637567722,637600490,637633258,637579784,637612552,637645320,637579658,637612426,637645194,637575242,637608010,637640778,637587976,637620744,637653512,637575466,637608234,637641002,637567524,637600292,637633060,637571658,637604426,637637194,637579882,637612650,637645418,637567462,637600230,637632998,637587556,637620324,637653092,637579942,637612710,637645478,637571210,637603978,637636746,637579748,637612516,637645284,637575338,637608106,637640874,637587976,637620744,637653512,637579910,637612678,637645446,637583944,637616712,637649480,637584106,637616874,637649642,637579782,637612550,637645318,637579592,637612360,637645128]),
           # Accept lists or regular expression as from:
           # http://www.cplusplus.com/reference/regex/ECMAScript/
           chamberRegEx = cms.vstring([
            # A chamber by chamber list in format CHAMBER:EFF
            # MB4 of top sectors
            "WH-2_ST4_SEC2:0.","WH-2_ST4_SEC3:0.","WH-2_ST4_SEC4:0.","WH-2_ST4_SEC5:0.","WH-2_ST4_SEC6:0.",
            "WH-1_ST4_SEC2:0.","WH-1_ST4_SEC3:0.","WH-1_ST4_SEC4:0.","WH-1_ST4_SEC5:0.","WH-1_ST4_SEC6:0.",
            "WH0_ST4_SEC2:0.","WH0_ST4_SEC3:0.","WH0_ST4_SEC4:0.","WH0_ST4_SEC5:0.","WH0_ST4_SEC6:0.",
            "WH1_ST4_SEC2:0.","WH1_ST4_SEC3:0.","WH1_ST4_SEC4:0.","WH1_ST4_SEC5:0.","WH1_ST4_SEC6:0.",
            "WH2_ST4_SEC2:0.","WH2_ST4_SEC3:0.","WH2_ST4_SEC4:0.","WH2_ST4_SEC5:0.","WH2_ST4_SEC6:0.",
            # MB1 of external wheels
            "WH-2_ST1_SEC1:0.","WH-2_ST1_SEC2:0.","WH-2_ST1_SEC3:0.","WH-2_ST1_SEC4:0.",
            "WH-2_ST1_SEC5:0.","WH-2_ST1_SEC6:0.","WH-2_ST1_SEC7:0.","WH-2_ST1_SEC8:0.",
            "WH-2_ST1_SEC9:0.","WH-2_ST1_SEC10:0.","WH-2_ST1_SEC11:0.","WH-2_ST1_SEC12:0.",
            "WH2_ST1_SEC1:0.","WH2_ST1_SEC2:0.","WH2_ST1_SEC3:0.","WH2_ST1_SEC4:0.",
            "WH2_ST1_SEC5:0.","WH2_ST1_SEC6:0.","WH2_ST1_SEC7:0.","WH2_ST1_SEC8:0.",
            "WH2_ST1_SEC9:0.","WH2_ST1_SEC10:0.","WH2_ST1_SEC11:0.","WH2_ST1_SEC12:0.",
            # 5 MB2s of external wheels
            "WH2_ST2_SEC3:0.","WH2_ST2_SEC6:0.","WH2_ST2_SEC9:0.",
            "WH-2_ST2_SEC2:0.","WH-2_ST2_SEC4:0.",
            # more sparse failures
            "WH-2_ST2_SEC8:0.","WH-1_ST1_SEC1:0.","WH-1_ST2_SEC1:0.","WH-1_ST1_SEC4:0.","WH-1_ST3_SEC7:0.",
            "WH0_ST2_SEC2:0.","WH0_ST3_SEC5:0.","WH0_ST4_SEC12:0.","WH1_ST1_SEC6:0.","WH1_ST1_SEC10:0.","WH1_ST3_SEC3:0."
            # Or a RegEx setting efficiency  for all chamber to 10%
            #"(WH-?\\\d_ST\\\d_SEC\\\\d+):0.1"
            ])
                                
)

process.p = cms.Path(process.mytest)

