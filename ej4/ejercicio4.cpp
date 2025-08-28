#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cmath>
#include <sys/time.h>
#include <iomanip>

using namespace std;

// Variables globales
vector<long long> primes;
mutex primes_mutex;

// Función para verificar si un número es primo usando la lista de primos ya encontrados
bool is_prime(long long n, const vector<long long>& known_primes)
{
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;

    long long sqrt_n = sqrt(n);
    for (long long p : known_primes)
    {
        if (p > sqrt_n) break;
        if (n % p == 0) return false;
    }
    return true;
}

// Función que ejecuta cada hilo para buscar primos en un rango
void find_primes_in_range(long long start, long long end, const vector<long long>& base_primes)
{
    // Asegurarnos de empezar con números impares
    if (start % 2 == 0) start++;

    for (long long i = start; i <= end; i += 2)
    {
        if (is_prime(i, base_primes))
        {
            lock_guard<mutex> lock(primes_mutex);
            primes.push_back(i);
        }
    }
}

// Versión sin hilos
void find_primes_single_thread(long long N)
{
    primes.clear();
    primes.push_back(2);

    // Encontrar primos base hasta sqrt(N)
    long long sqrt_N = sqrt(N);
    for (long long i = 3; i <= sqrt_N; i += 2)
    {
        if (is_prime(i, primes))
        {
            primes.push_back(i);
        }
    }

    // Encontrar el resto de primos
    long long start = sqrt_N + 1;
    if (start % 2 == 0) start++;

    for (long long i = start; i <= N; i += 2)
    {
        if (is_prime(i, primes))
        {
            primes.push_back(i);
        }
    }
}

// Versión con hilos
void find_primes_multi_thread(long long N, int num_threads)
{
    primes.clear();
    primes.push_back(2);

    // Paso 1: Encontrar primos base hasta sqrt(N)
    long long sqrt_N = sqrt(N);
    for (long long i = 3; i <= sqrt_N; i += 2)
    {
        if (is_prime(i, primes))
        {
            primes.push_back(i);
        }
    }

    vector<long long> base_primes = primes;

    // Paso 2: Crear hilos para buscar en diferentes rangos
    vector<thread> threads;
    long long chunk_size = (N - sqrt_N) / num_threads;

    for (int i = 0; i < num_threads; i++)
    {
        long long start = sqrt_N + 1 + i * chunk_size;
        long long end = (i == num_threads - 1) ? N : start + chunk_size - 1;

        // Ajustar para que empiece con número impar
        if (start % 2 == 0) start++;

        threads.emplace_back(find_primes_in_range, start, end, cref(base_primes));
    }

    // Esperar a que todos los hilos terminen
    for (auto& t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    // Paso 3: Ordenar los primos encontrados
    sort(primes.begin(), primes.end());
}

int main()
{
    long long N;
    int num_threads;

    cout << "Ingrese el valor de N (minimo 10000000): ";
    cin >> N;

    if (N < 10000000)
    {
        cout << "N debe ser al menos 10^7" << endl;
        return 1;
    }

    cout << "Ingrese el numero de hilos: ";
    cin >> num_threads;

    // // Medir tiempo versión single thread
    timeval time1, time2;
    gettimeofday(&time1, NULL);

    find_primes_single_thread(N);

    gettimeofday(&time2, NULL);
    double single_time = double(time2.tv_sec - time1.tv_sec) + double(time2.tv_usec - time1.tv_usec) / 1000000;

    int single_count = primes.size();
    vector<long long> single_primes = primes;

    // Medir tiempo versión multi thread
    gettimeofday(&time1, NULL);

    find_primes_multi_thread(N, num_threads);

    gettimeofday(&time2, NULL);
    double multi_time = double(time2.tv_sec - time1.tv_sec) + double(time2.tv_usec - time1.tv_usec) / 1000000;

    int multi_count = primes.size();

    // Mostrar resultados
    cout << "\n--- RESULTADOS ---" << endl;
    cout << "Cantidad de primos menores que " << N << ": " << multi_count << endl;

    // Mostrar los 10 mayores primos
    cout << "Los 10 mayores primos encontrados:" << endl;
    int start_index = max(0, multi_count - 10);
    for (int i = start_index; i < multi_count; i++)
    {
        cout << primes[i] << " ";
    }
    cout << endl;

    // Mostrar tiempos y speedup
    cout << fixed << setprecision(6);
    cout << "\nTiempo single thread: " << single_time << " segundos" << endl;
    cout << "Tiempo multi thread: " << multi_time << " segundos" << endl;
    cout << "Speedup: " << single_time / multi_time << endl;

    // Verificar que ambos métodos encontraron la misma cantidad
    if (single_count != multi_count)
    {
        cout << "ADVERTENCIA: Los dos métodos encontraron cantidades diferentes de primos!" << endl;
        cout << "Single thread: " << single_count << ", Multi thread: " << multi_count << endl;
    }

    return 0;
}
