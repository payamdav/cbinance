#pragma once
#include "raw_files/ob_raw_files.hpp"
#include <string>
#include <vector>
#include "snapshot.hpp"


using namespace std;


class Update {
    public:
        string symbol;
        string binary_path;
        string index_path;

        ifstream update_idx;
        ifstream update_data;

        Update(string symbol);
        ~Update();

        void open();
        void close();

        bool is_sorted_by_t();
        bool is_sorted_by_u_id();
        bool is_pu_aligned();
        bool is_index_size_correct();
        bool is_offset_correct();

        size_t count();
        UpdateIdx get_index(size_t idx);
        void get_update(const UpdateIdx& update_idx, vector<double>& bp, vector<double>& bv, vector<double>& ap, vector<double>& av);
};
