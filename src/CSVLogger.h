#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include "SolverObserver.h"

// ============================================================
// Resultado por instancia / seed / solver
// ============================================================

struct InstanceResult {
    std::string solver;
    std::string instance_type;

    int q = 0;
    int n = 0;
    int ratio = 0;

    std::string filename;

    double p = 0.0;
    int maxinc = 0;
    int seed = 0;
    double alpha = 1.0;
    double lambda = 0.0;
    double bridge_clause_probability = 0.0;

    bool solved = false;              // alias de verified_solved para compatibilidad
    bool reported_solved = false;
    bool verified_solved = false;
    bool invalid_solution = false;

    double runtime_ms = 0.0;

    GenericSolverStats stats;
};

// ============================================================
// Resultado agregado
// ============================================================

struct SummaryResult {
    std::string solver;
    std::string instance_type;

    int q = 0;
    int n = 0;
    int ratio = 0;

    double p = 0.0;
    int maxinc = 0;
    int seed = 0;
    double alpha = 1.0;
    double lambda = 0.0;
    double bridge_clause_probability = 0.0;

    int total_instances = 0;
    int solved_instances = 0;
    int reported_solved_instances = 0;
    int invalid_solution_instances = 0;

    double success_rate = 0.0;
    double reported_success_rate = 0.0;
    double invalid_solution_rate = 0.0;

    double mean_runtime_ms = 0.0;
    double mean_flips_used = 0.0;
    double mean_best_unsat = 0.0;
    double mean_final_unsat = 0.0;
    double mean_normalized_auc = 0.0;
    double mean_longest_stagnation = 0.0;
    double mean_flip_entropy = 0.0;

    double mean_random_steps = 0.0;
    double mean_random_improving_steps = 0.0;
    double mean_random_neutral_steps = 0.0;
    double mean_random_worsening_steps = 0.0;

    double mean_heuristic_steps = 0.0;
    double mean_heuristic_improving_steps = 0.0;
    double mean_heuristic_neutral_steps = 0.0;
    double mean_heuristic_worsening_steps = 0.0;

    double mean_unknown_steps = 0.0;
    double mean_unknown_improving_steps = 0.0;
    double mean_unknown_neutral_steps = 0.0;
    double mean_unknown_worsening_steps = 0.0;

    double mean_paws_penalty_events = 0.0;
    double mean_paws_weight_increments = 0.0;
    double mean_paws_smoothing_events = 0.0;
    double mean_paws_weight_decrements = 0.0;
    double mean_paws_final_weighted_clauses = 0.0;
    double mean_paws_max_weight_seen = 0.0;
    double mean_paws_final_max_weight = 0.0;
    double mean_paws_final_mean_weight = 0.0;
    double mean_paws_avg_weighted_clauses = 0.0;
    double mean_paws_avg_max_weight = 0.0;
};

// ============================================================
// Evento de auto-stopping
// ============================================================

struct AutoStoppingEvent {
    std::string instance_type;
    int q = 0;
    int n = 0;
    int ratio = 0;
    std::string solver;
    int solved_count = 0;
    int fail_streak = 0;
    int patience_ratios = 0;
    std::string event = "stopped";
};

// ============================================================
// Logger CSV
// ============================================================

class CSVLogger {
public:
    CSVLogger(
        const std::string& output_dir,
        const std::string& instance_results_file,
        const std::string& summary_results_file
    );

    ~CSVLogger();

    void logInstanceResult(
        const InstanceResult& result
    );

    void logSummaryResult(
        const SummaryResult& result
    );

    void logAutoStoppingEvent(
        const AutoStoppingEvent& event
    );

private:
    void writeHeaders();

    std::ofstream instance_stream;
    std::ofstream summary_stream;
    std::ofstream auto_stopping_stream;
};
