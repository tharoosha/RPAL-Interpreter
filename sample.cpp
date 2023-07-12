#include <iostream>
#include <fstream>
#include <stack>
#include <list>
#include <map>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

using namespace std;

/**
 * Author: Mickey Vellukunnel
 * UFID: 9413-9616
 *
 * University of Florida (Go Gators!)
 *
 * COP 5556 Spring 2016
 *
 * Project 2: RPal Interpreter (Built upon Project 1: RPal Parser)
 *
 * CSE machine: Control Stack Environment machine. The vehicle for understanding the meaning of an RPAL program.
 *
 */

string NT; //NextToken
const string IDENTIFIER_TOKEN = "IDENTIFIER";
const string INTEGER_TOKEN = "INTEGER";
const string STRING_TOKEN = "STRING";
const string OPERATOR_TOKEN = "OPERATOR";
const string KEYWORD_TOKEN = "KEYWORD";
const string UNDEFINED_TOKEN = "UNDEFINED";
const string PUNCTUATION_TOKEN = "PUNCTUATION";

const string FCN_FORM_LABEL = "function_form";

const string GAMMA_STD_LABEL = "std:Gamma";
const string LAMBDA_STD_LABEL = "std:Lambda";

const char *operatorArray = "+-*<>&.@/:=~|$!#!^_[]{}\"`?";

const char *stringAllowedCharArray = "();, ";

const char *stringAllowedEscapeCharArray = "tn\\\'";

const char *eolCharArray = "\r\n";

const char *punctuationArray = "();,";

string nextTokenType = UNDEFINED_TOKEN;

struct Node { // For the first child next sibling binary tree representation of nary trees.
    string label;
    struct Node *firstKid;
    struct Node *nextSibling;
};

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

stack<Node *> trees; //Stack of trees used to manipulate the AST/ST generation.

std::vector<std::list<MachineNode> > controlStructures(150); //each controlStructure would be a list of machineNodes
int numberOfControlStructures = 1;

EnvironmentNode *environments[1200];

stack<MachineNode> cseMachineControl; //the Control stack of the CSE machine
stack<MachineNode> cseMachineStack; //the "Stack" stack of values of the CSE machine

EnvironmentNode *currentEnvironment = new EnvironmentNode;

int environmentCounter = 0;

/*
 * Check if the end of file has been reached.
 */
bool checkIfEOF(ifstream &file) {
    if (!file.good() || file.peek() == EOF) {
        return true;
    }
    return false;
}

/*
 * Helper function to build a new tree node, and add it to the
 * stack of trees, if necessary by popping existing trees from
 * stack and adding it as children to the new node.
 */

void buildTree(string nodeLabel, int noOfTreesToPopAndMakeChildren) {
    //cout << "\n# Going to build the node: '" << nodeLabel << "' in tree!";
    Node *treeNode = new Node;
    treeNode->label = nodeLabel;
    treeNode->nextSibling = NULL;
    Node *treeNodePtr = NULL; //which will point to the first child of children (if any) of the newly added node
    if (noOfTreesToPopAndMakeChildren > 0 && trees.empty()) {
        cout << "\n\nERROR! Something went wrong in AST generation! Program will die now!\n\n";
        exit(0);
    }
    while (noOfTreesToPopAndMakeChildren > 0 && !trees.empty()) {
        if (treeNodePtr != NULL) {
            //cout << "\n# Node '" << trees.top()->label << "' to be the child of tree: '" << nodeLabel <<
            //"', and left-sibling of '" << treeNodePtr->label;
            trees.top()->nextSibling = treeNodePtr;
            treeNodePtr = trees.top();
        } else {
            treeNodePtr = trees.top();
            //cout << "\n# Node '" << treeNodePtr->label << "' to be the child of tree: '" << nodeLabel << "'";
        }
        trees.pop();
        noOfTreesToPopAndMakeChildren--;
    }
    treeNode->firstKid = treeNodePtr;
    //cout << "\n# Adding to tree the Node: '" << nodeLabel << "'";
    trees.push(treeNode);
    return;
}

/**
 * Read an identifier/keyword token into NT.
 */
void readIdentifierToken(ifstream &file) {
    if (checkIfEOF(file)) {
        cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = IDENTIFIER_TOKEN;
    char x; //get the next character in stream in this
    char peek = file.peek(); //peek and store the next character in stream in this
    while (isalpha(peek) || isdigit(peek) || peek == '_') {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    if (NT.compare("rec") == 0 || NT.compare("where") == 0 || NT.compare("in") == 0 || NT.compare("and") == 0 ||
        NT.compare("let") == 0 || NT.compare("fn") == 0 || NT.compare("or") == 0 || NT.compare("not") == 0 ||
        NT.compare("gr") == 0 || NT.compare("ge") == 0 || NT.compare("ls") == 0 || NT.compare("le") == 0 ||
        NT.compare("eq") == 0 || NT.compare("ne") == 0 || NT.compare("within") == 0 || NT.compare("true") == 0 ||
        NT.compare("false") == 0 || NT.compare("nil") == 0 || NT.compare("dummy") == 0 || NT.compare("aug") == 0) {
        nextTokenType = KEYWORD_TOKEN;
        //cout << "\nKeyword: " << NT << "\n";
    }
    else {
        //cout << "\nID: " << NT << "\n";
    }
}

/**
 * Read an integer token into NT.
 */
void readIntegerToken(ifstream &file) {
    if (checkIfEOF(file)) {
        cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = INTEGER_TOKEN;
    char x; //get the next character in stream in this
    char peek = file.peek(); //peek and store the next character in stream in this
    while (isdigit(peek)) {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    //cout << "\nINT: " << NT << "\n";
}

/**
 * Check if the char is an operator.
 */
bool isOperator(char c) {
    if (strchr(operatorArray, c))
        return true;
    else
        return false;
}

/**
 * Check if the char is a punctuation.
 */
bool isPunctuation(char c) {
    if (strchr(punctuationArray, c))
        return true;
    else
        return false;
}

/**
 * Read a punctuation token into NT.
 */
void readPunctuationChar(ifstream &file) {
    if (checkIfEOF(file)) {
        cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = PUNCTUATION_TOKEN;
    char x; //get the next character in stream in this
    char peek = file.peek(); //peek and store the next character in stream in this
    if (isPunctuation(peek)) {
        file.get(x);
        NT += x;
    }
    //cout << "\nPunc: " << NT << "\n";
}

/**
 * Check if the char is allowed in a string.
 */
bool isStringAllowedChar(char c) {
    if (strchr(stringAllowedCharArray, c) || isdigit(c) || isalpha(c) || isOperator(c))
        return true;
    else
        return false;
}

/**
 * Check if the char is an escape character.
 */
bool isEscapeCharInString(ifstream &file, char &peek) {
    char x; //get the next character in stream in this
    //peek and store the next character in stream in this
    if (peek == '\\') {
        file.get(x);
        NT += x; //Add the escape backslash to the string token (as per the reference implementation)
        peek = file.peek();
        if (strchr(stringAllowedEscapeCharArray, peek)) {
            file.get(x);
            NT += x;
            peek = file.peek();
            return true;
        }
        else {
            cout << "\n\nERROR! Expected an escape character, but " << peek << " happened! Parser will DIE now!\n\n";
            throw exception();
        }
    }
    else
        return false;
}

/**
 * Read a string token into NT.
 */
void readStringToken(ifstream &file) {
    if (checkIfEOF(file)) {
        cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = STRING_TOKEN;
    char x; //get the next character in stream in this
    char peek = file.peek(); //peek and store the next character in stream in this

    if (peek == '\'') { //check for the single quote to start the string
        file.get(x);
        NT += x; //Add quotes to the token to separate the string from non-string literals with same value
        peek = file.peek();
    } else {
        cout << "\n\nERROR! Expected start of string, but " << peek << " happened! Parser will DIE now!\n\n";
        throw exception();
    }
    while (isStringAllowedChar(peek) || (isEscapeCharInString(file, peek) && isStringAllowedChar(peek))) {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    if (peek == '\'') { //check for the single quote to close the string
        file.get(x);
        NT += x; //Add quotes to the token to separate the string from non-string literals with same value
    } else {
        cout << "\n\nERROR! Expected close of string, but " << peek << " happened! Parser will DIE now!\n\n";
        throw exception();
    }
    //cout << "\nString: " << NT << "\n";
}

/**
 * Read an operator token into NT.
 */
void readOperatorToken(ifstream &file) {
    if (checkIfEOF(file)) {
        cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = OPERATOR_TOKEN;
    char x; //get the next character in stream in this
    char peek = file.peek(); //peek and store the next character in stream in this
    while (isOperator(peek)) {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    //cout << "\nOp: " << NT << "\n";
}

void scan(ifstream &file);

/*
 * Check and resolve if the next token sequence is of a comment ("//") ,
 * or if it's an operator ("/").
 */
void resolveIfCommentOrOperator(ifstream &file) {
    char x;
    file.get(x); //Move past the first '/'
    char peek = file.peek();
    if (peek == '/') {
        //This means it's a comment line, so keep reading/updating file stream pointer without "tokenizing" (adding to NT) until an eol.
        while (!strchr(eolCharArray, peek)) {
            file.get(x); //move past the whitespaces until an eol
            peek = file.peek();
        }
        file.get(x); //Move past the EOL
//        cout << "\nComment ignored";
//        cout << "\nGoing to scan!";
        scan(file); //call scan to get the next token
    } else {
        // this means it's an operator sequence
        NT += '/'; //Add the first '/' that we moved past to the operator token
        readOperatorToken(file);
    }
}

void E(ifstream &file);

void D(ifstream &file);

void readToken(ifstream &file, string token);

/*
 * The procedure for the Vl non-terminal.
 */
int Vl(ifstream &file, int identifiersReadBefore, bool isRecursiveCall) {
    //cout << "\nVl!";
    buildTree("<ID:" + NT + ">", 0);
    readToken(file, IDENTIFIER_TOKEN);
    if (NT.compare(",") == 0) {
        readToken(file, ",");
        identifiersReadBefore += 1;
        identifiersReadBefore = Vl(file, identifiersReadBefore, true);
    }
    int identifiersInVList = identifiersReadBefore + 1;
    if (!isRecursiveCall && identifiersInVList > 1) {
        //cout << "\nBefore calling buildTree in Vl\n";
        //cout << "\nidentifiersInVList= " << identifiersInVList << ", and trees are of number: " << trees.size();
        buildTree(",", identifiersInVList);
    }
    return identifiersReadBefore;
}

/*
 * The procedure for the Vb non-terminal.
 */
void Vb(ifstream &file) {
    //cout << "\nVb!";
    if (NT.compare("(") == 0) {
        readToken(file, "(");
        bool isVl = false;
        if (nextTokenType.compare(IDENTIFIER_TOKEN) == 0) {
            Vl(file, 0, false);
            isVl = true;
        }
        readToken(file, ")");
        if (!isVl) {
            //cout << "\nBefore calling buildTree in Vb\n";
            buildTree("()", 0);
        }
    } else if (nextTokenType.compare(IDENTIFIER_TOKEN) == 0) {
        buildTree("<ID:" + NT + ">", 0);
        readToken(file, IDENTIFIER_TOKEN);
    }
}

/*
 * The procedure for the Db non-terminal.
 */
void Db(ifstream &file) {
    //cout << "\nDb!";
    if (NT.compare("(") == 0) {
        readToken(file, "(");
        D(file);
        readToken(file, ")");
    } else if (nextTokenType.compare(IDENTIFIER_TOKEN) == 0) {
        buildTree("<ID:" + NT + ">", 0);
        readToken(file, IDENTIFIER_TOKEN);
        if (NT.compare(",") == 0) {
            readToken(file, ",");
            Vl(file, 1, false);
            readToken(file, "=");
            E(file);
            //cout << "\nBefore calling buildTree in Db\n";
            buildTree("=", 2);
        } else if (NT.compare("=") == 0) {
            readToken(file, "=");
            E(file);
            //cout << "\nBefore calling buildTree in Db1\n";
            buildTree("=", 2);
        } else {
            int n = 1;
            while (nextTokenType.compare(IDENTIFIER_TOKEN) == 0 || NT.compare("(") == 0) {
                Vb(file);
                n++;
            }
            readToken(file, "=");
            E(file);
            //cout << "\nBefore calling buildTree in Db2\n";
            buildTree(FCN_FORM_LABEL, n + 1); //n + 'E'
        }
    }
}

/*
 * The procedure for the Dr non-terminal.
 */
void Dr(ifstream &file) {
    //cout << "\nDr!";
    int isRec = false;
    if (NT.compare("rec") == 0) {
        //cout << "\n Going to consume \"REC!\"";
        readToken(file, "rec");
        isRec = true;
    }
    Db(file);
    if (isRec) {
        //cout << "\nBefore calling buildTree in Dr\n";
        buildTree("rec", 1);
    }
}

/*
 * The procedure for the Da non-terminal.
 */
void Da(ifstream &file) {
    //cout << "\nDa!";
    Dr(file);
    int n = 1;
    while (NT.compare("and") == 0) {
        readToken(file, "and");
        Dr(file);
        n++;
    }
    if (n > 1) {
        //cout << "\nBefore calling buildTree in Da\n";
        buildTree("and", n);
    }
}

/*
 * The procedure for the D non-terminal.
 */
void D(ifstream &file) {
    //cout << "\nD!";
    Da(file);
    if (NT.compare("within") == 0) {
        readToken(file, "within");
        D(file);
        buildTree("within", 2);
    }
}

/*
 * The procedure for the Rn non-terminal.
 */
void Rn(ifstream &file) {
    //cout << "\nRn!";
    if (nextTokenType.compare(IDENTIFIER_TOKEN) == 0) {
        //cout << "\n\nbuildTreeNode ID:" + NT + "\n\n";
        buildTree("<ID:" + NT + ">", 0);
        readToken(file, IDENTIFIER_TOKEN);
    } else if (nextTokenType.compare(STRING_TOKEN) == 0) {
        //cout << "\n\nbuildTreeNode STR:" + NT + "\n\n";
        buildTree("<STR:" + NT + ">", 0);
        readToken(file, STRING_TOKEN);
    } else if (nextTokenType.compare(INTEGER_TOKEN) == 0) {
        //cout << "\n\nbuildTreeNode INT:" + NT + "\n\n";
        buildTree("<INT:" + NT + ">", 0);
        readToken(file, INTEGER_TOKEN);
    } else if (NT.compare("true") == 0 || NT.compare("false") == 0 ||
               NT.compare("nil") == 0 || NT.compare("dummy") == 0) {
        buildTree("<" + NT + ">", 0);
        readToken(file, NT);
    } else if (NT.compare("(") == 0) {
        readToken(file, "(");
        E(file);
        readToken(file, ")");
    }
}

/*
 * The procedure for the R non-terminal.
 */
void R(ifstream &file) {
    //cout << "\nR!";
    Rn(file);
    while (nextTokenType.compare(IDENTIFIER_TOKEN) == 0 || nextTokenType.compare(INTEGER_TOKEN) == 0 ||
           nextTokenType.compare(STRING_TOKEN) == 0 || NT.compare("true") == 0 || NT.compare("false") == 0 ||
           NT.compare("nil") == 0 || NT.compare("dummy") == 0 || NT.compare("(") == 0) {
        Rn(file);
        buildTree("gamma", 2);
    }
}

/*
 * The procedure for the Ap non-terminal.
 */
void Ap(ifstream &file) {
    //cout << "\nAp!";
    R(file);
    while (NT.compare("@") ==
           0) {
        readToken(file, "@");
        buildTree("<ID:" + NT + ">", 0); //the operator which is being inFixed, for example 'Conc'.
        readToken(file, IDENTIFIER_TOKEN);
        R(file);
        buildTree("@", 3);
    }
}

/*
 * The procedure for the Af non-terminal.
 */
void Af(ifstream &file) {
    //cout << "\nAf!";
    Ap(file);
    if (NT.compare("**") == 0) {
        readToken(file, "**");
        Af(file);
        buildTree("**", 2);
    }
}

/*
 * The procedure for the At non-terminal.
 */
void At(ifstream &file) {
    //cout << "\nAt!";
    Af(file);
    while (NT.compare("*") == 0 || NT.compare("/") == 0) {
        string token = NT; //saving token since call to read will update NT with the next token.
        readToken(file, NT);
        Af(file);
        buildTree(token, 2);
    }
}

/*
 * The procedure for the A non-terminal.
 */
void A(ifstream &file) {
    //cout << "\nA!";
    if (NT.compare("+") == 0) {
        readToken(file, "+");
        At(file);
    } else if (NT.compare("-") == 0) {
        readToken(file, "-");
        At(file);
        buildTree("neg", 1);
    } else {
        At(file);
    }
    while (NT.compare("+") == 0 || NT.compare("-") == 0) {
        string token = NT; //saving token since call to read will update NT with the next token.
        readToken(file, NT);
        At(file);
        buildTree(token, 2);
    }
}

/*
 * The procedure for the Bp non-terminal.
 */
void Bp(ifstream &file) {
    //cout << "\nBp!";
    A(file);
    if (NT.compare("gr") == 0 || NT.compare(">") == 0) {
        readToken(file, NT);
        A(file);
        buildTree("gr", 2);
    } else if (NT.compare("ge") == 0 || NT.compare(">=") == 0) {
        readToken(file, NT);
        A(file);
        buildTree("ge", 2);
    } else if (NT.compare("ls") == 0 || NT.compare("<") == 0) {
        readToken(file, NT);
        A(file);
        buildTree("ls", 2);
    } else if (NT.compare("le") == 0 || NT.compare("<=") == 0) {
        readToken(file, NT);
        A(file);
        buildTree("le", 2);
    } else if (NT.compare("eq") == 0) {
        readToken(file, "eq");
        A(file);
        buildTree("eq", 2);
    } else if (NT.compare("ne") == 0) {
        readToken(file, "ne");
        A(file);
        buildTree("ne", 2);
    }
}

/*
 * The procedure for the Bs non-terminal.
 */
void Bs(ifstream &file) {
    //cout << "\nBs!";
    bool isNeg = false;
    if (NT.compare("not") == 0) {
        readToken(file, "not");
        isNeg = true;
    }
    Bp(file);
    if (isNeg) {
        buildTree("not", 1);
    }
}

/*
 * The procedure for the Bt non-terminal.
 */
void Bt(ifstream &file) {
    //cout << "\nBt!";
    Bs(file);
    int n = 1;
    while (NT.compare("&") == 0) {
        readToken(file, "&");
        Bs(file);
        n++;
    }
    if (n > 1) {
        buildTree("&", n);
    }
}

/*
 * The procedure for the B non-terminal.
 */
void B(ifstream &file) {
    //cout << "\nB!";
    Bt(file);
    int n = 1;
    while (NT.compare("or") == 0) {
        readToken(file, "or");
        Bt(file);
        n++;
    }
    if (n > 1) {
        buildTree("or", n);
    }
}

/*
 * The procedure for the Tc non-terminal.
 */
void Tc(ifstream &file) {
    //cout << "\nTc!";
    B(file);
    if (NT.compare("->") ==
        0) {
        readToken(file, "->");
        Tc(file);
        readToken(file, "|");
        Tc(file);
        buildTree("->", 3);
    }
}

/*
 * The procedure for the Ta non-terminal.
 */
void Ta(ifstream &file) {
    //cout << "\nTa!";
    Tc(file);
    while (NT.compare("aug") == 0) { //left recursion
        readToken(file, "aug");
        Tc(file);
        buildTree("aug", 2);
    }
}

/*
 * The procedure for the T non-terminal.
 */
void T(ifstream &file) {
    //cout << "\nT!";
    Ta(file);
    int n = 1;
    while (NT.compare(",") == 0) { //combo of left recursion AND common prefix
        n++;
        readToken(file, ",");
        Ta(file);
    }
    if (n != 1) {
        buildTree("tau", n);
    }
}

/*
 * The procedure for the Ew non-terminal.
 */
void Ew(ifstream &file) {
    //cout << "\nEw!";
    T(file);
    if (NT.compare("where") == 0) { //common prefix
        //cout << "\n Going to consume \"WHERE!\"";
        readToken(file, "where");
        Dr(file);
        buildTree("where", 2);
    }
}

/*
 * The procedure for the E non-terminal.
 */
void E(ifstream &file) {
    //cout << "\nE!";
    int N = 0;
    if (NT.compare("let") == 0) {
        readToken(file, "let");
        D(file);
        readToken(file, "in");
        E(file);
        buildTree("let", 2);
    } else if (NT.compare("fn") == 0) {
        readToken(file, "fn");
        do {
            Vb(file);
            N++;
        } while (nextTokenType.compare(IDENTIFIER_TOKEN) == 0 || NT.compare("(") == 0);
        readToken(file, ".");
        E(file);
        buildTree("lambda", N + 1); //number of 'Vb's plus the 'E'
    } else {
        Ew(file);
    }

}

/*
 * Read the next token sequence into NT.
 */
void scan(ifstream &file) {
    if (checkIfEOF(file)) {
        //cout << "\n\nEOF reached !\n\n";
        return;
    }
    nextTokenType = UNDEFINED_TOKEN;

    char peek = file.peek(); //peek and store the next character in stream in this
    //cout << "\nIn scan, peek= '" << peek << "'";
    NT.clear(); //clear NT to get the next token in file

    if (isalpha(peek)) {
        readIdentifierToken(file);
    } else if (isdigit(peek)) {
        readIntegerToken(file);
    } else if (peek == '/') {
        resolveIfCommentOrOperator(file);
    } else if (isOperator(peek)) {
        readOperatorToken(file);
    } else if (peek == '\'') { //Start of a string
        readStringToken(file);
    } else if (iswspace(peek)) { //ignore whiteSpace chars (space, horTab, newLine, carrReturn, verTab)
        char x;
        file.get(x); //further the file pointer and ignore the whitespace character (no need to tokenize it)
        //NT += x;
        //cout << "\nGoing to scan!";
        scan(file); //call scan to get the next token
    } else if (isPunctuation(peek)) {
        readPunctuationChar(file);
    }
}


/*
 * Print the nodes of the tree in a pre-order fashion.
 */
void recursivelyPrintTree(Node *node, string indentDots) {
    //cout<<"\nPrinting tree for: "<<node->label<<"\n";
    cout << indentDots + node->label << "\n";
    if (node->firstKid != NULL) {
        //cout<<"\nPrinting firstKid tree for: "<<node->label<<"\n";
        recursivelyPrintTree(node->firstKid, indentDots + ".");
    }
    if (node->nextSibling != NULL) {
        //cout<<"\nPrinting nextSibling tree for: "<<node->label<<"\n";
        recursivelyPrintTree(node->nextSibling, indentDots);
    }
    //cout<<"\nDONE! Printing tree for: "<<node->label<<"\n";
}

/*
 * To be called from the standardizer functions.
 * Only prints the node and its children and not the siblings.
 */
void recursivelyPrintTreeNode(Node *node, string indentDots) {
    //cout<<"\nPrinting tree for: "<<node->label<<"\n";
    cout << indentDots + node->label << "\n";
    if (node->firstKid != NULL) {
        //cout<<"\nPrinting firstKid tree for: "<<node->label<<"\n";
        recursivelyPrintTree(node->firstKid, indentDots + "(-.#.-)");
    }
    //cout<<"\nDONE! Printing tree for: "<<node->label<<"\n";
}

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

/* DO not optimize the unary and binary operators (optimizations for the CISE machine)
void convertUop(Node *uopNode) {
    //cout<<"\nInside uop conversion!\nuop ast form before standardizing is:\n";
    //recursivelyPrintTree(uopNode, "");
    Node *uopLeftChild = new Node;
    uopLeftChild->label = uopNode->label;
    uopLeftChild->nextSibling = uopNode->firstKid;
    uopNode->label = GAMMA_STD_LABEL;
    uopNode->firstKid = uopLeftChild;
    //cout<<"\nThe standardized uop is:\n";
    //recursivelyPrintTree(uopNode, "");
}

void convertOperator(Node *binaryOperatorNode) {
    //cout<<"\nInside Operator conversion!\nOp ast form before standardizing is:\n";
    //recursivelyPrintTree(opNode, "");
    Node *leftGammaChild = new Node;
    Node *leftLeftOperatorChild = new Node;
    leftLeftOperatorChild->label = binaryOperatorNode->label;
    leftLeftOperatorChild->firstKid = NULL;
    leftLeftOperatorChild->nextSibling = binaryOperatorNode->firstKid; //E1
    leftGammaChild->label = GAMMA_STD_LABEL;
    leftGammaChild->firstKid = leftLeftOperatorChild;
    leftGammaChild->nextSibling = binaryOperatorNode->firstKid->nextSibling; //E2
    binaryOperatorNode->firstKid->nextSibling = NULL;
    binaryOperatorNode->firstKid = leftGammaChild;
    binaryOperatorNode->label = GAMMA_STD_LABEL;
    //cout<<"\nThe standardized operator is:\n";
    //recursivelyPrintTree(opNode, "");
}
 */

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
    while (temp->nextSibling->nextSibling !=NULL) {
         //temp->nextSibling->nextSibling == NULL implies temp->nextSibling is the "Expression" part of the fcnForm
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

/*
 * Standardize the nodes of the tree in a post-order fashion.
 */
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
        // Do not standardize binary operators (optimizations for the CISE machine) //convertOperator(node);
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

/*
 * Function to pop the (only) node
 * from the stack of trees and print the nodes of the tree
 * in a pre-order fashion.
 */
void printTree() {
    //cout << "\n\nGoing to print the tree now!\n\n";
    if (!trees.empty()) {
        //cout << "\n\nThis is supposed to be the only tree below!\n";
        Node *treeRoot = trees.top();
        recursivelyPrintTree(treeRoot, "");
    }
}

/*
 * Recursively flatten tree.
 */
void recursivelyFlattenTree(Node *treeNode, list<MachineNode> *controlStructure, int controlStructureIndex,
                            bool processKid, bool processSiblings) {
//    cout << "\n in recursivelyFlattenTree for node: " << treeNode->label << ", controlStructure: " <<
//    controlStructureIndex << " and size=" << controlStructure->size();
    MachineNode controlStructureNode = MachineNode();

    controlStructureNode.defaultLabel = treeNode->label;
    if (treeNode->label == "gamma" || treeNode->label == GAMMA_STD_LABEL) {
        controlStructureNode.isGamma = true;
        controlStructureNode.defaultLabel = "gamma";
        controlStructure->push_back(controlStructureNode);
//        cout << "\n it's a gamma!";
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == "Y") {
        controlStructureNode.isY = true;
        controlStructureNode.defaultLabel = "Y";
        controlStructure->push_back(controlStructureNode);
//        cout << "\n it's a Y!";
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label.compare(0, 6, "<STR:'") == 0) {
        controlStructureNode.isString = true;
        controlStructureNode.stringValue = treeNode->label.substr(6);
        controlStructureNode.stringValue = controlStructureNode.stringValue.substr(0,
                                                                                   controlStructureNode.stringValue.length() -
                                                                                   2);
        controlStructureNode.defaultLabel = controlStructureNode.stringValue;
        controlStructure->push_back(controlStructureNode);
//        cout << "\n it's a string!";
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label.compare(0, 4, "<ID:") == 0) {
        controlStructureNode.isName = true;
        controlStructureNode.nameValue = treeNode->label.substr(4);
        controlStructureNode.nameValue = controlStructureNode.nameValue.substr(0,
                                                                               controlStructureNode.nameValue.length() -
                                                                               1);
        controlStructureNode.defaultLabel = controlStructureNode.nameValue;
        controlStructure->push_back(controlStructureNode);
//        cout << "\n it's an identifier!";
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label.compare(0, 5, "<INT:") == 0) {
        controlStructureNode.isInt = true;
        string intString = treeNode->label.substr(5);
        //cout<<"\n intString= "<<intString<<" length= "<<intString.length();
        intString = intString.substr(0,
                                     intString.length() -
                                     1);
        //cout<<"\n intString= "<<intString;
        controlStructureNode.intValue = atoi(intString.c_str());
        controlStructureNode.defaultLabel = intString;
        controlStructure->push_back(controlStructureNode);
//        cout << "\n it's an integer!";
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == "<true>" || treeNode->label == "<false>") {
        controlStructureNode.isBoolean = true;
        controlStructureNode.defaultLabel = treeNode->label == "<true>" ? "true" : "false";
        controlStructure->push_back(controlStructureNode);
//        cout << "\n it's a truthValue!";
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == "<nil>") {
        controlStructureNode.isTuple = true;
        controlStructureNode.defaultLabel = "nil";
        controlStructureNode.numberOfElementsInTauTuple = 0;
        controlStructure->push_back(controlStructureNode);
//        cout << "\n it's nil!";
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == "<dummy>") {
        controlStructureNode.isDummy = true;
        controlStructureNode.defaultLabel = "dummy";
        controlStructure->push_back(controlStructureNode);
//        cout << "\n it's nil!";
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == LAMBDA_STD_LABEL || treeNode->label == "lambda") {
//        cout << "\n it's a lambda!";
        processKid = false;
        controlStructureNode.isLambda = true;
        int numberOfBoundVariables = 0;
        if (treeNode->firstKid->label == ",") {
//            cout << "\nIt's a comma node! bound variables!\n";
            Node *boundVariableNode = treeNode->firstKid->firstKid;
            while (boundVariableNode != NULL) {
                numberOfBoundVariables++;
                string variable = boundVariableNode->label.substr(
                        4);   //bound variables will always start with <ID: and end with >
                variable = variable.substr(0, variable.length() - 1);
                controlStructureNode.boundVariables.push_back(variable);
                boundVariableNode = boundVariableNode->nextSibling;
            }
        } else { //only one bound variable, which is first child (leftChild)
            numberOfBoundVariables++;
//            cout << "\nthe bound variable for this lambda= " << treeNode->firstKid->label << "\n";
            string variable = treeNode->firstKid->label.substr(
                    4);  //bound variables will always start with <ID: and end with >
            variable = variable.substr(0, variable.length() - 1);
            controlStructureNode.boundVariables.push_back(variable);
        }
        controlStructureNode.indexOfBodyOfLambda = numberOfControlStructures++;
        controlStructureNode.numberOfElementsInTauTuple = numberOfBoundVariables;
        string boundVariables;
        for (int i = 0; i < numberOfBoundVariables; i++) {
            boundVariables += controlStructureNode.boundVariables[i] + ", ";
        }
        controlStructureNode.defaultLabel =
                "Lambda with bound variables(" + boundVariables + ") and body(" +
                std::to_string(controlStructureNode.indexOfBodyOfLambda) + ")";
        controlStructure->push_back(controlStructureNode);
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
        list<MachineNode> *controlStructureOfLambda = new list<MachineNode>;
        recursivelyFlattenTree(treeNode->firstKid->nextSibling, controlStructureOfLambda,
                               controlStructureNode.indexOfBodyOfLambda, true, true);
    } else if (treeNode->label == "->") {
//        cout << "\n\n ****** Handle CONDITIONAL! ****** \n\n";
        processKid = false;
        MachineNode trueNode = MachineNode();
        MachineNode falseNode = MachineNode();
        MachineNode betaNode = MachineNode();
        betaNode.isConditional = true;
        trueNode.isConditional = true;
        falseNode.isConditional = true;
        betaNode.defaultLabel = "BetaNode";
        trueNode.defaultLabel = "trueNode";
        falseNode.defaultLabel = "falseNode";
        trueNode.indexOfBodyOfLambda = numberOfControlStructures++;
        falseNode.indexOfBodyOfLambda = numberOfControlStructures++;
        betaNode.indexOfBodyOfLambda = controlStructureIndex;
        list<MachineNode> *controlStructureOfTrueNode = new list<MachineNode>;
        recursivelyFlattenTree(treeNode->firstKid->nextSibling, controlStructureOfTrueNode,
                               trueNode.indexOfBodyOfLambda, true, false);
        list<MachineNode> *controlStructureOfFalseNode = new list<MachineNode>;
        recursivelyFlattenTree(treeNode->firstKid->nextSibling->nextSibling, controlStructureOfFalseNode,
                               falseNode.indexOfBodyOfLambda, true, false);
        controlStructure->push_back(trueNode);
        controlStructure->push_back(falseNode);
        controlStructure->push_back(betaNode);
        recursivelyFlattenTree(treeNode->firstKid, controlStructure,
                               controlStructureIndex, true, false);
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == "not" || treeNode->label == "neg") { //convert unary operators to standardized form
//        cout << "\n it's a " << treeNode->label;
        controlStructureNode.isUnaryOperator = true;
        controlStructureNode.operatorStringValue = treeNode->label;
        controlStructure->push_back(controlStructureNode);
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == "aug" || treeNode->label == "or" || treeNode->label == "&" ||
               treeNode->label == "gr" ||
               treeNode->label == "ge" || treeNode->label == "ls" || treeNode->label == "le" ||
               treeNode->label == "eq" ||
               treeNode->label == "ne" || treeNode->label == "+" || treeNode->label == "-" ||
               treeNode->label == "*" ||
               treeNode->label == "/" || treeNode->label == "**") {
//        cout << "\n it's a " << treeNode->label;
        controlStructureNode.isBinaryOperator = true;
        controlStructureNode.operatorStringValue = treeNode->label;
        controlStructure->push_back(controlStructureNode);
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == "tau") {
//        cout << "\n\n ****** Handle TAU! ****** \n\n";
        processKid = false;
        controlStructureNode.isTau = true;
        int numberOfElementsInTuple = 0;
        Node *tauElementNode = treeNode->firstKid;
        do {
            numberOfElementsInTuple++;
            tauElementNode = tauElementNode->nextSibling;
        } while (tauElementNode != NULL);
        controlStructureNode.numberOfElementsInTauTuple = numberOfElementsInTuple;
        controlStructureNode.defaultLabel =
                "TAU[" + std::to_string(controlStructureNode.numberOfElementsInTauTuple) + "]";
        controlStructure->push_back(controlStructureNode);
        tauElementNode = treeNode->firstKid;
        do {
            MachineNode tupleElementNode = MachineNode();
            if (tauElementNode->label.compare(0, 6, "<STR:'") == 0) {
                tupleElementNode.isString = true;
                tupleElementNode.stringValue = tauElementNode->label.substr(6);
                tupleElementNode.stringValue = tupleElementNode.stringValue.substr(0,
                                                                                   tupleElementNode.stringValue.length() -
                                                                                   2);
                tupleElementNode.defaultLabel = tupleElementNode.stringValue;
//                cout << "\n it's a string!";
                controlStructure->push_back(tupleElementNode);
//                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                controlStructure->size();
            } else if (tauElementNode->label.compare(0, 4, "<ID:") == 0) {
                tupleElementNode.isName = true;
                tupleElementNode.nameValue = tauElementNode->label.substr(4);
                tupleElementNode.nameValue = tupleElementNode.nameValue.substr(0,
                                                                               tupleElementNode.nameValue.length() -
                                                                               1);
                tupleElementNode.defaultLabel = tupleElementNode.nameValue;
//                cout << "\n it's an identifier!";
                controlStructure->push_back(tupleElementNode);
//                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                controlStructure->size();
            } else if (tauElementNode->label.compare(0, 5, "<INT:") == 0) {
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
                controlStructure->size();
            } else if (tauElementNode->label == "<true>" || tauElementNode->label == "<false>") {
                tupleElementNode.isBoolean = true;
                tupleElementNode.defaultLabel = tauElementNode->label == "<true>" ? "true" : "false";
//                cout << "\n it's a truthValue!";
                controlStructure->push_back(tupleElementNode);
//                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                controlStructure->size();
            } else if (tauElementNode->label == "gamma" || tauElementNode->label == GAMMA_STD_LABEL) {
                tupleElementNode.isGamma = true;
//                cout << "\n it's a gamma!";
                tupleElementNode.defaultLabel = "gamma";
                controlStructure->push_back(tupleElementNode);
//                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                controlStructure->size();
                recursivelyFlattenTree(tauElementNode->firstKid, controlStructure, controlStructureIndex, true, true);
//                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                controlStructure->size();
            } else if (tauElementNode->label == "aug" || tauElementNode->label == "or" ||
                       tauElementNode->label == "&" ||
                       tauElementNode->label == "gr" ||
                       tauElementNode->label == "ge" || tauElementNode->label == "ls" ||
                       tauElementNode->label == "le" ||
                       tauElementNode->label == "eq" ||
                       tauElementNode->label == "ne" || tauElementNode->label == "+" || tauElementNode->label == "-" ||
                       tauElementNode->label == "*" ||
                       tauElementNode->label == "/" || tauElementNode->label == "**") {
//                cout << "\n it's a " << tauElementNode->label;
                tupleElementNode.isBinaryOperator = true;
                tupleElementNode.operatorStringValue = tauElementNode->label;
                controlStructure->push_back(tupleElementNode);
//                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                controlStructure->size();
                recursivelyFlattenTree(tauElementNode->firstKid, controlStructure, controlStructureIndex, true, true);
//                cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " <<
                controlStructure->size();
            } else {
//                cout << "\n it's a " << tauElementNode->label;
                recursivelyFlattenTree(tauElementNode, controlStructure, controlStructureIndex, true, false);
            }
            tauElementNode = tauElementNode->nextSibling;
        } while (tauElementNode != NULL);

//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == ",") {
//        cout << "\n\n ****** Handle CommaNode! ****** \n\n";
        controlStructureNode.isComma = true;
        controlStructure->push_back(controlStructureNode);
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    } else if (treeNode->label == "true" || treeNode->label == "false") {
        controlStructureNode.isBoolean = true;
        controlStructure->push_back(controlStructureNode);
//        cout << "\n size of controlStructure '" << controlStructureIndex << "' is= " << controlStructure->size();
    }
    controlStructures[controlStructureIndex] = *controlStructure;

    if (processKid && treeNode->firstKid != NULL) {
        recursivelyFlattenTree(treeNode->firstKid, controlStructure, controlStructureIndex, true, true);
    }

    if (processSiblings && treeNode->nextSibling != NULL) {
        recursivelyFlattenTree(treeNode->nextSibling, controlStructure, controlStructureIndex, true, true);
    }
}

/*
 * Initialize the Control, Stack and environment structure of the CSE
 * machine with its initial values
 */
void initializeCSEMachine() {
    //initialize environment with the primitive environment (PE / e0)
    currentEnvironment->environmentIndex = 0;
    currentEnvironment->parentEnvironment = NULL;
    currentEnvironment->previousEnvironment = NULL;
    environments[environmentCounter++] = currentEnvironment;

//initialize control.
    //push the first token as the e0 environment variable
    MachineNode e0 = MachineNode();
    e0.isEnvironmentMarker = true;
    e0.environmentMarkerIndex = 0;
    e0.defaultLabel = "e0";
    cseMachineControl.push(e0);
// push the 0th control structure's elements
    std::list<MachineNode>::const_iterator iterator;
    for (iterator = controlStructures[0].begin(); iterator != controlStructures[0].end(); ++iterator) {
        MachineNode controlStructureToken = *iterator;
        cseMachineControl.push(controlStructureToken);
    }

    //initialize stack with e0 as well
    cseMachineStack.push(e0);
}

/*
 * Utility function which recognizes strings with escape sequences '\n' or '\t'
 * and prints to the output with those utilized.
 */
void printString(std::string stringToPrint) {
    for (size_t i = 0; i < stringToPrint.length(); i++) {
        if (stringToPrint.at(i) == '\\' && stringToPrint.at(i + 1) == 'n') {
            cout << "\n";
            i++;
        } else if (stringToPrint.at(i) == '\\' && stringToPrint.at(i + 1) == 't') {
            cout << "\t";
            i++;
        } else {
            cout << stringToPrint.at(i);
        }
    }
}

/*
 * process the CSE machine by checking the value of top of the
 * control stack and processing it properly.
 */
void processCSEMachine() {
    MachineNode controlTop = cseMachineControl.top();
    cseMachineControl.pop();

//    cout << "\n\n Control's top is: " << controlTop.defaultLabel;

    if (controlTop.isInt || controlTop.isString || controlTop.isBoolean ||
        controlTop.isDummy) { //CSE rule 1 for ints, booleans, dummy and strings
        cseMachineStack.push(controlTop);
    } else if (controlTop.isY) {
        cseMachineStack.push(controlTop);
    } else if (controlTop.isTuple) { //CSE rule 1 for 'nil', which can be the only tuple in a control structure
        cseMachineStack.push(controlTop);
    } else if (controlTop.isName) { //CSE rule 1 for variables
        controlTop.isName = false;
        EnvironmentNode *environmentWithVariableValue = currentEnvironment;
        MachineNode boundedValuesNode;
        bool variableValueFound = false;
        int indexOfBoundVariable = 0;

        while (environmentWithVariableValue != NULL) {
//            cout << "\n\nlooking for " << controlTop.nameValue << " in environment " <<
//            environmentWithVariableValue->environmentIndex;
            boundedValuesNode = environmentWithVariableValue->boundedValuesNode;
            for (int i = 0; i < boundedValuesNode.boundVariables.size(); i++) {
                if (boundedValuesNode.boundVariables[i] == controlTop.nameValue) {
                    indexOfBoundVariable = i;
                    variableValueFound = true;
                    break;
                }
            }
            if (variableValueFound) {
                break;
            } else {
                environmentWithVariableValue = environmentWithVariableValue->parentEnvironment;
            }
        }

        if (!variableValueFound) {
            //it could be a built-in function defined in the PE [e0]
            if (controlTop.nameValue == "Print" || controlTop.nameValue == "Conc" ||
                controlTop.nameValue == "Istuple" || controlTop.nameValue == "Isinteger" ||
                controlTop.nameValue == "Istruthvalue" || controlTop.nameValue == "Isstring" ||
                controlTop.nameValue == "Isfunction" || controlTop.nameValue == "Isdummy" ||
                controlTop.nameValue == "Stem" || controlTop.nameValue == "Stern" || controlTop.nameValue == "Order" ||
                controlTop.nameValue == "ItoS") {
                controlTop.isBuiltInFunction = true;
                controlTop.defaultLabel = controlTop.nameValue;
                cseMachineStack.push(controlTop);
            } else {
                cout << "\n\nERROR! Value for bound variable '" << controlTop.nameValue <<
                "' not found in environment tree! DIE!\n\n";
                exit(0);
            }
        } else {
//            cout << "\n\n Value of " << controlTop.nameValue << " is= ";
            controlTop = environmentWithVariableValue->boundedValuesNode.tupleElements[indexOfBoundVariable];
//            cout << controlTop.defaultLabel << "\n\n";
            cseMachineStack.push(controlTop);
        }
    } else if (controlTop.isEnvironmentMarker) { //CSE rule 5
        MachineNode stackTop = cseMachineStack.top();
        cseMachineStack.pop();
        if (!stackTop.isEnvironmentMarker) {
            MachineNode stackTopEnvironmentVariable = cseMachineStack.top();
            cseMachineStack.pop();
            if (!stackTopEnvironmentVariable.isEnvironmentMarker ||
                (controlTop.environmentMarkerIndex != stackTopEnvironmentVariable.environmentMarkerIndex)) {
                cout << "\n ERROR in resolving environment variables on control and stack! Die now! \n";
                exit(0);
            }
            cseMachineStack.push(stackTop);
        } else {
            if (controlTop.environmentMarkerIndex != stackTop.environmentMarkerIndex) {
                cout << "\n ERROR in resolving environment variables on control and stack! Die now! \n";
                exit(0);
            }
        }
        currentEnvironment = environments[controlTop.environmentMarkerIndex]->previousEnvironment;
    } else if (controlTop.isLambda) {  //CSE rule 2
        controlTop.environmentMarkerIndex = currentEnvironment->environmentIndex; //index of environment in which this lambda holds
        cseMachineStack.push(controlTop);
    } else if (controlTop.isGamma) {  //CSE rule 3 & 4
        MachineNode result = MachineNode();
        MachineNode operatorNode = cseMachineStack.top();
        cseMachineStack.pop();
        MachineNode firstOperand = cseMachineStack.top();
        cseMachineStack.pop();
        if (operatorNode.isUnaryOperator || operatorNode.isUnaryOperator) { //CSE rule 3
            if (operatorNode.isUnaryOperator) {
                if (operatorNode.operatorStringValue == "neg") {
                    if (!firstOperand.isInt) {
                        cout << "\n Operand is not int to apply 'neg', EXIT! \n";
                        exit(0);
                    } else {
                        result.isInt = true;
                        result.intValue = -firstOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                } else if (operatorNode.operatorStringValue == "not") {
                    if (!firstOperand.isBoolean) {
                        cout << "\n Operand is not boolean to apply 'not', EXIT! \n";
                        exit(0);
                    } else {
                        result.isBoolean = true;
                        if (firstOperand.defaultLabel == "true") {
                            result.defaultLabel = "false";
                        } else if (firstOperand.defaultLabel == "false") {
                            result.defaultLabel = "true";
                        }
                    }
                }
                cseMachineStack.push(result);
            }
            else if (operatorNode.isBinaryOperator) {
                MachineNode secondOperand = cseMachineStack.top();
                cseMachineStack.pop();
                if (operatorNode.operatorStringValue == "**") {
                    if (!firstOperand.isInt || !secondOperand.isInt) {
                        cout << "\n operands not int for ** operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = true;
                        result.intValue = pow(firstOperand.intValue, secondOperand.intValue);
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                } else if (operatorNode.operatorStringValue == "*") {
                    if (!firstOperand.isInt || !secondOperand.isInt) {
                        cout << "\n operands not int for * operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = true;
                        result.intValue = firstOperand.intValue * secondOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                } else if (operatorNode.operatorStringValue == "aug") {
                    if (!firstOperand.isTuple) {
                        cout << "\n first Operand is not a tuple for 'aug' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isTuple = true;
                        result.numberOfElementsInTauTuple = firstOperand.numberOfElementsInTauTuple + 1;

                        if (firstOperand.numberOfElementsInTauTuple == 0) { //if the first operand is nil
                            result.tupleElements.push_back(secondOperand);
                        } else {
                            result.tupleElements = firstOperand.tupleElements;
                            result.tupleElements.push_back(secondOperand);
                        }
                        result.defaultLabel = "TupleOfSize=" + std::to_string(result.numberOfElementsInTauTuple);
                    }
                } else if (operatorNode.operatorStringValue == "-") {
                    if (!firstOperand.isInt || !secondOperand.isInt) {
                        cout << "\n operands not int for - operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = true;
                        result.intValue = firstOperand.intValue - secondOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                } else if (operatorNode.operatorStringValue == "+") {
                    if (!firstOperand.isInt || !secondOperand.isInt) {
                        cout << "\n operands not int for + operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = true;
                        result.intValue = firstOperand.intValue + secondOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                } else if (operatorNode.operatorStringValue == "/") {
                    if (!firstOperand.isInt || !secondOperand.isInt) {
                        cout << "\n operands not int for '/' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = true;
                        result.intValue = firstOperand.intValue / secondOperand.intValue;
                        result.defaultLabel = std::to_string(result.intValue);
                    }
                } else if (operatorNode.operatorStringValue == "gr") {
                    if (!firstOperand.isInt || !secondOperand.isInt) {
                        cout << "\n operands not int for 'gr' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = firstOperand.intValue > secondOperand.intValue ? "true" : "false";
                    }
                } else if (operatorNode.operatorStringValue == "ge") {
                    if (!firstOperand.isInt || !secondOperand.isInt) {
                        cout << "\n operands not int for 'ge' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = firstOperand.intValue >= secondOperand.intValue ? "true" : "false";
                    }
                } else if (operatorNode.operatorStringValue == "ls") {
                    if (!firstOperand.isInt || !secondOperand.isInt) {
                        cout << "\n operands not int for 'ls' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = firstOperand.intValue < secondOperand.intValue ? "true" : "false";
                    }
                } else if (operatorNode.operatorStringValue == "le") {
                    if (!firstOperand.isInt || !secondOperand.isInt) {
                        cout << "\n operands not int for 'le' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = firstOperand.intValue <= secondOperand.intValue ? "true" : "false";
                    }
                } else if (operatorNode.operatorStringValue == "eq") {
                    if (!((!firstOperand.isInt || !secondOperand.isInt) ||
                          (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
                          (!firstOperand.isString || !secondOperand.isString))) {
                        cout << "\n operands not of same type for 'eq' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = false;
                        result.isBoolean = true;
                        if (firstOperand.isInt) {
                            result.defaultLabel = firstOperand.intValue == secondOperand.intValue ? "true" : "false";
                        } else if (firstOperand.isBoolean) {
                            result.defaultLabel =
                                    firstOperand.defaultLabel == secondOperand.defaultLabel ? "true" : "false";
                        } else if (firstOperand.isString) {
                            result.defaultLabel =
                                    firstOperand.stringValue == secondOperand.stringValue ? "true" : "false";
                        }
                    }
                } else if (operatorNode.operatorStringValue == "ne") {
                    if (!((!firstOperand.isInt || !secondOperand.isInt) ||
                          (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
                          (!firstOperand.isString || !secondOperand.isString))) {
                        cout << "\n operands not of same type for 'ne' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = false;
                        result.isBoolean = true;
                        if (firstOperand.isInt) {
                            result.defaultLabel = firstOperand.intValue != secondOperand.intValue ? "true" : "false";
                        } else if (firstOperand.isBoolean) {
                            result.defaultLabel =
                                    firstOperand.defaultLabel != secondOperand.defaultLabel ? "true" : "false";
                        } else if (firstOperand.isString) {
                            result.defaultLabel =
                                    firstOperand.stringValue != secondOperand.stringValue ? "true" : "false";
                        }
                    }
                } else if (operatorNode.operatorStringValue == "or") {
                    if (!firstOperand.isBoolean || !secondOperand.isBoolean) {
                        cout << "\n operands are not boolean for 'or' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = (firstOperand.defaultLabel == "true" ||
                                               secondOperand.defaultLabel == "true")
                                              ? "true" : "false";
                    }
                } else if (operatorNode.operatorStringValue == "&") {
                    if (!firstOperand.isBoolean || !secondOperand.isBoolean) {
                        cout << "\n operands are not boolean for '&' operation! exiting! \n";
                        exit(0);
                    } else {
                        result.isInt = false;
                        result.isBoolean = true;
                        result.defaultLabel = (firstOperand.defaultLabel == "true" &&
                                               secondOperand.defaultLabel == "true")
                                              ? "true" : "false";
                    }
                }
                cseMachineStack.push(result);
            }
        } else if (operatorNode.isLambda) {     //CSE rule 4

            //cout << "\n Lambda2 \n";
            //add new lambda's environment variable to control
            MachineNode newEnvironmentVariableForCurrentLambda = MachineNode();
            newEnvironmentVariableForCurrentLambda.isEnvironmentMarker = true;
            newEnvironmentVariableForCurrentLambda.environmentMarkerIndex = environmentCounter++;
            newEnvironmentVariableForCurrentLambda.defaultLabel =
                    "e" + std::to_string(newEnvironmentVariableForCurrentLambda.environmentMarkerIndex);
            cseMachineControl.push(newEnvironmentVariableForCurrentLambda);
            //cout << "\n Lambda3 \n";

            //update currentEnvironment
            EnvironmentNode *newEnvironmentForCurrentLambda = new EnvironmentNode();
            newEnvironmentForCurrentLambda->parentEnvironment = environments[operatorNode.environmentMarkerIndex];
            newEnvironmentForCurrentLambda->previousEnvironment = currentEnvironment;
            currentEnvironment = newEnvironmentForCurrentLambda;
            newEnvironmentForCurrentLambda->environmentIndex = newEnvironmentVariableForCurrentLambda.environmentMarkerIndex;
            newEnvironmentForCurrentLambda->boundedValuesNode = MachineNode();
            newEnvironmentForCurrentLambda->boundedValuesNode.boundVariables = operatorNode.boundVariables;
            environments[newEnvironmentForCurrentLambda->environmentIndex] = newEnvironmentForCurrentLambda;

            // We have separate cases here instead of just assigning
            // newEnvironmentForCurrentLambda->boundedValuesNode = firstOperand
            // because we need to have the boundVariables and the tupleElements in the same boundedValuesNode.


            if (operatorNode.boundVariables.size() ==
                1) { //only one bound variable, then the firstOperand is stored as it is in the tupleElements
                //first operand could be int/string/tuple
                newEnvironmentForCurrentLambda->boundedValuesNode.tupleElements.push_back(firstOperand);
            } else { //there are multiple variable bindings, so the firstOperand must be a tuple and that is what we assign
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

            //cout << "\n Lambda4 \n";

            //add new lambda environment variable to stack
            cseMachineStack.push(newEnvironmentVariableForCurrentLambda);

            //cout << "\n Lambda5 \n";
            //add lambda's control structure to control
            std::list<MachineNode>::const_iterator iterator;
            for (iterator = controlStructures[operatorNode.indexOfBodyOfLambda].begin();
                 iterator != controlStructures[operatorNode.indexOfBodyOfLambda].end(); ++iterator) {
                MachineNode controlStructureToken = *iterator;
                cseMachineControl.push(controlStructureToken);
            }
            //cout << "\n Lambda6 \n";
        } else if (operatorNode.isY) { //CSE rule 12 (applying Y)
            firstOperand.isYF = true;
            firstOperand.isLambda = false;
            cseMachineStack.push(firstOperand);
        } else if (operatorNode.isYF) { //CSE rule 13 (applying f.p.)
            cseMachineStack.push(firstOperand);
            cseMachineStack.push(operatorNode);
            MachineNode lambdaNode = operatorNode;
            lambdaNode.isYF = false;
            lambdaNode.isLambda = true;
            cseMachineStack.push(lambdaNode);
            MachineNode gammaNode = MachineNode();
            gammaNode.isGamma = true;
            gammaNode.defaultLabel = "gamma";
            cseMachineControl.push(gammaNode);
            cseMachineControl.push(gammaNode);
        } else if (operatorNode.isBuiltInFunction) {
            if (operatorNode.defaultLabel == "Print") {
                if (firstOperand.isBoolean) {
                    cout << firstOperand.defaultLabel;
                } else if (firstOperand.isInt) {
                    cout << firstOperand.intValue;
                } else if (firstOperand.isString) {
                    printString(firstOperand.stringValue);

                } else if (firstOperand.isDummy) {
                    //Do nothing
                } else if (firstOperand.isTuple) {
                    if (firstOperand.tupleElements.size() == 0) {
                        cout << "nil"; //empty tuple
                    } else {
                        cout << "(";
                        for (int i = 0; i < firstOperand.tupleElements.size(); i++) {
                            if (firstOperand.tupleElements[i].isBoolean) {
                                cout << firstOperand.tupleElements[i].defaultLabel;
                            } else if (firstOperand.tupleElements[i].isInt) {
                                cout << firstOperand.tupleElements[i].intValue;
                            } else if (firstOperand.tupleElements[i].isString) {
                                printString(firstOperand.tupleElements[i].stringValue);
                            }
                            if (i + 1 != firstOperand.tupleElements.size()) {
                                cout << ", ";
                            }
                        }
                        cout << ")";
                    }
                } else if (firstOperand.isLambda) {
                    cout << "[lambda closure: " + firstOperand.boundVariables[0] + ": " +
                            std::to_string(firstOperand.indexOfBodyOfLambda) + "]";
                } else {
                    cout <<
                    "\n\n ERROR! I don't know how to PRINT the value on stack= " + firstOperand.defaultLabel + "\n\n";
                    exit(0);
                }
            } else if (operatorNode.defaultLabel == "Conc") {
                cseMachineControl.pop(); //to pop out the second gamma node
                MachineNode secondOperand = cseMachineStack.top();
                cseMachineStack.pop();
                result.isString = true;
                result.stringValue = firstOperand.stringValue + secondOperand.stringValue;
                cseMachineStack.push(result);
            } else if (operatorNode.defaultLabel == "Order") {
                if (!firstOperand.isTuple) {
                    cout << "\n\n Error! can't apply 'Order' to a datatype other than tuple! DIE NO! \n\n ";
                    exit(0);
                } else {
                    result.isInt = true;
                    result.intValue = firstOperand.numberOfElementsInTauTuple;
                    result.defaultLabel = std::to_string(result.intValue);
                    cseMachineStack.push(result);
                }
            } else if (operatorNode.defaultLabel == "Stem") {
                result.isString = true;
                result.stringValue = firstOperand.stringValue[0];
                cseMachineStack.push(result);
            } else if (operatorNode.defaultLabel == "Stern") {
                result.isString = true;
                result.stringValue = firstOperand.stringValue.substr(1);
                cseMachineStack.push(result);
            } else if (operatorNode.defaultLabel == "Isstring") {
                result.isBoolean = true;
                result.defaultLabel = firstOperand.isString ? "true" : "false";
                cseMachineStack.push(result);
            } else if (operatorNode.defaultLabel == "Istuple") {
                result.isBoolean = true;
                result.defaultLabel = firstOperand.isTuple ? "true" : "false";
                cseMachineStack.push(result);
            } else if (operatorNode.defaultLabel == "Isinteger") {
                result.isBoolean = true;
                result.defaultLabel = firstOperand.isInt ? "true" : "false";
                cseMachineStack.push(result);
            } else if (operatorNode.defaultLabel == "ItoS") {
                if (!firstOperand.isInt) {
                    cout << "ERROR! operand to ItoS is not Int! DIE NOW!";
                    exit(0);
                }
                result.isString = true;
                result.defaultLabel = std::to_string(firstOperand.intValue);
                result.stringValue = std::to_string(firstOperand.intValue);
                cseMachineStack.push(result);
            } else if (operatorNode.defaultLabel == "Istruthvalue") {
                result.isBoolean = true;
                result.defaultLabel = firstOperand.isBoolean ? "true" : "false";
                cseMachineStack.push(result);
            }
            else {
                cout <<
                "\n\n AYO!! I haven't defined the behavior of the function= " + operatorNode.defaultLabel + "\n\n";
                exit(0);
            }
        } else if (operatorNode.isTuple) {  //  CSE rule 10 for Tuple selection
            result = operatorNode.tupleElements[firstOperand.intValue - 1];
            cseMachineStack.push(result);
        }
    } else if (controlTop.isBinaryOperator) {  //CSE rule 6
        MachineNode result = MachineNode();
        MachineNode operatorNode = controlTop;
        MachineNode firstOperand = cseMachineStack.top();
        cseMachineStack.pop();
        MachineNode secondOperand = cseMachineStack.top();
        cseMachineStack.pop();
        if (operatorNode.operatorStringValue == "**") {
            if (!firstOperand.isInt || !secondOperand.isInt) {
                cout << "\n operands not int for '**' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = true;
                result.intValue = pow(firstOperand.intValue, secondOperand.intValue);
                result.defaultLabel = std::to_string(result.intValue);
            }
        } else if (operatorNode.operatorStringValue == "*") {
            if (!firstOperand.isInt || !secondOperand.isInt) {
                cout << "\n operands not int for '*' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = true;
                result.intValue = firstOperand.intValue * secondOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        } else if (operatorNode.operatorStringValue == "aug") {
            if (!firstOperand.isTuple) {
                cout << "\n first Operand is not a tuple for 'aug' operation! exiting! \n";
                exit(0);
            } else {
                result.isTuple = true;
                result.numberOfElementsInTauTuple = firstOperand.numberOfElementsInTauTuple + 1;

                if (firstOperand.numberOfElementsInTauTuple == 0) { //if the first operand is nil
                    result.tupleElements.push_back(secondOperand);
                } else {
                    result.tupleElements = firstOperand.tupleElements;
                    result.tupleElements.push_back(secondOperand);
                }
                result.defaultLabel = "TupleOfSize=" + std::to_string(result.numberOfElementsInTauTuple);
            }
        } else if (operatorNode.operatorStringValue == "-") {
            if (!firstOperand.isInt || !secondOperand.isInt) {
                cout << "\n operands not int for '-' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = true;
                result.intValue = firstOperand.intValue - secondOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        } else if (operatorNode.operatorStringValue == "+") {
            if (!firstOperand.isInt || !secondOperand.isInt) {
                cout << "\n operands not int for '+' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = true;
                result.intValue = firstOperand.intValue + secondOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        } else if (operatorNode.operatorStringValue == "/") {
            if (!firstOperand.isInt || !secondOperand.isInt) {
                cout << "\n operands not int for '/' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = true;
                result.intValue = firstOperand.intValue / secondOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        } else if (operatorNode.operatorStringValue == "gr") {
            if (!firstOperand.isInt || !secondOperand.isInt) {
                cout << "\n operands not int for 'gr' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = firstOperand.intValue > secondOperand.intValue ? "true" : "false";
            }
        } else if (operatorNode.operatorStringValue == "ge") {
            if (!firstOperand.isInt || !secondOperand.isInt) {
                cout << "\n operands not int for 'ge' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = firstOperand.intValue >= secondOperand.intValue ? "true" : "false";
            }
        } else if (operatorNode.operatorStringValue == "ls") {
            if (!firstOperand.isInt || !secondOperand.isInt) {
                cout << "\n operands not int for 'ls' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = firstOperand.intValue < secondOperand.intValue ? "true" : "false";
            }
        } else if (operatorNode.operatorStringValue == "le") {
            if (!firstOperand.isInt || !secondOperand.isInt) {
                cout << "\n operands not int for 'le' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = firstOperand.intValue <= secondOperand.intValue ? "true" : "false";
            }
        } else if (operatorNode.operatorStringValue == "eq") {
            if (!((!firstOperand.isInt || !secondOperand.isInt) ||
                  (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
                  (!firstOperand.isString || !secondOperand.isString))) {
                cout << "\n operands not of same type for 'eq' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = false;
                result.isBoolean = true;
                if (firstOperand.isInt) {
                    result.defaultLabel = firstOperand.intValue == secondOperand.intValue ? "true" : "false";
                } else if (firstOperand.isBoolean) {
                    result.defaultLabel =
                            firstOperand.defaultLabel == secondOperand.defaultLabel ? "true" : "false";
                } else if (firstOperand.isString) {
                    result.defaultLabel =
                            firstOperand.stringValue == secondOperand.stringValue ? "true" : "false";
                }
            }
        } else if (operatorNode.operatorStringValue == "ne") {
            if (!((!firstOperand.isInt || !secondOperand.isInt) ||
                  (!firstOperand.isBoolean || !secondOperand.isBoolean) ||
                  (!firstOperand.isString || !secondOperand.isString))) {
                cout << "\n operands not of same type for 'ne' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = false;
                result.isBoolean = true;
                if (firstOperand.isInt) {
                    result.defaultLabel = firstOperand.intValue != secondOperand.intValue ? "true" : "false";
                } else if (firstOperand.isBoolean) {
                    result.defaultLabel =
                            firstOperand.defaultLabel != secondOperand.defaultLabel ? "true" : "false";
                } else if (firstOperand.isString) {
                    result.defaultLabel =
                            firstOperand.stringValue != secondOperand.stringValue ? "true" : "false";
                }
            }
        } else if (operatorNode.operatorStringValue == "or") {
            if (!firstOperand.isBoolean || !secondOperand.isBoolean) {
                cout << "\n operands are not boolean for 'or' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = (firstOperand.defaultLabel == "true" || secondOperand.defaultLabel == "true")
                                      ? "true" : "false";
            }
        } else if (operatorNode.operatorStringValue == "&") {
            if (!firstOperand.isBoolean || !secondOperand.isBoolean) {
                cout << "\n operands are not boolean for '&' operation! exiting! \n";
                exit(0);
            } else {
                result.isInt = false;
                result.isBoolean = true;
                result.defaultLabel = (firstOperand.defaultLabel == "true" && secondOperand.defaultLabel == "true")
                                      ? "true" : "false";
            }
        }
        cseMachineStack.push(result);
    } else if (controlTop.isUnaryOperator) {  //CSE rule 7
        MachineNode result = MachineNode();
        MachineNode operatorNode = controlTop;
        MachineNode firstOperand = cseMachineStack.top();
        cseMachineStack.pop();
        if (operatorNode.operatorStringValue == "neg") {
            if (!firstOperand.isInt) {
                cout << "\n Operand is not int to apply 'neg', EXIT! \n";
                exit(0);
            } else {
                result.isInt = true;
                result.intValue = -firstOperand.intValue;
                result.defaultLabel = std::to_string(result.intValue);
            }
        } else if (operatorNode.operatorStringValue == "not") {
            if (!firstOperand.isBoolean) {
                cout << "\n Operand is not boolean to apply not, EXIT! \n";
                exit(0);
            } else {
                result.isBoolean = true;
                if (firstOperand.defaultLabel == "true") {
                    result.defaultLabel = "false";
                } else if (firstOperand.defaultLabel == "false") {
                    result.defaultLabel = "true";
                }
            }
        }
        cseMachineStack.push(result);
    } else if (controlTop.isConditional) { //CSE rule 8
        MachineNode booleanNode = cseMachineStack.top();
        cseMachineStack.pop();
        MachineNode falseNode = cseMachineControl.top();
        cseMachineControl.pop();
        MachineNode trueNode = cseMachineControl.top();
        cseMachineControl.pop();
        int controlStructureIndexOfChosenConditional;
        if (booleanNode.defaultLabel == "true") {
            //choose the true control structure
            controlStructureIndexOfChosenConditional = trueNode.indexOfBodyOfLambda;
        } else if (booleanNode.defaultLabel == "false") {
            //choose the true control structure
            controlStructureIndexOfChosenConditional = falseNode.indexOfBodyOfLambda;
        }
        // push the 0th control structure's elements
        std::list<MachineNode>::const_iterator iterator;
        for (iterator = controlStructures[controlStructureIndexOfChosenConditional].begin();
             iterator != controlStructures[controlStructureIndexOfChosenConditional].end(); ++iterator) {
            MachineNode controlStructureToken = *iterator;
            cseMachineControl.push(controlStructureToken);
        }
    } else if (controlTop.isTau) {  //CSE rule 9 for Tau tuple formation on CSE's stack structure
        int numberOfTupleElements = controlTop.numberOfElementsInTauTuple;
        //TODO: This checking for if the popped elements are environmentMarkers and working around it was added to handle the 'recurs.1'
        //program. In that program, the tau selection didn't have enough elements for it on the stack. Is this the actual way to do it?
        stack<MachineNode> environmentVariablesToBePushedBackToStack;
        while (numberOfTupleElements > 0 && !cseMachineStack.empty()) {
            MachineNode poppedStackElement = cseMachineStack.top();
            cseMachineStack.pop();
            if (!poppedStackElement.isEnvironmentMarker) {
                numberOfTupleElements--;
                controlTop.tupleElements.push_back(poppedStackElement);
            } else {
                environmentVariablesToBePushedBackToStack.push(poppedStackElement);
            }
        }
        controlTop.isTau = false;
        controlTop.isTuple = true;
        controlTop.defaultLabel = "TupleOfSize=" + std::to_string(controlTop.tupleElements.size());
        controlTop.numberOfElementsInTauTuple = controlTop.tupleElements.size();
        while (!environmentVariablesToBePushedBackToStack.empty()) {
            cseMachineStack.push(environmentVariablesToBePushedBackToStack.top());
            environmentVariablesToBePushedBackToStack.pop();
        }
        cseMachineStack.push(controlTop);
    }
}

/*
 * Initialize and run the CSE machine to compute the value
 * of the RPAL program.
 */
void runCSEMachine() {
    initializeCSEMachine();
    while (!cseMachineControl.empty()) {
        processCSEMachine(); //process the value on top of the control stack one by one
        // according to the rules of the CSE machine
    }
}

/*
 * Flatten the standardized tree into control structures
 * with a pre-order traversal.
 */
void flattenStandardizedTree() {
//    cout << "\n\nGoing to flattenStandardizedTree now!\n\n";
    if (!trees.empty()) {
        Node *treeRoot = trees.top();
        //cout << "\n\nBefore pointer declare\n\n";
        list<MachineNode> *controlStructure = new list<MachineNode>;
        //cout << "\n\n after pointer declare\n\n";
        recursivelyFlattenTree(treeRoot, controlStructure, 0, true, true);
    }
}

/*
 * Function to pop the (only) node
 * from the stack of trees and convert the nodes of the tree
 * to a standardized tree (tree whose internal nodes are only gammas or lambdas)
 * in a post-order fashion.
 */
void convertASTToStandardizedTree() {
//    cout << "\n\nGoing to standardize the tree now!\n\n";
    if (!trees.empty()) {
        //cout << "\n\nThis is supposed to be the only tree below!\n";
        Node *treeRootOfAST = trees.top();
        recursivelyStandardizeTree(treeRootOfAST);
    }
}

/*
 *
 */
void printControlStructures() {
    cout << "\n Going to print control structures!\n";
    for (int i = 0; i < numberOfControlStructures; i++) {
        cout << "\nControl Structure Number: " << i << ", number of items in this controlStructure= " <<
        controlStructures[i].size();
        std::list<MachineNode>::const_iterator iterator;
        cout << "\n";
        for (iterator = controlStructures[i].begin(); iterator != controlStructures[i].end(); ++iterator) {
            MachineNode controlStructureToken = *iterator;
            if (controlStructureToken.isGamma) {
                cout << "gamma ";
            } else if (controlStructureToken.isName) {
                cout << controlStructureToken.nameValue << " ";
            } else if (controlStructureToken.isString) {
                cout << controlStructureToken.stringValue << " ";
            } else if (controlStructureToken.isLambda) {
                cout << "lambda[" << controlStructureToken.indexOfBodyOfLambda << "] ";
            } else if (controlStructureToken.isInt) {
                cout << controlStructureToken.intValue << " ";
            } else if (controlStructureToken.isUnaryOperator || controlStructureToken.isBinaryOperator) {
                cout << controlStructureToken.operatorStringValue << " ";
            } else if (controlStructureToken.isTau) {
                cout << "TAU[" + std::to_string(controlStructureToken.numberOfElementsInTauTuple) + "] ";
            } else if (controlStructureToken.isTuple) {
                cout << "nil "; //the only tuple which can occur within a control structure is 'nil'
            } else if (controlStructureToken.isComma) {
                cout << "COMMA ";
            } else if (controlStructureToken.isConditional) {
                cout << controlStructureToken.defaultLabel << "[" << controlStructureToken.indexOfBodyOfLambda << "] ";
            } else if (controlStructureToken.isY || controlStructureToken.isDummy) {
                cout << controlStructureToken.defaultLabel << " ";
            }
        }
        cout << "\n";
    }
}

/*
 * Function to consume the next token from input with the next
 * expected token by the RPAL grammar and move ahead to read the
 * next token from input.
 */
void readToken(ifstream &file, string token) {
    if (token.compare(NT) != 0 && token.compare(nextTokenType) != 0) {
        cout << "\n\nError! Expected '" << token << "' , but found '" << NT << "' !\n\n";
        throw exception();
    }
    //cout << "\ntoken '" << token << "' used! going to read next!";
    scan(file);
}

/*
 * Run the tree standardizer and CSE machine
 * on the standardized tree.
 */
void runAndShowOutput() {
    convertASTToStandardizedTree();
//                    cout << "\nThe standardized tree (ST) is:\n";
//                    printTree(); //print the standardized tree
//                    cout << "\nGoing to flatten the tree:\n";
    flattenStandardizedTree();
//                    cout << "\nThe control structures are:\n";
//                    printControlStructures();
//                    cout << "\nGoing to run the CSE machine now!\n";
    runCSEMachine();
//                    cout << "\n\nThe output of the RPAL program is:\n\n";
//                    int output = cseMachineStack.top().intValue;
//                    cout << output << "\n\n\n";
    cout << "\n";
}

int main(int argc, char *argv[]) {

    if (argc == 3) {
        if (argv[2][0] == '-') {
            cout << "\n\nUsages:\n" << argv[0] << " <filename>\n";
            cout << argv[0] << " -ast <filename>\n";
            cout << argv[0] << " -l <filename>\n\n";
        }
        else {
            // We assume argv[2] is a filename to open
            ifstream the_file(argv[2]);
            // Always check to see if file opening succeeded
            if (!the_file.is_open())
                cout << "\n\nCould not open file: '" << argv[2] << "'" << "\n\n";
            else {
                if (strcmp(argv[1], "-l") == 0) { //Print input? (//Listing of input?)
                    /* Optional switches: -l This produces a listing of the input. */
                    char x;
                    // the_file.get ( x ) returns false if the end of the file
                    //  is reached or an error occurs
                    while (the_file.get(x))
                        cout << x;
                    // the_file is closed implicitly here
                    ifstream file(argv[2]);
                    // Always check to see if file opening succeeded
                    if (!file.is_open())
                        cout << "\n\nCould not open file: '" << argv[2] << "'" << "\n\n";
                    else {
                        //Call ast generator
                        //Call parser
                        //cout << "\n\nShould lexically analyze by recursively descending!!\n\n";
                        scan(file); //Prepare the first token by placing it within 'NT'
                        E(file);    //Call the first non-terminal procedure to start parsing
                        //cout << " " << NT;
                        if (checkIfEOF(file)) {
//                    cout << "\n\nEOF successfully reached after complete parsing! Will exit now!!\n\n";
//                    exit(1);
                            runAndShowOutput();
                        } else {
                            cout <<
                            "\n\nERROR! EOF not reached but went through the complete grammar! Will exit now!!\n\n";
                            exit(0);
                        }
                    }
                } else if (strcmp(argv[1], "-ast") == 0) {
                    /*
                     * Required switches: -ast This switch prints the abstract syntax tree, and nothing else.
                     * No headers or footers.
                     * The AST must match exactly, character for character, the AST produced by rpal.
                     */
                    //Call ast generator
                    //Call parser
                    //cout << "\n\nShould lexically analyze by recursively descending!!\n\n";
                    scan(the_file); //Prepare the first token by placing it within 'NT'
                    E(the_file);    //Call the first non-terminal procedure to start parsing
                    //cout << " " << NT;
                    if (checkIfEOF(the_file)) {
//                    cout << "\n\nEOF successfully reached after complete parsing! Will exit now!!\n\n";
//                    exit(1);
                        //cout << "\n\nAST Tree should print!\n\n";
                        printTree();
                        runAndShowOutput();
                    } else {
                        cout <<
                        "\n\nERROR! EOF not reached but went through the complete grammar! Will exit now!!\n\n";
                        exit(0);
                    }
                }
            }
        }
    }


    else if (argc == 2) {
        if (argv[1][0] == '-') {
            cout << "\n\nUsages:\n" << argv[0] << " <filename>\n";
            cout << argv[0] << " -ast <filename>\n";
            cout << argv[0] << " -l <filename>\n\n";
        }
        else {
            /* p2 (without switches) should produce the result of the program. */
            // We assume argv[2] is a filename to open
            ifstream the_file(argv[1]);
            // Always check to see if file opening succeeded
            if (!the_file.is_open())
                cout << "\n\nCould not open file: '" << argv[1] << "'" << "\n\n";
            else {
                //Call parser
                //cout << "\n\nShould lexically analyze by recursively descending!!\n\n";
                scan(the_file); //Prepare the first token by placing it within 'NT'
                E(the_file);    //Call the first non-terminal procedure to start parsing
                //cout << " " << NT;
                if (checkIfEOF(the_file)) {
//                    cout << "\n\nEOF successfully reached after complete parsing! Will exit now!!\n\n";
//                    exit(1);
//                    cout << "\nThe AST is:\n";
//                    printTree(); //print the AST
                    runAndShowOutput();
                } else {
                    cout << "\n\nERROR! EOF not reached but went through the complete grammar! Will exit now!!\n\n";
                    exit(0);
                }
            }
        }
    }
    else if (argc != 2) {
        /*
         * p2 (without switches) should produce the result of the program.
         * Required switches: -ast This switch prints the abstract syntax tree, and nothing else.
         * No headers or footers. The AST must match exactly, character for character, the AST produced by rpal.
         * Optional switches: -l This produces a listing of the input.
         */
        cout << "\n\nUsages:\n" << argv[0] << " <filename>\n";
        cout << argv[0] << " -ast <filename>\n";
        cout << argv[0] << " -l <filename>\n\n";
    }

}

