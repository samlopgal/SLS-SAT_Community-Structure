// ConfigLoader.cpp

#include "ConfigLoader.h"

#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

ExperimentConfig ConfigLoader::load(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Cannot open configuration file: " + filename
        );
    }

    file.seekg(0, std::ios::end);
    if (file.tellg() == 0) {
        throw std::runtime_error(
            "Configuration file is empty: " + filename
        );
    }
    file.seekg(0, std::ios::beg);

    json j;

    try {
        file >> j;
    }
    catch (const json::parse_error& e) {
        throw std::runtime_error(
            "Invalid JSON in configuration file '" +
            filename + "': " + e.what()
        );
    }

    ExperimentConfig config;

    // ========================================================
    // experiment_name
    // ========================================================

    if (j.contains("experiment_name")) {
        config.experiment_name =
            j["experiment_name"].get<std::string>();
    }

    // ========================================================
    // instances
    // ========================================================

    if (j.contains("instances")) {
        const auto& ji = j["instances"];

        if (ji.contains("root_path"))
            config.instances.root_path =
                ji["root_path"].get<std::string>();

        if (ji.contains("types"))
            config.instances.types =
                ji["types"].get<std::vector<std::string>>();

        if (ji.contains("n_values"))
            config.instances.n_values =
                ji["n_values"].get<std::vector<int>>();

        if (ji.contains("ratios"))
            config.instances.ratios =
                ji["ratios"].get<std::vector<int>>();

        if (ji.contains("q_values"))
            config.instances.q_values =
                ji["q_values"].get<std::vector<int>>();

        if (ji.contains("instances_per_group"))
            config.instances.instances_per_group =
                ji["instances_per_group"].get<int>();
    }

    // ========================================================
    // precomputed
    // ========================================================

    if (j.contains("precomputed")) {
        const auto& jp = j["precomputed"];

        if (jp.contains("root_path"))
            config.precomputed.root_path =
                jp["root_path"].get<std::string>();

        if (jp.contains("auto_generate_if_missing"))
            config.precomputed.auto_generate_if_missing =
                jp["auto_generate_if_missing"].get<bool>();
    }

    // ========================================================
    // solvers
    // ========================================================

    if (j.contains("solvers")) {
        for (const auto& js : j["solvers"]) {
            SolverConfig solver;

            if (js.contains("name"))
                solver.name =
                    js["name"].get<std::string>();

            if (js.contains("enabled"))
                solver.enabled =
                    js["enabled"].get<bool>();

            if (js.contains("params")) {
                const auto& jp = js["params"];

                if (jp.contains("max_flips"))
                    solver.params.max_flips =
                        jp["max_flips"].get<int>();

                if (jp.contains("max_flips_per_variable"))
                    solver.params.max_flips_per_variable =
                        jp["max_flips_per_variable"].get<double>();

                if (jp.contains("max_tries"))
                    solver.params.max_tries =
                        jp["max_tries"].get<int>();

                if (jp.contains("p"))
                    solver.params.p =
                        jp["p"].get<double>();

                if (jp.contains("p_soft"))
                    solver.params.p_soft =
                        jp["p_soft"].get<double>();

                if (jp.contains("paws_p_soft"))
                    solver.params.p_soft =
                        jp["paws_p_soft"].get<double>();

                if (jp.contains("seed"))
                    solver.params.seed =
                        jp["seed"].get<int>();

                if (jp.contains("seeds"))
                    solver.params.seeds =
                        jp["seeds"].get<std::vector<int>>();

                if (jp.contains("alpha"))
                    solver.params.alpha =
                        jp["alpha"].get<double>();

                if (jp.contains("lambda"))
                    solver.params.lambda =
                        jp["lambda"].get<double>();

                if (jp.contains("maxinc"))
                    solver.params.maxinc =
                        jp["maxinc"].get<int>();

                if (jp.contains("paws_maxinc"))
                    solver.params.maxinc =
                        jp["paws_maxinc"].get<int>();
            }

            // Compatibilidad:
            // si no se especifica seeds, usar seed.
            if (solver.params.seeds.empty()) {
                solver.params.seeds.push_back(
                    solver.params.seed
                );
            }

            config.solvers.push_back(solver);
        }
    }

    // ========================================================
    // logging
    // ========================================================

    if (j.contains("logging")) {
        const auto& jl = j["logging"];

        if (jl.contains("output_dir"))
            config.logging.output_dir =
                jl["output_dir"].get<std::string>();

        if (jl.contains("instance_results_file"))
            config.logging.instance_results_file =
                jl["instance_results_file"].get<std::string>();

        if (jl.contains("summary_results_file"))
            config.logging.summary_results_file =
                jl["summary_results_file"].get<std::string>();
    }

    // ========================================================
    // execution
    // ========================================================

    if (j.contains("execution")) {
        const auto& je = j["execution"];

        if (je.contains("threads"))
            config.execution.threads =
                je["threads"].get<int>();
    }


    // ========================================================
    // auto_stopping
    // ========================================================

    // Formato en JSON:
    //   "auto_stopping": 2
    // Si no aparece, queda desactivado.
    // Si vale 0 o menos, queda desactivado.
    if (j.contains("auto_stopping")) {
        if (j["auto_stopping"].is_number_integer()) {
            config.auto_stopping_patience_ratios =
                j["auto_stopping"].get<int>();
        }
        else if (j["auto_stopping"].is_object()) {
            const auto& ja = j["auto_stopping"];
            if (ja.contains("patience_ratios")) {
                config.auto_stopping_patience_ratios =
                    ja["patience_ratios"].get<int>();
            }
        }
        else {
            throw std::runtime_error(
                "auto_stopping must be an integer, e.g. \"auto_stopping\": 2"
            );
        }

        if (config.auto_stopping_patience_ratios < 0) {
            throw std::runtime_error(
                "auto_stopping must be >= 0"
            );
        }
    }

    return config;
}
