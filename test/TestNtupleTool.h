/**
 *  @file   test/TestNtupleTool.h
 *
 *  @brief  Header file for the test ntuple tool class.
 *
 *  $Log: $
 */
#ifndef LAR_TEST_NTUPLE_TOOL_H
#define LAR_TEST_NTUPLE_TOOL_H 1

#include "larphysicscontent/LArNtuple/NtupleVariableBaseTool.h"

namespace lar_physics_content
{
/**
 *  @brief  TestTool class
 */
class TestNtupleTool : public NtupleVariableBaseTool
{
public:
    /**
     *  @brief  Default constructor
     */
    TestNtupleTool();

    /**
     *  @brief  Default copy constructor
     */
    TestNtupleTool(const TestNtupleTool &) = default;

    /**
     *  @brief  Default move constructor
     */
    TestNtupleTool(TestNtupleTool &&) = default;

    /**
     *  @brief  Default copy assignment operator
     */
    TestNtupleTool &operator=(const TestNtupleTool &) = default;

    /**
     *  @brief  Default move assignment operator
     */
    TestNtupleTool &operator=(TestNtupleTool &&) = default;

    /**
     *  @brief  Default destructor
     */
    ~TestNtupleTool() = default;

private:
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    int m_eventCounter;    ///< The event counter
    int m_neutrinoCounter; ///< The neutrino counter
    int m_cosmicCounter;   ///< The cosmic counter
    int m_primaryCounter;  ///< The primary counter

    std::vector<LArNtupleRecord> ProcessEvent(
        const pandora::PfoList &pfoList, const std::vector<std::shared_ptr<LArInteractionValidationInfo>> &eventValidationInfo) override;

    std::vector<LArNtupleRecord> ProcessNeutrino(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const std::shared_ptr<LArInteractionValidationInfo> &spInteractionInfo) override;

    std::vector<LArNtupleRecord> ProcessCosmicRay(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const std::shared_ptr<LArMCTargetValidationInfo> &spMcTarget) override;

    std::vector<LArNtupleRecord> ProcessPrimary(const pandora::ParticleFlowObject *const pPfo, const pandora::PfoList &pfoList,
        const std::shared_ptr<LArMCTargetValidationInfo> &spMcTarget) override;

    /**
     *  @brief  Get the set of test records for a given counter
     *
     *  @param  counter the counter value
     *
     *  @return the test records
     */
    std::vector<LArNtupleRecord> GetTestRecords(const int counter) const;
};

} // namespace lar_physics_content

#endif // #ifndef LAR_TEST_NTUPLE_TOOL_H
