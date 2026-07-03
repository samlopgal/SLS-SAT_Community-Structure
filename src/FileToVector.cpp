#include "FileToVector.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<std::vector<int>> FileToVector::readCNF(const std::string& filename, int& num_variables) {
    std::ifstream file(filename);
    std::vector<std::vector<int>> formula;
    num_variables = 0;

    if (!file) {
        std::cerr << "Error: no se pudo abrir el fichero " << filename << std::endl;
        return formula;
    }

    std::string line;
    while (std::getline(file, line)) {

        // Ignora las líneas vacías y comentarios
        if (line.empty() || line[0] == 'c') continue;

        // Lee cabecera p cnf n m
        if (line[0] == 'p') {
            std::istringstream iss(line);
            std::string tmp;
            int num_clauses;
            iss >> tmp >> tmp >> num_variables >> num_clauses;
            continue;
        }

        // Lee cláusulas
        std::istringstream iss(line);
        int lit;
        std::vector<int> clause;
        while (iss >> lit) {
            if (lit == 0) break;
            //Añade los literales a la clausula
            clause.push_back(lit);
        }

        if (!clause.empty()) {
        	//Añade las clausulas a la fórmula
            formula.push_back(clause);
        }
    }

    return formula;
}
