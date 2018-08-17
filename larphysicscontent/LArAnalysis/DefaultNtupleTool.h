/**
 *  @file   larphysicscontent/LArAnalysis/DefaultNtupleTool.h
 *
 *  @brief  Header file for the default ntuple tool class.
 *
 *  $Log: $
 */
#ifndef LAR_DEFAULT_NTUPLE_TOOL_H
#define LAR_DEFAULT_NTUPLE_TOOL_H 1

#include "larphysicscontent/LArNtuple/NtupleVariableBaseTool.h"

namespace lar_physics_content
{
/**
 *  @brief  DefaultNtupleTool class
 */
class DefaultNtupleTool : public NtupleVariableBaseTool
{
public:
    /**
     *  @brief  Default constructor
     */
    DefaultNtupleTool();

    /**
     *  @brief  Default copy constructor
     */
    DefaultNtupleTool(const DefaultNtupleTool &) = default;

    /**
     *  @brief  Default move constructor
     */
    DefaultNtupleTool(DefaultNtupleTool &&) = default;

    /**
     *  @brief  Default copy assignment operator
     */
    DefaultNtupleTool &operator=(const DefaultNtupleTool &) = default;

    /**
     *  @brief  Default move assignment operator
     */
    DefaultNtupleTool &operator=(DefaultNtupleTool &&) = default;

    /**
     *  @brief  Default destructor
     */
    ~DefaultNtupleTool() = default;

protected:
    std::vector<LArNtupleRecord> ProcessEvent(const pandora::PfoList &pfoList, const pandora::MCParticleList *const pMCParticleList) override;

    std::vector<LArNtupleRecord> ProcessNeutrino(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const pandora::MCParticle *const pMCParticle, const pandora::MCParticleList *const pMCParticleList) override;

    std::vector<LArNtupleRecord> ProcessCosmicRay(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const pandora::MCParticle *const pMCParticle, const pandora::MCParticleList *const pMCParticleList) override;

    std::vector<LArNtupleRecord> ProcessPrimary(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const pandora::MCParticle *const pMCParticle, const pandora::MCParticleList *const pMCParticleList) override;

private:
    pandora::CartesianVector m_fiducialCutLowMargins;  ///< The low-coordinate margins for the fiducial cut
    pandora::CartesianVector m_fiducialCutHighMargins; ///< The high-coordinate margins for the fiducial cut
    pandora::CartesianVector m_minFiducialCoordinates; ///< The minimum fiducial coordinates
    pandora::CartesianVector m_maxFiducialCoordinates; ///< The maximum fiducial coordinates

    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    /**
     *  @brief  Produce records generic to every considered particle class
     *
     *  @param  pPfo optional address of the PFO
     *  @param  pfoList the list of all PFOs
     *
     *  @return the records
     */
    std::vector<LArNtupleRecord> ProduceGenericPfoRecords(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList) const;

    /**
     *  @brief  Produce MC records generic to every considered particle class
     *
     *  @param  pPfo optional address of the PFO
     *  @param  pfoList the list of all PFOs
     *  @param  pMCParticle optional address of the MC particle
     *  @param  pMCParticleList optional list of all MC particles
     *
     *  @return the records
     */
    std::vector<LArNtupleRecord> ProduceGenericPfoMCRecords(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const pandora::MCParticle *const pMCParticle, const pandora::MCParticleList *const pMCParticleList) const;

    /**
     *  @brief  Get the fraction of fiducial 3D hits
     *
     *  @param  pPfo address of the PFO
     *
     *  @return the fraction of hits
     */
    float GetFractionOfFiducialThreeDHits(const pandora::ParticleFlowObject *const pPfo) const;
};

} // namespace lar_physics_content

#endif // #ifndef LAR_DEFAULT_NTUPLE_TOOL_H
