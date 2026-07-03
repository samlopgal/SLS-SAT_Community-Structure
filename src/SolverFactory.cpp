// SolverFactory.cpp

#include "SolverFactory.h"

#include <stdexcept>

// Wrappers
#include "BreakCountSolver.h"
#include "MakeCountSolver.h"
#include "BridgeBreakSolver.h"
#include "BridgeMakeBreakSolver.h"
#include "CommunityBreakSolver.h"
#include "CommunityMakeBreakSolver.h"


#include "BreakCountPAWSSolver_DET.h"
#include "MakeCountPAWSSolver_DET.h"

#include "BridgeBreakPAWSSolver_DET.h"
#include "BridgeMakeBreakPAWSSolver_DET.h"

#include "CommunityBreakPAWSSolver_DET.h"
#include "CommunityMakeBreakPAWSSolver_DET.h"

std::unique_ptr<ISolver> SolverFactory::create(
    const std::string& solver_name,
    const std::vector<std::vector<int>>& formula,
    int num_variables,
    const SolverParameters& params,
    const PrecomputedCommunityData* precomputed
)
{
    if (solver_name == "BreakCount") {
        return std::make_unique<BreakCountSolver>(
            formula,
            num_variables,
            params.seed
        );
    }



    if (solver_name == "MakeCount") {
        return std::make_unique<MakeCountSolver>(
            formula,
            num_variables,
            params.seed
        );
    }



    if (solver_name == "BridgeBreak") {

        if (precomputed == nullptr) {
            throw std::runtime_error(
                "BridgeBreak requires PrecomputedCommunityData."
            );
        }

        return std::make_unique<BridgeBreakSolver>(
            formula,
            num_variables,
            params.seed,
            precomputed
        );
    }

    if (solver_name == "BridgeMakeBreak") {

        if (precomputed == nullptr) {
            throw std::runtime_error(
                "BridgeMakeBreak requires PrecomputedCommunityData."
            );
        }

        return std::make_unique<BridgeMakeBreakSolver>(
            formula,
            num_variables,
            params.seed,
            precomputed
        );
    }

    if (solver_name == "CommunityBreak") {

        if (precomputed == nullptr) {
            throw std::runtime_error(
                "CommunityBreak requires PrecomputedCommunityData."
            );
        }

        return std::make_unique<CommunityBreakSolver>(
            formula,
            num_variables,
            params.seed,
            precomputed
        );
    }

    if (solver_name == "CommunityMakeBreak") {

        if (precomputed == nullptr) {
            throw std::runtime_error(
                "CommunityMakeBreak requires PrecomputedCommunityData."
            );
        }

        return std::make_unique<CommunityMakeBreakSolver>(
            formula,
            num_variables,
            params.seed,
            precomputed
        );
    }


    if (solver_name == "PAWS_Break_DET") {
        return std::make_unique<BreakCountPAWSSolver_DET>(
            formula,
            num_variables,
            params.seed,
            params.maxinc
        );
    }

    if (solver_name == "PAWS_Make_DET") {
        return std::make_unique<MakeCountPAWSSolver_DET>(
            formula,
            num_variables,
            params.seed,
            params.maxinc
        );
    }

    if (solver_name == "BridgeBreakPAWS_DET") {
        if (precomputed == nullptr) {
            throw std::runtime_error(
                "BridgeBreakPAWS_DET requires PrecomputedCommunityData."
            );
        }

        return std::make_unique<BridgeBreakPAWSSolver_DET>(
            formula,
            num_variables,
            params.seed,
            precomputed,
            params.maxinc
        );
    }

    if (solver_name == "BridgeMakeBreakPAWS_DET") {
        if (precomputed == nullptr) {
            throw std::runtime_error(
                "BridgeMakeBreakPAWS_DET requires PrecomputedCommunityData."
            );
        }

        return std::make_unique<BridgeMakeBreakPAWSSolver_DET>(
            formula,
            num_variables,
            params.seed,
            precomputed,
            params.maxinc
        );
    }

    if (solver_name == "CommunityBreakPAWS_DET") {
        if (precomputed == nullptr) {
            throw std::runtime_error(
                "CommunityBreakPAWS_DET requires PrecomputedCommunityData."
            );
        }

        return std::make_unique<CommunityBreakPAWSSolver_DET>(
            formula,
            num_variables,
            params.seed,
            precomputed,
            params.maxinc
        );
    }

    if (solver_name == "CommunityMakeBreakPAWS_DET") {
        if (precomputed == nullptr) {
            throw std::runtime_error(
                "CommunityMakeBreakPAWS_DET requires PrecomputedCommunityData."
            );
        }

        return std::make_unique<CommunityMakeBreakPAWSSolver_DET>(
            formula,
            num_variables,
            params.seed,
            precomputed,
            params.maxinc
        );
    }





    throw std::runtime_error(
        "Unknown solver: " + solver_name
    );
}
