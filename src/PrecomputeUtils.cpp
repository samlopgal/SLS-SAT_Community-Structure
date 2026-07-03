#include "PrecomputeUtils.h"

#include <filesystem>

namespace fs = std::filesystem;

fs::path getPrecomputedPath(
    const InstanceInfo& instance,
    const std::string& precomputed_root)
{
    fs::path path = precomputed_root;

    if (instance.type == "random") {
        path /= "random";
        path /= "n" + std::to_string(instance.n);
        path /= "r_" + std::to_string(instance.ratio);
    }
    else if (instance.type == "community" ||
             instance.type == "community_instances")
    {
        path /= "community_instances";
        path /= "q_" + std::to_string(instance.q);
        path /= "n" + std::to_string(instance.n);
        path /= "r_" + std::to_string(instance.ratio);
    }
    else {
        path /= instance.type;
        path /= "n" + std::to_string(instance.n);
        path /= "r_" + std::to_string(instance.ratio);
    }

    fs::path filename = instance.filename;
    filename.replace_extension(".pcd");

    path /= filename;

    return path;
}