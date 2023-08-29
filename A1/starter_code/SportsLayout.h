
#ifndef SPORTSLAYOUT_H
#define	SPORTSLAYOUT_H

#include <fstream>
#include <iostream>
#include <bits/stdc++.h>
using namespace std;

class SportsLayout{

    private:
    int z,l;
    vector<vector<int>> T;
    vector<vector<int>> N;
    int time;
    vector<int> mapping;
    vector<int> used;
    vector<int> not_used;
    public:

    SportsLayout(string inputfilename);

    bool check_output_format();

    // void readOutputFile(string output_filename);
    
    long long cost_fn();

    long long cost_fn(vector<int> &state);

    long long cost_fn_swap(vector<int> &state, long long curr_cost, int i, int j);

    void write_to_file(string outputfilename);

    void readInInputFile(string inputfilename);

    void compute_allocation();

    vector<int> hill_climbing_random_restarts(int max_restarts, std::chrono::high_resolution_clock::time_point start_time);
    vector<int> hill_climbing_random_walks(double prob, std::chrono::high_resolution_clock::time_point start_time);
    vector<int> hill_climbing_random_walks_restarts(int max_restarts, double prob, std::chrono::high_resolution_clock::time_point start_time);
    pair<vector<int>,long long> get_neighbour(vector<int> &current, long long curr_cost);
    pair<vector<int>,long long> get_random_neighour(vector<int> &current, long long curr_cost);
    vector<int> get_random_state();
    double get_prob();

};


#endif