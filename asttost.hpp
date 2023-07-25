#ifndef ASTTOST_HPP
#define ASTTOST_HPP
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>

using namespace std;

// struct Node { // For the first child next sibling binary tree representation of nary trees.
//     string label;
//     struct Node* firstKid;
//     struct Node* nextSibling;
// };

// extern stack<Node*> trees;

void convertFunctionForm(Node *functionFormNode);
void convertInfixOperator(Node *infixOperatorNode);
void convertLambdaExpression(Node *lambdaNode);
void convertRecExpression(Node *recNode);
void convertWhereExpression(Node *whereNode);
void convertWithinExpression(Node *withinNode);
void convertAndExpression(Node *andHeaderNode);
void convertLetExpression(Node *letNode);
void recursivelyStandardizeTree(Node *node);
void convertASTToStandardizedTree();

#endif