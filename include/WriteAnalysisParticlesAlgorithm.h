/**
 *  @file   LArPhysicsContent/include/WriteAnalysisParticlesAlgorithm.h
 *
 *  @brief  Header file for the write AnalysisParticles algorithm class.
 *
 *  $Log: $
 */
#ifndef LAR_WRITE_ANALYSIS_PARTICLES_ALGORITHM_H
#define LAR_WRITE_ANALYSIS_PARTICLES_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

#include "LArAnalysisParticle.h"
#include "larpandoracontent/LArHelpers/LArInteractionTypeHelper.h"

#include "TFile.h"
#include "TTree.h"

using namespace pandora;
using namespace lar_content;

namespace lar_physics_content
{

/**
 *  @brief  TreeParameters class
 */
class TreeParameters
{
public:
    using BoolVector     = std::vector<bool>;           ///< Alias for a vector of bools
    using UnsignedVector = std::vector<unsigned>;      ///< Alias for a vector of unsigned ints
    using UInt64Vector   = std::vector<std::uint64_t>; ///< Alias for a vector of unsigned 64-bit ints
    
    /**
     *  @brief  Constructor
     */
    TreeParameters() noexcept;

    bool              m_nu_WasReconstructed;                    ///< Whether the neutrino has been reconstructed
    bool              m_nu_IsVertexFiducial;                    ///< Whether the neutrino vertex is fiducial
    bool              m_nu_AreAllHitsFiducial;                  ///< Whether all the neutrino hits are fiducial
    bool              m_nu_HasMcInfo;                           ///< Whether the neutrino has MC information
    float             m_nu_Energy;                              ///< The neutrino energy
    float             m_nu_EnergyFromChargeOnly;                ///< The neutrino energy using only charge
    float             m_nu_LongitudinalEnergy;                  ///< The longitudinal neutrino energy
    float             m_nu_TransverseEnergy;                    ///< The transverse neutrino energy
    float             m_nu_VertexX;                             ///< The x-component of the neutrino vertex position
    float             m_nu_VertexY;                             ///< The y-component of the neutrino vertex position
    float             m_nu_VertexZ;                             ///< The z-component of the neutrino vertex position
    float             m_nu_DirectionCosineX;                    ///< The direction cosine of the neutrino in the x-direction
    float             m_nu_DirectionCosineY;                    ///< The direction cosine of the neutrino in the y-direction
    float             m_nu_DirectionCosineZ;                    ///< The direction cosine of the neutrino in the z-direction
    float             m_nu_MomentumX;                           ///< The momentum of the neutrino in the x-direction
    float             m_nu_MomentumY;                           ///< The momentum of the neutrino in the y-direction
    float             m_nu_MomentumZ;                           ///< The momentum of the neutrino in the z-direction
    unsigned          m_nu_NumberOf3dHits;                      ///< The number of 3D hits in the neutrino
    unsigned          m_nu_NumberOfCollectionPlaneHits;         ///< The number of collection-plane hits in the neutrino
    unsigned          m_nu_NumberOfDownstreamParticles;         ///< The number of particles downstream of the neutrino
    std::uint64_t     m_nu_mc_McParticleUid;                    ///< The UID of the MC particle
    float             m_nu_mc_Energy;                           ///< The MC energy of the neutrino
    float             m_nu_mc_LongitudinalEnergy;               ///< The MC longitudinal energy of the neutrino
    float             m_nu_mc_TransverseEnergy;                 ///< The MC transverse energy of the neutrino
    float             m_nu_mc_VisibleLongitudinalEnergy;        ///< The MC visible longitudinal energy of the neutrino
    float             m_nu_mc_VisibleTransverseEnergy;          ///< The MC visible transverse energy of the neutrino
    float             m_nu_mc_VertexX;                          ///< The MC x-component of the neutrino vertex position
    float             m_nu_mc_VertexY;                          ///< The MC y-component of the neutrino vertex position
    float             m_nu_mc_VertexZ;                          ///< The MC x-component of the neutrino vertex position
    float             m_nu_mc_DirectionCosineX;                 ///< The MC direction cosine of the neutrino in the x-direction
    float             m_nu_mc_DirectionCosineY;                 ///< The MC direction cosine of the neutrino in the y-direction
    float             m_nu_mc_DirectionCosineZ;                 ///< The MC direction cosine of the neutrino in the z-direction
    float             m_nu_mc_MomentumX;                        ///< The MC momentum of the neutrino in the x-direction
    float             m_nu_mc_MomentumY;                        ///< The MC momentum of the neutrino in the y-direction
    float             m_nu_mc_MomentumZ;                        ///< The MC momentum of the neutrino in the z-direction
    bool              m_nu_mc_IsVertexFiducial;                 ///< Whether the neutrino vertex is fiducial (MC quantity)
    bool              m_nu_mc_IsContained;                      ///< Whether the neutrino is contained (MC quantity)
    int               m_nu_mc_InteractionType;                  ///< The neutrino interaction type (MC quantity)
    bool              m_nu_mc_IsChargedCurrent;                 ///< Whether the interaction is charged-current (MC quantity)
    float             m_nu_mc_VisibleEnergyFraction;            ///< The fraction of the neutrino's energy that is visible (MC quantity)
    int               m_nu_mc_PdgCode;                          ///< The neutrino's PDG code (MC quantity)
                                                                
    unsigned          m_primary_Number;                         ///< The number of primary daughters
    BoolVector        m_primary_WasReconstructed;               ///< Whether each primary daughter's vertex has been reconstructed
    BoolVector        m_primary_IsVertexFiducial;               ///< Whether each primary daughter's vertex is fiducial
    BoolVector        m_primary_AreAllHitsFiducial;             ///< Whether each primary's hits are all fiducial
    BoolVector        m_primary_HasMcInfo;                      ///< Whether each primary daughter has MC information
    FloatVector       m_primary_Energy;                         ///< The energy of each primary daughter
    FloatVector       m_primary_EnergyFromChargeOnly;           ///< The energy of each primary daughter from charge only
    FloatVector       m_primary_VertexX;                        ///< The x-component of each primary daughter's vertex position 
    FloatVector       m_primary_VertexY;                        ///< The y-component of each primary daughter's vertex position 
    FloatVector       m_primary_VertexZ;                        ///< The z-component of each primary daughter's vertex position 
    FloatVector       m_primary_DirectionCosineX;               ///< The direction cosine of each primary daughter in the x-direction
    FloatVector       m_primary_DirectionCosineY;               ///< The direction cosine of each primary daughter in the y-direction
    FloatVector       m_primary_DirectionCosineZ;               ///< The direction cosine of each primary daughter in the z-direction
    FloatVector       m_primary_MomentumX;                      ///< The momentum of each primary daughter in the x-direction
    FloatVector       m_primary_MomentumY;                      ///< The momentum of each primary daughter in the y-direction
    FloatVector       m_primary_MomentumZ;                      ///< The momentum of each primary daughter in the z-direction
    IntVector         m_primary_ParticleType;                   ///< The type enum of each primary daughter
    UnsignedVector    m_primary_NumberOf3dHits;                 ///< The number of 3D hits in each primary daughter
    UnsignedVector    m_primary_NumberOfCollectionPlaneHits;    ///< The number of collection-plane hits in each primary daughter
    BoolVector        m_primary_IsShower;                       ///< Whether each primary daughter is a shower
    UnsignedVector    m_primary_NumberOfDownstreamParticles;    ///< The number of particles downstream of each primary daughter
    UInt64Vector      m_primary_mc_McParticleUid;               ///< The UID of the MC particle corresponding to each primary
    FloatVector       m_primary_mc_Energy;                      ///< The MC energy of each primary daughter
    FloatVector       m_primary_mc_VertexX;                     ///< The MC x-component of each primary daughter's vertex position 
    FloatVector       m_primary_mc_VertexY;                     ///< The MC y-component of each primary daughter's vertex position 
    FloatVector       m_primary_mc_VertexZ;                     ///< The MC z-component of each primary daughter's vertex position 
    FloatVector       m_primary_mc_DirectionCosineX;            ///< The MC direction cosine of each primary daughter in the x-direction
    FloatVector       m_primary_mc_DirectionCosineY;            ///< The MC direction cosine of each primary daughter in the y-direction
    FloatVector       m_primary_mc_DirectionCosineZ;            ///< The MC direction cosine of each primary daughter in the z-direction
    FloatVector       m_primary_mc_MomentumX;                   ///< The MC momentum of each primary daughter in the x-direction
    FloatVector       m_primary_mc_MomentumY;                   ///< The MC momentum of each primary daughter in the y-direction
    FloatVector       m_primary_mc_MomentumZ;                   ///< The MC momentum of each primary daughter in the z-direction
    BoolVector        m_primary_mc_IsVertexFiducial;            ///< Whether each primary daughter's vertex is fiducial (MC quantity)
    BoolVector        m_primary_mc_IsContained;                 ///< Whether each primary daughter is contained (MC quantity)
    IntVector         m_primary_mc_ParticleType;                ///< The MC type enum for each primary daughter
    BoolVector        m_primary_mc_IsShower;                    ///< Whether each primary daughter is a shower (MC quantity)
    IntVector         m_primary_mc_PdgCode;                     ///< The primary daughter's PDG code (MC quantity)
                                                                
    unsigned          m_cr_Number;                              ///< The number of cosmic rays
    BoolVector        m_cr_WasReconstructed;                     ///< Whether each cosmic ray's vertex has been reconstructed
    BoolVector        m_cr_IsVertexFiducial;                    ///< Whether each cosmic ray's vertex is fiducial
    BoolVector        m_cr_AreAllHitsFiducial;                  ///< Whether each cosmic ray's hits are all fiducial
    BoolVector        m_cr_HasMcInfo;                           ///< Whether each cosmic ray has MC information
    FloatVector       m_cr_Energy;                              ///< The energy of each cosmic ray
    FloatVector       m_cr_EnergyFromChargeOnly;                ///< The energy of each cosmic ray from charge only
    FloatVector       m_cr_VertexX;                             ///< The x-component of each cosmic ray's vertex position 
    FloatVector       m_cr_VertexY;                             ///< The y-component of each cosmic ray's vertex position 
    FloatVector       m_cr_VertexZ;                             ///< The z-component of each cosmic ray's vertex position 
    FloatVector       m_cr_DirectionCosineX;                    ///< The direction cosine of each cosmic ray in the x-direction
    FloatVector       m_cr_DirectionCosineY;                    ///< The direction cosine of each cosmic ray in the y-direction
    FloatVector       m_cr_DirectionCosineZ;                    ///< The direction cosine of each cosmic ray in the z-direction
    FloatVector       m_cr_MomentumX;                           ///< The momentum of each cosmic ray in the x-direction
    FloatVector       m_cr_MomentumY;                           ///< The momentum of each cosmic ray in the y-direction
    FloatVector       m_cr_MomentumZ;                           ///< The momentum of each cosmic ray in the z-direction
    UnsignedVector    m_cr_NumberOf3dHits;                      ///< The number of 3D hits in each cosmic ray
    UnsignedVector    m_cr_NumberOfCollectionPlaneHits;         ///< The number of collection-plane hits in each cosmic ray
    UnsignedVector    m_cr_NumberOfDownstreamParticles;         ///< The number of particles downstream of each cosmic ray
    UInt64Vector      m_cr_mc_McParticleUid;                    ///< The UID of the MC particle corresponding to each cosmic ray
    FloatVector       m_cr_mc_Energy;                           ///< The MC energy of each cosmic ray
    FloatVector       m_cr_mc_VertexX;                          ///< The MC x-component of each cosmic ray's vertex position
    FloatVector       m_cr_mc_VertexY;                          ///< The MC y-component of each cosmic ray's vertex position
    FloatVector       m_cr_mc_VertexZ;                          ///< The MC z-component of each cosmic ray's vertex position
    FloatVector       m_cr_mc_DirectionCosineX;                 ///< The MC direction cosine of each cosmic ray in the x-direction
    FloatVector       m_cr_mc_DirectionCosineY;                 ///< The MC direction cosine of each cosmic ray in the y-direction
    FloatVector       m_cr_mc_DirectionCosineZ;                 ///< The MC direction cosine of each cosmic ray in the z-direction
    FloatVector       m_cr_mc_MomentumX;                        ///< The MC momentum of each cosmic ray in the x-direction
    FloatVector       m_cr_mc_MomentumY;                        ///< The MC momentum of each cosmic ray in the y-direction
    FloatVector       m_cr_mc_MomentumZ;                        ///< The MC momentum of each cosmic ray in the z-direction
    BoolVector        m_cr_mc_IsVertexFiducial;                 ///< Whether each cosmic ray's vertex is fiducial (MC quantity)
    BoolVector        m_cr_mc_IsContained;                      ///< Whether each cosmic ray is contained (MC quantity)
};

//------------------------------------------------------------------------------------------------------------------------------------------

/**
 *  @brief  WriteAnalysisParticlesAlgorithm class
 * 
 */
class WriteAnalysisParticlesAlgorithm : public Algorithm
{
public:
    /**
     *  @brief  Constructor
     */
    WriteAnalysisParticlesAlgorithm();
    
    /**
     *  @brief  Destructor
     */
    ~WriteAnalysisParticlesAlgorithm();

protected:
    StatusCode ReadSettings(const TiXmlHandle xmlHandle);
    StatusCode Run();
    
private:
    using AnalysisParticleList = std::list<const LArAnalysisParticle *>; ///< Alias for a list of AnalysisParticles

    /**
     *  @brief  Populate the tree parameters with neutrino information
     * 
     *  @param  neutrinoAnalysisParticle the neutrino analysis particle
     *  @param  ...
     */
    void PopulateNeutrinoParameters(const LArAnalysisParticle &neutrinoAnalysisParticle, const MCParticleList *const pMCParticleList,
        const CaloHitList *const pCaloHitList) const;
    
    /**
     *  @brief  ...
     */
    void PopulateNeutrinoMcParameters(const MCParticle *const pMainMcParticle, const float mcEnergy, const CartesianVector &mcVertexPosition, 
        const CartesianVector &mcDirectionCosines, const CartesianVector &mcMomentum, const bool mcIsVertexFiducial, 
        const bool mcIsContained, const int mcPdgCode, const MCParticleList *const pMCParticleList, const CaloHitList *const pCaloHitList) const;
    
    /**
     *  @brief  ...
     */
    LArInteractionTypeHelper::InteractionType GetInteractionType(const MCParticleList *const pMCParticleList,
        const CaloHitList *const pCaloHitList) const;
    
    /**
     *  @brief  Add a primary daughter record to the tree parameters
     * 
     *  @param  analysisParticle the primary daughter analysis particle
     */
    void AddPrimaryDaughterRecord(const LArAnalysisParticle &primaryAnalysisParticle) const;
    
    /**
     *  @brief  ...
     */
    void AddMcOnlyPrimaryDaughterRecord(const MCParticle *const pMainMcParticle, const float mcEnergy, 
        const CartesianVector &mcVertexPosition, const CartesianVector &mcDirectionCosines, const CartesianVector &mcMomentum,
        const bool mcIsVertexFiducial, const bool mcIsContained, const LArAnalysisParticle::TYPE mcType, const bool mcIsShower, 
        const int mcPdgCode) const;
        
    /**
     *  @brief  Add a cosmic ray record to the tree parameters
     * 
     *  @param  cosmicRayAnalysisParticle the cosmic ray analysis particle
     */
    void AddCosmicRayRecord(const LArAnalysisParticle &cosmicRayAnalysisParticle) const;
    
    /**
     *  @brief  ...
     */
    void AddMcOnlyCosmicRayRecord(const MCParticle *const pMainMcParticle, const float mcEnergy, const CartesianVector &mcVertexPosition,
        const CartesianVector &mcDirectionCosines, const CartesianVector &mcMomentum, const bool mcIsVertexFiducial, 
        const bool mcIsContained) const;
    
    /**
     *  @brief  Dump the tree parameters
     */
    void DumpTree() const;
        
    /**
     *  @brief  Find out whether a given interaction type is charged-current
     * 
     *  @param  interactionType the interaction type enum
     * 
     *  @return whether the interaction type is charged-current
     */
    bool IsChargedCurrent(const LArInteractionTypeHelper::InteractionType interactionType) const;

    std::string               m_pfoListName;       ///< The neutrino PFO list name
    std::string               m_outputFile;        ///< The output file path
    TFile                    *m_pOutputTFile;      ///< The ROOT TFile associated with the tree
    TTree                    *m_pOutputTree;       ///< The ROOT TTree to which to write the data
    bool                      m_verbose;           ///< Whether to print some AnalysisParticle information to screen
    mutable TreeParameters    m_treeParameters;    ///< The tree parameters
    std::string               m_mcParticleListName; ///< ...
    std::string               m_caloHitListName; ///< ...
    float         m_fiducialCutXMargin;                 ///< 
    float         m_fiducialCutYMargin;                 ///< 
    float         m_fiducialCutZMargin;                 ///< 
    CartesianVector m_minCoordinates;
    CartesianVector m_maxCoordinates;
    float m_mcContainmentFractionLowerBound;
};

} // namespace lar_physics_content

#endif // #ifndef LAR_WRITE_ANALYSIS_PARTICLES_ALGORITHM_H
