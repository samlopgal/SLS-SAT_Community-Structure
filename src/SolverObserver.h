#pragma once

#include <cstdint>
#include <vector>

// Origen del movimiento: permite separar flips aleatorios y heurísticos.
// Unknown conserva compatibilidad con algoritmos no instrumentados.
enum class MoveOrigin {
    Unknown,
    Random,
    Heuristic
};

// Estadísticas genéricas: no dependen de BreakCount, PAWS, MakeCount,
// comunidades ni de ninguna heurística concreta.
struct GenericSolverStats {
    // Resultado
    bool reported_solved = false;
    bool verified_solved = false;
    bool invalid_solution = false;
    bool timeout = false;

    // Coste
    long long flips_used = 0;
    int tries_used = 0;
    double time_ms = 0.0;

    // Calidad de estado
    int initial_unsat = 0;
    int final_unsat = 0;
    int best_unsat = 0;
    long long flips_to_first_improvement = -1;
    long long flips_to_best = -1;

    // Progreso por pasos
    long long improving_steps = 0;
    long long neutral_steps = 0;
    long long worsening_steps = 0;

    // Descomposición por origen del movimiento
    long long random_steps = 0;
    long long random_improving_steps = 0;
    long long random_neutral_steps = 0;
    long long random_worsening_steps = 0;

    long long heuristic_steps = 0;
    long long heuristic_improving_steps = 0;
    long long heuristic_neutral_steps = 0;
    long long heuristic_worsening_steps = 0;

    long long unknown_steps = 0;
    long long unknown_improving_steps = 0;
    long long unknown_neutral_steps = 0;
    long long unknown_worsening_steps = 0;

    long long best_improvement_events = 0;
    int largest_single_improvement = 0;
    int largest_single_worsening = 0;
    double avg_delta_unsat = 0.0;
    double avg_abs_delta_unsat = 0.0;

    // Estancamiento respecto al mejor valor histórico
    long long longest_stagnation = 0;
    long long total_stagnation_steps = 0;
    int stagnation_episodes = 0;
    double avg_stagnation_length = 0.0;
    long long steps_since_last_best = 0;

    // Dinámica de variables
    int distinct_vars_flipped = 0;
    long long immediate_backflips = 0;
    double flip_entropy = 0.0;
    double normalized_flip_entropy = 0.0;
    double most_flipped_var_ratio = 0.0;
    double avg_revisit_distance = 0.0;

    // Dinámica de cláusulas conflictivas
    int distinct_unsat_clauses_seen = 0;
    double unsat_clause_entropy = 0.0;
    double normalized_unsat_clause_entropy = 0.0;
    double most_frequent_unsat_clause_ratio = 0.0;
    double avg_unsat_clause_lifetime = 0.0;
    long long max_unsat_clause_lifetime = 0;

    // Movilidad de la asignación
    double avg_hamming_from_initial = 0.0;
    double max_hamming_from_initial = 0.0;

    // Calidad de trayectoria
    double area_under_unsat_curve = 0.0;
    double normalized_auc = 0.0;
    double early_progress_rate = 0.0;
    double late_progress_rate = 0.0;
    double time_to_50pct_improvement = -1.0;
    double time_to_90pct_improvement = -1.0;

    // Reinicios
    int restarts = 0;
    int successful_try = -1;
    double avg_best_unsat_per_try = 0.0;
    double std_best_unsat_per_try = 0.0;


    // PAWS / clause-weight

    long long paws_penalty_events = 0;
    long long paws_weight_increments = 0;
    long long paws_smoothing_events = 0;
    long long paws_weight_decrements = 0;

    int paws_final_weighted_clauses = 0;
    int paws_max_weight_seen = 1;
    int paws_final_max_weight = 1;

    double paws_final_mean_weight = 1.0;
    double paws_avg_weighted_clauses = 0.0;
    double paws_avg_max_weight = 1.0;
};

class SolverObserver {
public:
    virtual ~SolverObserver() = default;

    // selected_var_dimacs y variables de cláusulas usan numeración DIMACS: 1..num_variables.
    virtual void onTryStart(
        int try_id,
        int initial_unsat,
        const std::vector<int>& unsat_clauses,
        const std::vector<uint8_t>& assignment
    ) = 0;

    virtual void onFlip(
        int try_id,
        int selected_var_dimacs,
        int old_unsat,
        int new_unsat,
        const std::vector<int>& unsat_clauses,
        const std::vector<uint8_t>& assignment,
        MoveOrigin origin = MoveOrigin::Unknown
    ) = 0;

    virtual void onTryEnd(
        int try_id,
        bool solved,
        int final_unsat
    ) = 0;

    virtual void onSolveEnd(
        bool reported_solved,
        int final_unsat
    ) = 0;

    virtual void onPawsStep(
        int try_id,
        int flip,
        bool penalty_applied,
        int weight_increments,
        bool smoothing_applied,
        int weight_decrements,
        int weighted_clauses,
        int max_clause_weight,
        double mean_clause_weight
    ) {
    }
};
