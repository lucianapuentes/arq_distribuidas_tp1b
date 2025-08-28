#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <cmath>
#include <sys/time.h>

#define M 1000000000

inline double seconds()
{
    timeval t{};
    gettimeofday(&t, nullptr);
    return t.tv_sec + t.tv_usec * 1e-6;
}

// --- Versión secuencial ---
long double ej1secuencial(long double x)
{
    long double y = (x - 1) / (x + 1);
    long double p = y; // y^(2*0+1)
    long double sum = p / 1; // primer término

    for (long n = 1; n < M; n++)
    {
        p *= y * y; // y^(2n+1) a partir del anterior
        sum += p / (2 * n + 1);
    }

    return 2.0L * sum;
}

// --- Worker para multihilo ---
void worker(long double y, long long k0, long long k1, long double& out)
{
    long double p = powl(y, 2.0L * k0 + 1.0L);
    long double s = 0.0L;

    for (long long k = k0; k < k1; k++)
    {
        s += p / (2 * k + 1);
        p *= y * y;
    }
    out = s;
}

// --- Versión multihilo ---
long double ej1hilos(long double x, int P)
{
    long double y = (x - 1) / (x + 1);
    long long chunk = M / P;

    std::vector<std::thread> threads;
    std::vector<long double> partials(P, 0.0L);

    for (int t = 0; t < P; t++)
    {
        long long k0 = t * chunk;
        long long k1 = (t == P - 1) ? M : (t + 1) * chunk;
        threads.emplace_back(worker, y, k0, k1, std::ref(partials[t]));
    }
    for (auto& th : threads) th.join();

    long double sum = 0.0L;
    for (int t = 0; t < P; t++) sum += partials[t];

    return 2.0L * sum;
}

int main()
{
    long double x;
    int P;

    std::cout << "Número a calcular logaritmo natural: ";
    std::cin >> x;
    std::cout << "Número de hilos en la paralelización: ";
    std::cin >> P;

    // --- Secuencial ---
    double t1 = seconds();
    long double res1 = ej1secuencial(x);
    double t2 = seconds();

    std::cout << std::setprecision(15)
        << "\n[Secuencial]"
        << "\nResultado: " << res1
        << "\nTiempo: " << (t2 - t1) << " s\n";

    // --- Multihilo ---
    double t3 = seconds();
    long double res2 = ej1hilos(x, P);
    double t4 = seconds();

    std::cout << std::setprecision(15)
        << "\n[Multihilo]"
        << "\nResultado: " << res2
        << "\nTiempo: " << (t4 - t3) << " s\n";

    // --- Comparación ---
    std::cout << "\n[Comparación]";
    std::cout << "\nSpeedup: " << (t2 - t1) / (t4 - t3) << "x";
    std::cout << std::setprecision(15) << "\nDiferencia entre resultados: " << std::abs(res1 - res2) << "\n";

    return 0;
}
