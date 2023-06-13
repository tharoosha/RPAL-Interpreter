#include <iostream>
#include <fstream>

using namespace std;

/*
    Representation of a node in nary tree
*/
struct Node {
    string label;
    struct Node *left;
    struct Note *right;
};

/*
    Checks if the end of the file has been reached using the good() function and peek() 
    function. The good() function checks if the stream is in a good state, and the peek() 
    function looks ahead to the next character without extracting it. 
*/
bool checkIfEOF(ifstream &file){
    if (!file.good() || file.peek() == EOF){
        return true;
    }
    return false;
}

/*
    
*/

