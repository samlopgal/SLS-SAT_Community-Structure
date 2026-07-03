// ExperimentConfig.h

#pragma once

#include <string>
#include <vector>

// ============================================================
// Configuración de instancias
// ============================================================

struct InstancesConfig {
    std::string root_path;
    std::vector<std::string> types;
    std::vector<int> n_values;
    std::vector<int> ratios;
    std::vector<int> q_values;
    int instances_per_group = 100;
};

// ============================================================
// Configuración de precomputados
// ============================================================

struct PrecomputedConfig {
    std::string root_path = "precomputed";
    bool auto_generate_if_missing = false;
};

// ============================================================
// Parámetros de solver
// ============================================================

struct SolverParamsConfig {
    // Si max_flips_per_variable > 0:
    //   effective_max_flips =
    //       max_flips_per_variable * num_variables
    //
    // Si <= 0:
    //   se utiliza max_flips.
    int max_flips = 100000;

    double max_flips_per_variable = 0.0;

    int max_tries = 10;
    double p = 0.5;

    // Compatibilidad con configuración antigua
    int seed = 42;

    // Configuración multiseed
    std::vector<int> seeds;

    // Nuevo parámetro
    double alpha = 1.0;

    // Parámetro existente
    double lambda = 1.0;

    // Solo afecta a las variantes PAWS no deterministas.
    // Valor por defecto compatible con la implementación anterior.
    double p_soft = 0.05;

    // PAWS_DET: intervalo determinista de suavizado.
    // Cada maxinc eventos de penalización se restan 1 a los pesos > 1.
    // Si maxinc <= 0, el suavizado queda desactivado.
    int maxinc = 20;
};

// ============================================================
// Configuración de un solver
// ============================================================

struct SolverConfig {
    std::string name;
    bool enabled = true;
    SolverParamsConfig params;
};

// ============================================================
// Configuración de logging
// ============================================================

struct LoggingConfig {
    std::string output_dir = "results";
    std::string instance_results_file = "instance_results.csv";
    std::string summary_results_file = "summary_results.csv";
};

// ============================================================
// Configuración de ejecución
// ============================================================

struct ExecutionConfig {
    int threads = 1;
};

// ============================================================
// Configuración global del experimento
// ============================================================

struct ExperimentConfig {
    std::string experiment_name = "sat_benchmark";

    InstancesConfig instances;
    PrecomputedConfig precomputed;
    std::vector<SolverConfig> solvers;
    LoggingConfig logging;
    ExecutionConfig execution;

    // Auto-stopping opcional.
    // 0: desactivado.
    // N > 0: un solver se desactiva dentro de cada grupo (type,n,q)
    //        tras N ratios consecutivos sin resolver ninguna instancia.
    int auto_stopping_patience_ratios = 0;
};
