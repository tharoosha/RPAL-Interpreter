#include "psg.hpp"
#include "lexicon.hpp"
#include <fstream>
#include <iostream>
#include <stack>

using namespace std; 

const string FCN_FORM_LABEL = "function_form";

const string GAMMA_STD_LABEL = "Gamma";
const string LAMBDA_STD_LABEL = "Lambda";

stack<Node *> trees;

using namespace std;

// struct Node { // For the first child next sibling binary tree representation of nary trees.
//     string label;
//     struct Node *firstKid;
//     struct Node *nextSibling;
// };

// stack<Node *> trees; //Stack of trees used to manipulate the AST/ST generation.



void buildTree(string nodeLabel, int noOfTreesToPopAndMakeChildren) {
    cout << "\n# Going to build the node: '" << nodeLabel << "' in tree!";
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
            cout << "\n# Node '" << trees.top()->label << "' to be the child of tree: '" << nodeLabel <<
            "', and left-sibling of '" << treeNodePtr->label;
            trees.top()->nextSibling = treeNodePtr;
            treeNodePtr = trees.top();
        } else {
            treeNodePtr = trees.top();
            cout << "\n# Node '" << treeNodePtr->label << "' to be the child of tree: '" << nodeLabel << "'";
        }
        trees.pop();
        noOfTreesToPopAndMakeChildren--;
    }
    treeNode->firstKid = treeNodePtr;
    cout << "\n# Adding to tree the Node: '" << nodeLabel << "'";
    trees.push(treeNode);
    return;
}



void E(ifstream &file);

void D(ifstream &file);

void readToken(ifstream &file, string token);

/*
 * The procedure for the Vl non-terminal.
 */
int Vl(ifstream &file, int identifiersReadBefore, bool isRecursiveCall) {
    cout << "\nVl!";
    buildTree("<ID:" + NT + ">", 0);
    readToken(file, IDENTIFIER_TOKEN);
    if (NT.compare(",") == 0) {
        readToken(file, ",");
        identifiersReadBefore += 1;
        identifiersReadBefore = Vl(file, identifiersReadBefore, true);
    }
    int identifiersInVList = identifiersReadBefore + 1;
    if (!isRecursiveCall && identifiersInVList > 1) {
        cout << "\nBefore calling buildTree in Vl\n";
        cout << "\nidentifiersInVList= " << identifiersInVList << ", and trees are of number: " << trees.size();
        buildTree(",", identifiersInVList);
    }
    return identifiersReadBefore;
}

/*
 * The procedure for the Vb non-terminal.
 */
void Vb(ifstream &file) {
    cout << "\nVb!";
    if (NT.compare("(") == 0) {
        readToken(file, "(");
        bool isVl = false;
        if (nextTokenType.compare(IDENTIFIER_TOKEN) == 0) {
            Vl(file, 0, false);
            isVl = true;
        }
        readToken(file, ")");
        if (!isVl) {
            cout << "\nBefore calling buildTree in Vb\n";
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
    cout << "\nDb!";
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
            cout << "\nBefore calling buildTree in Db\n";
            buildTree("=", 2);
        } else if (NT.compare("=") == 0) {
            readToken(file, "=");
            E(file);
            cout << "\nBefore calling buildTree in Db1\n";
            buildTree("=", 2);
        } else {
            int n = 1;
            while (nextTokenType.compare(IDENTIFIER_TOKEN) == 0 || NT.compare("(") == 0) {
                Vb(file);
                n++;
            }
            readToken(file, "=");
            E(file);
            cout << "\nBefore calling buildTree in Db2\n";
            buildTree(FCN_FORM_LABEL, n + 1); //n + 'E'
        }
    }
}

/*
 * The procedure for the Dr non-terminal.
 */
void Dr(ifstream &file) {
    cout << "\nDr!";
    int isRec = false;
    if (NT.compare("rec") == 0) {
        cout << "\n Going to consume \"REC!\"";
        readToken(file, "rec");
        isRec = true;
    }
    Db(file);
    if (isRec) {
        cout << "\nBefore calling buildTree in Dr\n";
        buildTree("rec", 1);
    }
}

/*
 * The procedure for the Da non-terminal.
 */
void Da(ifstream &file) {
    cout << "\nDa!";
    Dr(file);
    int n = 1;
    while (NT.compare("and") == 0) {
        readToken(file, "and");
        Dr(file);
        n++;
    }
    if (n > 1) {
        cout << "\nBefore calling buildTree in Da\n";
        buildTree("and", n);
    }
}

/*
 * The procedure for the D non-terminal.
 */
void D(ifstream &file) {
    cout << "\nD!";
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
    cout << "\nRn!";
    if (nextTokenType.compare(IDENTIFIER_TOKEN) == 0) {
        cout << "\n\nbuildTreeNode ID:" + NT + "\n\n";
        buildTree("<ID:" + NT + ">", 0);
        readToken(file, IDENTIFIER_TOKEN);
    } else if (nextTokenType.compare(STRING_TOKEN) == 0) {
        cout << "\n\nbuildTreeNode STR:" + NT + "\n\n";
        buildTree("<STR:" + NT + ">", 0);
        readToken(file, STRING_TOKEN);
    } else if (nextTokenType.compare(INTEGER_TOKEN) == 0) {
        cout << "\n\nbuildTreeNode INT:" + NT + "\n\n";
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
    cout << "\nR!";
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
    cout << "\nAp!";
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
    cout << "\nAf!";
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
    cout << "\nAt!";
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
    cout << "\nA!";
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
    cout << "\nBp!";
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
    cout << "\nBs!";
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
    cout << "\nBt!";
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
    cout << "\nB!";
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
    cout << "\nTc!";
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
    cout << "\nTa!";
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
    cout << "\nT!";
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
    cout << "\nEw!";
    T(file);
    if (NT.compare("where") == 0) { //common prefix
        cout << "\n Going to consume \"WHERE!\"";
        readToken(file, "where");
        Dr(file);
        buildTree("where", 2);
    }
}

void E(ifstream &file) {

    cout << "\nE!";
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

void readToken(ifstream &file, string token) {
    if (token.compare(NT) != 0 && token.compare(nextTokenType) != 0) {
        cout << "\n\nError! Expected '" << token << "' , but found '" << NT << "' !\n\n";
        throw exception();
    }
    cout << "\ntoken '" << token << "' used! going to read next!";
    scan(file);
}