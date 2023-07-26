#include "psg.hpp"
#include "lexicon.hpp"
#include "asttost.hpp"
#include "flattenst.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>

vector<list<CSEMachineNode>> controlStructures(150); // each controlStructure would be a list of CSEMachineNodes
int numberOfControlStructures = 1;

void recursivelyFlattenTree(Node *treeNode, list<CSEMachineNode> *controlStructure, int controlStructureIndex,
                            bool processKid, bool processSiblings)
{
    //    cout << "\n in recursivelyFlattenTree for node: " << treeNode->label << ", controlStructure: " <<
    //    controlStructureIndex << " and size=" << controlStructure->size();
    CSEMachineNode controlStructureNode = CSEMachineNode();

    controlStructureNode.defaultLabel = treeNode->label;
    if (treeNode->label == "gamma" || treeNode->label == GAMMA_STD_LABEL)
    {
        controlStructureNode.isGamma = true;
        controlStructureNode.defaultLabel = "gamma";
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n it's a gamma!";
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == "Y")
    {
        controlStructureNode.isY = true;
        controlStructureNode.defaultLabel = "Y";
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n it's a Y!";
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label.compare(0, 6, "<STR:'") == 0)
    {
        controlStructureNode.isString = true;
        controlStructureNode.stringValue = treeNode->label.substr(6);
        controlStructureNode.stringValue = controlStructureNode.stringValue.substr(0,
                                                                                   controlStructureNode.stringValue.length() -
                                                                                       2);
        controlStructureNode.defaultLabel = controlStructureNode.stringValue;
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n it's a string!";
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label.compare(0, 4, "<ID:") == 0)
    {
        controlStructureNode.isName = true;
        controlStructureNode.nameValue = treeNode->label.substr(4);
        controlStructureNode.nameValue = controlStructureNode.nameValue.substr(0,
                                                                               controlStructureNode.nameValue.length() -
                                                                                   1);
        controlStructureNode.defaultLabel = controlStructureNode.nameValue;
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n it's an identifier!";
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label.compare(0, 5, "<INT:") == 0)
    {
        controlStructureNode.isInt = true;
        string intString = treeNode->label.substr(5);
        // cout<<"\n intString= "<<intString<<" length= "<<intString.length();
        intString = intString.substr(0,
                                     intString.length() -
                                         1);
        // cout<<"\n intString= "<<intString;
        controlStructureNode.intValue = atoi(intString.c_str());
        controlStructureNode.defaultLabel = intString;
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n it's an integer!";
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == "<true>" || treeNode->label == "<false>")
    {
        controlStructureNode.isBoolean = true;
        controlStructureNode.defaultLabel = treeNode->label == "<true>" ? "true" : "false";
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n it's a truthValue!";
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == "<nil>")
    {
        controlStructureNode.isTuple = true;
        controlStructureNode.defaultLabel = "nil";
        controlStructureNode.numberOfElementsInTauTuple = 0;
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n it's nil!";
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == "<dummy>")
    {
        controlStructureNode.isDummy = true;
        controlStructureNode.defaultLabel = "dummy";
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n it's nil!";
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == LAMBDA_STD_LABEL || treeNode->label == "lambda")
    {
        //        cout << "\n it's a lambda!";
        processKid = false;
        controlStructureNode.isLambda = true;
        int numberOfBoundVariables = 0;
        if (treeNode->firstKid->label == ",")
        {
            //            cout << "\nIt's a comma node! bound variables!\n";
            Node *boundVariableNode = treeNode->firstKid->firstKid;
            while (boundVariableNode != NULL)
            {
                numberOfBoundVariables++;
                string variable = boundVariableNode->label.substr(
                    4); // bound variables will always start with <ID: and end with >
                variable = variable.substr(0, variable.length() - 1);
                controlStructureNode.boundVariables.push_back(variable);
                boundVariableNode = boundVariableNode->nextSibling;
            }
        }
        else
        { // only one bound variable, which is first child (leftChild)
            numberOfBoundVariables++;
            //            cout << "\nthe bound variable for this lambda= " << treeNode->firstKid->label << "\n";
            string variable = treeNode->firstKid->label.substr(
                4); // bound variables will always start with <ID: and end with >
            variable = variable.substr(0, variable.length() - 1);
            controlStructureNode.boundVariables.push_back(variable);
        }
        controlStructureNode.indexOfBodyOfLambda = numberOfControlStructures++;
        controlStructureNode.numberOfElementsInTauTuple = numberOfBoundVariables;
        string boundVariables;
        for (int i = 0; i < numberOfBoundVariables; i++)
        {
            boundVariables += controlStructureNode.boundVariables[i] + ", ";
        }
        controlStructureNode.defaultLabel =
            "Lambda with bound variables(" + boundVariables + ") and body(" +
            std::to_string(controlStructureNode.indexOfBodyOfLambda) + ")";
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
        list<CSEMachineNode> *controlStructureOfLambda = new list<CSEMachineNode>;
        recursivelyFlattenTree(treeNode->firstKid->nextSibling, controlStructureOfLambda,
                               controlStructureNode.indexOfBodyOfLambda, true, true);
    }
    else if (treeNode->label == "->")
    {
        //        cout << "\n\n ****** Handle CONDITIONAL! ****** \n\n";
        processKid = false;
        CSEMachineNode trueNode = CSEMachineNode();
        CSEMachineNode falseNode = CSEMachineNode();
        CSEMachineNode betaNode = CSEMachineNode();
        betaNode.isConditional = true;
        trueNode.isConditional = true;
        falseNode.isConditional = true;
        betaNode.defaultLabel = "BetaNode";
        trueNode.defaultLabel = "trueNode";
        falseNode.defaultLabel = "falseNode";
        trueNode.indexOfBodyOfLambda = numberOfControlStructures++;
        falseNode.indexOfBodyOfLambda = numberOfControlStructures++;
        betaNode.indexOfBodyOfLambda = controlStructureIndex;
        list<CSEMachineNode> *controlStructureOfTrueNode = new list<CSEMachineNode>;
        recursivelyFlattenTree(treeNode->firstKid->nextSibling, controlStructureOfTrueNode,
                               trueNode.indexOfBodyOfLambda, true, false);
        list<CSEMachineNode> *controlStructureOfFalseNode = new list<CSEMachineNode>;
        recursivelyFlattenTree(treeNode->firstKid->nextSibling->nextSibling, controlStructureOfFalseNode,
                               falseNode.indexOfBodyOfLambda, true, false);
        controlStructure->push_back(trueNode);
        controlStructure->push_back(falseNode);
        controlStructure->push_back(betaNode);
        recursivelyFlattenTree(treeNode->firstKid, controlStructure,
                               controlStructureIndex, true, false);
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == "not" || treeNode->label == "neg")
    { // convert unary operators to standardized form
        //        cout << "\n it's a " << treeNode->label;
        controlStructureNode.isUnaryOperator = true;
        controlStructureNode.operatorStringValue = treeNode->label;
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == "aug" || treeNode->label == "or" || treeNode->label == "&" ||
             treeNode->label == "gr" ||
             treeNode->label == "ge" || treeNode->label == "ls" || treeNode->label == "le" ||
             treeNode->label == "eq" ||
             treeNode->label == "ne" || treeNode->label == "+" || treeNode->label == "-" ||
             treeNode->label == "*" ||
             treeNode->label == "/" || treeNode->label == "**")
    {
        //        cout << "\n it's a " << treeNode->label;
        controlStructureNode.isBinaryOperator = true;
        controlStructureNode.operatorStringValue = treeNode->label;
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == "tau")
    {
        //        cout << "\n\n ****** Handle TAU! ****** \n\n";
        processKid = false;
        controlStructureNode.isTau = true;
        int numberOfElementsInTuple = 0;
        Node *tauElementNode = treeNode->firstKid;
        do
        {
            numberOfElementsInTuple++;
            tauElementNode = tauElementNode->nextSibling;
        } while (tauElementNode != NULL);
        controlStructureNode.numberOfElementsInTauTuple = numberOfElementsInTuple;
        controlStructureNode.defaultLabel =
            "TAU[" + std::to_string(controlStructureNode.numberOfElementsInTauTuple) + "]";
        controlStructure->push_back(controlStructureNode);
        tauElementNode = treeNode->firstKid;
        do
        {
            CSEMachineNode tupleElementNode = CSEMachineNode();
            if (tauElementNode->label.compare(0, 6, "<STR:'") == 0)
            {
                tupleElementNode.isString = true;
                tupleElementNode.stringValue = tauElementNode->label.substr(6);
                tupleElementNode.stringValue = tupleElementNode.stringValue.substr(0,
                                                                                   tupleElementNode.stringValue.length() -
                                                                                       2);
                tupleElementNode.defaultLabel = tupleElementNode.stringValue;
                //                cout << "\n it's a string!";
                controlStructure->push_back(tupleElementNode);
                //                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                // controlStructure->size();
            }
            else if (tauElementNode->label.compare(0, 4, "<ID:") == 0)
            {
                tupleElementNode.isName = true;
                tupleElementNode.nameValue = tauElementNode->label.substr(4);
                tupleElementNode.nameValue = tupleElementNode.nameValue.substr(0,
                                                                               tupleElementNode.nameValue.length() -
                                                                                   1);
                tupleElementNode.defaultLabel = tupleElementNode.nameValue;
                //                cout << "\n it's an identifier!";
                controlStructure->push_back(tupleElementNode);
                //                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                // controlStructure->size();
            }
            else if (tauElementNode->label.compare(0, 5, "<INT:") == 0)
            {
                tupleElementNode.isInt = true;
                string intString = tauElementNode->label.substr(5);
                intString = intString.substr(0,
                                             intString.length() -
                                                 1);
                tupleElementNode.intValue = atoi(intString.c_str());
                tupleElementNode.defaultLabel = intString;
                //                cout << "\n it's an integer!";
                controlStructure->push_back(tupleElementNode);
                //                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                // controlStructure->size();
            }
            else if (tauElementNode->label == "<true>" || tauElementNode->label == "<false>")
            {
                tupleElementNode.isBoolean = true;
                tupleElementNode.defaultLabel = tauElementNode->label == "<true>" ? "true" : "false";
                //                cout << "\n it's a truthValue!";
                controlStructure->push_back(tupleElementNode);
                //                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                // controlStructure->size();
            }
            else if (tauElementNode->label == "gamma" || tauElementNode->label == GAMMA_STD_LABEL)
            {
                tupleElementNode.isGamma = true;
                //                cout << "\n it's a gamma!";
                tupleElementNode.defaultLabel = "gamma";
                controlStructure->push_back(tupleElementNode);
                //                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                // controlStructure->size();
                recursivelyFlattenTree(tauElementNode->firstKid, controlStructure, controlStructureIndex, true, true);
                //                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
            }
            else if (tauElementNode->label == "aug" || tauElementNode->label == "or" ||
                     tauElementNode->label == "&" ||
                     tauElementNode->label == "gr" ||
                     tauElementNode->label == "ge" || tauElementNode->label == "ls" ||
                     tauElementNode->label == "le" ||
                     tauElementNode->label == "eq" ||
                     tauElementNode->label == "ne" || tauElementNode->label == "+" || tauElementNode->label == "-" ||
                     tauElementNode->label == "*" ||
                     tauElementNode->label == "/" || tauElementNode->label == "**")
            {
                //                cout << "\n it's a " << tauElementNode->label;
                tupleElementNode.isBinaryOperator = true;
                tupleElementNode.operatorStringValue = tauElementNode->label;
                controlStructure->push_back(tupleElementNode);
                //                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                // controlStructure->size();
                recursivelyFlattenTree(tauElementNode->firstKid, controlStructure, controlStructureIndex, true, true);
                //                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                // controlStructure->size();
            }
            else
            {
                //                cout << "\n it's a " << tauElementNode->label;
                recursivelyFlattenTree(tauElementNode, controlStructure, controlStructureIndex, true, false);
            }
            tauElementNode = tauElementNode->nextSibling;
        } while (tauElementNode != NULL);

        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == ",")
    {
        //        cout << "\n\n ****** Handle CommaNode! ****** \n\n";
        controlStructureNode.isComma = true;
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    else if (treeNode->label == "true" || treeNode->label == "false")
    {
        controlStructureNode.isBoolean = true;
        controlStructure->push_back(controlStructureNode);
        //        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    controlStructures[controlStructureIndex] = *controlStructure;

    if (processKid && treeNode->firstKid != NULL)
    {
        recursivelyFlattenTree(treeNode->firstKid, controlStructure, controlStructureIndex, true, true);
    }

    if (processSiblings && treeNode->nextSibling != NULL)
    {
        recursivelyFlattenTree(treeNode->nextSibling, controlStructure, controlStructureIndex, true, true);
    }
}

void flattenStandardizedTree()
{
    //    cout << "\n\nGoing to flattenStandardizedTree now!\n\n";
    if (!trees.empty())
    {
        Node *treeRoot = trees.top();
        // cout << "\n\nBefore pointer declare\n\n";
        list<CSEMachineNode> *controlStructure = new list<CSEMachineNode>;
        // cout << "\n\n after pointer declare\n\n";
        recursivelyFlattenTree(treeRoot, controlStructure, 0, true, true);
    }
}