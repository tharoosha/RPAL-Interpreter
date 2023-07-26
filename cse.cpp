#include "psg.hpp"
#include "lexicon.hpp"
#include "asttost.hpp"
#include "flattenst.hpp"
#include "cse.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <list>
#include <cmath>

EnvironmentNode *environments[1200];

stack<CSEMachineNode> cseMachineControl; // the Control stack of the CSE machine
stack<CSEMachineNode> cseMachineStack;   // the "Stack" stack of values of the CSE machine

EnvironmentNode *currentEnvironment = new EnvironmentNode;

int environmentCounter = 0;

void initializeCSEMachine()
{
    // initialize environment with the primitive environment (PE / e0)
    currentEnvironment->environmentIndex = 0;
    currentEnvironment->parentEnvironment = NULL;
    currentEnvironment->previousEnvironment = NULL;
    environments[environmentCounter++] = currentEnvironment;

    // initialize control.
    // push the first token as the e0 environment variable
    CSEMachineNode e0 = CSEMachineNode();
    e0.isEnvironmentMarker = true;
    e0.environmentMarkerIndex = 0;
    e0.defaultLabel = "e0";
    cseMachineControl.push(e0);
    // push the 0th control structure's elements
    std::list<CSEMachineNode>::const_iterator iterator;
    for (iterator = controlStructures[0].begin(); iterator != controlStructures[0].end(); ++iterator)
    {
        CSEMachineNode controlStructureToken = *iterator;
        cseMachineControl.push(controlStructureToken);
    }

    // initialize stack with e0 as well
    cseMachineStack.push(e0);
}

void printTree()
{
    // cout << "\n\nGoing to print the tree now!\n\n";
    if (!trees.empty())
    {
        // cout << "\n\nThis is supposed to be the only tree below!\n";
        Node *treeRoot = trees.top();
        recursivelyPrintTree(treeRoot, "");
    }
}
void recursivelyPrintTree(Node *node, string indentDots)
{
    // cout<<"\nPrinting tree for: "<<node->label<<"\n";
    cout << indentDots + node->label << "\n";
    if (node->firstKid != NULL)
    {
        // cout<<"\nPrinting firstKid tree for: "<<node->label<<"\n";
        recursivelyPrintTree(node->firstKid, indentDots + ".");
    }
    if (node->nextSibling != NULL)
    {
        // cout<<"\nPrinting nextSibling tree for: "<<node->label<<"\n";
        recursivelyPrintTree(node->nextSibling, indentDots);
    }
    // cout<<"\nDONE! Printing tree for: "<<node->label<<"\n";
}

void recursivelyPrintTreeNode(Node *node, string indentDots)
{
    // cout<<"\nPrinting tree for: "<<node->label<<"\n";
    cout << indentDots + node->label << "\n";
    if (node->firstKid != NULL)
    {
        // cout<<"\nPrinting firstKid tree for: "<<node->label<<"\n";
        recursivelyPrintTree(node->firstKid, indentDots + "(-.#.-)");
    }
    // cout<<"\nDONE! Printing tree for: "<<node->label<<"\n";
}

void printString(std::string stringToPrint)
{
    for (size_t i = 0; i < stringToPrint.length(); i++)
    {
        if (stringToPrint.at(i) == '\\' && stringToPrint.at(i + 1) == 'n')
        {
            cout << "\n";
            i++;
        }
        else if (stringToPrint.at(i) == '\\' && stringToPrint.at(i + 1) == 't')
        {
            cout << "\t";
            i++;
        }
        else
        {
            cout << stringToPrint.at(i);
        }
    }
}

void processCSEMachine()
{
    CSEMachineNode controlTop = cseMachineControl.top();
    cseMachineControl.pop();

    //    cout << "\n\n Control's top is: " << controlTop.defaultLabel;

    if (controlTop.isInt || controlTop.isString || controlTop.isBoolean ||
        controlTop.isDummy)
    { // CSE rule 1 for ints, booleans, dummy and strings
        cseMachineStack.push(controlTop);
    }
    else if (controlTop.isY)
    {
        cseMachineStack.push(controlTop);
    }
    else if (controlTop.isTuple)
    { // CSE rule 1 for 'nil', which can be the only tuple in a control structure
        cseMachineStack.push(controlTop);
    }
    else if (controlTop.isName)
    { // CSE rule 1 for variables
        controlTop.isName = false;
        EnvironmentNode *environmentWithVariableValue = currentEnvironment;
        CSEMachineNode boundedValuesNode;
        bool variableValueFound = false;
        int indexOfBoundVariable = 0;

        while (environmentWithVariableValue != NULL)
        {
            //            cout << "\n\nlooking for " << controlTop.nameValue << " in environment " <<
            //            environmentWithVariableValue->environmentIndex;
            boundedValuesNode = environmentWithVariableValue->boundedValuesNode;
            for (int i = 0; i < boundedValuesNode.boundVariables.size(); i++)
            {
                if (boundedValuesNode.boundVariables[i] == controlTop.nameValue)
                {
                    indexOfBoundVariable = i;
                    variableValueFound = true;
                    break;
                }
            }
            if (variableValueFound)
            {
                break;
            }
            else
            {
                environmentWithVariableValue = environmentWithVariableValue->parentEnvironment;
            }
        }

        if (!variableValueFound)
        {
            // it could be a built-in function defined in the PE [e0]
            if (controlTop.nameValue == "Print" || controlTop.nameValue == "Conc" ||
                controlTop.nameValue == "Istuple" || controlTop.nameValue == "Isinteger" ||
                controlTop.nameValue == "Istruthvalue" || controlTop.nameValue == "Isstring" ||
                controlTop.nameValue == "Isfunction" || controlTop.nameValue == "Isdummy" ||
                controlTop.nameValue == "Stem" || controlTop.nameValue == "Stern" || controlTop.nameValue == "Order" ||
                controlTop.nameValue == "ItoS")
            {
                controlTop.isBuiltInFunction = true;
                controlTop.defaultLabel = controlTop.nameValue;
                cseMachineStack.push(controlTop);
            }
            else
            {
                // cout << "\n\nERROR! Value for bound variable '" << controlTop.nameValue << "' not found in environment tree! DIE!\n\n";
                exit(0);
            }
        }
        else
        {
            //            cout << "\n\n Value of " << controlTop.nameValue << " is= ";
            controlTop = environmentWithVariableValue->boundedValuesNode.tupleElements[indexOfBoundVariable];
            //            cout << controlTop.defaultLabel << "\n\n";
            cseMachineStack.push(controlTop);
        }
    }
    else if (controlTop.isEnvironmentMarker)
    { // CSE rule 5
        CSEMachineNode stackTop = cseMachineStack.top();
        cseMachineStack.pop();
        if (!stackTop.isEnvironmentMarker)
        {
            CSEMachineNode stackTopEnvironmentVariable = cseMachineStack.top();
            cseMachineStack.pop();
            if (!stackTopEnvironmentVariable.isEnvironmentMarker ||
                (controlTop.environmentMarkerIndex != stackTopEnvironmentVariable.environmentMarkerIndex))
            {
                cout << "\n ERROR in resolving environment variables on control and stack! Die now! \n";
                exit(0);
            }
            cseMachineStack.push(stackTop);
        }
        else
        {
            if (controlTop.environmentMarkerIndex != stackTop.environmentMarkerIndex)
            {
                // cout << "\n ERROR in resolving environment variables on control and stack! Die now! \n";
                exit(0);
            }
        }
        currentEnvironment = environments[controlTop.environmentMarkerIndex]->previousEnvironment;
    }
    else if (controlTop.isLambda)
    {                                                                             // CSE rule 2
        controlTop.environmentMarkerIndex = currentEnvironment->environmentIndex; // index of environment in which this lambda holds
        cseMachineStack.push(controlTop);
    }
    else if (controlTop.isGamma)
    { // CSE rule 3 & 4
        CSEMachineNode result = CSEMachineNode();
        CSEMachineNode operatorNode = cseMachineStack.top();
        cseMachineStack.pop();
        CSEMachineNode firstOperand = cseMachineStack.top();
        cseMachineStack.pop();
        if (operatorNode.isUnaryOperator || operatorNode.isUnaryOperator)
        { // CSE rule 3
            if (operatorNode.isUnaryOperator)
            {
                if (operatorNode.operatorStringValue == "neg")
                {
                    if (!firstOperand.isInt)
                    {
                        cout << "\n Operand is not int to apply 'neg', EXIT! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = true;
                        result.intValue = -firstOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                }
                else if (operatorNode.operatorStringValue == "not")
                {
                    if (!firstOperand.isBoolean)
                    {
                        cout << "\n Operand is not boolean to apply 'not', EXIT! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isBoolean = true;
                        if (firstOperand.defaultLabel == "true")
                        {
                            result.defaultLabel = "false";
                        }
                        else if (firstOperand.defaultLabel == "false")
                        {
                            result.defaultLabel = "true";
                        }
                    }
                }
                cseMachineStack.push(result);
            }
            else if (operatorNode.isBinaryOperator)
            {
                CSEMachineNode secondOperand = cseMachineStack.top();
                cseMachineStack.pop();
                if (operatorNode.operatorStringValue == "**")
                {
                    if (!firstOperand.isInt || !secondOperand.isInt)
                    {
                        cout << "\n operands not int for ** operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = true;
                        result.intValue = pow(firstOperand.intValue, secondOperand.intValue);
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                }
                else if (operatorNode.operatorStringValue == "*")
                {
                    if (!firstOperand.isInt || !secondOperand.isInt)
                    {
                        cout << "\n operands not int for * operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = true;
                        result.intValue = firstOperand.intValue * secondOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                }
                else if (operatorNode.operatorStringValue == "aug")
                {
                    if (!firstOperand.isTuple)
                    {
                        cout << "\n first Operand is not a tuple for 'aug' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isTuple = true;
                        result.numberOfElementsInTauTuple = firstOperand.numberOfElementsInTauTuple + 1;

                        if (firstOperand.numberOfElementsInTauTuple == 0)
                        { // if the first operand is nil
                            result.tupleElements.push_back(secondOperand);
                        }
                        else
                        {
                            result.tupleElements = firstOperand.tupleElements;
                            result.tupleElements.push_back(secondOperand);
                        }
                        result.defaultLabel = "TupleOfSize=" + std::to_string(result.numberOfElementsInTauTuple);
                    }
                }
                else if (operatorNode.operatorStringValue == "-")
                {
                    if (!firstOperand.isInt || !secondOperand.isInt)
                    {
                        cout << "\n operands not int for - operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = true;
                        result.intValue = firstOperand.intValue - secondOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                }
                else if (operatorNode.operatorStringValue == "+")
                {
                    if (!firstOperand.isInt || !secondOperand.isInt)
                    {
                        cout << "\n operands not int for + operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = true;
                        result.intValue = firstOperand.intValue + secondOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                }
                else if (operatorNode.operatorStringValue == "/")
                {
                    if (!firstOperand.isInt || !secondOperand.isInt)
                    {
                        cout << "\n operands not int for '/' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = true;
                        result.intValue = firstOperand.intValue / secondOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                }
                else if (operatorNode.operatorStringValue == "gr")
                {
                    if (!firstOperand.isInt || !secondOperand.isInt)
                    {
                        cout << "\n operands not int for 'gr' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = firstOperand.intValue > secondOperand.intValue ? "true" : "false";
                    }
                }
                else if (operatorNode.operatorStringValue == "ge")
                {
                    if (!firstOperand.isInt || !secondOperand.isInt)
                    {
                        cout << "\n operands not int for 'ge' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = firstOperand.intValue >= secondOperand.intValue ? "true" : "false";
                    }
                }
                else if (operatorNode.operatorStringValue == "ls")
                {
                    if (!firstOperand.isInt || !secondOperand.isInt)
                    {
                        cout << "\n operands not int for 'ls' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = firstOperand.intValue < secondOperand.intValue ? "true" : "false";
                    }
                }
                else if (operatorNode.operatorStringValue == "le")
                {
                    if (!firstOperand.isInt || !secondOperand.isInt)
                    {
                        cout << "\n operands not int for 'le' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = firstOperand.intValue <= secondOperand.intValue ? "true" : "false";
                    }
                }
                else if (operatorNode.operatorStringValue == "eq")
                {
                    if (!((!firstOperand.isInt || !secondOperand.isInt) ||
                          (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
                          (!firstOperand.isString || !secondOperand.isString)))
                    {
                        cout << "\n operands not of same type for 'eq' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = false;
                        result.isBoolean = true;
                        if (firstOperand.isInt)
                        {
                            result.defaultLabel = firstOperand.intValue == secondOperand.intValue ? "true" : "false";
                        }
                        else if (firstOperand.isBoolean)
                        {
                            result.defaultLabel =
                                firstOperand.defaultLabel == secondOperand.defaultLabel ? "true" : "false";
                        }
                        else if (firstOperand.isString)
                        {
                            result.defaultLabel =
                                firstOperand.stringValue == secondOperand.stringValue ? "true" : "false";
                        }
                    }
                }
                else if (operatorNode.operatorStringValue == "ne")
                {
                    if (!((!firstOperand.isInt || !secondOperand.isInt) ||
                          (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
                          (!firstOperand.isString || !secondOperand.isString)))
                    {
                        cout << "\n operands not of same type for 'ne' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = false;
                        result.isBoolean = true;
                        if (firstOperand.isInt)
                        {
                            result.defaultLabel = firstOperand.intValue != secondOperand.intValue ? "true" : "false";
                        }
                        else if (firstOperand.isBoolean)
                        {
                            result.defaultLabel =
                                firstOperand.defaultLabel != secondOperand.defaultLabel ? "true" : "false";
                        }
                        else if (firstOperand.isString)
                        {
                            result.defaultLabel =
                                firstOperand.stringValue != secondOperand.stringValue ? "true" : "false";
                        }
                    }
                }
                else if (operatorNode.operatorStringValue == "or")
                {
                    if (!firstOperand.isBoolean || !secondOperand.isBoolean)
                    {
                        cout << "\n operands are not boolean for 'or' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = (firstOperand.defaultLabel == "true" ||
                                               secondOperand.defaultLabel == "true")
                                                  ? "true"
                                                  : "false";
                    }
                }
                else if (operatorNode.operatorStringValue == "&")
                {
                    if (!firstOperand.isBoolean || !secondOperand.isBoolean)
                    {
                        cout << "\n operands are not boolean for '&' operation! exiting! \n";
                        exit(0);
                    }
                    else
                    {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = (firstOperand.defaultLabel == "true" &&
                                               secondOperand.defaultLabel == "true")
                                                  ? "true"
                                                  : "false";
                    }
                }
                cseMachineStack.push(result);
            }
        }
        else if (operatorNode.isLambda)
        { // CSE rule 4

            // cout << "\n Lambda2 \n";
            // add new lambda's environment variable to control
            CSEMachineNode newEnvironmentVariableForCurrentLambda = CSEMachineNode();
            newEnvironmentVariableForCurrentLambda.isEnvironmentMarker = true;
            newEnvironmentVariableForCurrentLambda.environmentMarkerIndex = environmentCounter++;
            newEnvironmentVariableForCurrentLambda.defaultLabel =
                "e" + std::to_string(newEnvironmentVariableForCurrentLambda.environmentMarkerIndex);
            cseMachineControl.push(newEnvironmentVariableForCurrentLambda);
            // cout << "\n Lambda3 \n";

            // update currentEnvironment
            EnvironmentNode *newEnvironmentForCurrentLambda = new EnvironmentNode();
            newEnvironmentForCurrentLambda->parentEnvironment = environments[operatorNode.environmentMarkerIndex];
            newEnvironmentForCurrentLambda->previousEnvironment = currentEnvironment;
            currentEnvironment = newEnvironmentForCurrentLambda;
            newEnvironmentForCurrentLambda->environmentIndex = newEnvironmentVariableForCurrentLambda.environmentMarkerIndex;
            newEnvironmentForCurrentLambda->boundedValuesNode = CSEMachineNode();
            newEnvironmentForCurrentLambda->boundedValuesNode.boundVariables = operatorNode.boundVariables;
            environments[newEnvironmentForCurrentLambda->environmentIndex] = newEnvironmentForCurrentLambda;

            // We have separate cases here instead of just assigning
            // newEnvironmentForCurrentLambda->boundedValuesNode = firstOperand
            // because we need to have the boundVariables and the tupleElements in the same boundedValuesNode.

            if (operatorNode.boundVariables.size() ==
                1)
            { // only one bound variable, then the firstOperand is stored as it is in the tupleElements
                // first operand could be int/string/tuple
                newEnvironmentForCurrentLambda->boundedValuesNode.tupleElements.push_back(firstOperand);
            }
            else
            { // there are multiple variable bindings, so the firstOperand must be a tuple and that is what we assign
                // CSE Rule 11 (n-ary function)
                newEnvironmentForCurrentLambda->boundedValuesNode.tupleElements = firstOperand.tupleElements;
            }

            //            cout << "\n\nNew environment[" + std::to_string(newEnvironmentForCurrentLambda->environmentIndex) +
            //                    "] created with parent environment[" +
            //                    std::to_string(operatorNode.environmentMarkerIndex) + "], bound variables are:\n";
            //            for (int i = 0; i < newEnvironmentForCurrentLambda->boundedValuesNode.boundVariables.size(); i++) {
            //                cout << "\n" << newEnvironmentForCurrentLambda->boundedValuesNode.boundVariables[i] << "= " <<
            //                newEnvironmentForCurrentLambda->boundedValuesNode.tupleElements[i].defaultLabel <<
            //                "\n\n";
            //            }

            // cout << "\n Lambda4 \n";

            // add new lambda environment variable to stack
            cseMachineStack.push(newEnvironmentVariableForCurrentLambda);

            // cout << "\n Lambda5 \n";
            // add lambda's control structure to control
            std::list<CSEMachineNode>::const_iterator iterator;
            for (iterator = controlStructures[operatorNode.indexOfBodyOfLambda].begin();
                 iterator != controlStructures[operatorNode.indexOfBodyOfLambda].end(); ++iterator)
            {
                CSEMachineNode controlStructureToken = *iterator;
                cseMachineControl.push(controlStructureToken);
            }
            // cout << "\n Lambda6 \n";
        }
        else if (operatorNode.isY)
        { // CSE rule 12 (applying Y)
            firstOperand.isYF = true;
            firstOperand.isLambda = false;
            cseMachineStack.push(firstOperand);
        }
        else if (operatorNode.isYF)
        { // CSE rule 13 (applying f.p.)
            cseMachineStack.push(firstOperand);
            cseMachineStack.push(operatorNode);
            CSEMachineNode lambdaNode = operatorNode;
            lambdaNode.isYF = false;
            lambdaNode.isLambda = true;
            cseMachineStack.push(lambdaNode);
            CSEMachineNode gammaNode = CSEMachineNode();
            gammaNode.isGamma = true;
            gammaNode.defaultLabel = "gamma";
            cseMachineControl.push(gammaNode);
            cseMachineControl.push(gammaNode);
        }
        else if (operatorNode.isBuiltInFunction)
        {
            if (operatorNode.defaultLabel == "Print")
            {
                if (firstOperand.isBoolean)
                {
                    cout << firstOperand.defaultLabel;
                }
                else if (firstOperand.isInt)
                {
                    cout << firstOperand.intValue;
                }
                else if (firstOperand.isString)
                {
                    printString(firstOperand.stringValue);
                }
                else if (firstOperand.isDummy)
                {
                    // Do nothing
                }
                else if (firstOperand.isTuple)
                {
                    if (firstOperand.tupleElements.size() == 0)
                    {
                        cout << "nil"; // empty tuple
                    }
                    else
                    {
                        cout << "(";
                        for (int i = 0; i < firstOperand.tupleElements.size(); i++)
                        {
                            if (firstOperand.tupleElements[i].isBoolean)
                            {
                                cout << firstOperand.tupleElements[i].defaultLabel;
                            }
                            else if (firstOperand.tupleElements[i].isInt)
                            {
                                cout << firstOperand.tupleElements[i].intValue;
                            }
                            else if (firstOperand.tupleElements[i].isString)
                            {
                                printString(firstOperand.tupleElements[i].stringValue);
                            }
                            if (i + 1 != firstOperand.tupleElements.size())
                            {
                                cout << ", ";
                            }
                        }
                        cout << ")";
                    }
                }
                else if (firstOperand.isLambda)
                {
                    cout << "[lambda closure: " + firstOperand.boundVariables[0] + ": " +
                                std::to_string(firstOperand.indexOfBodyOfLambda) + "]";
                }
                else
                {
                    cout << "\n\n ERROR! I don't know how to PRINT the value on stack= " + firstOperand.defaultLabel + "\n\n";
                    exit(0);
                }
            }
            else if (operatorNode.defaultLabel == "Conc")
            {
                cseMachineControl.pop(); // to pop out the second gamma node
                CSEMachineNode secondOperand = cseMachineStack.top();
                cseMachineStack.pop();
                result.isString = true;
                result.stringValue = firstOperand.stringValue + secondOperand.stringValue;
                cseMachineStack.push(result);
            }
            else if (operatorNode.defaultLabel == "Order")
            {
                if (!firstOperand.isTuple)
                {
                    cout << "\n\n Error! can't apply 'Order' to a datatype other than tuple! DIE NO! \n\n ";
                    exit(0);
                }
                else
                {
                    result.isInt = true;
                    result.intValue = firstOperand.numberOfElementsInTauTuple;
                    result.defaultLabel = std::to_string(result.intValue);
                    cseMachineStack.push(result);
                }
            }
            else if (operatorNode.defaultLabel == "Stem")
            {
                result.isString = true;
                result.stringValue = firstOperand.stringValue[0];
                cseMachineStack.push(result);
            }
            else if (operatorNode.defaultLabel == "Stern")
            {
                result.isString = true;
                result.stringValue = firstOperand.stringValue.substr(1);
                cseMachineStack.push(result);
            }
            else if (operatorNode.defaultLabel == "Isstring")
            {
                result.isBoolean = true;
                result.defaultLabel = firstOperand.isString ? "true" : "false";
                cseMachineStack.push(result);
            }
            else if (operatorNode.defaultLabel == "Istuple")
            {
                result.isBoolean = true;
                result.defaultLabel = firstOperand.isTuple ? "true" : "false";
                cseMachineStack.push(result);
            }
            else if (operatorNode.defaultLabel == "Isinteger")
            {
                result.isBoolean = true;
                result.defaultLabel = firstOperand.isInt ? "true" : "false";
                cseMachineStack.push(result);
            }
            else if (operatorNode.defaultLabel == "ItoS")
            {
                if (!firstOperand.isInt)
                {
                    cout << "ERROR! operand to ItoS is not Int! DIE NOW!";
                    exit(0);
                }
                result.isString = true;
                result.defaultLabel = std::to_string(firstOperand.intValue);
                result.stringValue = std::to_string(firstOperand.intValue);
                cseMachineStack.push(result);
            }
            else if (operatorNode.defaultLabel == "Istruthvalue")
            {
                result.isBoolean = true;
                result.defaultLabel = firstOperand.isBoolean ? "true" : "false";
                cseMachineStack.push(result);
            }
            else
            {
                cout << "\n\n AYO!! I haven't defined the behavior of the function= " + operatorNode.defaultLabel + "\n\n";
                exit(0);
            }
        }
        else if (operatorNode.isTuple)
        { //  CSE rule 10 for Tuple selection
            result = operatorNode.tupleElements[firstOperand.intValue - 1];
            cseMachineStack.push(result);
        }
    }
    else if (controlTop.isBinaryOperator)
    { // CSE rule 6
        CSEMachineNode result = CSEMachineNode();
        CSEMachineNode operatorNode = controlTop;
        CSEMachineNode firstOperand = cseMachineStack.top();
        cseMachineStack.pop();
        CSEMachineNode secondOperand = cseMachineStack.top();
        cseMachineStack.pop();
        if (operatorNode.operatorStringValue == "**")
        {
            if (!firstOperand.isInt || !secondOperand.isInt)
            {
                cout << "\n operands not int for '**' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = true;
                result.intValue = pow(firstOperand.intValue, secondOperand.intValue);
                result.defaultLabel = std::to_string(result.intValue);
            }
        }
        else if (operatorNode.operatorStringValue == "*")
        {
            if (!firstOperand.isInt || !secondOperand.isInt)
            {
                cout << "\n operands not int for '*' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = true;
                result.intValue = firstOperand.intValue * secondOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        }
        else if (operatorNode.operatorStringValue == "aug")
        {
            if (!firstOperand.isTuple)
            {
                cout << "\n first Operand is not a tuple for 'aug' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isTuple = true;
                result.numberOfElementsInTauTuple = firstOperand.numberOfElementsInTauTuple + 1;

                if (firstOperand.numberOfElementsInTauTuple == 0)
                { // if the first operand is nil
                    result.tupleElements.push_back(secondOperand);
                }
                else
                {
                    result.tupleElements = firstOperand.tupleElements;
                    result.tupleElements.push_back(secondOperand);
                }
                result.defaultLabel = "TupleOfSize=" + std::to_string(result.numberOfElementsInTauTuple);
            }
        }
        else if (operatorNode.operatorStringValue == "-")
        {
            if (!firstOperand.isInt || !secondOperand.isInt)
            {
                cout << "\n operands not int for '-' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = true;
                result.intValue = firstOperand.intValue - secondOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        }
        else if (operatorNode.operatorStringValue == "+")
        {
            if (!firstOperand.isInt || !secondOperand.isInt)
            {
                cout << "\n operands not int for '+' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = true;
                result.intValue = firstOperand.intValue + secondOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        }
        else if (operatorNode.operatorStringValue == "/")
        {
            if (!firstOperand.isInt || !secondOperand.isInt)
            {
                cout << "\n operands not int for '/' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = true;
                result.intValue = firstOperand.intValue / secondOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        }
        else if (operatorNode.operatorStringValue == "gr")
        {
            if (!firstOperand.isInt || !secondOperand.isInt)
            {
                cout << "\n operands not int for 'gr' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = firstOperand.intValue > secondOperand.intValue ? "true" : "false";
            }
        }
        else if (operatorNode.operatorStringValue == "ge")
        {
            if (!firstOperand.isInt || !secondOperand.isInt)
            {
                cout << "\n operands not int for 'ge' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = firstOperand.intValue >= secondOperand.intValue ? "true" : "false";
            }
        }
        else if (operatorNode.operatorStringValue == "ls")
        {
            if (!firstOperand.isInt || !secondOperand.isInt)
            {
                cout << "\n operands not int for 'ls' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = firstOperand.intValue < secondOperand.intValue ? "true" : "false";
            }
        }
        else if (operatorNode.operatorStringValue == "le")
        {
            if (!firstOperand.isInt || !secondOperand.isInt)
            {
                cout << "\n operands not int for 'le' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = firstOperand.intValue <= secondOperand.intValue ? "true" : "false";
            }
        }
        else if (operatorNode.operatorStringValue == "eq")
        {
            if (!((!firstOperand.isInt || !secondOperand.isInt) ||
                  (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
                  (!firstOperand.isString || !secondOperand.isString)))
            {
                cout << "\n operands not of same type for 'eq' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = false;
                result.isBoolean = true;
                if (firstOperand.isInt)
                {
                    result.defaultLabel = firstOperand.intValue == secondOperand.intValue ? "true" : "false";
                }
                else if (firstOperand.isBoolean)
                {
                    result.defaultLabel =
                        firstOperand.defaultLabel == secondOperand.defaultLabel ? "true" : "false";
                }
                else if (firstOperand.isString)
                {
                    result.defaultLabel =
                        firstOperand.stringValue == secondOperand.stringValue ? "true" : "false";
                }
            }
        }
        else if (operatorNode.operatorStringValue == "ne")
        {
            if (!((!firstOperand.isInt || !secondOperand.isInt) ||
                  (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
                  (!firstOperand.isString || !secondOperand.isString)))
            {
                cout << "\n operands not of same type for 'ne' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = false;
                result.isBoolean = true;
                if (firstOperand.isInt)
                {
                    result.defaultLabel = firstOperand.intValue != secondOperand.intValue ? "true" : "false";
                }
                else if (firstOperand.isBoolean)
                {
                    result.defaultLabel =
                        firstOperand.defaultLabel != secondOperand.defaultLabel ? "true" : "false";
                }
                else if (firstOperand.isString)
                {
                    result.defaultLabel =
                        firstOperand.stringValue != secondOperand.stringValue ? "true" : "false";
                }
            }
        }
        else if (operatorNode.operatorStringValue == "or")
        {
            if (!firstOperand.isBoolean || !secondOperand.isBoolean)
            {
                cout << "\n operands are not boolean for 'or' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = (firstOperand.defaultLabel == "true" || secondOperand.defaultLabel == "true")
                                          ? "true"
                                          : "false";
            }
        }
        else if (operatorNode.operatorStringValue == "&")
        {
            if (!firstOperand.isBoolean || !secondOperand.isBoolean)
            {
                cout << "\n operands are not boolean for '&' operation! exiting! \n";
                exit(0);
            }
            else
            {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = (firstOperand.defaultLabel == "true" && secondOperand.defaultLabel == "true")
                                          ? "true"
                                          : "false";
            }
        }
        cseMachineStack.push(result);
    }
    else if (controlTop.isUnaryOperator)
    { // CSE rule 7
        CSEMachineNode result = CSEMachineNode();
        CSEMachineNode operatorNode = controlTop;
        CSEMachineNode firstOperand = cseMachineStack.top();
        cseMachineStack.pop();
        if (operatorNode.operatorStringValue == "neg")
        {
            if (!firstOperand.isInt)
            {
                cout << "\n Operand is not int to apply 'neg', EXIT! \n";
                exit(0);
            }
            else
            {
                result.isInt = true;
                result.intValue = -firstOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        }
        else if (operatorNode.operatorStringValue == "not")
        {
            if (!firstOperand.isBoolean)
            {
                cout << "\n Operand is not boolean to apply not, EXIT! \n";
                exit(0);
            }
            else
            {
                result.isBoolean = true;
                if (firstOperand.defaultLabel == "true")
                {
                    result.defaultLabel = "false";
                }
                else if (firstOperand.defaultLabel == "false")
                {
                    result.defaultLabel = "true";
                }
            }
        }
        cseMachineStack.push(result);
    }
    else if (controlTop.isConditional)
    { // CSE rule 8
        CSEMachineNode booleanNode = cseMachineStack.top();
        cseMachineStack.pop();
        CSEMachineNode falseNode = cseMachineControl.top();
        cseMachineControl.pop();
        CSEMachineNode trueNode = cseMachineControl.top();
        cseMachineControl.pop();
        int controlStructureIndexOfChosenConditional;
        if (booleanNode.defaultLabel == "true")
        {
            // choose the true control structure
            controlStructureIndexOfChosenConditional = trueNode.indexOfBodyOfLambda;
        }
        else if (booleanNode.defaultLabel == "false")
        {
            // choose the true control structure
            controlStructureIndexOfChosenConditional = falseNode.indexOfBodyOfLambda;
        }
        // push the 0th control structure's elements
        std::list<CSEMachineNode>::const_iterator iterator;
        for (iterator = controlStructures[controlStructureIndexOfChosenConditional].begin();
             iterator != controlStructures[controlStructureIndexOfChosenConditional].end(); ++iterator)
        {
            CSEMachineNode controlStructureToken = *iterator;
            cseMachineControl.push(controlStructureToken);
        }
    }
    else if (controlTop.isTau)
    { // CSE rule 9 for Tau tuple formation on CSE's stack structure
        int numberOfTupleElements = controlTop.numberOfElementsInTauTuple;
        // TODO: This checking for if the popped elements are environmentMarkers and working around it was added to handle the 'recurs.1'
        // program. In that program, the tau selection didn't have enough elements for it on the stack. Is this the actual way to do it?
        stack<CSEMachineNode> environmentVariablesToBePushedBackToStack;
        while (numberOfTupleElements > 0 && !cseMachineStack.empty())
        {
            CSEMachineNode poppedStackElement = cseMachineStack.top();
            cseMachineStack.pop();
            if (!poppedStackElement.isEnvironmentMarker)
            {
                numberOfTupleElements--;
                controlTop.tupleElements.push_back(poppedStackElement);
            }
            else
            {
                environmentVariablesToBePushedBackToStack.push(poppedStackElement);
            }
        }
        controlTop.isTau = false;
        controlTop.isTuple = true;
        controlTop.defaultLabel = "TupleOfSize=" + std::to_string(controlTop.tupleElements.size());
        controlTop.numberOfElementsInTauTuple = controlTop.tupleElements.size();
        while (!environmentVariablesToBePushedBackToStack.empty())
        {
            cseMachineStack.push(environmentVariablesToBePushedBackToStack.top());
            environmentVariablesToBePushedBackToStack.pop();
        }
        cseMachineStack.push(controlTop);
    }
}

void runCSEMachine()
{
    initializeCSEMachine();
    while (!cseMachineControl.empty())
    {
        processCSEMachine(); // process the value on top of the control stack one by one
        // according to the rules of the CSE machine
    }
}