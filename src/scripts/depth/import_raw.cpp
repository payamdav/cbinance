#include "../../lib/ob/raw_files/ob_raw_files.hpp"
#include <iostream>
#include <string>


using namespace std;

int main(int argc, char *argv[]) {
    cout << "Importing depth raw files" << endl;
    int start_id = 0;
    int end_id = 0;
    string file_type = "snapshot";

    if (argc == 3) {
        file_type = argv[1];
        start_id = stoi(argv[2]);
        end_id = start_id;
    }
    else if (argc == 4) {
        file_type = argv[1];
        start_id = stoi(argv[2]);
        end_id = stoi(argv[3]);
    }
    else {
        cout << "Usage: import_raw <snapshot/update> <file_id_from> <file_id_to>" << endl;
        return 1;
    }
    
    if (file_type == "snapshot") {
        ob::ObRawFiles ob_raw_files;
        ob_raw_files.import_snapshot(start_id, end_id);
    }
    else if (file_type == "update") {
        ob::ObRawFiles ob_raw_files;
        ob_raw_files.import_update(start_id, end_id);
    }
    else {
        cout << "Invalid file type: " << file_type << endl;
        cout << "Usage: import_raw <snapshot/update> <file_id_from> <file_id_to>" << endl;
        return 1;
    }

    return 0;
}
