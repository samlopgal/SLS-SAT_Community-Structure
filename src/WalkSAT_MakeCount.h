#ifndef WALKSAT_MAKECOUNT_H
#define WALKSAT_MAKECOUNT_H

#include <vector>
#include <random>
#include <cstdint>
#include "Ocurrence.h"
#include "SolverObserver.h"

class WalkSAT_MakeCount {
public:
    WalkSAT_MakeCount(const std::vector<std::vector<int>>& formula_,
                       int num_variables_,
                       int seed_);

    bool solve(int max_flips, int max_tries, double p, SolverObserver* observer = nullptr);
    const std::vector<uint8_t>& getAssignment() const;

private:

    // Fórmula en CNF (estructura estática del problema)
    std::vector<std::vector<int>> formula;

    int num_variables;

    // Asignación actual de variables (estado del sistema)
    std::vector<uint8_t> assignment;

    // Número de literales verdaderos por cláusula (estado incremental)
    std::vector<int> clause_sat_count;

    // Variable que es el único literal verdadero de una cláusula.
    // Vale -1 si la cláusula tiene 0 o más de 1 literales verdaderos.
    std::vector<int> clause_true_lit;

    // Lista de cláusulas actualmente insatisfechas
    std::vector<int> unsat_clauses;

    // Posición de cada cláusula dentro de unsat_clauses (para borrado O(1))
    std::vector<int> clause_pos_in_unsat;

    // Para cada variable: lista de (cláusula, signo) donde aparece
    std::vector<std::vector<Occurrence>> var_occ;

    // Heurística WalkSAT:
    // breakcount[v] = nº de cláusulas que se rompen si flip(v)
    std::vector<int> breakcount;

    // Heurística WalkSAT:
    // makecount[v] = nº de cláusulas que se satisfacen si flip(v)
    std::vector<int> makecount;

    std::mt19937 gen;

    // Inicializa asignación aleatoria
    void randomAssignment();

    // Construye estado inicial (sat counts + heurísticas)
    void initializeClauseData();

    // Añade cláusula a lista de insatisfechas (O(1))
    void addUnsatClause(int ci);

    // Elimina cláusula de lista de insatisfechas (O(1))
    void removeUnsatClause(int ci);

    int findUniqueTrueVar(int ci) const;
};

#endif
