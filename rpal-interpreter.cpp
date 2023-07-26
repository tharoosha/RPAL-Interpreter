#include <iostream>
#include <fstream>
#include <cstring>
#include "lexicon.hpp"
#include "psg.hpp"
#include "asttost.hpp"
#include "flattenst.hpp"
#include "cse.hpp"


using namespace std;

// /*
//     Representation of a node in nary tree
// */
// struct Node {
//     string label;
//     struct Node *left;
//     struct Note *right;
// };

int main(int argc, char* argv[]){
    
    if (argc == 3){
        string filename = argv[2];

        ifstream file(filename);

        if (!file) {
            cout << "Failed to open the file." << endl;
            return 1;
        };
        
        scan(file); //Prepare the first token by placing it within 'NT'
        E(file);    //Call the first non-terminal procedure to start parsing

        if (checkIfEOF(file)) {
        // cout << "\n\nEOF successfully reached after complete parsing! Will exit now!!\n\n";
        // exit(1);
            // runAndShowOutput();
            printTree();
            convertASTToStandardizedTree();
            flattenStandardizedTree();
            runCSEMachine();
            cout << "\n";
        } else {
            cout <<"\n\nERROR! EOF not reached but went through the complete grammar! Will exit now!!\n\n";
            exit(0);
        };
        // string fileContent;
        // string line;
        // while (getline(file, line)) {
        //     fileContent += line + '\n';
        // }

        // cout << "File contents:\n" << fileContent << endl;

        
        file.close();

        return 0;
    }else {
        string filename = argv[1];

        ifstream file(filename);

        if (!file) {
            cout << "Failed to open the file." << endl;
            return 1;
        };
        
        scan(file); //Prepare the first token by placing it within 'NT'
        E(file);    //Call the first non-terminal procedure to start parsing

        if (checkIfEOF(file)) {
        // cout << "\n\nEOF successfully reached after complete parsing! Will exit now!!\n\n";
        // exit(1);
            // runAndShowOutput();
            convertASTToStandardizedTree();
            flattenStandardizedTree();
            runCSEMachine();
            cout << "\n";
        } else {
            cout <<"\n\nERROR! EOF not reached but went through the complete grammar! Will exit now!!\n\n";
            exit(0);
        };
        // string fileContent;
        // string line;
        // while (getline(file, line)) {
        //     fileContent += line + '\n';
        // }

        // cout << "File contents:\n" << fileContent << endl;

        
        file.close();

        return 0;
    }
    
};


