#include "depth_file.hpp"
#include <iomanip>




string string_characters_filter(string str, string filter) {
    string result;
    for (char c : str) {
        if (filter.find(c) != string::npos) {
            result += c;
        }
    }
    return result;
}


vector<Level> parse_levels(string line) {
    vector<Level> levels;
    stringstream ss(line);
    string token;
    while (getline(ss, token, '[')) {
        token = string_characters_filter(token, "0123456789.,");
        // cout << "Token: " << token << endl;
        // split token by comma
        stringstream token_stream(token);
        string price_str, volume_str;
        getline(token_stream, price_str, ',');
        getline(token_stream, volume_str, ',');
        // cout << "price_str: " << price_str << endl;
        // cout << "volume_str: " << volume_str << endl;
        // convert to double
        if (price_str.empty() || volume_str.empty()) {
            continue;
        }
        double price = stod(price_str);
        double volume = stod(volume_str);
        // cout << "price: " << price << endl;
        // cout << "volume: " << volume << endl;
        // add to levels
        Level level;
        level.price = price;
        level.volume = volume;
        levels.push_back(level);
        // cout << "Level: " << level.price << ", " << level.volume << endl;
        // cout << "Level: " << string_characters_filter(token, "0123456789.,") << endl;
        // cout << "Level: " << token << endl;
    }
    // for (int i = 0; i < levels.size(); i++) {
    //     cout << "Level " << i << ": " << levels[i].price << ", " << levels[i].volume << endl;
    // }
    // cout << "Levels size: " << levels.size() << endl;
    return levels;
}

DepthSnapshot::DepthSnapshot(PipLevelizer levelizer) : levelizer(levelizer), bids_volumes(levelizer.levels.size(), 0), asks_volumes(levelizer.levels.size(), 0), bids_min_level(levelizer.levels.size() - 1), bids_max_level(0), asks_min_level(levelizer.levels.size() - 1), asks_max_level(0) {}

void DepthSnapshot::parse_string(string line) {
    // cout << "Parsing line: " << line << endl;
    // cout << "line length: " << line.length() << endl;

    size_t bids_pos = line.find("bids");
    size_t asks_pos = line.find("asks");
    size_t ts_pos = line.find("T");
    size_t end_pos = line.size() - 1;


    ts = stoll(line.substr(ts_pos + 3, 13));
    // cout << "ts: " << ts << endl;

    bids = parse_levels(line.substr(bids_pos + 6, asks_pos - bids_pos - 7));
    asks = parse_levels(line.substr(asks_pos + 6, end_pos - asks_pos - 7));

}

void DepthSnapshot::calculate_levelized_volumes() {
    for (size_t i = 0; i < bids.size(); i++) {
        size_t l = levelizer.get_level_binary_search(bids[i].price);
        bids_volumes[l] += bids[i].volume;
        if (l < bids_min_level) bids_min_level = l;
        if (l > bids_max_level) bids_max_level = l;
    }
    for (size_t i = 0; i < asks.size(); i++) {
        size_t l = levelizer.get_level_binary_search(asks[i].price);
        asks_volumes[l] += asks[i].volume;
        if (l < asks_min_level) asks_min_level = l;
        if (l > asks_max_level) asks_max_level = l;
    }
}

void DepthSnapshot::write_levelized_to_file(string filename) {
    // open file for appending and if file does not exist create it
    ofstream file(filename, ios::app);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }
    file << "ts:" << ts << ",";
    file << "bids:" << bids_min_level << ":";
    for (size_t i = bids_min_level; i <= bids_max_level; i++) {
        file << (int)bids_volumes[i] << ",";
    }
    file << "asks:" << asks_min_level << ":";
    for (size_t i = asks_min_level; i <= asks_max_level; i++) {
        file << (int)asks_volumes[i];
        if (i != asks_max_level) file << ",";
    }
    file << endl;

    file.close();
}


void process_file(string filename, PipLevelizer levelizer) {
    cout << "Processing file: " << filename << endl;
    const string base_path = "/home/payam/data/depth/";
    string full_path = base_path + filename;
    cout << "Full path: " << full_path << endl;
    // read file line by line
    ifstream file(full_path);
    string line;
    int line_counter = 0;
    while (getline(file, line)) {
        line_counter++;
        DepthSnapshot snapshot(levelizer);
        snapshot.parse_string(line);
        snapshot.calculate_levelized_volumes();

        // cout << "ts: " << snapshot.ts << " bids_level_count: " << snapshot.bids_max_level - snapshot.bids_min_level + 1 << " asks_level_count: " << snapshot.asks_max_level - snapshot.asks_min_level + 1 << endl;
        // if (line_counter == 1) break; // todo: remove this line
        snapshot.write_levelized_to_file(base_path + "levelized.txt");
    }
    file.close();
    cout << "Number of lines: " << line_counter << endl;
}






Level_Depth::Level_Depth() : ts(0), bids_min_level(0), bids_max_level(0), asks_min_level(0), asks_max_level(0) {}

void Level_Depth::parse_string(string line) {
    // cout << "Parsing line: " << line << endl;
    // cout << "line length: " << line.length() << endl;

    size_t bids_pos = line.find("bids");
    size_t asks_pos = line.find("asks");
    size_t end_pos = line.size() - 1;

    ts = stoll(line.substr(3, 16));
    // cout << "ts: " << ts << endl;

    string bids_str = line.substr(bids_pos + 5, asks_pos - bids_pos - 6);
    string asks_str = line.substr(asks_pos + 5, end_pos - asks_pos - 6 + 2); // +2 for the last comma befor asks and -1 in calc of end_pos
    // cout << "bids_str: " << bids_str << endl;
    // cout << "asks_str: " << asks_str << endl;

    stringstream bids_stream(bids_str);
    stringstream asks_stream(asks_str);
    string token;
    getline(bids_stream, token, ':');
    bids_min_level = stoi(token);
    // cout << "bids_min_level: " << bids_min_level << endl;
    while (getline(bids_stream, token, ',')) {
        bids.push_back(stoi(token));
        // cout << "bids: " << bids.back() << endl;
    }
    // cout << "bids size: " << bids.size() << endl;
    getline(asks_stream, token, ':');
    asks_min_level = stoi(token);
    // cout << "asks_min_level: " << asks_min_level << endl;
    while (getline(asks_stream, token, ',')) {
        asks.push_back(stoi(token));
        // cout << "asks: " << asks.back() << endl;
    }
    // cout << "asks size: " << asks.size() << endl;
    bids_max_level = bids_min_level + bids.size() - 1;
    asks_max_level = asks_min_level + asks.size() - 1;
    // cout << "bids_max_level: " << bids_max_level << endl;
    // cout << "asks_max_level: " << asks_max_level << endl;
    // cout << "bids size: " << bids.size() << endl;
    // cout << "asks size: " << asks.size() << endl;


}

int Level_Depth::get_bids_volume(size_t level) {
    if (level < bids_min_level || level > bids_max_level) {
        cerr << "Level out of range: " << level << endl;
        return 0;
    }
    return bids[level - bids_min_level];
}

int Level_Depth::get_asks_volume(size_t level) {
    if (level < asks_min_level || level > asks_max_level) {
        cerr << "Level out of range: " << level << endl;
        return 0;
    }
    return asks[level - asks_min_level];
}


Level_Depths::Level_Depths() {
    // constructor
    // cout << "Level_Depths constructor" << endl;
}

void Level_Depths::read_from_file(string filename) {
    const string base_path = "/home/payam/data/depth/";
    string full_path = base_path + filename;
    cout << "Reading from file: " << full_path << endl;
    ifstream file(full_path);
    if (!file) {
        cerr << "Error opening file: " << full_path << endl;
        return;
    }
    string line;
    int line_counter = 0;
    while (getline(file, line)) {
        line_counter++;
        Level_Depth level_depth;
        level_depth.parse_string(line);
        if (is_ts_exist(level_depth.ts)) {
            cout << "Timestamp already exists: " << level_depth.ts << endl;
            continue;
        }
        this->push_back(level_depth);
        // if (line_counter == 1) break; // todo: remove this line
    }
    file.close();
    cout << "Number of lines: " << line_counter << endl;
}


bool Level_Depths::is_sorted() {
    for (size_t i = 1; i < this->size(); i++) {
        if ((*this)[i].ts < (*this)[i - 1].ts) {
            return false;
        }
    }
    return true;
}

bool Level_Depths::is_ts_exist(long long ts) {
    for (size_t i = 0; i < this->size(); i++) {
        if ((*this)[i].ts == ts) {
            return true;
        }
    }
    return false;
}

size_t Level_Depths::min_bids_level() const {
    size_t min_level = (*this)[0].bids_min_level;
    for (size_t i = 1; i < this->size(); i++) {
        if ((*this)[i].bids_min_level < min_level) {
            min_level = (*this)[i].bids_min_level;
        }
    }
    return min_level;
}

size_t Level_Depths::max_bids_level() const {
    size_t max_level = (*this)[0].bids_max_level;
    for (size_t i = 1; i < this->size(); i++) {
        if ((*this)[i].bids_max_level > max_level) {
            max_level = (*this)[i].bids_max_level;
        }
    }
    return max_level;
}

size_t Level_Depths::min_asks_level() const {
    size_t min_level = (*this)[0].asks_min_level;
    for (size_t i = 1; i < this->size(); i++) {
        if ((*this)[i].asks_min_level < min_level) {
            min_level = (*this)[i].asks_min_level;
        }
    }
    return min_level;
}

size_t Level_Depths::max_asks_level() const {
    size_t max_level = (*this)[0].asks_max_level;
    for (size_t i = 1; i < this->size(); i++) {
        if ((*this)[i].asks_max_level > max_level) {
            max_level = (*this)[i].asks_max_level;
        }
    }
    return max_level;
}

Level_Depth Level_Depths::get_level_depth(long long ts) {
    size_t last_index = 0;
    for (size_t i = 0; i < this->size(); i++) {
        if ((*this)[i].ts > ts) {
            return (*this)[last_index];
        }
        last_index = i;
    }
    return (*this)[last_index];
}

Level_Depths Level_Depths::get_level_depths(vector<long long> tss) {
    Level_Depths level_depths;
    for (size_t i = 0; i < tss.size(); i++) {
        level_depths.push_back(get_level_depth(tss[i]));
        level_depths.back().ts = tss[i];
    }
    return level_depths;
}

Level_Depths Level_Depths::get_level_depths(long long ts1, long long ts2, long long step) {
    Level_Depths level_depths;
    for (long long ts = ts1; ts <= ts2; ts += step) {
        level_depths.push_back(get_level_depth(ts));
        level_depths.back().ts = ts;
    }
    return level_depths;
}

vector<int> Level_Depths::bids_volumes() {
    vector<int> volumes;
    for (auto & level_depth : *this) {
        for (auto & bid : level_depth.bids) {
            volumes.push_back(bid);
        }
    }
    return volumes;
}

vector<int> Level_Depths::asks_volumes() {
    vector<int> volumes;
    for (auto & level_depth : *this) {
        for (auto & ask : level_depth.asks) {
            volumes.push_back(ask);
        }
    }
    return volumes;
}

vector<int> Level_Depths::volumes() {
    vector<int> volumes;
    for (auto & level_depth : *this) {
        for (auto & bid : level_depth.bids) {
            volumes.push_back(bid);
        }
        for (auto & ask : level_depth.asks) {
            volumes.push_back(ask);
        }
    }
    return volumes;
}

Level_Depths Level_Depths::filter(long long ts1, long long ts2) {
    Level_Depths level_depths;
    for (size_t i = 0; i < this->size(); i++) {
        if ((*this)[i].ts >= ts1 && (*this)[i].ts <= ts2) {
            level_depths.push_back((*this)[i]);
        }
    }
    return level_depths;
}

int Level_Depths::average_asks_volume() {
    int sum = 0;
    int count = 0;
    for (auto & level_depth : *this) {
        for (auto & ask : level_depth.asks) {
            sum += ask;
            count++;
        }
    }
    return count == 0 ? 0 : sum / count;
}

int Level_Depths::average_bids_volume() {
    int sum = 0;
    int count = 0;
    for (auto & level_depth : *this) {
        for (auto & bid : level_depth.bids) {
            sum += bid;
            count++;
        }
    }
    return count == 0 ? 0 : sum / count;
}

int Level_Depths::max_asks_volume() {
    int max_volume = 0;
    for (auto & level_depth : *this) {
        for (auto & ask : level_depth.asks) {
            if (ask > max_volume) {
                max_volume = ask;
            }
        }
    }
    return max_volume;
}

int Level_Depths::max_bids_volume() {
    int max_volume = 0;
    for (auto & level_depth : *this) {
        for (auto & bid : level_depth.bids) {
            if (bid > max_volume) {
                max_volume = bid;
            }
        }
    }
    return max_volume;
}

