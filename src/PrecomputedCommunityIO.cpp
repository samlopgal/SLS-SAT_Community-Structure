// PrecomputedCommunityIO.cpp

#include "PrecomputedCommunityIO.h"

#include <fstream>
#include <stdexcept>

// ======================================================
// SAVE
// ======================================================

void PrecomputedCommunityIO::save(
    const std::string& path,
    const PrecomputedCommunityData& data
)
{
    std::ofstream out(
        path,
        std::ios::binary
    );

    if (!out)
        throw std::runtime_error(
            "Cannot open file for writing"
        );

    size_t sz;

    // ==========================================
    // var_to_community
    // ==========================================

    sz = data.var_to_community.size();

    out.write(
        (char*)&sz,
        sizeof(size_t)
    );

    out.write(
        (char*)data.var_to_community.data(),
        sz * sizeof(int)
    );

    // ==========================================
    // clause_communities
    // ==========================================

    sz = data.clause_communities.size();

    out.write(
        (char*)&sz,
        sizeof(size_t)
    );

    for (const auto& vec :
         data.clause_communities)
    {
        size_t s = vec.size();

        out.write(
            (char*)&s,
            sizeof(size_t)
        );

        out.write(
            (char*)vec.data(),
            s * sizeof(int)
        );
    }

    // ==========================================
    // community_total
    // ==========================================

    sz = data.community_total.size();

    out.write(
        (char*)&sz,
        sizeof(size_t)
    );

    out.write(
        (char*)data.community_total.data(),
        sz * sizeof(int)
    );

    // ==========================================
    // community_size
    // ==========================================

    sz = data.community_size.size();

    out.write(
        (char*)&sz,
        sizeof(size_t)
    );

    out.write(
        (char*)data.community_size.data(),
        sz * sizeof(int)
    );

    // ==========================================
    // total_community_size
    // ==========================================

    out.write(
        (char*)&data.total_community_size,
        sizeof(double)
    );
}

// ======================================================
// LOAD
// ======================================================

PrecomputedCommunityData
PrecomputedCommunityIO::load(
    const std::string& path
)
{
    std::ifstream in(
        path,
        std::ios::binary
    );

    if (!in)
        throw std::runtime_error(
            "Cannot open file for reading"
        );

    PrecomputedCommunityData data;

    size_t sz;

    // ==========================================
    // var_to_community
    // ==========================================

    in.read(
        (char*)&sz,
        sizeof(size_t)
    );

    data.var_to_community.resize(sz);

    in.read(
        (char*)data.var_to_community.data(),
        sz * sizeof(int)
    );

    // ==========================================
    // clause_communities
    // ==========================================

    in.read(
        (char*)&sz,
        sizeof(size_t)
    );

    data.clause_communities.resize(sz);

    for (size_t i = 0; i < sz; ++i) {

        size_t s;

        in.read(
            (char*)&s,
            sizeof(size_t)
        );

        data.clause_communities[i].resize(s);

        in.read(
            (char*)data.clause_communities[i].data(),
            s * sizeof(int)
        );
    }

    // ==========================================
    // community_total
    // ==========================================

    in.read(
        (char*)&sz,
        sizeof(size_t)
    );

    data.community_total.resize(sz);

    in.read(
        (char*)data.community_total.data(),
        sz * sizeof(int)
    );

    // ==========================================
    // community_size
    // ==========================================

    in.read(
        (char*)&sz,
        sizeof(size_t)
    );

    data.community_size.resize(sz);

    in.read(
        (char*)data.community_size.data(),
        sz * sizeof(int)
    );

    // ==========================================
    // total_community_size
    // ==========================================

    in.read(
        (char*)&data.total_community_size,
        sizeof(double)
    );

    return data;
}
