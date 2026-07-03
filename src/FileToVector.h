#ifndef FILETOVECTOR_H
#define FILETOVECTOR_H

#include <vector>
#include <string>

class FileToVector {
public:
    // Constructor por defecto
    FileToVector() = default;

    // Método estático: recibe el nombre de fichero y devuelve la fórmula
    static std::vector<std::vector<int>> readCNF(const std::string& filename, int& num_variables);
};

#endif
