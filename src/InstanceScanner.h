#pragma once

#include "ExperimentConfig.h"

#include <string>
#include <vector>

// ============================================================
// Información de una instancia concreta
// ============================================================

struct InstanceInfo {
    std::string filepath;   // ruta completa al .cnf
    std::string filename;   // nombre del fichero
    std::string type;       // "random" o "community_instances"

    int n = 0;             // número de variables
    int ratio = 0;         // ratio m/n codificado sin decimal
    int q = 0;             // parámetro q (solo community_instances)
};

// ============================================================
// Escáner de instancias
// ============================================================

class InstanceScanner {
public:
    static std::vector<InstanceInfo> scan(
        const InstancesConfig& config
    );
};
