#ifndef MAP_H
#define MAP_H

struct Map
{
    double x, a, b, A, B, C, D;
    int K;

    Map(double x, double a, double b, int K, double A, double B, double C, double D);

    double next();
};

double smht(double x, double A, double B, double C, double D);

double maxsmht(double A, double B, double C, double D, int steps = 1000);

bool check_for_no_equilibra(double a, double b, int K, double A, double B, double C, double D, int j_min = -100, int j_max = 100);

#endif