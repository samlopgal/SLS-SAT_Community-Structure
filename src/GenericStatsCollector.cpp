#include "GenericStatsCollector.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>

GenericStatsCollector::GenericStatsCollector(
    int num_variables,
    int num_clauses)
    : num_variables_(num_variables),
      num_clauses_(num_clauses),
      var_flip_count_(num_variables + 1, 0),
      last_flip_time_(num_variables + 1, -1),
      unsat_seen_count_(num_clauses, 0),
      unsat_lifetime_(num_clauses, 0),
      unsat_start_flip_(num_clauses, -1)
{
}

int GenericStatsCollector::assignmentIndexForVar(
    int var_dimacs,
    const std::vector<uint8_t>& assignment) const
{
    if (assignment.size() == static_cast<std::size_t>(num_variables_ + 1)) {
        return var_dimacs;      // asignación 1-based
    }
    return var_dimacs - 1;      // asignación 0-based
}

void GenericStatsCollector::accountUnsatClauses(
    const std::vector<int>& unsat_clauses)
{
    for (int ci : unsat_clauses) {
        if (ci < 0 || ci >= num_clauses_) {
            continue;
        }

        ++unsat_seen_count_[ci];

        if (unsat_start_flip_[ci] == -1) {
            unsat_start_flip_[ci] = stats_.flips_used;
        }
    }
}

void GenericStatsCollector::closeUnsatLifetimes()
{
    for (int ci = 0; ci < num_clauses_; ++ci) {
        if (unsat_start_flip_[ci] != -1) {
            long long lifetime = stats_.flips_used - unsat_start_flip_[ci];
            unsat_lifetime_[ci] += lifetime;
            stats_.max_unsat_clause_lifetime = std::max(
                stats_.max_unsat_clause_lifetime,
                lifetime
            );
            unsat_start_flip_[ci] = -1;
        }
    }
}

void GenericStatsCollector::onTryStart(
    int try_id,
    int initial_unsat,
    const std::vector<int>& unsat_clauses,
    const std::vector<uint8_t>& assignment)
{
    has_any_try_ = true;
    stats_.tries_used = std::max(stats_.tries_used, try_id);
    stats_.restarts = std::max(0, stats_.tries_used - 1);

    current_try_best_unsat_ = initial_unsat;
    current_try_flips_ = 0;

    if (stats_.flips_used == 0) {
        stats_.initial_unsat = initial_unsat;
        stats_.best_unsat = initial_unsat;
        stats_.final_unsat = initial_unsat;

        initial_assignment_ = assignment;
        current_hamming_from_initial_ = 0;
        reached_50pct_ = false;
        reached_90pct_ = false;
    }

    accountUnsatClauses(unsat_clauses);
}

void GenericStatsCollector::onFlip(
    int try_id,
    int selected_var_dimacs,
    int old_unsat,
    int new_unsat,
    const std::vector<int>& unsat_clauses,
    const std::vector<uint8_t>& assignment,
    MoveOrigin origin)
{
    (void)try_id;

    ++stats_.flips_used;
    ++current_try_flips_;
    stats_.tries_used = std::max(stats_.tries_used, try_id);
    stats_.final_unsat = new_unsat;

    // Progreso del estado actual.
    int delta = new_unsat - old_unsat;
    delta_sum_ += delta;
    abs_delta_sum_ += std::llabs(static_cast<long long>(delta));

    if (delta < 0) {
        ++stats_.improving_steps;
        stats_.largest_single_improvement = std::max(
            stats_.largest_single_improvement,
            -delta
        );
    }
    else if (delta > 0) {
        ++stats_.worsening_steps;
        stats_.largest_single_worsening = std::max(
            stats_.largest_single_worsening,
            delta
        );
    }
    else {
        ++stats_.neutral_steps;
    }

    // Descomposición por origen del movimiento.
    // reutiliza el delta ya calculado para improving/neutral/worsening.
    auto account_by_origin = [delta](
        long long& steps,
        long long& improving,
        long long& neutral,
        long long& worsening
    ) {
        ++steps;
        if (delta < 0) {
            ++improving;
        }
        else if (delta > 0) {
            ++worsening;
        }
        else {
            ++neutral;
        }
    };

    switch (origin) {
        case MoveOrigin::Random:
            account_by_origin(
                stats_.random_steps,
                stats_.random_improving_steps,
                stats_.random_neutral_steps,
                stats_.random_worsening_steps
            );
            break;

        case MoveOrigin::Heuristic:
            account_by_origin(
                stats_.heuristic_steps,
                stats_.heuristic_improving_steps,
                stats_.heuristic_neutral_steps,
                stats_.heuristic_worsening_steps
            );
            break;

        case MoveOrigin::Unknown:
        default:
            account_by_origin(
                stats_.unknown_steps,
                stats_.unknown_improving_steps,
                stats_.unknown_neutral_steps,
                stats_.unknown_worsening_steps
            );
            break;
    }

    // Mejor histórico y estancamiento.
    if (new_unsat < stats_.best_unsat) {
        if (stats_.flips_to_first_improvement == -1) {
            stats_.flips_to_first_improvement = stats_.flips_used;
        }

        ++stats_.best_improvement_events;
        stats_.best_unsat = new_unsat;
        stats_.flips_to_best = stats_.flips_used;

        if (current_stagnation_ > 0) {
            ++stats_.stagnation_episodes;
            stagnation_length_sum_ += current_stagnation_;
            stats_.longest_stagnation = std::max(
                stats_.longest_stagnation,
                current_stagnation_
            );
        }
        current_stagnation_ = 0;
    }
    else {
        ++current_stagnation_;
        ++stats_.total_stagnation_steps;
        stats_.longest_stagnation = std::max(
            stats_.longest_stagnation,
            current_stagnation_
        );
    }

    stats_.steps_since_last_best =
        (stats_.flips_to_best == -1)
            ? stats_.flips_used
            : stats_.flips_used - stats_.flips_to_best;

    current_try_best_unsat_ = std::min(current_try_best_unsat_, new_unsat);

    // Dinámica de variables.
    if (selected_var_dimacs >= 1 && selected_var_dimacs <= num_variables_) {
        if (var_flip_count_[selected_var_dimacs] == 0) {
            ++stats_.distinct_vars_flipped;
        }
        ++var_flip_count_[selected_var_dimacs];

        if (previous_var_ == selected_var_dimacs) {
            ++stats_.immediate_backflips;
        }
        previous_var_ = selected_var_dimacs;

        if (last_flip_time_[selected_var_dimacs] != -1) {
            revisit_distance_sum_ +=
                stats_.flips_used - last_flip_time_[selected_var_dimacs];
            ++revisit_events_;
        }
        last_flip_time_[selected_var_dimacs] = stats_.flips_used;
    }

    // Movilidad respecto a la asignación inicial.
    if (!initial_assignment_.empty() &&
        selected_var_dimacs >= 1 && selected_var_dimacs <= num_variables_)
    {
        int idx = assignmentIndexForVar(selected_var_dimacs, assignment);
        if (idx >= 0 && idx < static_cast<int>(assignment.size()) &&
            idx < static_cast<int>(initial_assignment_.size()))
        {
            // Como onFlip se llama después del cambio, basta con recalcular
            // la contribución de esta variable respecto a la asignación inicial.
            // Para evitar almacenar el valor anterior, reconstruimos el hamming
            // incrementalmente invirtiendo la contribución por el cambio de estado.
            // El flip cambia exactamente una variable.
            bool now_diff = assignment[idx] != initial_assignment_[idx];
            if (now_diff) {
                ++current_hamming_from_initial_;
            }
            else {
                --current_hamming_from_initial_;
            }
        }
    }

    hamming_sum_ += current_hamming_from_initial_;
    ++hamming_samples_;
    stats_.max_hamming_from_initial = std::max(
        stats_.max_hamming_from_initial,
        static_cast<double>(current_hamming_from_initial_)
    );

    // Curva de calidad.
    stats_.area_under_unsat_curve += static_cast<double>(new_unsat);

    if (stats_.initial_unsat > 0) {
        const double improvement =
            static_cast<double>(stats_.initial_unsat - new_unsat) /
            static_cast<double>(stats_.initial_unsat);

        if (!reached_50pct_ && improvement >= 0.50) {
            reached_50pct_ = true;
            stats_.time_to_50pct_improvement =
                static_cast<double>(stats_.flips_used);
        }
        if (!reached_90pct_ && improvement >= 0.90) {
            reached_90pct_ = true;
            stats_.time_to_90pct_improvement =
                static_cast<double>(stats_.flips_used);
        }
    }

    accountUnsatClauses(unsat_clauses);
}

void GenericStatsCollector::onPawsStep(
    int try_id,
    int flip,
    bool penalty_applied,
    int weight_increments,
    bool smoothing_applied,
    int weight_decrements,
    int weighted_clauses,
    int max_clause_weight,
    double mean_clause_weight)
{
    (void)try_id;
    (void)flip;

    if (penalty_applied) {
        ++stats_.paws_penalty_events;
    }

    stats_.paws_weight_increments += weight_increments;

    if (smoothing_applied) {
        ++stats_.paws_smoothing_events;
    }

    stats_.paws_weight_decrements += weight_decrements;

    stats_.paws_final_weighted_clauses = weighted_clauses;
    stats_.paws_final_max_weight = max_clause_weight;
    stats_.paws_final_mean_weight = mean_clause_weight;

    if (max_clause_weight > stats_.paws_max_weight_seen) {
        stats_.paws_max_weight_seen = max_clause_weight;
    }

    paws_weighted_clause_sum_ += weighted_clauses;
    paws_max_weight_sum_ += max_clause_weight;
    ++paws_observed_steps_;
}

void GenericStatsCollector::onTryEnd(
    int try_id,
    bool solved,
    int final_unsat)
{
    (void)try_id;

    stats_.final_unsat = final_unsat;
    best_unsat_by_try_.push_back(current_try_best_unsat_);

    if (solved && stats_.successful_try == -1) {
        stats_.successful_try = try_id;
    }
}

void GenericStatsCollector::onSolveEnd(
    bool reported_solved,
    int final_unsat)
{
    stats_.reported_solved = reported_solved;
    stats_.final_unsat = final_unsat;

    if (current_stagnation_ > 0) {
        ++stats_.stagnation_episodes;
        stagnation_length_sum_ += current_stagnation_;
        stats_.longest_stagnation = std::max(
            stats_.longest_stagnation,
            current_stagnation_
        );
        current_stagnation_ = 0;
    }

    closeUnsatLifetimes();
    finalizeDerivedStats();
}

void GenericStatsCollector::finalizeDerivedStats()
{
    if (stats_.flips_used > 0) {
        stats_.avg_delta_unsat =
            static_cast<double>(delta_sum_) /
            static_cast<double>(stats_.flips_used);

        stats_.avg_abs_delta_unsat =
            static_cast<double>(abs_delta_sum_) /
            static_cast<double>(stats_.flips_used);

        if (stats_.initial_unsat > 0) {
            stats_.normalized_auc =
                stats_.area_under_unsat_curve /
                (static_cast<double>(stats_.flips_used) *
                 static_cast<double>(stats_.initial_unsat));
        }

        if (hamming_samples_ > 0) {
            stats_.avg_hamming_from_initial =
                hamming_sum_ /
                static_cast<double>(hamming_samples_);
        }

        const long long early_limit = std::max<long long>(1, stats_.flips_used / 10);
        const long long late_limit = std::max<long long>(1, stats_.flips_used - stats_.flips_used / 10);
        // Aproximación compacta: progreso total normalizado por zona temporal.
        // Si quieres curvas exactas, usa trace snapshots.
        stats_.early_progress_rate =
            stats_.flips_to_first_improvement > 0 && stats_.flips_to_first_improvement <= early_limit
                ? 1.0 : 0.0;
        stats_.late_progress_rate =
            stats_.flips_to_best >= late_limit
                ? 1.0 : 0.0;
    }

    if (stats_.stagnation_episodes > 0) {
        stats_.avg_stagnation_length =
            static_cast<double>(stagnation_length_sum_) /
            static_cast<double>(stats_.stagnation_episodes);
    }

    if (revisit_events_ > 0) {
        stats_.avg_revisit_distance =
            static_cast<double>(revisit_distance_sum_) /
            static_cast<double>(revisit_events_);
    }

    // Entropía de variables.
    if (stats_.flips_used > 0) {
        double h = 0.0;
        long long max_count = 0;

        for (int v = 1; v <= num_variables_; ++v) {
            long long c = var_flip_count_[v];
            max_count = std::max(max_count, c);
            if (c > 0) {
                double p = static_cast<double>(c) /
                           static_cast<double>(stats_.flips_used);
                h -= p * std::log(p);
            }
        }

        stats_.flip_entropy = h;
        stats_.normalized_flip_entropy =
            (num_variables_ > 1) ? h / std::log(static_cast<double>(num_variables_)) : 0.0;
        stats_.most_flipped_var_ratio =
            static_cast<double>(max_count) /
            static_cast<double>(stats_.flips_used);
    }

    // Entropía de cláusulas insatisfechas observadas.
    long long total_unsat_observations = 0;
    long long max_unsat_observations = 0;
    int distinct = 0;
    long long lifetime_sum = 0;
    int lifetime_count = 0;

    for (int c = 0; c < num_clauses_; ++c) {
        long long count = unsat_seen_count_[c];
        total_unsat_observations += count;
        max_unsat_observations = std::max(max_unsat_observations, count);
        if (count > 0) {
            ++distinct;
        }
        if (unsat_lifetime_[c] > 0) {
            lifetime_sum += unsat_lifetime_[c];
            ++lifetime_count;
        }
    }

    stats_.distinct_unsat_clauses_seen = distinct;

    if (total_unsat_observations > 0) {
        double h = 0.0;
        for (int c = 0; c < num_clauses_; ++c) {
            long long count = unsat_seen_count_[c];
            if (count > 0) {
                double p = static_cast<double>(count) /
                           static_cast<double>(total_unsat_observations);
                h -= p * std::log(p);
            }
        }
        stats_.unsat_clause_entropy = h;
        stats_.normalized_unsat_clause_entropy =
            (num_clauses_ > 1) ? h / std::log(static_cast<double>(num_clauses_)) : 0.0;
        stats_.most_frequent_unsat_clause_ratio =
            static_cast<double>(max_unsat_observations) /
            static_cast<double>(total_unsat_observations);
    }

    if (lifetime_count > 0) {
        stats_.avg_unsat_clause_lifetime =
            static_cast<double>(lifetime_sum) /
            static_cast<double>(lifetime_count);
    }

    if (paws_observed_steps_ > 0) {
        stats_.paws_avg_weighted_clauses =
            static_cast<double>(paws_weighted_clause_sum_) /
            static_cast<double>(paws_observed_steps_);

        stats_.paws_avg_max_weight =
            static_cast<double>(paws_max_weight_sum_) /
            static_cast<double>(paws_observed_steps_);
    }

    // Estadística por try.
    if (!best_unsat_by_try_.empty()) {
        double mean = 0.0;
        for (int x : best_unsat_by_try_) {
            mean += x;
        }
        mean /= static_cast<double>(best_unsat_by_try_.size());
        stats_.avg_best_unsat_per_try = mean;

        double var = 0.0;
        for (int x : best_unsat_by_try_) {
            double d = static_cast<double>(x) - mean;
            var += d * d;
        }
        var /= static_cast<double>(best_unsat_by_try_.size());
        stats_.std_best_unsat_per_try = std::sqrt(var);
    }
}

const GenericSolverStats& GenericStatsCollector::stats() const
{
    return stats_;
}

GenericSolverStats& GenericStatsCollector::mutableStats()
{
    return stats_;
}
