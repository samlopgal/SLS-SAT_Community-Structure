# WalkSAT informado por estructura comunitaria
<a id="indice"></a>

## Índice

1. [Descripción general](#descripcion-general)
2. [Requisitos](#requisitos)
3. [Compilación](#compilacion)
4. [Ejecución](#ejecucion)
5. [Estructura esperada de carpetas](#estructura-esperada-de-carpetas)
6. [Precomputados de comunidades](#precomputados-de-comunidades)
7. [Formato de las instancias](#formato-de-las-instancias)
8. [Plantilla de configuración](#plantilla-de-configuracion)
9. [Cómo organizar un experimento usando la plantilla](#como-organizar-un-experimento-usando-la-plantilla)
10. [Archivos principales del proyecto](#archivos-principales-del-proyecto)
11. [Familias de solvers](#familias-de-solvers)
12. [Añadir un nuevo algoritmo](#anadir-un-nuevo-algoritmo)
13. [Resultados generados](#resultados-generados)

<a id="descripcion-general"></a>

## Descripción general
Este proyecto implementa y compara variantes de WalkSAT para instancias SAT en formato DIMACS CNF. El framework permite ejecutar varios solvers sobre grupos de instancias aleatorias y con estructura de comunidades, registrar resultados por instancia/semilla y generar resúmenes agregados en CSV.

El código está organizado como un proyecto Eclipse CDT C/C++ con compilación mediante MinGW GCC. También puede compilarse fuera de Eclipse siempre que se respeten las dependencias externas.


<a id="requisitos"></a>

## Requisitos


<a id="version-minima-de-c"></a>

### Versión mínima de C++

El código requiere como mínimo **C++17**, porque utiliza `std::filesystem` y características modernas del lenguaje. La configuración `Release` incluida en `.cproject` compila con:

```bash
-std=c++20 -DNDEBUG -march=native -fopenmp
```

Por tanto, se recomienda compilar con **C++20**, aunque C++17 debería ser suficiente para las construcciones usadas en el código actual.


<a id="compilador-y-entorno-usados"></a>

### Compilador y entorno usados

Según los archivos de configuración de Eclipse incluidos, el proyecto se preparó como:

- Proyecto Eclipse CDT.
- Toolchain: **MinGW GCC**.
- Configuraciones: `Debug` y `Release`.
- Librería de sistema usada desde MSYS2/MinGW64: `C:\msys64\mingw64\lib`.
- Paralelización con **OpenMP**.
- Detección de comunidades mediante **igraph**.
- Parseo JSON mediante **nlohmann/json**.

La configuración `Release` enlaza con:

```bash
-fopenmp -ligraph -lgomp
```

La configuración `Debug` también activa OpenMP, pero no fija explícitamente `-std=c++20`; si aparecen errores relacionados con `std::filesystem` o características modernas de C++, añade manualmente `-std=c++20` también en `Debug`.


<a id="dependencias-externas"></a>

### Dependencias externas

El proyecto necesita:

- Un compilador compatible con C++17 o C++20.
- OpenMP.
- igraph C library, con cabeceras accesibles como `<igraph/igraph.h>`.
- nlohmann/json, con cabecera accesible como `<nlohmann/json.hpp>`.

En MSYS2/MinGW64, normalmente las dependencias se instalan en el entorno `mingw64`. En Linux, la ruta de librerías puede ser distinta y habría que ajustar el enlazado.


<a id="compilacion"></a>

## Compilación


<a id="desde-eclipse"></a>

### Desde Eclipse

1. Importa el proyecto como proyecto C/C++ existente.
2. Asegúrate de importar también `.project`, `.cproject` y `.settings/`.
3. Selecciona la configuración `Release`.
4. Comprueba que las rutas de igraph corresponden a tu instalación.
5. Ejecuta `Build Project`.


<a id="desde-terminal"></a>

### Desde terminal

Si las dependencias están instaladas y el compilador encuentra las cabeceras, una compilación equivalente sería:

```bash
g++ -std=c++20 -O3 -DNDEBUG -march=native -fopenmp src/*.cpp -o walksat_framework -ligraph -lgomp
```

En algunos sistemas puede bastar con:

```bash
g++ -std=c++20 -O3 -fopenmp src/*.cpp -o walksat_framework -ligraph
```

Si `nlohmann/json.hpp` o `igraph/igraph.h` no están en rutas estándar, añade las opciones `-I` y `-L` necesarias.


<a id="ejecucion"></a>

## Ejecución

El programa recibe uno o varios archivos JSON de configuración como argumentos:

```bash
./walksat_framework config_template.json
```

En Windows, si se usa el ejecutable generado por Eclipse:

```bash
Release\WalkSat_Informado_FRAME_New.exe config_template.json
```

También se pueden ejecutar varios experimentos en secuencia:

```bash
./walksat_framework config_random.json config_community.json config_paws.json
```

Si no se pasa ningún argumento, `main.cpp` intenta usar:

```text
src/prueba.json
```

Por eso se recomienda pasar siempre la ruta del JSON explícitamente.


<a id="estructura-esperada-de-carpetas"></a>

## Estructura esperada de carpetas

La raíz de instancias se configura con:

```json
"instances": {
  "root_path": "instancias"
}
```

La raíz es configurable, pero el árbol interno esperado por `InstanceScanner.cpp` es fijo.


<a id="instancias-aleatorias"></a>

### Instancias aleatorias

Para `"types": ["random"]`, el código busca archivos `.cnf` en:

```text
<root_path>/random/n<N>/r_<R>/*.cnf
```

Ejemplo:

```text
instancias/random/n100/r_420/formula_001.cnf
instancias/random/n100/r_420/formula_002.cnf
```


<a id="instancias-con-comunidades"></a>

### Instancias con comunidades

Para `"types": ["community"]`, el código busca archivos `.cnf` en:

```text
<root_path>/community_instances/q_<Q>/n<N>/r_<R>/*.cnf
```

Ejemplo:

```text
instancias/community_instances/q_500/n100/r_420/formula_001.cnf
instancias/community_instances/q_500/n100/r_420/formula_002.cnf
```


<a id="arbol-recomendado"></a>

### Árbol recomendado

```text
proyecto/
├── src/
├── instancias/
│   ├── random/
│   │   └── n100/
│   │       └── r_420/
│   │           ├── formula_001.cnf
│   │           └── formula_002.cnf
│   └── community_instances/
│       └── q_500/
│           └── n100/
│               └── r_420/
│                   ├── formula_001.cnf
│                   └── formula_002.cnf
├── precomputed/
├── results/
└── config_template.json
```


<a id="precomputados-de-comunidades"></a>

## Precomputados de comunidades

Los solvers estructurales necesitan información comunitaria precomputada. Esta información se guarda en archivos binarios `.pcd`.

La ruta raíz se configura con:

```json
"precomputed": {
  "root_path": "precomputed"
}
```

Para instancias aleatorias, los `.pcd` se guardan en:

```text
precomputed/random/n<N>/r_<R>/<nombre_instancia>.pcd
```

Para instancias comunitarias, los `.pcd` se guardan en:

```text
precomputed/community_instances/q_<Q>/n<N>/r_<R>/<nombre_instancia>.pcd
```

La función `getPrecomputedPath` de `PrecomputeUtils.cpp` trata correctamente como equivalentes los tipos internos `community` y `community_instances`, generando siempre la ruta comunitaria con el nivel `q_<Q>`.

En la versión actual, `main.cpp` ejecuta una fase inicial de comprobación de precomputados antes de lanzar el experimento. Si faltan archivos `.pcd`, los genera mediante:

1. Lectura de la fórmula CNF.
2. Construcción de la VIG ponderada.
3. Detección de comunidades con Louvain de igraph.
4. Construcción de `PrecomputedCommunityData`.
5. Escritura binaria del `.pcd`.

La clave `auto_generate_if_missing` está presente en la configuración y se carga por compatibilidad, pero el flujo actual llama siempre a la fase de comprobación/generación de precomputados al inicio.


<a id="formato-de-las-instancias"></a>

## Formato de las instancias

Las instancias deben estar en formato DIMACS CNF. `FileToVector.cpp` interpreta:

- Líneas de comentario que empiezan por `c`.
- Cabecera `p cnf <num_variables> <num_clauses>`.
- Cláusulas terminadas en `0`.

Ejemplo mínimo:

```text
c ejemplo
p cnf 3 2
1 -2 0
2 3 -1 0
```


<a id="plantilla-de-configuracion"></a>

## Plantilla de configuración

Se incluye una plantilla ejecutable en:

```text
config_template.json
```

La plantilla contiene todos los bloques principales y todos los parámetros leídos por `ConfigLoader.cpp`.


<a id="bloque-experiment-name"></a>

### Bloque `experiment_name`

```json
"experiment_name": "template_experiment"
```

Nombre lógico del experimento. Se usa para identificar la ejecución, aunque los nombres de salida se controlan en `logging`.


<a id="bloque-instances"></a>

### Bloque `instances`

```json
"instances": {
  "root_path": "instancias",
  "types": ["random", "community"],
  "n_values": [100],
  "ratios": [420],
  "q_values": [500],
  "instances_per_group": 10
}
```

Parámetros:

- `root_path`: carpeta raíz donde están las instancias.
- `types`: tipos de instancia a escanear. Valores implementados: `random` y `community`.
- `n_values`: valores de número de variables usados para construir las rutas `n<N>`.
- `ratios`: valores usados para construir las rutas `r_<R>`.
- `q_values`: valores usados solo por instancias `community`, construyendo rutas `q_<Q>`.
- `instances_per_group`: máximo de archivos `.cnf` que se cargan por combinación de tipo, `n`, `q` y `ratio`. Si es menor o igual que cero, no se aplica límite.


<a id="bloque-precomputed"></a>

### Bloque `precomputed`

```json
"precomputed": {
  "root_path": "precomputed",
  "auto_generate_if_missing": true
}
```

Parámetros:

- `root_path`: carpeta raíz donde se guardan o buscan los `.pcd`.
- `auto_generate_if_missing`: campo de compatibilidad. En el flujo actual, el programa comprueba y genera los `.pcd` que falten al inicio.


<a id="bloque-solvers"></a>

### Bloque `solvers`

Cada solver se declara así:

```json
{
  "name": "MakeCount",
  "enabled": true,
  "params": {
    "max_flips": 100000,
    "max_flips_per_variable": 0.0,
    "max_tries": 10,
    "p": 0.5,
    "seed": 42,
    "seeds": [42, 43, 44],
    "alpha": 1.0,
    "lambda": 1.0,
    "p_soft": 0.05,
    "maxinc": 20
  }
}
```

Solvers registrados en `SolverFactory.cpp`:

| Nombre en JSON | Descripción |
|---|---|
| `BreakCount` | WalkSAT con criterio BreakCount. |
| `MakeCount` | WalkSAT con criterio MakeCount. |
| `BridgeBreak` | Variante estructural que usa cláusulas inter-comunidad y criterio tipo Break. |
| `BridgeMakeBreak` | Variante estructural que usa cláusulas inter-comunidad y criterio Make-Break. |
| `CommunityBreak` | Variante estructural que usa cláusulas intra-comunidad y criterio tipo Break. |
| `CommunityMakeBreak` | Variante estructural que usa cláusulas intra-comunidad y criterio Make-Break. |
| `PAWS_Break_DET` | BreakCount con ponderación PAWS determinista. |
| `PAWS_Make_DET` | MakeCount con ponderación PAWS determinista. |
| `BridgeBreakPAWS_DET` | BridgeBreak con PAWS determinista. |
| `BridgeMakeBreakPAWS_DET` | BridgeMakeBreak con PAWS determinista. |
| `CommunityBreakPAWS_DET` | CommunityBreak con PAWS determinista. |
| `CommunityMakeBreakPAWS_DET` | CommunityMakeBreak con PAWS determinista. |

Parámetros de solver:

- `max_flips`: límite máximo de flips si `max_flips_per_variable` es `0.0` o menor.
- `max_flips_per_variable`: si es mayor que cero, el límite efectivo se calcula como `max_flips_per_variable * num_variables` y sustituye a `max_flips`.
- `max_tries`: número máximo de reinicios/intentonas por ejecución.
- `p`: probabilidad de elegir un movimiento aleatorio en lugar de usar la heurística.
- `seed`: semilla única usada si no se define `seeds`.
- `seeds`: lista de semillas. Si aparece y no está vacía, se ejecuta una vez por cada semilla.
- `alpha`: parámetro leído y registrado para compatibilidad/extensión.
- `lambda`: parámetro leído y registrado para compatibilidad/extensión.
- `p_soft`: parámetro leído para variantes PAWS probabilísticas; las variantes actuales registradas son deterministas.
- `maxinc`: intervalo de suavizado de PAWS determinista. Cada `maxinc` eventos de penalización se reducen pesos mayores que uno. Si `maxinc <= 0`, el suavizado queda desactivado.

`ConfigLoader.cpp` también acepta los alias `paws_p_soft` para `p_soft` y `paws_maxinc` para `maxinc`, pero la plantilla usa los nombres canónicos.


<a id="bloque-logging"></a>

### Bloque `logging`

```json
"logging": {
  "output_dir": "results/template_experiment",
  "instance_results_file": "instance_results.csv",
  "summary_results_file": "summary_results.csv"
}
```

Genera:

- `instance_results.csv`: una fila por solver, instancia y semilla.
- `summary_results.csv`: resultados agregados por solver, tipo de instancia, `q`, `n`, `ratio`, semilla y parámetros.
- `auto_stopping_events.csv`: eventos de parada automática si se activa `auto_stopping`.


<a id="bloque-execution"></a>

### Bloque `execution`

```json
"execution": {
  "threads": 4
}
```

Controla los hilos OpenMP usados durante la ejecución del experimento. Si `threads <= 0`, se usa el máximo disponible reportado por OpenMP.

La fase de precomputación de `main.cpp` usa actualmente `omp_set_num_threads(8)` de forma fija.


<a id="bloque-auto-stopping"></a>

### Bloque `auto_stopping`

Formato recomendado:

```json
"auto_stopping": {
  "patience_ratios": 0
}
```

También se acepta el formato corto:

```json
"auto_stopping": 2
```

Significado:

- `0`: desactivado.
- `N > 0`: dentro de cada grupo `(type, n, q)`, un solver se desactiva tras `N` ratios consecutivos sin resolver ninguna instancia.


<a id="como-organizar-un-experimento-usando-la-plantilla"></a>

## Cómo organizar un experimento usando la plantilla


<a id="1-preparar-las-instancias"></a>

### 1. Preparar las instancias

Primero decide qué familias vas a comparar.

Para aleatorias:

```text
instancias/random/n100/r_420/*.cnf
instancias/random/n100/r_430/*.cnf
```

Para comunitarias:

```text
instancias/community_instances/q_500/n100/r_420/*.cnf
instancias/community_instances/q_500/n100/r_430/*.cnf
```

Los nombres de carpeta deben coincidir exactamente con los valores del JSON.

Si en el JSON pones:

```json
"n_values": [100],
"ratios": [420, 430],
"q_values": [500]
```

entonces el programa buscará:

```text
instancias/random/n100/r_420/
instancias/random/n100/r_430/
instancias/community_instances/q_500/n100/r_420/
instancias/community_instances/q_500/n100/r_430/
```


<a id="2-seleccionar-tipos-de-instancia"></a>

### 2. Seleccionar tipos de instancia

Para ejecutar solo aleatorias:

```json
"types": ["random"]
```

Para ejecutar solo comunitarias:

```json
"types": ["community"]
```

Para ejecutar ambas:

```json
"types": ["random", "community"]
```


<a id="3-ajustar-tamano-del-experimento"></a>

### 3. Ajustar tamaño del experimento

Para una prueba rápida:

```json
"instances_per_group": 2
```

Para un experimento completo con todos los archivos disponibles por grupo, puedes usar un valor alto o no restrictivo:

```json
"instances_per_group": 100
```


<a id="4-ajustar-presupuesto-de-busqueda"></a>

### 4. Ajustar presupuesto de búsqueda

Presupuesto fijo:

```json
"max_flips": 100000,
"max_flips_per_variable": 0.0
```

Presupuesto proporcional al número de variables:

```json
"max_flips": 100000,
"max_flips_per_variable": 1000.0
```

En el segundo caso, si la instancia tiene `n = 100`, el límite efectivo será `1000.0 * 100 = 100000` flips.


<a id="5-elegir-semillas"></a>

### 5. Elegir semillas

Para una única ejecución por instancia:

```json
"seed": 42,
"seeds": [42]
```

Para varias repeticiones:

```json
"seeds": [1, 2, 3, 4, 5]
```

El resumen se agrupa también por semilla, por lo que cada semilla queda separada en `summary_results.csv`.


<a id="6-activar-o-desactivar-solvers"></a>

### 6. Activar o desactivar solvers

Para excluir un solver sin borrarlo del JSON:

```json
"enabled": false
```

Para probar un algoritmo nuevo junto a una línea base, deja activados solo los necesarios:

```json
{
  "name": "MakeCount",
  "enabled": true,
  "params": { ... }
},
{
  "name": "MiNuevoSolver",
  "enabled": true,
  "params": { ... }
}
```


<a id="7-separar-salidas-por-experimento"></a>

### 7. Separar salidas por experimento

Cambia siempre `output_dir` para no sobrescribir resultados previos:

```json
"logging": {
  "output_dir": "results/exp_n100_q500_ratios_420_430",
  "instance_results_file": "instance_results.csv",
  "summary_results_file": "summary_results.csv"
}
```


<a id="8-ejecutar"></a>

### 8. Ejecutar

```bash
./walksat_framework config_template.json
```


<a id="archivos-principales-del-proyecto"></a>

## Archivos principales del proyecto

| Archivo | Función |
|---|---|
| `main.cpp` | Punto de entrada. Carga uno o varios JSON, asegura los precomputados y lanza `ExperimentRunner`. |
| `ExperimentConfig.h` | Define las estructuras de configuración: instancias, precomputados, solvers, logging, ejecución y auto-stopping. |
| `ConfigLoader.cpp/.h` | Lee el JSON mediante nlohmann/json y rellena `ExperimentConfig`. |
| `InstanceScanner.cpp/.h` | Escanea el árbol de carpetas y construye la lista de instancias `.cnf` a ejecutar. |
| `FileToVector.cpp/.h` | Lee archivos DIMACS CNF y los convierte en `std::vector<std::vector<int>>`. |
| `InstanceProcessing.cpp/.h` | Construye la VIG ponderada, ejecuta Louvain con igraph y caracteriza cláusulas por comunidades. |
| `PrecomputeUtils.cpp/.h` | Construye la ruta donde se guarda o busca cada archivo `.pcd`. |
| `PrecomputedCommunityBuilder.cpp/.h` | Construye `PrecomputedCommunityData` a partir de la fórmula y la asignación variable-comunidad. |
| `PrecomputedCommunityData.h` | Estructura que almacena comunidades de variables, comunidades por cláusula y estadísticas de comunidades. |
| `PrecomputedCommunityIO.cpp/.h` | Guarda y carga precomputados `.pcd` en binario. |
| `ExperimentRunner.cpp/.h` | Ejecuta los solvers sobre las instancias, aplica semillas, hilos, logging y auto-stopping. |
| `SolverFactory.cpp/.h` | Registra los nombres de solver aceptados en JSON y crea el objeto correspondiente. |
| `ISolver.h` | Interfaz común que deben cumplir los wrappers de solver. |
| `SolverObserver.h` | Interfaz de observación e instrumentación de flips, reinicios y métricas PAWS. |
| `GenericStatsCollector.cpp/.h` | Recoge métricas genéricas de trayectoria y comportamiento del solver. |
| `CSVLogger.cpp/.h` | Escribe resultados por instancia, resúmenes y eventos de auto-stopping. |
| `Statistics.cpp/.h` | Agrega resultados y calcula métricas resumen. |
| `SolutionVerifier.cpp/.h` | Verifica que una asignación reportada como SAT satisface realmente la fórmula. |
| `RandomSatGenerator.cpp/.h` | Generador auxiliar de instancias SAT aleatorias. |
| `Ocurrence.h` | Define ocurrencias variable-cláusula usadas por los solvers. |


<a id="familias-de-solvers"></a>

## Familias de solvers


<a id="solvers-globales"></a>

### Solvers globales

- `WalkSAT_BreakCount.*`
- `WalkSAT_MakeCount.*`
- Wrappers: `BreakCountSolver.h`, `MakeCountSolver.h`

Usan criterios globales clásicos basados en cláusulas que se romperían o se satisfarían al hacer un flip.


<a id="solvers-puente"></a>

### Solvers puente

- `WalkSAT_Bridge_Break.*`
- `WalkSAT_Bridge_MakeBreak.*`
- Wrappers: `BridgeBreakSolver.h`, `BridgeMakeBreakSolver.h`

Usan cláusulas que conectan más de una comunidad. En este proyecto, una cláusula puente es cualquier cláusula con:

```cpp
clause_communities[c].size() > 1
```

Por tanto, se agrupan como puente tanto cláusulas que conectan exactamente dos comunidades como cláusulas que involucran más de dos comunidades.


<a id="solvers-comunitarios"></a>

### Solvers comunitarios

- `WalkSAT_Community_Break.*`
- `WalkSAT_Community_MakeBreak.*`
- Wrappers: `CommunityBreakSolver.h`, `CommunityMakeBreakSolver.h`

Usan cláusulas contenidas en una sola comunidad:

```cpp
clause_communities[c].size() == 1
```


<a id="variantes-paws-deterministas"></a>

### Variantes PAWS deterministas

- `WalkSAT_BreakCount_PAWS_DET.*`
- `WalkSAT_MakeCount_PAWS_DET.*`
- `WalkSAT_Bridge_Break_PAWS_DET.*`
- `WalkSAT_Bridge_MakeBreak_PAWS_DET.*`
- `WalkSAT_Community_Break_PAWS_DET.*`
- `WalkSAT_Community_MakeBreak_PAWS_DET.*`

Añaden ponderación adaptativa de cláusulas. El parámetro relevante de suavizado es `maxinc`.


<a id="anadir-un-nuevo-algoritmo"></a>

## Añadir un nuevo algoritmo

Para añadir un nuevo algoritmo y poder testearlo desde JSON, hay que tocar varias piezas.


<a id="1-implementar-el-solver"></a>

### 1. Implementar el solver

Crea los archivos, por ejemplo:

```text
src/WalkSAT_MiAlgoritmo.cpp
src/WalkSAT_MiAlgoritmo.h
```

El solver debe poder:

- Recibir la fórmula.
- Recibir `num_variables`.
- Recibir una semilla.
- Ejecutar `solve(max_flips, max_tries, p, observer)` o una función equivalente.
- Devolver la asignación final.

La asignación debe seguir la convención del proyecto:

```text
assignment[1..num_variables]
```

La posición `0` no representa una variable DIMACS.


<a id="2-crear-un-wrapper-que-implemente-isolver"></a>

### 2. Crear un wrapper que implemente `ISolver`

Si el solver interno no hereda directamente de `ISolver`, crea un wrapper similar a:

```text
src/MiAlgoritmoSolver.h
```

Debe implementar:

```cpp
bool solve(
    int max_flips,
    int max_tries,
    double p,
    SolverObserver* observer
) override;

const std::vector<uint8_t>& getAssignment() const override;
```

Si tu solver necesita comunidades, el wrapper debe recibir también:

```cpp
const PrecomputedCommunityData* precomputed
```


<a id="3-registrar-el-solver-en-solverfactory-cpp"></a>

### 3. Registrar el solver en `SolverFactory.cpp`

Añade el `#include` del wrapper:

```cpp
#include "MiAlgoritmoSolver.h"
```

Y registra el nombre que se usará en JSON:

```cpp
if (solver_name == "MiAlgoritmo") {
    return std::make_unique<MiAlgoritmoSolver>(
        formula,
        num_variables,
        params.seed
    );
}
```

Si necesita precomputados:

```cpp
if (solver_name == "MiAlgoritmoEstructural") {
    if (precomputed == nullptr) {
        throw std::runtime_error(
            "MiAlgoritmoEstructural requires PrecomputedCommunityData."
        );
    }

    return std::make_unique<MiAlgoritmoEstructuralSolver>(
        formula,
        num_variables,
        params.seed,
        precomputed
    );
}
```


<a id="4-anadir-parametros-si-el-algoritmo-necesita-nuevos-valores"></a>

### 4. Añadir parámetros si el algoritmo necesita nuevos valores

Si el nuevo algoritmo solo usa parámetros existentes, no hay que tocar la configuración.

Parámetros ya disponibles:

- `seed`
- `alpha`
- `lambda`
- `p_soft`
- `maxinc`
- `p`
- `max_flips`
- `max_flips_per_variable`
- `max_tries`

Si necesitas otro parámetro, por ejemplo `beta`, modifica:

1. `SolverParamsConfig` en `ExperimentConfig.h`.
2. `ConfigLoader.cpp` para leerlo desde JSON.
3. `SolverParameters` en `SolverFactory.h`, si debe llegar a la factory.
4. `ExperimentRunner.cpp`, donde se copian parámetros desde `solver_cfg.params` a `SolverParameters`.
5. `SolverFactory.cpp`, para pasarlo al constructor del nuevo solver.
6. `CSVLogger` y `Statistics` si quieres que aparezca en los CSV y en los agrupamientos.


<a id="5-instrumentar-estadisticas"></a>

### 5. Instrumentar estadísticas

Para que el nuevo algoritmo contribuya a las métricas genéricas, llama al observer igual que los solvers existentes:

- `observer->onTryStart(...)` al iniciar cada intento.
- `observer->onFlip(...)` tras cada flip.
- `observer->onTryEnd(...)` al terminar un intento.
- `observer->onSolveEnd(...)` al terminar la ejecución.

En `onFlip`, usa `MoveOrigin::Random` si el movimiento ha sido aleatorio y `MoveOrigin::Heuristic` si lo ha elegido la heurística.

Si es un algoritmo con pesos PAWS, llama además a:

```cpp
observer->onPawsStep(...)
```


<a id="6-anadirlo-al-json"></a>

### 6. Añadirlo al JSON

Una vez registrado en `SolverFactory.cpp`, basta con declararlo en el archivo de configuración:

```json
{
  "name": "MiAlgoritmo",
  "enabled": true,
  "params": {
    "max_flips": 100000,
    "max_flips_per_variable": 0.0,
    "max_tries": 10,
    "p": 0.5,
    "seed": 42,
    "seeds": [42, 43, 44],
    "alpha": 1.0,
    "lambda": 1.0,
    "p_soft": 0.05,
    "maxinc": 20
  }
}
```


<a id="7-compilar-y-probar"></a>

### 7. Compilar y probar

Primero prueba con pocas instancias:

```json
"instances_per_group": 1,
"seeds": [42],
"max_flips": 10000
```

Después compara contra una línea base como `MakeCount` o `BreakCount`.


<a id="resultados-generados"></a>

## Resultados generados


<a id="instance-results-csv"></a>

### `instance_results.csv`

Contiene una fila por combinación:

```text
solver, instancia, seed
```

Incluye, entre otros campos:

- Solver.
- Tipo de instancia.
- `q`, `n`, `ratio`.
- Nombre de archivo.
- Semilla.
- Parámetros principales.
- Si el solver reportó solución.
- Si la solución fue verificada.
- Tiempo.
- Flips usados.
- Métricas de trayectoria.
- Métricas PAWS si aplican.


<a id="summary-results-csv"></a>

### `summary_results.csv`

Agrega resultados por:

```text
solver, instance_type, q, n, ratio, seed, p, maxinc, alpha, lambda
```

Incluye tasas de resolución, medias de tiempo, flips, mejor número de cláusulas insatisfechas, AUC normalizada, estancamiento y métricas PAWS.


<a id="auto-stopping-events-csv"></a>

### `auto_stopping_events.csv`

Se genera siempre, pero solo contiene eventos si `auto_stopping` está activado y algún solver se detiene.
