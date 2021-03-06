/**
 *  @file   larphysicscontent/LArAnalysis/EventValidationNtupleTool.h
 *
 *  @brief  Header file for the event validation ntuple tool class.
 *
 *  $Log: $
 */
#ifndef LAR_EVENT_VALIDATION_NTUPLE_TOOL_H
#define LAR_EVENT_VALIDATION_NTUPLE_TOOL_H 1

#include "larphysicscontent/LArNtuple/NtupleVariableBaseTool.h"

namespace lar_physics_content
{
/**
 *  @brief  EventValidationNtupleTool class
 */
class EventValidationNtupleTool : public NtupleVariableBaseTool
{
public:
    /**
     *  @brief  Default constructor
     */
    EventValidationNtupleTool();

    /**
     *  @brief  Default copy constructor
     */
    EventValidationNtupleTool(const EventValidationNtupleTool &) = default;

    /**
     *  @brief  Default move constructor
     */
    EventValidationNtupleTool(EventValidationNtupleTool &&) = default;

    /**
     *  @brief  Default copy assignment operator
     */
    EventValidationNtupleTool &operator=(const EventValidationNtupleTool &) = default;

    /**
     *  @brief  Default move assignment operator
     */
    EventValidationNtupleTool &operator=(EventValidationNtupleTool &&) = default;

    /**
     *  @brief  Default destructor
     */
    ~EventValidationNtupleTool() = default;

protected:
    void PrepareEvent(const pandora::PfoList &pfoList, const std::vector<std::shared_ptr<LArInteractionValidationInfo>> &eventValidationInfo) override;

    std::vector<LArNtupleRecord> ProcessEvent(
        const pandora::PfoList &pfoList, const std::vector<std::shared_ptr<LArInteractionValidationInfo>> &eventValidationInfo) override;

    std::vector<LArNtupleRecord> ProcessNeutrino(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const std::shared_ptr<LArInteractionValidationInfo> &spInteractionInfo) override;

    std::vector<LArNtupleRecord> ProcessCosmicRay(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const std::shared_ptr<LArMCTargetValidationInfo> &spMcTarget) override;

    std::vector<LArNtupleRecord> ProcessPrimary(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const std::shared_ptr<LArMCTargetValidationInfo> &spMcTarget) override;

private:
    typedef std::unordered_map<const pandora::ParticleFlowObject *, unsigned int> PfoToIdMap;

    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    /**
     *  @brief  Write interaction records
     * 
     *  @param  spMcInteraction shared pointer to the MC interaction
     * 
     *  @return the records
     */
    std::vector<LArNtupleRecord> WriteInteractionRecords(const std::shared_ptr<LArInteractionValidationInfo> &spMcInteraction) const;

    /**
     *  @brief  Write match records
     * 
     *  @param  spMcTarget shared pointer to the MC target
     * 
     *  @return the records
     */
    std::vector<LArNtupleRecord> WriteMatchRecords(const pandora::ParticleFlowObject *const pPfo, const std::shared_ptr<LArMCTargetValidationInfo> &spMcTarget) const;
};

} // namespace lar_physics_content

#endif // #ifndef LAR_EVENT_VALIDATION_NTUPLE_TOOL_H
