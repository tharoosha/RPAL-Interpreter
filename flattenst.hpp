#ifndef FLATTENST_HPP
#define FLATTENST_HPP
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>
#include <vector>

using namespace std;

struct MachineNode { // Node abstraction for the CSE machine for both the control and stack
    string nameValue;
    bool isName;    //whether it's an identifier
    bool isString;
    string stringValue;
    bool isGamma;
    bool isLambda;
    std::vector<string> boundVariables;
    int indexOfBodyOfLambda; //index of the controlStructure of this lambda expression
    bool isTau;  //refers to the control stack variable which will convert stack elements to tuple
    int numberOfElementsInTauTuple;
    bool isTuple;  //refers to the CSE stack structure variable containing variables
    std::vector<MachineNode> tupleElements; //can be either int/bool/string
    bool isComma;
    bool isEnvironmentMarker;
    int environmentMarkerIndex;  //for a lambda, it means the environment in which it was placed on the stack.
    bool isInt;
    int intValue;
    bool isConditional;
    bool isUnaryOperator;
    bool isBinaryOperator;
    string operatorStringValue;
    string defaultLabel;
    bool isBoolean;
    bool isBuiltInFunction;
    bool isY;
    bool isYF;
    bool isDummy;

    MachineNode() {
        isName = false;
        isString = false;
        isGamma = false;
        isLambda = false;
        isTau = false;
        isEnvironmentMarker = false;
        isInt = false;
        isConditional = false;
        isUnaryOperator = false;
        isBinaryOperator = false;
        isComma = false;
        isBoolean = false;
        isBuiltInFunction = false;
        isTuple = false;
        isY = false;
        isYF = false;
        isDummy = false;
    }
};

struct EnvironmentNode { //Node abstraction for an environment marker in the CSE machine
    EnvironmentNode *parentEnvironment;
    EnvironmentNode *previousEnvironment;
    MachineNode boundedValuesNode;
    //the bounded values node will have the bounded variable mappings from the boundedVariables string vector to the tupleElements MachineNode vector.
    // boundedVariables (string) -> tupleElements (MachineNode) [the tupleElement could be int/string/Lambda]
    int environmentIndex;
};

extern vector<list<MachineNode>> controlStructures; // Declaration of the 'controlStructures' vector
extern int numberOfControlStructures;

void recursivelyFlattenTree(Node *treeNode, list<MachineNode> *controlStructure, int controlStructureIndex,bool processKid, bool processSiblings);
void flattenStandardizedTree();

#endif