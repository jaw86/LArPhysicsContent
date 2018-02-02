/**
 *  @file LArPhysicsContent/src/AnalysisAlgorithm.cxx
 *
 *  @brief Implementation of the LEE analysis algorithm class.
 *
 *  $Log: $
 */

#include "AnalysisAlgorithm.h"
#include "Pandora/AlgorithmHeaders.h"

#include "larpandoracontent/LArHelpers/LArMCParticleHelper.h"
#include "larpandoracontent/LArHelpers/LArClusterHelper.h"
#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArHelpers/LArGeometryHelper.h"
#include "larpandoracontent/LArHelpers/LArPcaHelper.h"

#include "TFile.h"

using namespace pandora;
using namespace lar_content;

namespace lar_physics_content
{
AnalysisAlgorithm::AnalysisAlgorithm() :
    m_mcParticleListName(),
    m_fiducialCutXMargin(10.f),
    m_fiducialCutYMargin(20.f),
    m_fiducialCutZMargin(10.f),
    m_trackSlidingFitWindow(25U),
    m_pLArAnalysisParticleFile(nullptr),
    m_pLArAnalysisParticleNtuple(nullptr),
    m_parametersFile(),
    m_birksFitNtupleName("BirksFit"),
    m_protonEnergyFromRangeNtupleName("EnergyFromRangeProtons"),
    m_pionMuonEnergyFromRangeNtupleName("EnergyFromRangePionsMuons"),
    m_birksFitAlpha(0.f),
    m_birksFitBeta(0.f),
    m_birksFitPole(0.f),
    m_birksSelectionMaxdEdX(500.f),
    m_uniquePlotIdentifier(0),
    m_addMcInformation(true),
    m_pTmvaReader(nullptr),
    m_tmvaTrackLength(0.f),
    m_tmvaAvgEnergyDeposition(0.f),
    m_minCoordinates(0.f, 0.f, 0.f),
    m_maxCoordinates(0.f, 0.f, 0.f),
    m_pBirksHitSelectionTool(nullptr),
    m_mcContainmentFractionLowerBound(0.9f),
    m_caloHitListName()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

AnalysisAlgorithm::~AnalysisAlgorithm()
{
    if (this->m_pTmvaReader)
        delete this->m_pTmvaReader;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void AnalysisAlgorithm::CreatePfo(const ParticleFlowObject *const pInputPfo, const ParticleFlowObject*& pOutputPfo) const
{
    // Work out what kind of PFO we're dealing with.
    bool isNeutrino(false), isPrimaryNeutrinoDaughter(false), isCosmicRay(false);
    
    if (LArAnalysisParticleHelper::IsNeutrino(pInputPfo))
        isNeutrino = true;
    
    else if (LArAnalysisParticleHelper::IsPrimaryNeutrinoDaughter(pInputPfo))
        isPrimaryNeutrinoDaughter = true;
   
    else if (LArAnalysisParticleHelper::IsCosmicRay(pInputPfo))
        isCosmicRay = true;

    else
        return;
        
    // Check there is one vertex for this primary PFO and get it.
    if (pInputPfo->GetVertexList().size() != 1)
    {
        CERR("Could not create LArAnalysisParticle as the number of PFO vertices was " << pInputPfo->GetVertexList().size());
        return;
    }
    
    const Vertex *const pVertex = pInputPfo->GetVertexList().front();
    bool gotMcInformation = false;
    
    float mcEnergy = 0.f, mcContainmentFraction = 0.f, mcHitPurity = 0.f, mcHitCompleteness = 0.f;
    LArAnalysisParticle::TypeTree mcTypeTree;
    LArAnalysisParticle::TYPE mcType(LArAnalysisParticle::TYPE::UNKNOWN);
    CartesianVector mcVertexPosition(0.f, 0.f, 0.f), mcMomentum(0.f, 0.f, 0.f);
    int mcPdgCode(0);
    const MCParticle *pMcMainMCParticle(nullptr);
    
    if (this->m_addMcInformation)
    {
        gotMcInformation = this->GetMcInformation(pInputPfo, mcEnergy, mcTypeTree, mcType, mcVertexPosition, mcMomentum, mcPdgCode, isNeutrino, 
            mcContainmentFraction, pMcMainMCParticle, mcHitPurity, mcHitCompleteness);
    }
    
    // Common parameter calculations.
    LArAnalysisParticleParameters analysisParticleParameters;
    
    analysisParticleParameters.m_particleId = pInputPfo->GetParticleId();
    analysisParticleParameters.m_charge     = pInputPfo->GetCharge();
    analysisParticleParameters.m_mass       = pInputPfo->GetMass();
    analysisParticleParameters.m_energy     = pInputPfo->GetEnergy();
    analysisParticleParameters.m_momentum   = pInputPfo->GetMomentum();
    
    analysisParticleParameters.m_isVertexFiducial = LArAnalysisParticleHelper::IsPointFiducial(pVertex->GetPosition(), 
        this->m_minCoordinates, this->m_maxCoordinates);
        
    analysisParticleParameters.m_vertexPosition = pVertex->GetPosition();
    analysisParticleParameters.m_isShower       = LArPfoHelper::IsShower(pInputPfo);
    
    unsigned numberOfDownstreamParticles(0U);
    this->CountNumberOfDownstreamParticles(pInputPfo, numberOfDownstreamParticles);
    analysisParticleParameters.m_numberOfDownstreamParticles = numberOfDownstreamParticles;
    
    analysisParticleParameters.m_hasMcInfo = gotMcInformation;

    if (gotMcInformation)
    {
        analysisParticleParameters.m_mcEnergy           = mcEnergy;
        analysisParticleParameters.m_mcMomentum         = mcMomentum;
        analysisParticleParameters.m_mcVertexPosition   = mcVertexPosition;
        
        if (mcMomentum.GetMagnitude() > std::numeric_limits<float>::epsilon())
            analysisParticleParameters.m_mcDirectionCosines = mcMomentum.GetUnitVector();
            
        else
        {
            std::cout << "AnalysisAlgorithm: could not get direction from MC momentum as it was too small" << std::endl;
            analysisParticleParameters.m_mcDirectionCosines = CartesianVector(0.f, 0.f, 0.f);
        }
        
        analysisParticleParameters.m_mcIsVertexFiducial = LArAnalysisParticleHelper::IsPointFiducial(mcVertexPosition, 
            this->m_minCoordinates, this->m_maxCoordinates);
            
        analysisParticleParameters.m_mcIsContained      = (mcContainmentFraction > this->m_mcContainmentFractionLowerBound);
        analysisParticleParameters.m_mcIsShower         = (mcType == LArAnalysisParticle::TYPE::SHOWER);
        analysisParticleParameters.m_mcPdgCode          = mcPdgCode;
        analysisParticleParameters.m_mcType             = mcType;
        analysisParticleParameters.m_mcHitPurity        = mcHitPurity;
        analysisParticleParameters.m_mcHitCompleteness  = mcHitCompleteness;
        analysisParticleParameters.m_pMcMainMCParticle  = pMcMainMCParticle;
    }
    
    // Neutrino-specific calculations.
    if (isNeutrino)
    {
        analysisParticleParameters.m_type             = LArAnalysisParticle::TYPE::NEUTRINO;
        analysisParticleParameters.m_typeTree         = LArAnalysisParticle::TypeTree(LArAnalysisParticle::TYPE::NEUTRINO);
        analysisParticleParameters.m_directionCosines = CartesianVector(0.f, 0.f, 1.f);
        
        float analysisEnergy(0.f);
        float energyFromCharge(0.f);
        bool areAllHitsFiducial(true);
        CartesianVector analysisMomentum(0.f, 0.f, 0.f);
        unsigned numberOf3dHits(0U), numberOfCollectionPlaneHits(0U);
        
        for (const ParticleFlowObject *const pPrimary : pInputPfo->GetDaughterPfoList())
        {
            if (const LArAnalysisParticle *const pAnalysisPrimary = dynamic_cast<const LArAnalysisParticle *>(pPrimary))
            {
                analysisEnergy              += pAnalysisPrimary->AnalysisEnergy();
                energyFromCharge            += pAnalysisPrimary->EnergyFromCharge();
                areAllHitsFiducial          &= pAnalysisPrimary->AreAllHitsFiducial();
                analysisMomentum            += pAnalysisPrimary->AnalysisMomentum();
                numberOf3dHits              += pAnalysisPrimary->NumberOf3dHits();
                numberOfCollectionPlaneHits += pAnalysisPrimary->NumberOfCollectionPlaneHits();
            }
            
            else
                std::cout << "AnalysisAlgorithm: primary daughter of neutrino could not be cast an analysis particle" << std::endl;
        }
        
        analysisParticleParameters.m_analysisEnergy              = analysisEnergy;
        analysisParticleParameters.m_energyFromCharge            = energyFromCharge;
        analysisParticleParameters.m_areAllHitsFiducial          = areAllHitsFiducial;
        analysisParticleParameters.m_analysisMomentum            = analysisMomentum;
        analysisParticleParameters.m_numberOf3dHits              = numberOf3dHits;
        analysisParticleParameters.m_numberOfCollectionPlaneHits = numberOfCollectionPlaneHits;
    }
    
    // Calculations specific cosmic rays and primary daughters.
    else if (isCosmicRay || isPrimaryNeutrinoDaughter)
    {
        // For each tracklike PFO, try to perform a 3D sliding linear fit and store it in a map.
        LArAnalysisParticleHelper::TrackFitMap trackFitMap;
        LArAnalysisParticleHelper::RecursivelyAppendTrackFitMap(this->GetPandora(), pInputPfo, trackFitMap, this->m_trackSlidingFitWindow);
        
        // For each tracklike PFO, decide which hits we want to Birks-correct.
        LArAnalysisParticleHelper::LArTrackHitEnergyMap trackHitEnergyMap;
        this->RecursivelyAppendLArTrackHitEnergyMap(pInputPfo, trackHitEnergyMap, trackFitMap);
        
        // Derive a type for all PFOs.
        LArAnalysisParticle::PfoTypeMap particleTypeMap;
        this->RecursivelyAppendParticleTypeMap(pInputPfo, particleTypeMap, trackHitEnergyMap, trackFitMap);
        
        float particleEnergy(0.f), particleEnergyFromCharge(0.f);
        this->EstimateParticleEnergy(pInputPfo, particleTypeMap, trackFitMap, trackHitEnergyMap, particleEnergy, particleEnergyFromCharge);
        
        const LArAnalysisParticle::TypeTree typeTree = this->CreateTypeTree(pInputPfo, particleTypeMap);
        
        const CartesianVector initialDirection = this->GetDirectionAtVertex(pInputPfo, trackFitMap, pVertex);
        
        const int num3DHits = LArAnalysisParticleHelper::GetHitsOfType(pInputPfo, TPC_3D, true).size();
        const int numWHits  = LArAnalysisParticleHelper::GetHitsOfType(pInputPfo, TPC_VIEW_W, true).size();
        
        const LArAnalysisParticle::TYPE particleType = particleTypeMap.at(pInputPfo);
        
        bool fiducialCutSatisfied = true;
        
        if (!LArAnalysisParticleHelper::RecursivelyCheckFiducialCut(pInputPfo, this->m_minCoordinates, this->m_maxCoordinates))
            fiducialCutSatisfied = false;
        
        analysisParticleParameters.m_analysisEnergy              = particleEnergy;
        analysisParticleParameters.m_energyFromCharge            = particleEnergyFromCharge;
        analysisParticleParameters.m_areAllHitsFiducial          = fiducialCutSatisfied;
        analysisParticleParameters.m_directionCosines            = initialDirection;
        analysisParticleParameters.m_analysisMomentum            = initialDirection * particleEnergy;
        analysisParticleParameters.m_numberOf3dHits              = num3DHits;
        analysisParticleParameters.m_numberOfCollectionPlaneHits = numWHits;
        analysisParticleParameters.m_type                        = particleType;
        analysisParticleParameters.m_typeTree                    = typeTree;
        
    }

    // Build AnalysisParticle.
    LArAnalysisParticleFactory analysisParticleFactory;

    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::ParticleFlowObject::Create(*this, analysisParticleParameters,
        pOutputPfo, analysisParticleFactory));
        
    const LArAnalysisParticle *const pLArAnalysisParticle = dynamic_cast<const LArAnalysisParticle*>(pOutputPfo);
    
    if (!pLArAnalysisParticle)
    {
        CERR("Failed to cast");
        throw StatusCodeException(STATUS_CODE_FAILURE);
    }
    
    // Now update vertex and direction
    PandoraContentApi::ParticleFlowObject::Metadata pfoData;
    pfoData.m_momentum = pInputPfo->GetMomentum();
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::ParticleFlowObject::AlterMetadata(*this, pOutputPfo, pfoData));

    const Vertex *pOutputVertex(NULL);

    // Need an input vertex to provide a track propagation direction
    const Vertex *const pInputVertex = LArPfoHelper::GetVertex(pInputPfo);

    PandoraContentApi::Vertex::Parameters vtxParameters;
    vtxParameters.m_position    = pInputVertex->GetPosition();
    vtxParameters.m_vertexLabel = pInputVertex->GetVertexLabel();
    vtxParameters.m_vertexType  = pInputVertex->GetVertexType();

    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Vertex::Create(*this, vtxParameters, pOutputVertex));
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddToPfo(*this, pOutputPfo, pOutputVertex));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void AnalysisAlgorithm::RecursivelyAppendLArTrackHitEnergyMap(const ParticleFlowObject *const pPfo, 
                                                              LArAnalysisParticleHelper::LArTrackHitEnergyMap &trackHitEnergyMap, 
                                                              const LArAnalysisParticleHelper::TrackFitMap &trackFitMap) const
{
    const auto findIter = trackFitMap.find(pPfo);
    
    if (findIter != trackFitMap.end())
        trackHitEnergyMap.emplace(pPfo, this->AppendLArTrackHitEnergyMap(pPfo, findIter->second));
    
    for (const ParticleFlowObject *const pDaughterPfo : pPfo->GetDaughterPfoList())
        this->RecursivelyAppendLArTrackHitEnergyMap(pDaughterPfo, trackHitEnergyMap, trackFitMap);
}

//------------------------------------------------------------------------------------------------------------------------------------------

LArTrackHitEnergy::Vector AnalysisAlgorithm::AppendLArTrackHitEnergyMap(const ParticleFlowObject *const pPfo, 
                                                                     const ThreeDSlidingFitResult &trackFit) const
{
    // Get all hits and order them by projection along the track fit.
    const CaloHitList collectionPlaneHits = LArAnalysisParticleHelper::GetHitsOfType(pPfo, TPC_VIEW_W, false);
    
    const LArAnalysisParticleHelper::HitProjectionVector orderedHitProjections = LArAnalysisParticleHelper::OrderHitsByProjectionOnToTrackFit(
                                                                                                             collectionPlaneHits, trackFit);
    
    LArTrackHitEnergy::Vector trackHitEnergyVector;
    
    for (const LArAnalysisParticleHelper::HitProjectionPair &projectionPair : orderedHitProjections)
    {
        const CaloHit *const pCaloHit = projectionPair.first;
        const float coordinate        = projectionPair.second;
        const float uncorrectedEnergy = this->CaloHitToScaledEnergy(pCaloHit);
        
        const float threeDDistance    = LArAnalysisParticleHelper::CaloHitToThreeDDistance(this->GetPandora(), pCaloHit, trackFit);
        const float correctedEnergy   = this->RecombinationCorrectEnergy(uncorrectedEnergy, threeDDistance);
        
        trackHitEnergyVector.emplace_back(pCaloHit, coordinate, uncorrectedEnergy * 1000.f, correctedEnergy * 1000.f);
    }                                                                                       
    
    this->m_pBirksHitSelectionTool->Run(this, trackHitEnergyVector, this->m_uniquePlotIdentifier);

    // Also, anything sent negative or larger than some reasonable maximum clearly shouldn't be corrected.
    for (LArTrackHitEnergy &trackHitEnergy : trackHitEnergyVector)
    {
        if (trackHitEnergy.UncorrectedEnergy() >= this->m_birksFitBeta)
            trackHitEnergy.ApplyCorrection(false);
        
        if (trackHitEnergy.CorrectedEnergy() <= 0.f)
            trackHitEnergy.ApplyCorrection(false);
        
        if (trackHitEnergy.CorrectedEnergy() >= this->m_birksSelectionMaxdEdX)
            trackHitEnergy.ApplyCorrection(false);
    }
    
    return trackHitEnergyVector;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void AnalysisAlgorithm::RecursivelyAppendParticleTypeMap(const ParticleFlowObject *const pPfo,
                                                            LArAnalysisParticle::PfoTypeMap &pfoTypeMap, 
                                                            const LArAnalysisParticleHelper::LArTrackHitEnergyMap &trackHitEnergyMap,
                                                            const LArAnalysisParticleHelper::TrackFitMap &trackFitMap) const
{
    pfoTypeMap.emplace(pPfo, this->EstimateParticleType(pPfo, trackHitEnergyMap, trackFitMap));
    
    for (const ParticleFlowObject *const pDaughterPfo : pPfo->GetDaughterPfoList())
        RecursivelyAppendParticleTypeMap(pDaughterPfo, pfoTypeMap, trackHitEnergyMap, trackFitMap);
}

//------------------------------------------------------------------------------------------------------------------------------------------

LArAnalysisParticle::TYPE AnalysisAlgorithm::EstimateParticleType(const ParticleFlowObject *const pPfo,
                                                                  const LArAnalysisParticleHelper::LArTrackHitEnergyMap &trackHitEnergyMap,
                                                                  const LArAnalysisParticleHelper::TrackFitMap &trackFitMap) const
{
    if (!LArPfoHelper::IsTrack(pPfo))
        return LArAnalysisParticle::TYPE::SHOWER;
        
    const auto trackHitFindIter = trackHitEnergyMap.find(pPfo);
    
    if (trackHitFindIter == trackHitEnergyMap.end())
        return LArAnalysisParticle::TYPE::TRACK;
    
    const auto trackFitFindIter = trackFitMap.find(pPfo);
    
    if (trackFitFindIter == trackFitMap.end())
    {
        CERR("Found track hit energy object but not track fit - this should be impossible");
        return LArAnalysisParticle::TYPE::TRACK;
    }
    
    const float particleEnergy = this->EstimateTrackEnergyFromCharge(trackHitFindIter->second);
    const float particleRange = LArAnalysisParticleHelper::GetParticleRange(pPfo, trackFitFindIter->second);
    
    if (particleEnergy > 0.f && particleRange > 0.f)
    {
        this->m_tmvaTrackLength         = particleRange;
        this->m_tmvaAvgEnergyDeposition = particleEnergy / particleRange;
        
        const float bdtResponse = this->m_pTmvaReader->EvaluateMVA("BDT");
        
        if (bdtResponse > 0.14)
            return LArAnalysisParticle::TYPE::PROTON;

        if (bdtResponse < -0.14)
            return LArAnalysisParticle::TYPE::PION_MUON;
    }
    
    return LArAnalysisParticle::TYPE::TRACK;
}

//------------------------------------------------------------------------------------------------------------------------------------------
        
void AnalysisAlgorithm::EstimateParticleEnergy(const ParticleFlowObject *const pPfo, const LArAnalysisParticle::PfoTypeMap &typeMap,
    const LArAnalysisParticleHelper::TrackFitMap &trackFitMap, const LArAnalysisParticleHelper::LArTrackHitEnergyMap &trackHitEnergyMap, float &particleEnergy,
    float &particleEnergyFromCharge) const
{
    const LArAnalysisParticle::TYPE particleType = typeMap.at(pPfo);
    
    const auto fitFindIter       = trackFitMap.find(pPfo);
    const auto hitEnergyFindIter = trackHitEnergyMap.find(pPfo);
    
    const bool fitFound       = (fitFindIter != trackFitMap.end());
    const bool hitEnergyFound = (hitEnergyFindIter != trackHitEnergyMap.end());
    
    if (fitFound != hitEnergyFound)
        CERR("Fits should be found for tracks iff track hit energies are found");
    
    switch (particleType)
    {
        // For showers, we assume everything downstream of a showerlike PFO is part of the shower.
        case LArAnalysisParticle::TYPE::SHOWER:
        {
            const float showerEnergy = this->EstimateShowerEnergy(pPfo);
            particleEnergy += showerEnergy;
            particleEnergyFromCharge += showerEnergy;
        }
        
        case LArAnalysisParticle::TYPE::PION_MUON:
        case LArAnalysisParticle::TYPE::PROTON:   
        {
            const float energyFromCharge = this->EstimateEnergyFromCharge(pPfo);
            
            if (fitFound && hitEnergyFound)
                particleEnergy += this->EstimateTrackEnergyFromRange(pPfo, fitFindIter->second, particleType, hitEnergyFindIter->second);
                
            else
                particleEnergy += energyFromCharge; // not including recombination correction (need a fit)
                
            particleEnergyFromCharge += energyFromCharge;
                
            break;
        }
        
        case LArAnalysisParticle::TYPE::TRACK:
        {
            if (hitEnergyFound)
            {
                const float energyFromCharge = this->EstimateTrackEnergyFromCharge(hitEnergyFindIter->second); // incl recombination correction
                particleEnergy += energyFromCharge; 
                particleEnergyFromCharge += energyFromCharge;
            }
                
            else
            {
                const float energyFromCharge = this->EstimateEnergyFromCharge(pPfo); // not including recombination correction (need a fit)
                particleEnergy += energyFromCharge; 
                particleEnergyFromCharge += energyFromCharge;
            }
            
            break;
        }
        
        default: CERR("Unknown particle type - could not calculate energy"); break;
    }
    
    for (const ParticleFlowObject *const pDaughterPfo : pPfo->GetDaughterPfoList())
        this->EstimateParticleEnergy(pDaughterPfo, typeMap, trackFitMap, trackHitEnergyMap, particleEnergy, particleEnergyFromCharge);
}

//------------------------------------------------------------------------------------------------------------------------------------------
        
float AnalysisAlgorithm::EstimateShowerEnergy(const ParticleFlowObject *const pPfo) const
{
    float showerEnergy = this->EstimateEnergyFromCharge(pPfo);
    
    for (const ParticleFlowObject *const pDaughterPfo : pPfo->GetDaughterPfoList())
        showerEnergy += this->EstimateShowerEnergy(pDaughterPfo);
        
    return showerEnergy;
}

//------------------------------------------------------------------------------------------------------------------------------------------
        
float AnalysisAlgorithm::EstimateEnergyFromCharge(const ParticleFlowObject *const pPfo) const
{
    float energy = 0.f;
    
    for (const CaloHit *const pCaloHit : LArAnalysisParticleHelper::GetHitsOfType(pPfo, TPC_VIEW_W, false))
        energy += this->CaloHitToScaledEnergy(pCaloHit);
   
    return energy;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float AnalysisAlgorithm::CaloHitToScaledEnergy(const CaloHit *const pCaloHit) const
{
    const float dQ = pCaloHit->GetInputEnergy();
    return dQ / (this->m_birksFitAlpha * 1000.f); // GeV
}

//------------------------------------------------------------------------------------------------------------------------------------------

float AnalysisAlgorithm::RecombinationCorrectEnergy(const float scaledEnergy, const float threeDDistance) const
{
    const float dEdx_uncorrected = 1000.f * scaledEnergy / threeDDistance;                             // MeV / cm
    const float dEdx_corrected   = dEdx_uncorrected / (1.f - dEdx_uncorrected / this->m_birksFitBeta); // MeV / cm
    return dEdx_corrected * threeDDistance / 1000.f; // GeV
}

//------------------------------------------------------------------------------------------------------------------------------------------
    
float AnalysisAlgorithm::EstimateTrackEnergyFromCharge(const LArTrackHitEnergy::Vector &trackHitEnergyVector) const
{
    float trackEnergy = 0.f;
    
    for (const LArTrackHitEnergy &trackHitEnergy : trackHitEnergyVector)
    {
        trackEnergy += trackHitEnergy.ApplyCorrection() ?
                       trackHitEnergy.CorrectedEnergy() / 1000.f : trackHitEnergy.UncorrectedEnergy() / 1000.f;
    }
    
    return trackEnergy;
}

//------------------------------------------------------------------------------------------------------------------------------------------
        
float AnalysisAlgorithm::EstimateTrackEnergyFromRange(const ParticleFlowObject *const pPfo, const ThreeDSlidingFitResult &trackFit,
                                                         const LArAnalysisParticle::TYPE particleType,
                                                         const LArTrackHitEnergy::Vector &trackHitEnergyVector) const
{
    const float trackRange = LArAnalysisParticleHelper::GetParticleRange(pPfo, trackFit);
    float trackEnergy = 0.f;
    
    switch (particleType)
    {
        case LArAnalysisParticle::TYPE::PION_MUON:
        case LArAnalysisParticle::TYPE::PROTON:
        {
            bool foundEnergy = false;
            
            for (const EnergyFromRangeData &energyFromRangeData : (particleType == LArAnalysisParticle::TYPE::PROTON) ? 
                                                                  this->m_protonEnergyFromRangeDataVector : 
                                                                  this->m_pionMuonEnergyFromRangeDataVector)
            {
                if ((trackRange >= energyFromRangeData.m_rangeMin) && (trackRange < energyFromRangeData.m_rangeMax))
                {
                    trackEnergy = energyFromRangeData.m_energy;
                    foundEnergy = true;
                    break;
                } 
            }
            
            if (!foundEnergy)
                trackEnergy = this->EstimateTrackEnergyFromCharge(trackHitEnergyVector); // incl recombination correction
            
            break;
        }
        
        default:
        {
            CERR("Given a non-known-track particle to calculate energy from range");
            break;
        }
    }
    
    return trackEnergy;
}

//------------------------------------------------------------------------------------------------------------------------------------------
        
LArAnalysisParticle::TypeTree AnalysisAlgorithm::CreateTypeTree(const ParticleFlowObject *const pPfo, 
                                                                const LArAnalysisParticle::PfoTypeMap &typeMap) const
{
    LArAnalysisParticle::TypeTree::List daughterTypeTrees;
    
    for (const ParticleFlowObject *const pDaughterPfo : pPfo->GetDaughterPfoList())
        daughterTypeTrees.push_back(this->CreateTypeTree(pDaughterPfo, typeMap));
    
    return {typeMap.at(pPfo), daughterTypeTrees};
}

//------------------------------------------------------------------------------------------------------------------------------------------

CartesianVector AnalysisAlgorithm::GetDirectionAtVertex(const ParticleFlowObject *const pPfo, 
                                                           const LArAnalysisParticleHelper::TrackFitMap &trackFitMap, 
                                                           const Vertex *const pVertex) const
{
    const auto findIter = trackFitMap.find(pPfo);
    
    if (findIter != trackFitMap.end())
        return LArAnalysisParticleHelper::GetFittedDirectionAtPosition(findIter->second, pVertex->GetPosition());

    const CaloHitList all3DHits = LArAnalysisParticleHelper::GetHitsOfType(pPfo, TPC_3D, true);
    
    LArPcaHelper::EigenVectors eigenVectors;
    LArPcaHelper::EigenValues  eigenValues{0.f, 0.f, 0.f};
    CartesianVector            centroid{0.f, 0.f, 0.f};
    LArPcaHelper::RunPca(all3DHits, centroid, eigenValues, eigenVectors);
    
    if (eigenVectors.empty())
    {
        CERR("PCA eigenvectors were empty");
        return CartesianVector{0.f, 0.f, 0.f};
    }
    
    return eigenVectors.at(0);
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool AnalysisAlgorithm::GetMcInformation(const ParticleFlowObject *const pPfo, float &mcEnergy, LArAnalysisParticle::TypeTree &typeTree,
    LArAnalysisParticle::TYPE &mcType, CartesianVector &mcVertexPosition, CartesianVector &mcMomentum, int &mcPdgCode, const bool isNeutrino,
    float &mcContainmentFraction, const MCParticle * &pMcMainMCParticle, float &mcHitPurity, float &mcHitCompleteness) const
{
    if (isNeutrino)
    {
        const MCParticleList *pMCParticleList(nullptr);

        if ((PandoraContentApi::GetList(*this, this->m_mcParticleListName, pMCParticleList) != STATUS_CODE_SUCCESS) || !pMCParticleList)
        {
            std::cout << "AnalysisAlgorithm: could not get MC information because no valid MC particle list name was provided" << std::endl;
            return false;
        }
        
        MCParticleVector trueNeutrinos;
        LArMCParticleHelper::GetTrueNeutrinos(pMCParticleList, trueNeutrinos);
        
        
        if (trueNeutrinos.empty())
            return false;
            
        if (trueNeutrinos.size() > 1)
        {
            std::sort(trueNeutrinos.begin(), trueNeutrinos.end(), LArMCParticleHelper::SortByMomentum);
            std::cout << "AnalysisAlgorithm: more than one MC neutrino; picking one with largest momentum" << std::endl;
        }
        
        pMcMainMCParticle = trueNeutrinos.front();
    }
   
    else
        pMcMainMCParticle = LArAnalysisParticleHelper::GetMainMCParticle(pPfo);

    if (!pMcMainMCParticle)
        return false;
        
    const CaloHitList *pCaloHitList(nullptr);

    if ((PandoraContentApi::GetList(*this, this->m_caloHitListName, pCaloHitList) != STATUS_CODE_SUCCESS) || !pCaloHitList)
    {
        std::cout << "AnalysisAlgorithm: could not get MC information because no valid CaloHit list name was provided" << std::endl;
        return false;
    }
        
    this->CalculateHitPurityAndCompleteness(pPfo, pMcMainMCParticle, pCaloHitList, isNeutrino, mcHitPurity, mcHitCompleteness);
        
    return LArAnalysisParticleHelper::GetMcInformation(pMcMainMCParticle, mcEnergy, typeTree, mcType, mcVertexPosition, mcMomentum, 
        mcPdgCode, mcContainmentFraction, this->m_minCoordinates, this->m_maxCoordinates);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void AnalysisAlgorithm::CalculateHitPurityAndCompleteness(const ParticleFlowObject *const pPfo, const MCParticle *const pMCParticle, 
    const CaloHitList *const pCaloHitList, const bool isNeutrino, float &hitPurity, float &hitCompleteness) const
{
    // Purity       = (num 2D hits assoc with PFO or its descendents && assoc with MC particle or its descendents) / 
    //                (num 2D hits assoc with PFO or its descendents)
    
    // Completeness = (num 2D hits assoc with PFO or its descendents && assoc with MC particle or its descendents) / 
    //                (num 2D hits assoc with MC particle or its descendents)
    
    PfoList downstreamPfos;
    LArPfoHelper::GetAllDownstreamPfos(pPfo, downstreamPfos);
    
    CaloHitList pfoAssociatedCaloHits;
    LArPfoHelper::GetCaloHits(downstreamPfos, TPC_VIEW_U, pfoAssociatedCaloHits);
    LArPfoHelper::GetCaloHits(downstreamPfos, TPC_VIEW_V, pfoAssociatedCaloHits);
    LArPfoHelper::GetCaloHits(downstreamPfos, TPC_VIEW_W, pfoAssociatedCaloHits);
    
    std::unordered_map<const CaloHit *, float> mcAssociatedCaloHits;
    float totalMcHitWeight(0.f);
    
    for (const CaloHit *const pCaloHit : *pCaloHitList)
    {
        for (const MCParticleWeightMap::value_type &mapPair : pCaloHit->GetMCParticleWeightMap())
        {
            try
            {
                if ((isNeutrino && LArMCParticleHelper::IsBeamNeutrinoFinalState(mapPair.first)) ||
                    (!isNeutrino && LArMCParticleHelper::GetPrimaryMCParticle(mapPair.first) == pMCParticle))
                {
                    mcAssociatedCaloHits.emplace(pCaloHit, mapPair.second);
                    totalMcHitWeight += mapPair.second;
                }
            }
            
            catch (...)
            {
                continue;
            }
        }
    }
    
    float numerator(0.f);
    
    for (const CaloHit *const pPfoAssocCaloHit : pfoAssociatedCaloHits)
    {
        const auto findIter = mcAssociatedCaloHits.find(pPfoAssocCaloHit);
        
        if (findIter != mcAssociatedCaloHits.end())
            numerator += findIter->second;
    }
    
    if (pfoAssociatedCaloHits.empty() || (totalMcHitWeight < std::numeric_limits<float>::epsilon()))
    {
        hitPurity       = 0.f;
        hitCompleteness = 0.f;
        return;
    }
    
    hitPurity       = numerator / static_cast<float>(pfoAssociatedCaloHits.size());
    hitCompleteness = numerator / totalMcHitWeight;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void AnalysisAlgorithm::CountNumberOfDownstreamParticles(const ParticleFlowObject *const pPfo, unsigned &numberOfParticles) const
{
    for (const ParticleFlowObject *const pDaughterPfo : pPfo->GetDaughterPfoList())
    {
        ++numberOfParticles;
        this->CountNumberOfDownstreamParticles(pDaughterPfo, numberOfParticles);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode AnalysisAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "MCParticleListName", this->m_mcParticleListName));
    
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "FiducialCutXMargin", this->m_fiducialCutXMargin));
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "FiducialCutYMargin", this->m_fiducialCutYMargin));
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "FiducialCutZMargin", this->m_fiducialCutZMargin));
    
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "TrackSlidingFitWindow", this->m_trackSlidingFitWindow));
    
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "ParametersFile", this->m_parametersFile));
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "BirksFitNtupleName", this->m_birksFitNtupleName));
    
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "ProtonEnergyFromRangeNtupleName", this->m_protonEnergyFromRangeNtupleName));
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "PionMuonEnergyFromRangeNtupleName", this->m_pionMuonEnergyFromRangeNtupleName));
    
    
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "BirksSelectionMaxdEdX", this->m_birksSelectionMaxdEdX));
    
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "AddMcInformation", this->m_addMcInformation));
    
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "McContainmentFractionLowerBound", this->m_mcContainmentFractionLowerBound));
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "CaloHitListName", this->m_caloHitListName));
    
    TNtuple *const pBirksNtuple = LArAnalysisParticleHelper::LoadNTupleFromFile(this->m_parametersFile, this->m_birksFitNtupleName);
    
    if (!pBirksNtuple)
    {
        CERR("Failed to load Birks fit ntuple from file");
        return STATUS_CODE_NOT_FOUND;
    }
    
    if (pBirksNtuple->GetEntries() != 1)
    {
        CERR("Birks fit ntuple did not have exactly one entry: " << pBirksNtuple->GetEntries());
        return STATUS_CODE_NOT_FOUND;
    }
    
    float alpha = 0.f, beta = 0.f, dQdxPole = 0.f;
    
    pBirksNtuple->SetBranchAddress("Alpha",    &alpha);
    pBirksNtuple->SetBranchAddress("Beta",     &beta);
    pBirksNtuple->SetBranchAddress("dQdXPole", &dQdxPole);
    pBirksNtuple->GetEntry(0);
    
    this->m_birksFitAlpha = alpha;
    this->m_birksFitBeta  = beta;
    this->m_birksFitPole  = dQdxPole;
    
    //--------------------------------------------------------------------------------------------------------------------------------------
    
    TNtuple *const pEnergyFromRangeNtupleProton = LArAnalysisParticleHelper::LoadNTupleFromFile(this->m_parametersFile, this->m_protonEnergyFromRangeNtupleName);
    
    if (!pEnergyFromRangeNtupleProton)
    {
        CERR("Failed to load proton energy from range ntuple from file");
        return STATUS_CODE_NOT_FOUND;
    }
    
    const int nEfrProtonEntries = pEnergyFromRangeNtupleProton->GetEntries();
    
    float efrProtonRangeMin = 0.f, efrProtonRangeMax = 0.f, efrProtonEnergy = 0.f;
    
    pEnergyFromRangeNtupleProton->SetBranchAddress("RangeMin", &efrProtonRangeMin);
    pEnergyFromRangeNtupleProton->SetBranchAddress("RangeMax", &efrProtonRangeMax);
    pEnergyFromRangeNtupleProton->SetBranchAddress("Energy",   &efrProtonEnergy);
    
    for (int i = 0; i < nEfrProtonEntries; ++i)
    {
        pEnergyFromRangeNtupleProton->GetEntry(i);
        this->m_protonEnergyFromRangeDataVector.emplace_back(efrProtonRangeMin, efrProtonRangeMax, efrProtonEnergy);
    }
    
    //--------------------------------------------------------------------------------------------------------------------------------------
    
    TNtuple *const pEnergyFromRangeNtuplePionMuon = LArAnalysisParticleHelper::LoadNTupleFromFile(this->m_parametersFile, this->m_pionMuonEnergyFromRangeNtupleName);
    
    if (!pEnergyFromRangeNtuplePionMuon)
    {
        CERR("Failed to load proton energy from range ntuple from file");
        return STATUS_CODE_NOT_FOUND;
    }
    
    const int nEfrPionMuonEntries = pEnergyFromRangeNtuplePionMuon->GetEntries();
    
    float efrPionMuonRangeMin = 0.f, efrPionMuonRangeMax = 0.f, efrPionMuonEnergy = 0.f;
    
    pEnergyFromRangeNtuplePionMuon->SetBranchAddress("RangeMin", &efrPionMuonRangeMin);
    pEnergyFromRangeNtuplePionMuon->SetBranchAddress("RangeMax", &efrPionMuonRangeMax);
    pEnergyFromRangeNtuplePionMuon->SetBranchAddress("Energy",   &efrPionMuonEnergy);
    
    for (int i = 0; i < nEfrPionMuonEntries; ++i)
    {
        pEnergyFromRangeNtuplePionMuon->GetEntry(i);
        this->m_pionMuonEnergyFromRangeDataVector.emplace_back(efrPionMuonRangeMin, efrPionMuonRangeMax, efrPionMuonEnergy);
    }
    
    TMVA::Tools::Instance();
    this->m_pTmvaReader = new TMVA::Reader("!Color:Silent");    
    this->m_pTmvaReader->AddVariable("TrackLength", &this->m_tmvaTrackLength);
    this->m_pTmvaReader->AddVariable("AvgEnergyDeposition := TrueEnergy/TrackLength", &this->m_tmvaAvgEnergyDeposition);
    this->m_pTmvaReader->BookMVA("BDT", "/Users/jaw/Dropbox/PhD/weights/TMVAClassification_BDT.weights.xml"); 
        
    // Use the detector geometry and the margins to get the maximum and minimum fiducial volume coordinates.
    LArAnalysisParticleHelper::GetFiducialCutParameters(this->GetPandora(), this->m_fiducialCutXMargin, this->m_fiducialCutYMargin,
        this->m_fiducialCutZMargin, this->m_minCoordinates, this->m_maxCoordinates);
    
    AlgorithmTool *pAlgorithmTool(nullptr);
    
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ProcessAlgorithmTool(*this, xmlHandle,
        "BirksHitSelection", pAlgorithmTool));
        
    if (!(this->m_pBirksHitSelectionTool = dynamic_cast<BirksHitSelectionTool *>(pAlgorithmTool)))
        throw STATUS_CODE_FAILURE;

    return CustomParticleCreationAlgorithm::ReadSettings(xmlHandle);
}
} // namespace lar_physics_content
