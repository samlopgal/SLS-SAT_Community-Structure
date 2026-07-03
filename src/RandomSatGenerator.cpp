#include "RandomSatGenerator.h"
#include <random>
#include <set>
#include <fstream>
#include <iostream>
#include <filesystem>

using namespace std;

void RandomSatGenerator::generate(int n, int m, int k, int num_instancias, const string& prefix,
                                  const std::string& output_dir, unsigned int seed) {

    mt19937 gen; // Generador de números aleatorios

    if (seed == 0) {
        random_device rd;
        gen.seed(rd()); // Si no se pasa seed, usar entropía del sistema
    } else {
        gen.seed(seed); // Usar semilla proporcionada
    }

    uniform_int_distribution<> dist_variables(1, n);
    uniform_int_distribution<> dist_signo(0, 1);

    filesystem::create_directories(output_dir);

    for (int inst = 1; inst <= num_instancias; inst++) {

        string nombre = output_dir + "/" + prefix + "_" + to_string(inst) + ".cnf";
        ofstream file(nombre);

        if (!file) {
            cout << "Error al crear el fichero " << nombre << endl;
            continue;
        }

        file << "p cnf " << n << " " << m << endl;

        for (int i = 0; i < m; i++) {
            set<int> vars_usadas;

            for (int j = 0; j < k; j++) {
                int var;
                do {
                    var = dist_variables(gen);
                } while (vars_usadas.count(var));
                vars_usadas.insert(var);

                int signo = dist_signo(gen);
                int literal = signo ? var : -var;

                file << literal << " ";
            }

            file << "0\n";
        }

        file.close();
    }

    cout << "Se generaron " << num_instancias << " instancias." << endl;
}
