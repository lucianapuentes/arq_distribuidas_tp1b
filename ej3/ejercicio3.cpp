#include <iostream>
#include <vector>
#include <thread>
#include <iomanip>
#include <sys/time.h>
#include <cmath>

using namespace std;

// Función para inicializar matrices con valores
void initialize_matrix(vector<vector<float>>& matrix, int size, float value) {
    matrix.resize(size, vector<float>(size, value));
}

// Función para multiplicar matrices (versión sin hilos)
float matrix_multiply_sequential(const vector<vector<float>>& A,
                                const vector<vector<float>>& B,
                                vector<vector<float>>& C) {
    int n = A.size();
    float total_sum = 0.0f;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0.0f;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
            total_sum += C[i][j];
        }
    }

    return total_sum;
}

// Función que ejecuta cada hilo para multiplicar un grupo de filas
void multiply_rows_range(const vector<vector<float>>& A,
                        const vector<vector<float>>& B,
                        vector<vector<float>>& C,
                        int start_row, int end_row,
                        float& partial_sum) {
    int n = A.size();
    partial_sum = 0.0f;

    for (int i = start_row; i <= end_row; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0.0f;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
            partial_sum += C[i][j];
        }
    }
}

// Función para multiplicar matrices con hilos
float matrix_multiply_parallel(const vector<vector<float>>& A,
                              const vector<vector<float>>& B,
                              vector<vector<float>>& C,
                              int num_threads) {
    int n = A.size();
    vector<thread> threads;
    vector<float> partial_sums(num_threads, 0.0f);
    float total_sum = 0.0f;

    int rows_per_thread = n / num_threads;
    int extra_rows = n % num_threads;

    int current_row = 0;
    for (int i = 0; i < num_threads; i++) {
        int rows_this_thread = rows_per_thread + (i < extra_rows ? 1 : 0);
        int end_row = current_row + rows_this_thread - 1;

        threads.emplace_back(multiply_rows_range,
                           cref(A), cref(B), ref(C),
                           current_row, end_row,
                           ref(partial_sums[i]));

        current_row = end_row + 1;
    }

    // Esperar a que todos los hilos terminen
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Sumar los resultados parciales
    for (float sum : partial_sums) {
        total_sum += sum;
    }

    return total_sum;
}

// Función para mostrar las esquinas de una matriz
void show_corners(const vector<vector<float>>& matrix, const string& name) {
    int n = matrix.size();
    if (n == 0) return;

    cout << "\nEsquinas de la matriz " << name << ":" << endl;
    cout << fixed << setprecision(6);

    // Esquina superior izquierda
    cout << matrix[0][0] << " ... " << matrix[0][n-1] << endl;
    cout << "  ...   ...  " << endl;
    // Esquina inferior derecha
    cout << matrix[n-1][0] << " ... " << matrix[n-1][n-1] << endl;
}

// Función para medir el tiempo de ejecución
double get_time() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int main() {
    int N, num_threads;

    cout << "Multiplicacion de matrices NxN" << endl;
    cout << "Ingrese el tamaño N de las matrices (minimo 100): ";
    cin >> N;

    if (N < 100) {
        cout << "El tamaño debe ser al menos 100" << endl;
        return 1;
    }

    cout << "Ingrese el numero de hilos (recomendado 10-20): ";
    cin >> num_threads;

    if (num_threads <= 0 || num_threads > N) {
        cout << "Numero de hilos invalido" << endl;
        return 1;
    }

    // Crear e inicializar matrices
    vector<vector<float>> A, B, C_seq(N, vector<float>(N)), C_par(N, vector<float>(N));

    initialize_matrix(A, N, 0.1f);
    initialize_matrix(B, N, 0.2f);

    cout << "\nMatrices inicializadas con valores 0.1 y 0.2" << endl;
    show_corners(A, "A");
    show_corners(B, "B");

    // Versión secuencial
    cout << "\n--- VERSION SECUENCIAL ---" << endl;
    double start_time = get_time();
    float sum_seq = matrix_multiply_sequential(A, B, C_seq);
    double seq_time = get_time() - start_time;

    show_corners(C_seq, "Resultado secuencial");
    cout << "Sumatoria total: " << scientific << setprecision(6) << sum_seq << endl;
    cout << fixed << setprecision(6);
    cout << "Tiempo secuencial: " << seq_time << " segundos" << endl;

    // Versión paralela
    cout << "\n--- VERSION PARALELA (" << num_threads << " hilos) ---" << endl;
    start_time = get_time();
    float sum_par = matrix_multiply_parallel(A, B, C_par, num_threads);
    double par_time = get_time() - start_time;

    show_corners(C_par, "Resultado paralelo");
    cout << "Sumatoria total: " << scientific << setprecision(6) << sum_par << endl;
    cout << fixed << setprecision(6);
    cout << "Tiempo paralelo: " << par_time << " segundos" << endl;

    // Calcular speedup
    double speedup = seq_time / par_time;
    cout << "\n--- RESULTADOS ---" << endl;
    cout << "Speedup: " << speedup << endl;
    cout << "Eficiencia: " << (speedup / num_threads) * 100 << "%" << endl;

    // Verificar que los resultados sean iguales
    float tolerance = 1e-6f;
    if (abs(sum_seq - sum_par) > tolerance) {
        cout << "ADVERTENCIA: Los resultados no coinciden!" << endl;
        cout << "Diferencia: " << abs(sum_seq - sum_par) << endl;
    } else {
        cout << "Resultados verificados correctamente" << endl;
    }

    return 0;
}