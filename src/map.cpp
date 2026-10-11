#include "map.h"

#include <cmath>
#include <iostream>

double smht(double x, double A, double B, double C, double D)
{
    return (std::exp(A*x) - std::exp(-B*x))/(std::exp(C*x) + std::exp(-D*x));
}

Map::Map(double x, double a, double b, int K, double A, double B, double C, double D) : x(x), a(a), b(b), K(K), A(A), B(B), C(C), D(D) {}

double Map::next()
{
    return x = std::fmod(x + a + b * smht(x/K, A, B, C, D), K);
}

double maxsmht(double A, double B, double C, double D, int steps)
{
    double max_val = -1e9;
    for (int i = 0; i <= steps; ++i) {
        double x = static_cast<double>(i) / steps;
        double val = smht(x, A, B, C, D);
        if (val > max_val) {
            max_val = val;
        }
    }
    return max_val;
}

bool check_for_no_equilibra(double a, double b, int K, double A, double B, double C, double D, int j_min, int j_max)
{
    if (A>C || B>D)
        return false;
    double max_smht = maxsmht(A, B, C, D);
    double interval_min = 0.0;
    double interval_max = max_smht;
    double a0 = std::fmod(a,K);
    for (int j = j_min; j <= j_max; ++j) {
        double term = (-a0 + j * K) / b;

        if (term >= interval_min && term <= interval_max) {
            std::cout << "Znaleziono naruszenie dla j = " << j << " (wartość: " << term << " mieści się w przedziale)\n";
            return false;
        }
    }

    return true;
}
