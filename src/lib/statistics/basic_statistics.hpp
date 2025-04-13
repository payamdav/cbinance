#pragma once
#include <vector>
#include <utility>
#include <string>


using namespace std;

vector<double> i2d(const vector<int>& data);
double sum(const vector<double>& data);
double mean(const vector<double>& data);
double variance(const vector<double>& data);
double std_dev(const vector<double>& data);
double median(const vector<double>& data);
double max(const vector<double>& data);
double min(const vector<double>& data);
double range(const vector<double>& data);
vector<double> quantiles(const vector<double>& data, int q);
pair<vector<int>, vector<double>> histogram(const vector<double>& data, int bins_number, string filename="");

