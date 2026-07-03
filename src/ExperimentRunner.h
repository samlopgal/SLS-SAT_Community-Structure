#pragma once

#include "ExperimentConfig.h"
#include "CSVLogger.h"
#include "Statistics.h"

class ExperimentRunner {
public:
    explicit ExperimentRunner(const ExperimentConfig& config);

    void run();

private:
    ExperimentConfig config;
};
