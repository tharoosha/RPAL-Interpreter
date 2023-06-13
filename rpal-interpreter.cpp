#include <iostream>
#include <fstream>
#include <cstring>
#include "lexicon.hpp"

using namespace std;

// /*
//     Representation of a node in nary tree
// */
// struct Node {
//     string label;
//     struct Node *left;
//     struct Note *right;
// };

int main(){
    string filename;
    cout << "Enter the file name: ";
    cin >> filename;

    ifstream file(filename);

    if (!file) {
        cout << "Failed to open the file." << endl;
        return 1;
    }

    
    scan(file);

    // string fileContent;
    // string line;
    // while (getline(file, line)) {
    //     fileContent += line + '\n';
    // }

    // cout << "File contents:\n" << fileContent << endl;

    
    file.close();

    return 0;
};


