#pragma once

#include "ExperimentConfig.h"

#include <string>
#include <nlohmann/json.hpp>

class ConfigLoader {
public:
    static ExperimentConfig load(
        const std::string& filename
    );
};
