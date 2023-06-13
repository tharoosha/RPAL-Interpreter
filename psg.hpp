#ifndef PSG_HPP
#define PSG_HPP
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>

using namespace std;

struct Node { // For the first child next sibling binary tree representation of nary trees.
    string label;
    struct Node *firstKid;
    struct Node *nextSibling;
};

extern const string FCN_FORM_LABEL;

extern const string GAMMA_STD_LABEL;
extern const string LAMBDA_STD_LABEL;

extern stack<Node *> trees;

void buildTree(string nodeLabel, int noOfTreesToPopAndMakeChildren);

void D(ifstream &file);

void readToken(ifstream &file, string token);
int Vl(ifstream &file, int identifiersReadBefore, bool isRecursiveCall);
void Vb(ifstream &file);
void Db(ifstream &file);
void Dr(ifstream &file);
void Da(ifstream &file);
void D(ifstream &file);
void Rn(ifstream &file);
void R(ifstream &file);
void Ap(ifstream &file);
void Af(ifstream &file);
void At(ifstream &file);
void A(ifstream &file);
void Bp(ifstream &file);
void Bs(ifstream &file);
void Bt(ifstream &file);
void B(ifstream &file);
void Tc(ifstream &file);
void Ta(ifstream &file);
void T(ifstream &file);
void Ew(ifstream &file);
void E(ifstream &file);


void E(ifstream &file);

#endif //RPAL_INTERPRETER_PSG_H
