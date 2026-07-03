#ifndef RANDOM_SAT_GENERATOR_H
#define RANDOM_SAT_GENERATOR_H

#include <string>

class RandomSatGenerator {
public:
    // Añadimos parámetro seed opcional
    static void generate(int n, int m, int k, int num_instancias,
                         const std::string &prefix = "instancia",
                         const std::string &output_dir = "../instancias",
                         unsigned int seed = 0);
};

#endif
