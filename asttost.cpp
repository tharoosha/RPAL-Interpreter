#include "psg.hpp"
#include "lexicon.hpp"
#include "asttost.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>

void convertFunctionForm(Node *functionFormNode) {
//    cout<<"\nInside function form conversion!\nFunction form is:\n";
//    if(functionFormNode->nextSibling != NULL) {
//        cout<<"functionFormNode's sibling is: "<<functionFormNode->nextSibling->label<<"\n";
//    }
//    recursivelyPrintTreeNode(functionFormNode, "");
    Node *fcnLambdaRightChildNodeHeader = new Node; //the "lambda" right child node header of the final standardized sub-tree
    fcnLambdaRightChildNodeHeader->label = LAMBDA_STD_LABEL;
    fcnLambdaRightChildNodeHeader->nextSibling = NULL;

    list<Node *> fcnVariableList;
    functionFormNode->label = "=";  //the "=" header node of the final standardized sub-tree

    Node *temp = functionFormNode->firstKid;    //the fcn label left child node of the final standardized sub-tree
    while (temp->nextSibling->nextSibling !=
           NULL) { //temp->nextSibling->nextSibling == NULL implies temp->nextSibling is the "Expression" part of the fcnForm
        temp = temp->nextSibling;
        fcnVariableList.push_back(temp);
    }
    //temp = temp->nextSibling; //this would be the expression part of the fcn form //the final expression node which is the rightMost child of the right sub-tree

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
    //lambdaTemp->firstKid->nextSibling = temp;
//    cout<<"\nInside function form conversion!\nThe standardized Function form is:\n";
//    recursivelyPrintTreeNode(functionFormNode, "");
//
//    if(functionFormNode->nextSibling != NULL) {
//        cout<<"functionFormNode's sibling is: "<<functionFormNode->nextSibling->label<<"\n";
//    }
}

void convertInfixOperator(Node *infixOperatorNode) {
    //cout<<"\nInside Infix Operator conversion!\nInfix ast form before standardizing is:\n";
    //recursivelyPrintTree(opNode, "");
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
    //cout<<"\nThe standardized Infix operator is:\n";
    //recursivelyPrintTree(opNode, "");
}

void convertLambdaExpression(Node *lambdaNode) {
//    cout << "\nInside lambda expression conversion!\n lambda expression ast form before standardizing is:\n";
//    recursivelyPrintTreeNode(lambdaNode, "");
    lambdaNode->label = LAMBDA_STD_LABEL;

    list<Node *> fcnVariableList;

    Node *temp = lambdaNode->firstKid;    //the top left child node of the final standardized lambda sub-tree
    while (temp->nextSibling->nextSibling !=
           NULL) { //temp->nextSibling->nextSibling == NULL implies temp->nextSibling is the "Expression" part of the fcnForm
        temp = temp->nextSibling;
        fcnVariableList.push_back(temp);
    }
    temp = temp->nextSibling; //this would be the expression part of the fcn form //the final expression node which is the rightMost child of the right sub-tree

    Node *lambdaTemp = lambdaNode;
    while (fcnVariableList.size() > 0) {
        Node *newLambdaRightNode = new Node;
        lambdaTemp->firstKid->nextSibling = newLambdaRightNode;
        newLambdaRightNode->nextSibling = NULL;
        newLambdaRightNode->label = LAMBDA_STD_LABEL;
        lambdaTemp = newLambdaRightNode;
        lambdaTemp->firstKid = fcnVariableList.front();
        fcnVariableList.pop_front();
    }
    lambdaTemp->firstKid->nextSibling = temp; // E
//    cout << "\nThe standardized lambda expression is:\n";
//    recursivelyPrintTreeNode(lambdaNode, "");
}

void convertRecExpression(Node *recNode) {
//    cout<< "\nThe recNode label is: "<<recNode->label<<"\n";
//    cout << "\nInside convertRecExpression conversion!\nrecNode ast form before standardizing is:\n";
//    recursivelyPrintTreeNode(recNode, "");
//    cout<< "\nThe recNode label is: "<<recNode->label<<"\n";
//
//    if(recNode->nextSibling != NULL) {
//        cout<<"recNode's sibling is: "<<recNode->nextSibling->label<<"\n";
//    }
//
//    if(recNode->firstKid->nextSibling != NULL) {
//        cout<<"recNode's firstKid's sibling is: "<<recNode->firstKid->nextSibling->label<<"\n";
//    }

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

//    cout<< "\nThe recNode label is: "<<recNode->label<<"\n";
//    cout << "\nInside convertRecExpression conversion!\nrecNode ast form after standardizing is:\n";
//    recursivelyPrintTreeNode(recNode, "");
//    cout<< "\nThe recNode label is: "<<recNode->label<<"\n";

}

void convertWhereExpression(Node *whereNode) {
    whereNode->label = GAMMA_STD_LABEL;


    Node *pNode = whereNode->firstKid;
    Node *leftChildLambdaNode = pNode->nextSibling;
    leftChildLambdaNode->label = LAMBDA_STD_LABEL;
    Node *eNode = leftChildLambdaNode->firstKid->nextSibling;

    whereNode->firstKid = leftChildLambdaNode;

    //switch the p and e nodes

    leftChildLambdaNode->nextSibling = eNode;
    leftChildLambdaNode->firstKid->nextSibling = pNode;

    pNode->nextSibling = NULL;
}

void convertWithinExpression(Node *withinNode) {
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

void convertAndExpression(Node *andHeaderNode) {
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

void convertLetExpression(Node *letNode) {
//    cout << "\nInside convertLetExpression conversion!\nletNode ast form before standardizing is:\n";
//    recursivelyPrintTreeNode(letNode, "");
    letNode->label = GAMMA_STD_LABEL;

    letNode->firstKid->label = LAMBDA_STD_LABEL;

    Node *pNode = letNode->firstKid->nextSibling;
    Node *eNode = letNode->firstKid->firstKid->nextSibling;

    //switch the p and e nodes

    letNode->firstKid->nextSibling = eNode;
    letNode->firstKid->firstKid->nextSibling = pNode;

//    cout << "\nInside convertLetExpression conversion!\nletNode ast form after standardizing is:\n";
//    recursivelyPrintTreeNode(letNode, "");
}

void recursivelyStandardizeTree(Node *node) {
    if (node->firstKid != NULL) {
        recursivelyStandardizeTree(node->firstKid);
    }
    if (node->nextSibling != NULL) {
        recursivelyStandardizeTree(node->nextSibling);
    }
    if (node->label == "->") {
        //Do not standardize conditionals (optimizations for the CISE machine)
    } else if (node->label == "not" || node->label == "neg") { //convert unary operators to standardized form
        //Do not standardize unary operators (optimizations for the CISE machine) //convertUop(node);
    } else if (node->label == "aug" || node->label == "or" || node->label == "&" || node->label == "gr" ||
               node->label == "ge" || node->label == "ls" || node->label == "le" || node->label == "eq" ||
               node->label == "ne" || node->label == "+" || node->label == "-" || node->label == "*" ||
               node->label == "/" || node->label == "**") {
        //Do not standardize binary operators (optimizations for the CISE machine) //convertOperator(node);
    } else if (node->label == "tau") {
        //Do not standardize tau (optimizations for the CISE machine)
    } else if (node->label == "lambda") {    //convert lambda expression to standardized form
        if (node->firstKid->label == ",") { //lambda expression with a tuple of variables
            //Do not standardize lambda with a tuple of variables (optimizations for the CISE machine)
        } else {    //lambda expression with a list(?) of variable(s)
//            cout << "\nGoing to convertLambdaExpression\n";
            convertLambdaExpression(node);
        }
    } else if (node->label == FCN_FORM_LABEL) {    //convert function_form to standardized form
//        cout << "\nGoing to convertFunctionForm\n";
        convertFunctionForm(node);
    } else if (node->label == "@") {    //convert infix operator to standardized form
//        cout << "\nGoing to convertInfixOperator\n";
        convertInfixOperator(node);
    } else if (node->label == "and") {
//        cout << "\nGoing to convertAndExpression\n";
        convertAndExpression(node);
    } else if (node->label == "within") {
//        cout << "\nGoing to convertWithinExpression\n";
        convertWithinExpression(node);
    } else if (node->label == "rec") {
//        cout << "\nGoing to convertRecExpression for nodeLabel= " << node->label << "\n";
        convertRecExpression(node);
//        cout << "\nAfter convertRecExpression for nodeLabel= " << node->label << "\n";
    } else if (node->label == "let") {
//        cout << "\nGoing to convertLetExpression\n";
        convertLetExpression(node);
    } else if (node->label == "where") {
//        cout << "\nGoing to convertWhereExpression\n";
        convertWhereExpression(node);
    }
}

void convertASTToStandardizedTree() {
   cout << "\n\nGoing to standardize the tree now!\n\n";
    if (!trees.empty()) {
        cout << "\n\nThis is supposed to be the only tree below!\n";
        Node *treeRootOfAST = trees.top();
        recursivelyStandardizeTree(treeRootOfAST);
    }
}