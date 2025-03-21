// Variable.h
#ifndef SIMULATION_DATA_H
#define SIMULATION_DATA_H

#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <sstream>

extern double RHO;
extern double INSERT_RHO;
extern double POWER;
extern double FR_POSITION;
extern double CR_POSITION;
extern double PN_LIFE;
extern double T_INTERVAL;
extern double BETA_EFF;
extern double END_TIME;

extern std::map<int, double> BETA;
extern std::map<int, double> LAMBDA;
extern std::map<int, double> PRECURSOR;

extern std::map<double, double> INTERPOLATED_CR;
extern std::map<double, double> INTERPOLATED_FR;
extern std::map<double, std::tuple<double, double, double>> INTERPOLATED_HISTORY;

double findWorth(const std::map<double, double>& data, double position);
void calculateInsertRho(double time);
void loadSimulationData(const std::string& filename);
void loadHistoryData(const std::string& filename);
void printSimulationData();
void printHistoryData();

std::tuple<double, double> findRodPosition(double time);


#endif // SIMULATION_DATA_H
