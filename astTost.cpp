#include "astTo.hpp"
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
    Node *

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
}