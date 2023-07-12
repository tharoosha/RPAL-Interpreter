#define ASTTOST_HPP
#define ASTTOST_HPP
#include <cstring>
#include <fstream>

using namespace std;

void ASTToST();
void lambdaConvertion(Node *lnode);
void recursivelyStandardizer(Node *root);
void whereConvertion(Node *whereNode);
void letConvertion(Node *letNode);
void recConvertion(Node *recNode);
void withinConvertion(Node *withinNode);
void andConvertion(Node *andHeaderNode) 
void infixConvertion(Node *infixOperatorNode);
void functionFormConvertion(Node *functionFormNode);

