#include "astTost.hpp"
#include "iostream"
#include <fstream>
#include <cstring>

using namespace std;

void ASTToST(){
    if(!trees.empty()){
        Node *rootNode = trees.top();
        recursivelyStandardizer(rootNode);
    }
}

void lambdaConvertion(Node *lnode){
    lnode->label = LAMBDA_STD_LABEL;
    list<Node*> charList;
    Node *temp = lnode->firstKid;

    while(temp->nextSibling->nextSibling!=NULL){
        charList.push_back(temp->nextSibling);
        temp = temp->nextSibling;
    }
    temp=temp->nextSibling;

    Node *lnodeTemp=lnode;
    while(!charList.empty()){
        Node *newLNode = new Node();
        lnodeTemp->firstKid->nextSibling = newLNode;
        newLNode->nextSibling = NULL;
        newLNode->label=LAMBDA_STD_LABEL;
        lnodeTemp = newLNode;
        lnodeTemp->firstKid = charList.front();
        charList.pop_front();
    }
    lnodeTemp->firstKid->nextSibling = temp;
}

void whereConvertion(Node *whereNode){
    whereNode->label=GAMMA_STD_LABEL;
    Node *pNode=whereNode->firstKid;
    Node *LCNode=pNode->nextSibling;
    LCNode->label=LAMBDA_STD_LABEL;
    Node *eNode=LCNode->firstKid->nextSibling;	

    whereNode->firstKid=LCNode;

    LCNode->nextSibling=eNode;
    LCNode->firstKid->nextSibling=pNode;


}

void letConvertion(Node *letNode){
    letNode->label = LET_STD_LABEL;
    letNode->firstKid->label = LAMBDA_STD_LABEL;
    Node *pNode = letNode->firstKid->nextSibling;
    Node *eNode = letNode->firstKid->firstKid->nextSibling;

    letNode->firstKid->nextSibling = eNode;
    letNode->firstKid->firstKid->nextSibling = pNode;
}

void recConvertion(Node *recNode){
     Node *recNodeOriginalEqualsChild = recNode->firstKid;

    recNode->label = recNodeOriginalEqualsChild->label;
    recNode->firstKid = recNodeOriginalEqualsChild->firstKid;

    Node *rightGammaChild = new Node;
    rightGammaChild->label = GAMMA_STD_LABEL;
    rightGammaChild->nextSibling = NULL;
    Node *rightRightLambdaChild = new Node;
    rightRightLambdaChild->label = LAMBDA_STD_LABEL;
    rightRightLambdaChild->nextSibling = NULL;

    Node *leftChildYNode = new Node;
    leftChildYNode->label = "Y";
    leftChildYNode->firstKid = NULL;

    rightGammaChild->firstKid = leftChildYNode;
    leftChildYNode->nextSibling = rightRightLambdaChild;

    Node *functionNameNode = new Node;
    functionNameNode->label = recNode->firstKid->label;
    functionNameNode->firstKid = NULL;

    rightRightLambdaChild->firstKid = functionNameNode;
    functionNameNode->nextSibling = recNode->firstKid->nextSibling; //E

    recNode->firstKid->nextSibling = rightGammaChild;
}

void withinConvertion(Node *withinNode) {
    withinNode->label = "=";

    Node *withinOne = withinNode->firstKid;
    Node *withinTwo = withinOne->nextSibling;

    Node *rightGammaChild = new Node;
    Node *rightLeftLambdaChild = new Node;
    rightGammaChild->label = GAMMA_STD_LABEL;
    rightGammaChild->nextSibling = NULL;
    rightLeftLambdaChild->label = LAMBDA_STD_LABEL;

    rightGammaChild->firstKid = rightLeftLambdaChild;
    rightLeftLambdaChild->nextSibling = withinOne->firstKid->nextSibling; //E1
    rightLeftLambdaChild->firstKid = withinOne->firstKid; //X1
    rightLeftLambdaChild->firstKid->nextSibling = withinTwo->firstKid->nextSibling; //E2

    withinNode->firstKid = withinTwo->firstKid; //X2
    withinNode->firstKid->nextSibling = rightGammaChild;


}

void andConvertion(Node *andHeaderNode) {
    andHeaderNode->label = "=";
    Node *tempEqualsChildHeaderNode = andHeaderNode->firstKid;
    list<Node *> variableNodesList;
    list<Node *> expressionNodesList;
    while (tempEqualsChildHeaderNode != NULL) {
        variableNodesList.push_back(tempEqualsChildHeaderNode->firstKid);
        expressionNodesList.push_back(tempEqualsChildHeaderNode->firstKid->nextSibling);
        tempEqualsChildHeaderNode = tempEqualsChildHeaderNode->nextSibling;
    }

    Node *commaHeaderNode = new Node;
    Node *tauHeaderNode = new Node;

    commaHeaderNode->label = ",";
    tauHeaderNode->label = "tau";
    tauHeaderNode->nextSibling = NULL;
    commaHeaderNode->nextSibling = tauHeaderNode;

    andHeaderNode->firstKid = commaHeaderNode;

    Node *commaVariableTempNode, *tauExpressionTempNode;

    commaVariableTempNode = variableNodesList.front();
    variableNodesList.pop_front();
    tauExpressionTempNode = expressionNodesList.front();
    expressionNodesList.pop_front();

    commaHeaderNode->firstKid = commaVariableTempNode;

    tauHeaderNode->firstKid = tauExpressionTempNode;

    while (!variableNodesList.empty()) {
        commaVariableTempNode->nextSibling = variableNodesList.front();
        variableNodesList.pop_front();
        tauExpressionTempNode->nextSibling = expressionNodesList.front();
        expressionNodesList.pop_front();
        commaVariableTempNode = commaVariableTempNode->nextSibling;
        tauExpressionTempNode = tauExpressionTempNode->nextSibling;
    }

    commaVariableTempNode->nextSibling = NULL;
    tauExpressionTempNode->nextSibling = NULL;

}

void infixConvertion(Node *infixOperatorNode) {
    Node *leftGammaChild = new Node;
    Node *leftLeftOperatorChild = new Node;
    leftLeftOperatorChild->label = infixOperatorNode->firstKid->nextSibling->label;
    leftLeftOperatorChild->nextSibling = infixOperatorNode->firstKid; //E1
    leftLeftOperatorChild->firstKid = NULL;
    leftGammaChild->label = GAMMA_STD_LABEL;
    leftGammaChild->firstKid = leftLeftOperatorChild;
    leftGammaChild->nextSibling = infixOperatorNode->firstKid->nextSibling->nextSibling; //E2
    infixOperatorNode->firstKid->nextSibling = NULL;
    infixOperatorNode->firstKid = leftGammaChild;
    infixOperatorNode->label = GAMMA_STD_LABEL;
}

void functionFormConvertion(Node *functionFormNode) {
    Node *fcnLambdaRightChildNodeHeader = new Node; //the "lambda" right child node header of the final standardized sub-tree
    fcnLambdaRightChildNodeHeader->label = LAMBDA_STD_LABEL;
    fcnLambdaRightChildNodeHeader->nextSibling = NULL;

    list<Node *> fcnVariableList;
    functionFormNode->label = "=";  //the "=" header node of the final standardized sub-tree

    Node *temp = functionFormNode->firstKid;    //the fcn label left child node of the final standardized sub-tree
        while (temp->nextSibling->nextSibling !=NULL) { //temp->nextSibling->nextSibling == NULL implies temp->nextSibling is the "Expression" part of the fcnForm
        temp = temp->nextSibling;
        fcnVariableList.push_back(temp);
        }
    functionFormNode->firstKid->nextSibling = fcnLambdaRightChildNodeHeader;
    fcnLambdaRightChildNodeHeader->nextSibling = NULL;
    fcnLambdaRightChildNodeHeader->firstKid = fcnVariableList.front();
    Node *lambdaTemp = fcnLambdaRightChildNodeHeader;
    fcnVariableList.pop_front();
    while (fcnVariableList.size() > 0) {
        Node *newLambdaRightNode = new Node;
        lambdaTemp->firstKid->nextSibling = newLambdaRightNode;
        newLambdaRightNode->label = LAMBDA_STD_LABEL;
        newLambdaRightNode->nextSibling = NULL;
        lambdaTemp = newLambdaRightNode;
        lambdaTemp->firstKid = fcnVariableList.front();
        fcnVariableList.pop_front();
    }
}

void recursivelyStandardizer(Node *root){
    if(node->firstKid!=NULL){
        recursivelyStandardizer(node->firstKid);
    }
    if(node->nextSibling!=NULL){
        recursivelyStandardizer(node->nextSibling);
    }
    if (node->label=="->"){
        // not standardizing, just checking 
        -
    }else if(node->label=="not"||node->label=="neg"){
        // not standardizing, just checking it works
        //unary operator standardization
    }else if(node->label == "aug" || node->label == "or" || node->label == "&" || node->label == "gr" ||
               node->label == "ge" || node->label == "ls" || node->label == "le" || node->label == "eq" ||
               node->label == "ne" || node->label == "+" || node->label == "-" || node->label == "*" ||
               node->label == "/" || node->label == "**"){
        // binary operator standardization
        //not standardizing, just checking it works
    }else if(node->label=="tau"){
        // not standardizing, just checking it works
    }else if(node->label=="lambda"){
        if(node->firstKid->label==","){
            // not standardizing, just checking it works
        }else{
            lambdaConvertion(root);
        }
    }
    else if(node->label=="where"){
        whereConvertion(root);
    } 
    else if (node->label == "let") {
        letConvertion(node);
    }
    else if (node->label == "rec") {
        // not standardizing, just checking it works
        recConvertion(node);
       // not standardizing, just checking it works
    }
    else if (node->label == "within") {
//        cout << "\nGoing to convertWithinExpression\n";
        withinConvertion(node);
    }
     else if (node->label == "and") {
//        cout << "\nGoing to convertAndExpression\n";
        andConvertion(node);
    }
     else if (node->label == "@") {    //convert infix operator to standardized form
//        cout << "\nGoing to convertInfixOperator\n";
        infixConvertion(node);
    }
    else if (node->label == FCN_FORM_LABEL) {    //convert function_form to standardized form
//        cout << "\nGoing to convertFunctionForm\n";
        functionFormConvertion(node);
    }
}