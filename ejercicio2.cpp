#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>

// Función para contar ocurrencias
int contarOcurrencias(const std::string& linea, const std::string& patron) {
    int count = 0;
    size_t pos = linea.find(patron);

    while (pos != std::string::npos) {
        count++;
        pos = linea.find(patron, pos + patron.length());
    }
    return count;
}

//  Conteo secuencial
double versionSecuencial(const std::vector<std::string>& texto,
                         const std::vector<std::string>& patrones,
                         std::vector<int>& conteo) {
    auto inicio = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < patrones.size(); i++) {
        for (const auto& l : texto) {
            conteo[i] += contarOcurrencias(l, patrones[i]);
        }
    }

    auto fin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duracion = fin - inicio;
    return duracion.count();
}

// Conteo paralelo
void buscarPatron(const std::vector<std::string>& texto,
                  const std::vector<std::string>& patrones,
                  std::vector<int>& conteo, size_t indice) {
    for (const auto& linea : texto) {
        conteo[indice] += contarOcurrencias(linea, patrones[indice]);
    }
}

double versionParalela(const std::vector<std::string>& texto,
                       const std::vector<std::string>& patrones,
                       std::vector<int>& conteo) {
    auto inicio = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> hilos;
    size_t nHilos = std::min((size_t)32, patrones.size());

    for (size_t i = 0; i < nHilos; i++) {
        hilos.emplace_back(buscarPatron, std::cref(texto), std::cref(patrones), std::ref(conteo), i);
    }
    for (auto& h : hilos) {
        h.join();
    }

    auto fin = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duracion = fin - inicio;
    return duracion.count();
}

// MAIN
int main() {
    // Leer patrones
    std::ifstream archivoPatrones("patrones.txt");
    if (!archivoPatrones.is_open()) {
        std::cerr << "No se pudo abrir patrones.txt\n";
        return 1;
    }
    std::vector<std::string> patrones;
    std::string linea;
    while (std::getline(archivoPatrones, linea)) {
        patrones.push_back(linea);
    }
    archivoPatrones.close();

    // Leer archivo donde buscar
    std::ifstream archivoTexto("texto.txt");
    if (!archivoTexto.is_open()) {
        std::cerr << "No se pudo abrir texto.txt\n";
        return 1;
    }
    std::vector<std::string> texto;
    while (std::getline(archivoTexto, linea)) {
        texto.push_back(linea);
    }
    archivoTexto.close();

    //Ejecutar versión secuencial
    std::vector<int> conteoSecuencial(patrones.size(), 0);
    double tiempoSecuencial = versionSecuencial(texto, patrones, conteoSecuencial);

    std::cout << "\n Resultados SECUENCIAL\n";
    for (size_t i = 0; i < patrones.size(); i++) {
        std::cout << "Patron " << i << " aparece " << conteoSecuencial[i] << " veces\n";
    }
    std::cout << "Tiempo secuencial: " << tiempoSecuencial << " segundos\n";

    // Ejecutar versión paralela
    std::vector<int> conteoParalelo(patrones.size(), 0);
    double tiempoParalelo = versionParalela(texto, patrones, conteoParalelo);

    std::cout << "\n Resultados PARALELO (32 hilos)\n";
    for (size_t i = 0; i < patrones.size(); i++) {
        std::cout << "Patron " << i << " aparece " << conteoParalelo[i] << " veces\n";
    }
    std::cout << "Tiempo paralelo: " << tiempoParalelo << " segundos\n";

    // Calcular speedup
    if (tiempoParalelo > 0) {
        double speedup = tiempoSecuencial / tiempoParalelo;
        std::cout << "\n Speedup obtenido: " << speedup << " \n";
    }

    return 0;
}
