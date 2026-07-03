#include "InstanceScanner.h"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

std::vector<fs::directory_entry> collectCNFFiles(const fs::path& dir)
{
    std::vector<fs::directory_entry> files;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() != ".cnf")
            continue;

        files.push_back(entry);
    }

    std::sort(
        files.begin(),
        files.end(),
        [](const fs::directory_entry& a,
           const fs::directory_entry& b)
        {
            return a.path().filename().string()
                 < b.path().filename().string();
        }
    );

    return files;
}

int groupLimit(const InstancesConfig& config)
{
    return config.instances_per_group;
}

} // namespace

std::vector<InstanceInfo> InstanceScanner::scan(
    const InstancesConfig& config)
{
    std::vector<InstanceInfo> result;

    const int limit = groupLimit(config);

    for (const auto& type : config.types) {

        // ====================================================
        // RANDOM
        // instancias/random/n100/r_420/*.cnf
        // ====================================================

        if (type == "random") {
            for (int n : config.n_values) {
                for (int ratio : config.ratios) {

                    fs::path dir =
                        fs::path(config.root_path)
                        / "random"
                        / ("n" + std::to_string(n))
                        / ("r_" + std::to_string(ratio));

                    if (!fs::exists(dir) ||
                        !fs::is_directory(dir))
                        continue;

                    std::vector<fs::directory_entry> files =
                        collectCNFFiles(dir);

                    int added = 0;

                    for (const auto& entry : files)
                    {
                        if (limit > 0 && added >= limit)
                            break;

                        InstanceInfo info;
                        info.filepath =
                            entry.path().string();
                        info.filename =
                            entry.path().filename().string();
                        info.type = "random";
                        info.n = n;
                        info.ratio = ratio;
                        info.q = 0;

                        result.push_back(info);
                        ++added;
                    }
                }
            }
        }

        // ====================================================
        // COMMUNITY
        // instancias/community_instances/q_500/n100/r_420/*.cnf
        // ====================================================

        else if (type == "community") {
            for (int q : config.q_values) {
                for (int n : config.n_values) {
                    for (int ratio : config.ratios) {

                        fs::path dir =
                            fs::path(config.root_path)
                            / "community_instances"
                            / ("q_" + std::to_string(q))
                            / ("n" + std::to_string(n))
                            / ("r_" + std::to_string(ratio));

                        if (!fs::exists(dir) ||
                            !fs::is_directory(dir))
                            continue;

                        std::vector<fs::directory_entry> files =
                            collectCNFFiles(dir);

                        int added = 0;

                        for (const auto& entry : files)
                        {
                            if (limit > 0 && added >= limit)
                                break;

                            InstanceInfo info;
                            info.filepath =
                                entry.path().string();
                            info.filename =
                                entry.path().filename().string();
                            info.type = "community";
                            info.n = n;
                            info.ratio = ratio;
                            info.q = q;

                            result.push_back(info);
                            ++added;
                        }
                    }
                }
            }
        }
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const InstanceInfo& a,
           const InstanceInfo& b)
        {
            return a.filepath < b.filepath;
        }
    );

    return result;
}
