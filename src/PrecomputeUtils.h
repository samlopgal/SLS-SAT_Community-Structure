#pragma once

#include "InstanceScanner.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// Devuelve la ruta donde se almacenará el fichero .pcd
// correspondiente a una instancia SAT.
fs::path getPrecomputedPath(
    const InstanceInfo& instance,
    const std::string& precomputed_root
);
