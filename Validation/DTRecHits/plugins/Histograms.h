#ifndef Validation_DTRecHits_Histograms_h
#define Validation_DTRecHits_Histograms_h

/** \class Histograms
 *  Collection of histograms for DT RecHit and Segment test.
 *
 *  \author S. Bolognesi and G. Cerminara - INFN Torino
 */

#include <cmath>
#include <iostream>
#include <string>

#include <TH1F.h>

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

//---------------------------------------------------------------------------------------
/// Function to fill an efficiency histograms with binomial errors
inline
TH1F* divide(const TH1F * numerator, const TH1F * denominator, std::string const& name, std::string const& title)
{
  TH1F* ratio = static_cast<TH1F *>(numerator->Clone());
  ratio->SetName(name.c_str());
  ratio->SetTitle(title.c_str());
  ratio->Divide(denominator);
  // Set the error accordingly to binomial statistics
  int bins = ratio->GetNbinsX();
  for (int bin = 1; bin <= bins; ++bin) {
    float den = denominator->GetBinContent(bin);
    float eff = ratio->GetBinContent(bin);
    float err = 0;
    if (den != 0) {
      err = sqrt(eff*(1-eff)/den);
    }
    ratio->SetBinError(bin, err);
  }
  return ratio;
}

//---------------------------------------------------------------------------------------
/// A set of histograms of residuals and pulls for 1D RecHits
class HRes1DHit {
  public:
    HRes1DHit(std::string name, DQMStore* dbe_, bool doall=true, bool local=true){
      std::string pre ="1D_";
      pre += name;
      doall_ = doall;
      dbe_->setCurrentFolder("DT/1DRecHits/Res/");

      if(doall){
	hDist=nullptr; hDist = dbe_->book1D(pre + "_hDist" ,"1D RHit distance from wire", 100, 0, 2.5);
	hResVsAngle = nullptr; hResVsAngle   = dbe_->book2D(pre+"_hResVsAngle", "1D RHit residual vs impact angle", 100, -1.2, 1.2, 100, -0.2, 0.2);
	hResVsDistFE = nullptr; hResVsDistFE = dbe_->book2D(pre+"_hResVsDistFE", "1D RHit residual vs FE distance", 100, 0., 400., 150, -0.5, 0.5);
	dbe_->setCurrentFolder("DT/1DRecHits/Pull/");
	hPullVsPos= nullptr; hPullVsPos  = dbe_->book2D (pre+"_hPullVsPos", "1D RHit pull vs position", 100, 0, 2.5, 100, -5, 5);
	hPullVsAngle = nullptr; hPullVsAngle  = dbe_->book2D (pre+"_hPullVsAngle", "1D RHit pull vs impact angle", 100, -1.2, 1.2, 100, -5, 5);
	hPullVsDistFE = nullptr; hPullVsDistFE  = dbe_->book2D (pre+"_hPullVsDistFE", "1D RHit pull vs FE distance", 100, 0., 400., 100, -5, 5);
      }
      dbe_->setCurrentFolder("DT/1DRecHits/Res/");
      hRes=nullptr; hRes = dbe_->book1D(pre + "_hRes","1D RHit residual", 300, -0.5, 0.5);
      hResSt[0] = nullptr; hResSt[0] = dbe_->book1D(pre + "_hResMB1","1D RHit residual", 300, -0.5, 0.5);
      hResSt[1] = nullptr; hResSt[1] = dbe_->book1D(pre + "_hResMB2","1D RHit residual", 300, -0.5, 0.5);
      hResSt[2] = nullptr; hResSt[2] = dbe_->book1D(pre + "_hResMB3","1D RHit residual", 300, -0.5, 0.5);
      hResSt[3] = nullptr; hResSt[3] = dbe_->book1D(pre + "_hResMB4","1D RHit residual", 300, -0.5, 0.5);

      hResVsEta=nullptr; hResVsEta = dbe_->book2D(pre +"_hResVsEta" , "1D RHit residual vs eta", 50, -1.25, 1.25, 150,-0.5, 0.5);
      hResVsPhi = nullptr; hResVsPhi   = dbe_->book2D(pre+"_hResVsPhi" , "1D RHit residual vs phi", 100, -3.2, 3.2, 150, -0.5, 0.5);
      hResVsPos = nullptr; hResVsPos   = dbe_->book2D(pre+"_hResVsPos", "1D RHit residual vs position", 100, 0, 2.5, 150, -0.5, 0.5);
      dbe_->setCurrentFolder("DT/1DRecHits/Pull/");
      hPull =nullptr; hPull       = dbe_->book1D (pre+"_hPull", "1D RHit pull", 100, -5, 5);
      hPullSt[0] = nullptr; hPullSt[0] = dbe_->book1D(pre + "_hPullMB1","1D RHit residual", 100, -5, 5);
      hPullSt[1] = nullptr; hPullSt[1] = dbe_->book1D(pre + "_hPullMB2","1D RHit residual", 100, -5, 5);
      hPullSt[2] = nullptr; hPullSt[2] = dbe_->book1D(pre + "_hPullMB3","1D RHit residual", 100, -5, 5);
      hPullSt[3] = nullptr; hPullSt[3] = dbe_->book1D(pre + "_hPullMB4","1D RHit residual", 100, -5, 5);
    }

    void Fill(float distSimHit,
              float thetaSimHit,
              float distFESimHit,
              float distRecHit,
              float etaSimHit,
              float phiSimHit,
              float errRecHit,
	      int station) {
      // Reso, pull
      float res = distRecHit-distSimHit;
      if(doall_){
	hDist->Fill(distRecHit);
	hResVsAngle->Fill(thetaSimHit, res);
	hResVsDistFE->Fill(distFESimHit, res);
      }
      hRes->Fill(res); hResSt[station-1]->Fill(res);
      hResVsEta->Fill(etaSimHit, res);
      hResVsPhi->Fill(phiSimHit, res);
      hResVsPos->Fill(distSimHit, res);
      if(errRecHit!=0) {
        float pull=res/errRecHit;
        hPull->Fill(pull);hPullSt[station-1]->Fill(pull);
	if(doall_){
	  hPullVsPos->Fill(distSimHit, pull);
	  hPullVsAngle->Fill(thetaSimHit, pull);
	  hPullVsDistFE->Fill(distFESimHit, pull);
	}
      }
      else std::cout<<"Error: RecHit error = 0" << std::endl;
    }

  private:
    MonitorElement* hDist;
    MonitorElement* hRes;
    MonitorElement* hResSt[4];
    MonitorElement* hResVsEta;
    MonitorElement* hResVsPhi;
    MonitorElement* hResVsPos;
    MonitorElement* hResVsAngle;
    MonitorElement* hResVsDistFE;

    MonitorElement* hPull;
    MonitorElement* hPullSt[4];
    MonitorElement* hPullVsPos;
    MonitorElement* hPullVsAngle;
    MonitorElement* hPullVsDistFE;
    bool doall_;
    std::string name_;
};

//---------------------------------------------------------------------------------------
class HEff1DHit {
  public:
    HEff1DHit(std::string name, DQMStore *dbe_){
      std::string pre ="1D_";
      pre += name;
      name_ = pre;
      dbe_->setCurrentFolder("DT/1DRecHits/");
      hEtaMuSimHit=nullptr; hEtaMuSimHit = dbe_->book1D(pre+"_hEtaMuSimHit", "SimHit Eta distribution", 100, -1.5, 1.5);
      hEtaRecHit=nullptr; hEtaRecHit = dbe_->book1D(pre+"_hEtaRecHit", "SimHit Eta distribution with 1D RecHit", 100, -1.5, 1.5);
      hEffVsEta = nullptr;
      hPhiMuSimHit=nullptr; hPhiMuSimHit = dbe_->book1D(pre+"_hPhiMuSimHit", "SimHit Phi distribution", 100, -M_PI, M_PI);
      hPhiRecHit=nullptr; hPhiRecHit = dbe_->book1D(pre+"_hPhiRecHit", "SimHit Phi distribution with 1D RecHit", 100, -M_PI, M_PI);
      hEffVsPhi = nullptr;
      hDistMuSimHit=nullptr;hDistMuSimHit = dbe_->book1D(pre+"_hDistMuSimHit", "SimHit Distance from wire distribution", 100, 0, 2.5);
      hDistRecHit=nullptr; hDistRecHit = dbe_->book1D(pre+"_hDistRecHit", "SimHit Distance from wire distribution with 1D RecHit", 100, 0, 2.5);
      hEffVsDist = nullptr;

    }

    void Fill(float distSimHit,
              float etaSimHit,
              float phiSimHit,
              bool fillRecHit) {

      hEtaMuSimHit->Fill(etaSimHit);
      hPhiMuSimHit->Fill(phiSimHit);
      hDistMuSimHit->Fill(distSimHit);
      if(fillRecHit) {
        hEtaRecHit->Fill(etaSimHit);
        hPhiRecHit->Fill(phiSimHit);
        hDistRecHit->Fill(distSimHit);
      }
    }

    /* FIXME these shoud be moved to the harvesting step
    void computeEfficiency() {
      hEffVsEta = divide(hEtaRecHit, hEtaMuSimHit, name_+"_hEffVsEta", "1D RecHit Efficiency as a function of Eta");
      hEffVsPhi = divide(hPhiRecHit, hPhiMuSimHit, name_+"_hEffVsPhi", "1D RecHit Efficiency as a function of Phi");
      hEffVsDist = divide(hDistRecHit, hDistMuSimHit, name_+"_hEffVsDist", "1D RecHit Efficiency as a function of Dist");
    }
    */

  private:
    MonitorElement* hEtaMuSimHit;
    MonitorElement* hEtaRecHit;
    TH1F* hEffVsEta;

    MonitorElement* hPhiMuSimHit;
    MonitorElement* hPhiRecHit;
    TH1F* hEffVsPhi;

    MonitorElement* hDistMuSimHit;
    MonitorElement* hDistRecHit;
    TH1F* hEffVsDist;

    std::string name_;
};

//---------------------------------------------------------------//

// Histos of residuals for 2D rechits
class HRes2DHit {
  public:
    HRes2DHit(std::string name, DQMStore* dbe_, bool doall=true, bool local=true){
      doall_ = doall;
      std::string pre ="2D_";
      pre += name;
      dbe_->setCurrentFolder("DT/2DSegments/Res/");
      if(doall){
	hRecAngle=nullptr;hRecAngle = dbe_->book1D (pre+"_hRecAngle", "Distribution of Rec segment angles;angle (rad)", 100, -1.5, 1.5);
	hSimAngle=nullptr;hSimAngle = dbe_->book1D (pre+"_hSimAngle", "Distribution of segment angles from SimHits;angle (rad)", 100, -1.5, 1.5);
	hRecVsSimAngle=nullptr;hRecVsSimAngle = dbe_->book2D (pre+"_hRecVsSimAngle", "Rec angle vs sim angle;angle (rad)", 100, -1.5, 1.5, 100, -1.5, 1.5);
	hResAngleVsEta=nullptr;hResAngleVsEta   = dbe_->book2D (pre+"_hResAngleVsEta", "Residual on 2D segment angle vs Eta; #eta; res (rad)", 100, -2.5, 2.5, 200, -0.2, 0.2);
	hResAngleVsPhi=nullptr;hResAngleVsPhi   = dbe_->book2D (pre+"_hResAngleVsPhi", "Residual on 2D segment angle vs Phi; #phi (rad);res (rad)",
							  100, -3.2, 3.2, 150, -0.2, 0.2);
	hResPosVsEta=nullptr;hResPosVsEta   = dbe_->book2D (pre+"_hResPosVsEta", "Residual on 2D segment position vs Eta;#eta;res (cm)",
						      100, -2.5, 2.5, 150, -0.2, 0.2);
	hResPosVsPhi=nullptr;hResPosVsPhi   = dbe_->book2D (pre+"_hResPosVsPhi", "Residual on 2D segment position vs Phi;#phi (rad);res (cm)",
						      100, -3.2, 3.2, 150, -0.2, 0.2);
	
	hResPosVsResAngle=nullptr;hResPosVsResAngle = dbe_->book2D(pre+"_hResPosVsResAngle",
							     "Residual on 2D segment position vs Residual on 2D segment angle;angle (rad);res (cm)",
							     100, -0.3, 0.3, 150, -0.2, 0.2);
	
      }

      hResAngle=nullptr; hResAngle   = dbe_->book1D (pre+"_hResAngle", "Residual on 2D segment angle;angle_{rec}-angle_{sim} (rad)", 50, -0.01, 0.01);

      hResPos=nullptr;hResPos   = dbe_->book1D (pre+"_hResPos", "Residual on 2D segment position (x at SL center);x_{rec}-x_{sim} (cm)",
					  150, -0.2, 0.2);
      dbe_->setCurrentFolder("DT/2DSegments/Pull/");

      hPullAngle=nullptr;hPullAngle   = dbe_->book1D (pre+"_hPullAngle", "Pull on 2D segment angle;(angle_{rec}-angle_{sim})/#sigma (rad)", 150, -5, 5);
      hPullPos=nullptr;hPullPos   = dbe_->book1D (pre+"_hPullPos", "Pull on 2D segment position (x at SL center);(x_{rec}-x_{sim} (cm))/#sigma",
					    150, -5, 5);
    }

    void Fill(float angleSimSegment,
              float angleRecSegment,
              float posSimSegment,
              float posRecSegment,
              float etaSimSegment,
              float phiSimSegment,
              float sigmaPos,
              float sigmaAngle) {

      float resAngle = angleRecSegment-angleSimSegment;
      hResAngle->Fill(resAngle);
      float resPos = posRecSegment-posSimSegment;
      hResPos->Fill(resPos);
      hPullAngle->Fill(resAngle/sigmaAngle);
      hPullPos->Fill(resPos/sigmaPos);
      if(doall_){
	hRecAngle->Fill(angleRecSegment);
	hSimAngle->Fill(angleSimSegment);
	hRecVsSimAngle->Fill(angleSimSegment, angleRecSegment);
	hResAngleVsEta->Fill(etaSimSegment, resAngle);
	hResAngleVsPhi->Fill(phiSimSegment, resAngle);
	hResPosVsEta->Fill(etaSimSegment, resPos);
	hResPosVsPhi->Fill(phiSimSegment, resPos);
	hResPosVsResAngle->Fill(resAngle, resPos);
      }
    }

  private:
    MonitorElement *hRecAngle;
    MonitorElement *hSimAngle;
    MonitorElement *hRecVsSimAngle;
    MonitorElement *hResAngle;
    MonitorElement *hResAngleVsEta;
    MonitorElement *hResAngleVsPhi;
    MonitorElement *hResPos;
    MonitorElement *hResPosVsEta;
    MonitorElement *hResPosVsPhi;
    MonitorElement *hResPosVsResAngle;
    MonitorElement *hPullAngle;
    MonitorElement *hPullPos;

    std::string name_;
    bool doall_;
};

//--------------------------------------------------------------------------------//

// Histos for 2D RecHit efficiency
class HEff2DHit {
  public:
    HEff2DHit(std::string name, DQMStore * dbe_){
      std::string pre ="2D_";
      pre += name;
      name_ = pre;
      dbe_->setCurrentFolder("DT/2DSegments/");
      hEtaSimSegm=nullptr;hEtaSimSegm     = dbe_->book1D(pre+"_hEtaSimSegm", "Eta of SimHit segment", 100, -1.5, 1.5);
      hEtaRecHit=nullptr;hEtaRecHit      = dbe_->book1D(pre+"_hEtaRecHit", "Eta distribution of SimHit segment with 2D RecHit",
                                 100, -1.5, 1.5);

      hPhiSimSegm=nullptr;hPhiSimSegm     = dbe_->book1D(pre+"_hPhiSimSegm", "Phi of SimHit segment",
                                 100, -M_PI, M_PI);
      hPhiRecHit=nullptr;hPhiRecHit      = dbe_->book1D(pre+"_hPhiRecHit", "Phi distribution of SimHit segment with 2D RecHit",
                                 100, -M_PI, M_PI);

      hPosSimSegm=nullptr;hPosSimSegm     = dbe_->book1D(pre+"_hPosSimSegm", "Position in SL of SimHit segment (cm)",
                                 100, -250, 250);
      hPosRecHit=nullptr;hPosRecHit      = dbe_->book1D(pre+"_hPosRecHit", "Position in SL of SimHit segment with 2D RecHit (cm)",
                                 100, -250, 250);

      hAngleSimSegm=nullptr;hAngleSimSegm   = dbe_->book1D(pre+"_hAngleSimSegm", "Angle of SimHit segment (rad)",
                                 100, -2, 2);
      hAngleRecHit=nullptr;hAngleRecHit    = dbe_->book1D(pre+"_hAngleRecHit", "Angle of SimHit segment with 2D RecHit (rad)",
                                 100, -2, 2);
      hEffVsAngle=nullptr;hEffVsAngle     = nullptr;

    }

    void Fill(float etaSimSegm,
              float phiSimSegm,
              float posSimSegm,
              float angleSimSegm,
              bool fillRecHit) {

      hEtaSimSegm->Fill(etaSimSegm);
      hPhiSimSegm->Fill(phiSimSegm);
      hPosSimSegm->Fill(posSimSegm);
      hAngleSimSegm->Fill(angleSimSegm);

      if(fillRecHit) {
        hEtaRecHit->Fill(etaSimSegm);
        hPhiRecHit->Fill(phiSimSegm);
        hPosRecHit->Fill(posSimSegm);
        hAngleRecHit->Fill(angleSimSegm);
      }
    }

    /* FIXME these shoud be moved to the harvesting step
    void computeEfficiency() {
      hEffVsEta = divide(hEtaRecHit, hEtaSimSegm, name_+"_hEffVsEta", "2D RecHit Efficiency as a function of Eta");
      hEffVsPhi = divide(hPhiRecHit, hPhiSimSegm, name_+"_hEffVsPhi", "2D RecHit Efficiency as a function of Phi");
      hEffVsPos = divide(hPosRecHit, hPosSimSegm, name_+"_hEffVsPos", "2D RecHit Efficiency as a function of position in SL");
      hEffVsAngle = divide(hAngleRecHit, hAngleSimSegm, name_+"_hEffVsAngle", "2D RecHit Efficiency as a function of angle");
    }
    */

  private:
    MonitorElement *hEtaSimSegm;
    MonitorElement  *hEtaRecHit;
    TH1F  *hEffVsEta;
    MonitorElement  *hPhiSimSegm;
    MonitorElement  *hPhiRecHit;
    TH1F  *hEffVsPhi;
    MonitorElement  *hPosSimSegm;
    MonitorElement  *hPosRecHit;
    TH1F  *hEffVsPos;
    MonitorElement  *hAngleSimSegm;
    MonitorElement  *hAngleRecHit;
    TH1F   *hEffVsAngle;

    std::string name_;
};

//---------------------------------------------------------------------------------------
// Histos of residuals for 4D rechits
class HRes4DHit {
  public:
  HRes4DHit(std::string name, DQMStore *dbe_, bool doall=true, bool local=true) : local_(local){
      std::string pre ="4D_";
      pre += name;
      doall_ = doall;

      dbe_->setCurrentFolder("DT/4DSegments/Res/");
      if(doall){
	hRecAlpha=nullptr;hRecAlpha  = dbe_->book1D (pre+"_hRecAlpha", "4D RecHit alpha (RPhi) distribution;#alpha^{x} (rad)", 100, -1.5, 1.5);
	hRecBeta=nullptr;hRecBeta = dbe_->book1D (pre+"_hRecBeta", "4D RecHit beta distribution:#alpha^{y} (rad)", 100, -1.5, 1.5);
	
	hSimAlpha=nullptr;hSimAlpha = dbe_->book1D(pre+"_hSimAlpha", "4D segment from SimHit alpha (RPhi) distribution;i#alpha^{x} (rad)",
					     100, -1.5, 1.5);
	hSimBeta=nullptr;hSimBeta = dbe_->book1D(pre+"_hSimBeta", "4D segment from SimHit beta distribution;#alpha^{y} (rad)",
					   100, -1.5, 1.5);
	hRecVsSimAlpha=nullptr;hRecVsSimAlpha = dbe_->book2D(pre+"_hRecVsSimAlpha", "4D segment rec alpha {v}s sim alpha (RPhi);#alpha^{x} (rad)",
						       100, -1.5, 1.5, 100, -1.5, 1.5);
	hRecVsSimBeta=nullptr;hRecVsSimBeta = dbe_->book2D(pre+"_hRecVsSimBeta", "4D segment rec beta vs sim beta (RZ);#alpha^{y} (rad)",
						     100, -1.5, 1.5, 100, -1.5, 1.5);
	
	hResAlphaVsEta=nullptr;hResAlphaVsEta = dbe_->book2D (pre+"_hResAlphaVsEta",
							"4D RecHit residual on #alpha_x direction vs eta;#eta;#alpha^{x}_{rec}-#alpha^{x}_{sim} (rad)",
							100, -2.5, 2.5, 100, -0.025, 0.025);
	hResAlphaVsPhi=nullptr;hResAlphaVsPhi = dbe_->book2D (pre+"_hResAlphaVsPhi",
							"4D RecHit residual on #alpha_x direction vs phi (rad);#phi (rad);#alpha^{x}_{rec}-#alpha^{x}_{sim} (rad)",
							100, -3.2, 3.2, 100, -0.025, 0.025);
	hResBetaVsEta=nullptr;hResBetaVsEta = dbe_->book2D (pre+"_hResBetaVsEta",
						      "4D RecHit residual on beta direction vs eta;#eta;#alpha^{y}_{rec}-#alpha^{y}_{sim} (rad)",
						      100, -2.5, 2.5, 200, -0.2, 0.2);
	hResBetaVsPhi=nullptr;hResBetaVsPhi = dbe_->book2D (pre+"_hResBetaVsPhi",
						      "4D RecHit residual on beta direction vs phi;#phi (rad);#alpha^{y}_{rec}-#alpha^{y}_{sim} (rad)",
						      100, -3.2, 3.2, 200, -0.2, 0.2);
	
	hResXVsEta=nullptr;hResXVsEta = dbe_->book2D (pre+"_hResXVsEta", "4D RecHit residual on position (x) in chamber vs eta;#eta;x_{rec}-x_{sim} (cm)",
						100, -2.5, 2.5, 150, -0.3, 0.3);
	hResXVsPhi=nullptr;hResXVsPhi= dbe_->book2D (pre+"_hResXVsPhi", "4D RecHit residual on position (x) in chamber vs phi;#phi (rad);x_{rec}-x_{sim} (cm)",
					       100, -3.2, 3.2, 150, -0.3, 0.3);
	
	hResYVsEta=nullptr;hResYVsEta = dbe_->book2D (pre+"_hResYVsEta", "4D RecHit residual on position (y) in chamber vs eta;#eta;y_{rec}-y_{sim} (cm)",
						100, -2.5, 2.5, 150, -0.6, 0.6);
	hResYVsPhi=nullptr;hResYVsPhi = dbe_->book2D (pre+"_hResYVsPhi", "4D RecHit residual on position (y) in chamber vs phi;#phi (rad);y_{rec}-y_{sim} (cm)",
						100, -3.2, 3.2, 150, -0.6, 0.6);
	
	hResAlphaVsResBeta=nullptr;hResAlphaVsResBeta = dbe_->book2D(pre+"_hResAlphaVsResBeta", "4D RecHit residual on alpha vs residual on beta",
							       200, -0.3, 0.3, 500, -0.15, 0.15);
	hResXVsResY=nullptr;hResXVsResY= dbe_->book2D(pre+"_hResXVsResY", "4D RecHit residual on X vs residual on Y",
						150, -0.6, 0.6, 50, -0.3, 0.3);
	hResAlphaVsResX=nullptr;hResAlphaVsResX = dbe_->book2D(pre+"_hResAlphaVsResX", "4D RecHit residual on alpha vs residual on x",
							 150, -0.3, 0.3, 500, -0.15, 0.15);
	
	hResAlphaVsResY=nullptr;hResAlphaVsResY = dbe_->book2D(pre+"_hResAlphaVsResY", "4D RecHit residual on alpha vs residual on y",
							 150, -0.6, 0.6, 500, -0.15, 0.15);
	
	hRecBetaRZ=nullptr;hRecBetaRZ        = dbe_->book1D (pre+"_hRecBetaRZ", "4D RecHit beta distribution:#alpha^{y} (rad)", 100, -1.5, 1.5);
	
	hSimBetaRZ=nullptr;hSimBetaRZ      = dbe_->book1D(pre+"_hSimBetaRZ", "4D segment from SimHit beta distribution in RZ SL;#alpha^{y} (rad)",
						    100, -1.5, 1.5);
	hRecVsSimBetaRZ=nullptr;hRecVsSimBetaRZ = dbe_->book2D(pre+"_hRecVsSimBetaRZ", "4D segment rec beta vs sim beta (RZ) in RZ SL;#alpha^{y} (rad)",
							 100, -1.5, 1.5, 100, -1.5, 1.5);
	
	hResBetaVsEtaRZ=nullptr;hResBetaVsEtaRZ = dbe_->book2D (pre+"_hResBetaVsEtaRZ",
							  "4D RecHit residual on beta direction vs eta;#eta in RZ SL;#alpha^{y}_{rec}-#alpha^{y}_{sim} (rad)",
							  100, -2.5, 2.5, 200, -0.2, 0.2);
	hResBetaVsPhiRZ=nullptr;hResBetaVsPhiRZ = dbe_->book2D (pre+"_hResBetaVsPhiRZ",
							  "4D RecHit residual on beta direction vs phi in RZ SL;#phi (rad);#alpha^{y}_{rec}-#alpha^{y}_{sim} (rad)",
							  100, -3.2, 3.2, 200, -0.2, 0.2);
	hResYVsEtaRZ=nullptr;hResYVsEtaRZ    = dbe_->book2D (pre+"_hResYVsEtaRZ",
						       "4D RecHit residual on position (y) in chamber vs eta in RZ SL;#eta;y_{rec}-y_{sim} (cm)",
						       100, -2.5, 2.5, 150, -0.6, 0.6);
	hResYVsPhiRZ=nullptr;hResYVsPhiRZ    = dbe_->book2D (pre+"_hResYVsPhiRZ",
						       "4D RecHit residual on position (y) in chamber vs phi in RZ SL;#phi (rad);y_{rec}-y_{sim} (cm)",
						       100, -3.2, 3.2, 150, -0.6, 0.6);
	dbe_->setCurrentFolder("DT/4DSegments/Pull/");
	hPullAlphaVsEta=nullptr;hPullAlphaVsEta  = dbe_->book2D (pre+"_hPullAlphaVsEta",
							   "4D RecHit pull on #alpha_x direction vs eta;#eta;(#alpha^{x}_{rec}-#alpha^{x}_{sim})/#sigma",
							   100, -2.5, 2.5, 100, -5, 5);
	hPullAlphaVsPhi=nullptr;hPullAlphaVsPhi  = dbe_->book2D (pre+"_hPullAlphaVsPhi",
							   "4D RecHit pull on #alpha_x direction vs phi (rad);#phi (rad);(#alpha^{x}_{rec}-#alpha^{x}_{sim})/#sigma",
							   100, -3.2, 3.2, 100, -5, 5);
	hPullBetaVsEta=nullptr;hPullBetaVsEta   = dbe_->book2D (pre+"_hPullBetaVsEta",
							  "4D RecHit pull on beta direction vs eta;#eta;(#alpha^{y}_{rec}-#alpha^{y}_{sim})/#sigma",
							  100, -2.5, 2.5, 200, -5, 5);
	hPullBetaVsPhi=nullptr;hPullBetaVsPhi   = dbe_->book2D (pre+"_hPullBetaVsPhi",
							  "4D RecHit pull on beta direction vs phi;#phi (rad);(#alpha^{y}_{rec}-#alpha^{y}_{sim})/#sigma",
							  100, -3.2, 3.2, 200, -5, 5);
	hPullXVsEta=nullptr;hPullXVsEta      = dbe_->book2D (pre+"_hPullXVsEta",
						       "4D RecHit pull on position (x) in chamber vs eta;#eta;(x_{rec}-x_{sim})#sigma",
						       100, -2.5, 2.5, 150, -5, 5);
	hPullXVsPhi=nullptr;hPullXVsPhi      = dbe_->book2D (pre+"_hPullXVsPhi",
						       "4D RecHit pull on position (x) in chamber vs phi;#phi (rad);(x_{rec}-x_{sim})/#sigma",
						       100, -3.2, 3.2, 150, -5, 5);
	hPullYVsEta=nullptr;hPullYVsEta      = dbe_->book2D (pre+"_hPullYVsEta",
						       "4D RecHit pull on position (y) in chamber vs eta;#eta;(y_{rec}-y_{sim})/#sigma",
						       100, -2.5, 2.5, 150, -5, 5);
	hPullYVsPhi=nullptr;hPullYVsPhi      = dbe_->book2D (pre+"_hPullYVsPhi",
						       "4D RecHit pull on position (y) in chamber vs phi;#phi (rad);(y_{rec}-y_{sim})/#sigma",
						       100, -3.2, 3.2, 150, -5, 5);
	hPullBetaVsEtaRZ=nullptr;hPullBetaVsEtaRZ = dbe_->book2D (pre+"_hPullBetaVsEtaRZ",
                                   "4D RecHit pull on beta direction vs eta;#eta in RZ SL;(#alpha^{y}_{rec}-#alpha^{y}_{sim})/#sigma",
							    100, -2.5, 2.5, 200, -5, 5);
	hPullBetaVsPhiRZ=nullptr;hPullBetaVsPhiRZ = dbe_->book2D (pre+"_hPullBetaVsPhiRZ",
							    "4D RecHit pull on beta direction vs phi in RZ SL;#phi (rad);(#alpha^{y}_{rec}-#alpha^{y}_{sim})/#sigma",
							    100, -3.2, 3.2, 200, -5, 5);
	hPullYVsEtaRZ=nullptr;hPullYVsEtaRZ    = dbe_->book2D (pre+"_hPullYVsEtaRZ",
							 "4D RecHit pull on position (y) in chamber vs eta in RZ SL;#eta;(y_{rec}-y_{sim})/#sigma",
							 100, -2.5, 2.5, 150, -5, 5);
	hPullYVsPhiRZ=nullptr;hPullYVsPhiRZ    = dbe_->book2D (pre+"_hPullYVsPhiRZ",
                                   "4D RecHit pull on position (y) in chamber vs phi in RZ SL;#phi (rad);(y_{rec}-y_{sim})/#sigma",
                                   100, -3.2, 3.2, 150, -5, 5);

      }
      dbe_->setCurrentFolder("DT/4DSegments/Res/");
      hResAlpha=nullptr;hResAlpha = dbe_->book1D (pre+"_hResAlpha",
                                  "4D RecHit residual on #alpha_x direction;#alpha^{x}_{rec}-#alpha^{x}_{sim} (rad)",
					    200, -0.015, 0.015);

      hResBeta=nullptr;hResBeta = dbe_->book1D (pre+"_hResBeta",
                                  "4D RecHit residual on beta direction;#alpha^{y}_{rec}-#alpha^{y}_{sim} (rad)",
                                  200, -0.1, 0.1);
      hResX=nullptr;hResX = dbe_->book1D (pre+"_hResX", "4D RecHit residual on position (x) in chamber;x_{rec}-x_{sim} (cm)",
                                  150, -0.15, 0.15);
      hResY=nullptr;hResY = dbe_->book1D (pre+"_hResY", "4D RecHit residual on position (y) in chamber;y_{rec}-y_{sim} (cm)", 150, -0.6, 0.6);

      // histo in rz SL reference frame.

      hResBetaRZ=nullptr;hResBetaRZ      = dbe_->book1D (pre+"_hResBetaRZ",
                                  "4D RecHit residual on beta direction in RZ SL;#alpha^{y}_{rec}-#alpha^{y}_{sim} (rad)",
                                  200, -0.1, 0.1);

      hResYRZ=nullptr;hResYRZ         = dbe_->book1D (pre+"_hResYRZ",
                                  "4D RecHit residual on position (y) in chamber in RZ SL;y_{rec}-y_{sim} (cm)",
                                  150, -0.15, 0.15);

      // Pulls
      dbe_->setCurrentFolder("DT/4DSegments/Pull/");

      hPullAlpha=nullptr;hPullAlpha = dbe_->book1D (pre+"_hPullAlpha",
                                   "4D RecHit pull on #alpha_x direction;(#alpha^{x}_{rec}-#alpha^{x}_{sim})/#sigma",
                                   200, -5, 5);
      hPullBeta=nullptr;hPullBeta  = dbe_->book1D (pre+"_hPullBeta",
                                   "4D RecHit pull on beta direction;(#alpha^{y}_{rec}-#alpha^{y}_{sim})/#sigma",
                                   200, -5, 5);

      hPullX=nullptr;hPullX           = dbe_->book1D (pre+"_hPullX",
                                   "4D RecHit pull on position (x) in chamber;(x_{rec}-x_{sim})#sigma",
                                   150, -5, 5);

      hPullY=nullptr;hPullY          = dbe_->book1D (pre+"_hPullY",
                                   "4D RecHit pull on position (y) in chamber;(y_{rec}-y_{sim})/#sigma", 150, -5, 5);

     hPullBetaRZ=nullptr;hPullBetaRZ      = dbe_->book1D (pre+"_hPullBetaRZ",
                                   "4D RecHit pull on beta direction in RZ SL;(#alpha^{y}_{rec}-#alpha^{y}_{sim})/#sigma",
                                   200, -5, 5);

      hPullYRZ=nullptr;hPullYRZ         = dbe_->book1D (pre+"_hPullYRZ",
                                   "4D RecHit pull on position (y) in chamber in RZ SL;(y_{rec}-y_{sim})/#sigma",
                                   150, -5, 5);

      // NHits, t0
      if (local_) {
	dbe_->setCurrentFolder("DT/4DSegments/");
	hHitMult                  = dbe_->book2D(pre+"_hNHits", "NHits", 12, 0, 12, 6, 0, 6);
	ht0                       = dbe_->book2D(pre+"_ht0",    "t0",    200,-25, 25, 200,-25, 25);
      }

    }

    void Fill(float simDirectionAlpha,
              float recDirectionAlpha,
              float simDirectionBeta,
              float recDirectionBeta,
              float simX,
              float recX,
              float simY,
              float recY,
              float simEta,
              float simPhi,
              float recYRZ,
              float simYRZ,
              float recBetaRZ,
              float simBetaRZ,
              float sigmaAlpha,
              float sigmaBeta,
              float sigmaX,
              float sigmaY,
              float sigmaBetaRZ,
              float sigmaYRZ,
	      int nHitsPhi,
	      int nHitsTheta,
	      float t0Phi,
	      float t0Theta
             ) {
      float resAlpha = recDirectionAlpha - simDirectionAlpha;
      hResAlpha->Fill(resAlpha);
      hPullAlpha->Fill(resAlpha/sigmaAlpha);
      float resBeta = recDirectionBeta - simDirectionBeta;
      hResBeta->Fill(resBeta);
      hPullBeta->Fill(resBeta/sigmaBeta);
      float resX = recX - simX;
      hResX->Fill(resX);
      hPullX->Fill(resX/sigmaX);
      float resY = recY - simY;
      hResY->Fill(resY);
      hPullY->Fill(resY/sigmaY);

      float resBetaRZ = recBetaRZ - simBetaRZ;
      hResBetaRZ->Fill(resBetaRZ);
      hPullBetaRZ->Fill(resBetaRZ/sigmaBetaRZ);
      float resYRZ = recYRZ - simYRZ;
      hResYRZ->Fill(resYRZ);
      hPullYRZ->Fill(resYRZ/sigmaYRZ);
      if(doall_){
	hRecAlpha->Fill(recDirectionAlpha);
	hRecBeta->Fill(recDirectionBeta);
	hSimAlpha->Fill(simDirectionAlpha);
	hSimBeta->Fill(simDirectionBeta);
	hRecVsSimAlpha->Fill(simDirectionAlpha, recDirectionAlpha);
	hRecVsSimBeta->Fill(simDirectionBeta, recDirectionBeta);
	hResAlphaVsEta->Fill(simEta, resAlpha);
	hResAlphaVsPhi->Fill(simPhi, resAlpha);
	hPullAlphaVsEta->Fill(simEta, resAlpha/sigmaAlpha);
	hPullAlphaVsPhi->Fill(simPhi, resAlpha/sigmaAlpha);
	hResBetaVsEta->Fill(simEta, resBeta);
	hResBetaVsPhi->Fill(simPhi, resBeta);
	hPullBetaVsEta->Fill(simEta, resBeta/sigmaBeta);
	hPullBetaVsPhi->Fill(simPhi, resBeta/sigmaBeta);
	hResXVsEta->Fill(simEta, resX);
	hResXVsPhi->Fill(simPhi, resX);
	hPullXVsEta->Fill(simEta, resX/sigmaX);
	hPullXVsPhi->Fill(simPhi, resX/sigmaX);
	hResYVsEta->Fill(simEta, resY);
	hResYVsPhi->Fill(simPhi, resY);
	hPullYVsEta->Fill(simEta, resY/sigmaY);
	hPullYVsPhi->Fill(simPhi, resY/sigmaY);
	hResAlphaVsResBeta->Fill(resBeta, resAlpha);
	hResXVsResY->Fill(resY, resX);
	hResAlphaVsResX->Fill(resX, resAlpha);
	hResAlphaVsResY->Fill(resY, resAlpha);
	// RZ SuperLayer
	hRecBetaRZ->Fill(recBetaRZ);
	hSimBetaRZ->Fill(simBetaRZ);
	hRecVsSimBetaRZ->Fill(simBetaRZ, recBetaRZ);
	hResBetaVsEtaRZ->Fill(simEta, resBetaRZ);
	hResBetaVsPhiRZ->Fill(simPhi, resBetaRZ);
	hPullBetaVsEtaRZ->Fill(simEta, resBetaRZ/sigmaBetaRZ);
	hPullBetaVsPhiRZ->Fill(simPhi, resBetaRZ/sigmaBetaRZ);
	hResYVsEtaRZ->Fill(simEta, resYRZ);
	hResYVsPhiRZ->Fill(simPhi, resYRZ);
	hPullYVsEtaRZ->Fill(simEta, resYRZ/sigmaYRZ);
	hPullYVsPhiRZ->Fill(simPhi, resYRZ/sigmaYRZ);
      }
      if (local_){
	hHitMult->Fill(nHitsPhi, nHitsTheta);
	ht0->Fill(t0Phi, t0Theta);
      }
    }

  private:
    MonitorElement *hRecAlpha;
    MonitorElement *hRecBeta;

    MonitorElement *hSimAlpha;
    MonitorElement *hSimBeta;

    MonitorElement *hRecVsSimAlpha;
    MonitorElement *hRecVsSimBeta;

    MonitorElement *hResAlpha;
    MonitorElement *hResAlphaVsEta;
    MonitorElement *hResAlphaVsPhi;

    MonitorElement *hResBeta;
    MonitorElement *hResBetaVsEta;
    MonitorElement *hResBetaVsPhi;

    MonitorElement *hResX;
    MonitorElement *hResXVsEta;
    MonitorElement  *hResXVsPhi;

    MonitorElement *hResY;
    MonitorElement *hResYVsEta;
    MonitorElement *hResYVsPhi;

    MonitorElement *hResAlphaVsResBeta;
    MonitorElement *hResXVsResY;
    MonitorElement *hResAlphaVsResX;
    MonitorElement *hResAlphaVsResY;

    MonitorElement *hPullAlpha;
    MonitorElement *hPullAlphaVsEta;
    MonitorElement *hPullAlphaVsPhi;

    MonitorElement *hPullBeta;
    MonitorElement *hPullBetaVsEta;
    MonitorElement *hPullBetaVsPhi;

    MonitorElement *hPullX;
    MonitorElement *hPullXVsEta;
    MonitorElement *hPullXVsPhi;

    MonitorElement *hPullY;
    MonitorElement *hPullYVsEta;
    MonitorElement *hPullYVsPhi;

    // RZ SL
    MonitorElement *hRecBetaRZ;

    MonitorElement *hSimBetaRZ;

    MonitorElement *hRecVsSimBetaRZ;

    MonitorElement *hResBetaRZ;
    MonitorElement *hResBetaVsEtaRZ;
    MonitorElement *hResBetaVsPhiRZ;

    MonitorElement *hResYRZ;
    MonitorElement *hResYVsEtaRZ;
    MonitorElement *hResYVsPhiRZ;

    MonitorElement *hPullBetaRZ;
    MonitorElement *hPullBetaVsEtaRZ;
    MonitorElement *hPullBetaVsPhiRZ;

    MonitorElement *hPullYRZ;
    MonitorElement *hPullYVsEtaRZ;
    MonitorElement *hPullYVsPhiRZ;

    MonitorElement *hHitMult;
    MonitorElement *ht0;

    bool doall_;
    bool local_;
    std::string name_;
};

//---------------------------------------------------------------------------------------
/// A set of histograms for efficiency 4D RecHits
class HEff4DHit {
  public:
    HEff4DHit(std::string name, DQMStore *dbe_){
      std::string pre ="4D_";
      pre += name;
      name_ = pre;
      dbe_->setCurrentFolder("DT/4DSegments/");
      hEtaSimSegm=nullptr;hEtaSimSegm     = dbe_->book1D(pre+"_hEtaSimSegm", "Eta of SimHit segment", 100, -1.5, 1.5);
      hEtaRecHit=nullptr;hEtaRecHit      = dbe_->book1D(pre+"_hEtaRecHit", "Eta distribution of SimHit segment with 4D RecHit",
                                 100, -1.5, 1.5);
      hEffVsEta       = nullptr;

      hPhiSimSegm=nullptr;hPhiSimSegm     = dbe_->book1D(pre+"_hPhiSimSegm", "Phi of SimHit segment",
                                 100, -M_PI, M_PI);
      hPhiRecHit=nullptr;hPhiRecHit      = dbe_->book1D(pre+"_hPhiRecHit", "Phi distribution of SimHit segment with 4D RecHit",
                                 100, -M_PI, M_PI);
      hEffVsPhi       = nullptr;

      hXSimSegm=nullptr;hXSimSegm       = dbe_->book1D(pre+"_hXSimSegm", "X position in Chamber of SimHit segment (cm)",
                                 100, -200, 200);
      hXRecHit=nullptr;hXRecHit        = dbe_->book1D(pre+"_hXRecHit", "X position in Chamber of SimHit segment with 4D RecHit (cm)",
                                 100, -200, 200);
      hEffVsX         = nullptr;

      hYSimSegm=nullptr;hYSimSegm       = dbe_->book1D(pre+"_hYSimSegm", "Y position in Chamber of SimHit segment (cm)",
                                 100, -200, 200);
      hYRecHit=nullptr;hYRecHit        = dbe_->book1D(pre+"_hYRecHit", "Y position in Chamber of SimHit segment with 4D RecHit (cm)",
                                 100, -200, 200);
      hEffVsY         = nullptr;

      hAlphaSimSegm=nullptr;hAlphaSimSegm   = dbe_->book1D(pre+"_hAlphaSimSegm", "Alpha of SimHit segment (rad)",
                                 100, -1.5, 1.5);
      hAlphaRecHit=nullptr;hAlphaRecHit    = dbe_->book1D(pre+"_hAlphaRecHit", "Alpha of SimHit segment with 4D RecHit (rad)",
                                 100, -1.5, 1.5);
      hEffVsAlpha     = nullptr;

      hBetaSimSegm=nullptr;hBetaSimSegm   = dbe_->book1D(pre+"_hBetaSimSegm", "Beta of SimHit segment (rad)",
                                100, -2, 2);
      hBetaRecHit=nullptr;hBetaRecHit    = dbe_->book1D(pre+"_hBetaRecHit", "Beta of SimHit segment with 4D RecHit (rad)",
                                100, -2, 2);
      hEffVsBeta     = nullptr;

      hNSeg =nullptr; hNSeg            = dbe_->book1D(pre+"_hNSeg", "Number of rec segment per sim seg",
                                 20, 0, 20);

    }

    ~HEff4DHit(){

    }

    void Fill(float etaSimSegm,
              float phiSimSegm,
              float xSimSegm,
              float ySimSegm,
              float alphaSimSegm,
              float betaSimSegm,
              bool fillRecHit,
              int nSeg) {

      hEtaSimSegm->Fill(etaSimSegm);
      hPhiSimSegm->Fill(phiSimSegm);
      hXSimSegm->Fill(xSimSegm);
      hYSimSegm->Fill(ySimSegm);
      hAlphaSimSegm->Fill(alphaSimSegm);
      hBetaSimSegm->Fill(betaSimSegm);
      hNSeg->Fill(nSeg);

      if(fillRecHit) {
        hEtaRecHit->Fill(etaSimSegm);
        hPhiRecHit->Fill(phiSimSegm);
        hXRecHit->Fill(xSimSegm);
        hYRecHit->Fill(ySimSegm);
        hAlphaRecHit->Fill(alphaSimSegm);
        hBetaRecHit->Fill(betaSimSegm);
      }
    }

    /* FIXME these shoud be moved to the harvesting step
    void computeEfficiency() {
      hEffVsEta = divide(hEtaRecHit, hEtaSimSegm, name_+"_hEffVsEta", "4D RecHit Efficiency as a function of Eta");
      hEffVsPhi = divide(hPhiRecHit, hPhiSimSegm, name_+"_hEffVsPhi", "4D RecHit Efficiency as a function of Phi");
      hEffVsX = divide(hXRecHit, hXSimSegm, name_+"_hEffVsX", "4D RecHit Efficiency as a function of x position in Chamber");
      hEffVsY = divide(hYRecHit, hYSimSegm, name_+"_hEffVsY", "4D RecHit Efficiency as a function of y position in Chamber");
      hEffVsAlpha = divide(hAlphaRecHit, hAlphaSimSegm, name_+"_hEffVsAlpha", "4D RecHit Efficiency as a function of alpha");
      hEffVsBeta = divide(hBetaRecHit, hBetaSimSegm, name_+"_hEffVsBeta", "4D RecHit Efficiency as a function of beta");
    }
    */

  private:
    MonitorElement *hEtaSimSegm;
    MonitorElement *hEtaRecHit;
    TH1F *hEffVsEta;
    MonitorElement *hPhiSimSegm;
    MonitorElement *hPhiRecHit;
    TH1F *hEffVsPhi;
    MonitorElement *hXSimSegm;
    MonitorElement *hXRecHit;
    TH1F *hEffVsX;
    MonitorElement *hYSimSegm;
    MonitorElement *hYRecHit;
    TH1F *hEffVsY;
    MonitorElement *hAlphaSimSegm;
    MonitorElement *hAlphaRecHit;
    TH1F *hEffVsAlpha;
    MonitorElement *hBetaSimSegm;
    MonitorElement *hBetaRecHit;
    TH1F *hEffVsBeta;
    MonitorElement *hNSeg;

    std::string name_;
};

#endif // Validation_DTRecHits_Histograms_h
