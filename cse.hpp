#ifndef CSE_HPP
#define CSE_HPP
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>
#include <vector>
#include <cmath>
#include "psg.hpp"

using namespace std;

void processCSEMachine();
void initializeCSEMachine();
void printTree();
void recursivelyPrintTree(Node *node, string indentDots);
void recursivelyPrintTreeNode(Node *node, string indentDots);

void printString(std::string stringToPrint);
void runCSEMachine();
#endif