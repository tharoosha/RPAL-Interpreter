#include "lexicon.hpp"
#include "iostream"
#include <fstream>
#include <cstring>

using namespace std;

string NT;
const string IDENTIFIER_TOKEN = "IDENTIFIER";
const string STRING_TOKEN = "STRING";
const string OPERATOR_TOKEN = "OPERATOR";
const string INTEGER_TOKEN = "INTEGER";
const string KEYWORD_TOKEN = "KEYWORD";
const string UNDEFINED_TOKEN = "UNDEFINED";
const string PUNCTUATION_TOKEN = "PUNCTUATION";

const char *operatorArray = "+-*<>&.@/:=~|$!#%^_[]{}\"`?";

const char *stringAllowedCharArray = "();, ";
const char *stringAllowedEscapeCharArray = "tn\\\'";

const char *eolCharArray = "\r\n";

const char *punctuationArray = "();,";

string nextTokenType = UNDEFINED_TOKEN;

/*
    Checks if the end of the file has been reached using the good() function and peek()
    function. The good() function checks if the stream is in a good state, and the peek()
    function looks ahead to the next character without extracting it.
*/
bool checkIfEOF(ifstream &file)
{
    if (!file.good() || file.peek() == EOF)
    {
        return true;
    }
    return false;
}

/*
    Read an identifier/keyword token into NT.
*/
void readIdentifierToken(ifstream &file)
{
    if (checkIfEOF(file))
    {
        // cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = IDENTIFIER_TOKEN;
    char x;                  // get the next character in input stream
    char peek = file.peek(); // peek and store the next character in input stream

    while (isdigit(peek) || isalpha(peek) || peek == '_')
    {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    if (NT.compare("rec") == 0 || NT.compare("where") == 0 || NT.compare("in") == 0 || NT.compare("and") == 0 ||
        NT.compare("let") == 0 || NT.compare("fn") == 0 || NT.compare("or") == 0 || NT.compare("not") == 0 ||
        NT.compare("gr") == 0 || NT.compare("ge") == 0 || NT.compare("ls") == 0 || NT.compare("le") == 0 ||
        NT.compare("eq") == 0 || NT.compare("ne") == 0 || NT.compare("within") == 0 || NT.compare("true") == 0 ||
        NT.compare("false") == 0 || NT.compare("nil") == 0 || NT.compare("dummy") == 0 || NT.compare("aug") == 0)
    {
        nextTokenType = KEYWORD_TOKEN;
        // cout << "\nKeyword: " << NT << "\n";
    }
    /*else
    {
        cout << '\n'
             << nextTokenType << ": " << NT << "\n";
    }*/
}

void readIntegerToken(ifstream &file)
{
    if (checkIfEOF(file))
    {
        // cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = INTEGER_TOKEN;
    char x;                  // get the next character in input stream
    char peek = file.peek(); // peek and store the next character input stream

    while (isdigit(peek))
    {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    // cout << "\n" << nextTokenType << ": " << NT << "\n";
}

bool isOperator(char c)
{
    if (strchr(operatorArray, c))
        return true;
    else
        return false;
}

void readOperatorToken(ifstream &file)
{
    if (checkIfEOF(file))
    {
        // cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = OPERATOR_TOKEN;
    char x;                  // get the next character in stream in this
    char peek = file.peek(); // peek and store the next character in stream in this
    while (isOperator(peek))
    {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    // cout << '\n' << nextTokenType << ": " << NT << "\n";
}

bool isPunctuation(char c)
{
    if (strchr(punctuationArray, c))
        return true;
    else
        return false;
}

void readPunctuationChar(ifstream &file)
{
    if (checkIfEOF(file))
    {
        // cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = PUNCTUATION_TOKEN;
    char x;                  // get the next character in input stream
    char peek = file.peek(); // peek and store the next character input stream
    if (isPunctuation(peek))
    {
        file.get(x);
        NT += x;
    }
    // cout << "\n" << nextTokenType << ": " << NT << "\n";
}

bool isStringAllowedChar(char c)
{
    if (strchr(stringAllowedCharArray, c) || isdigit(c) || isalpha(c) || isOperator(c))
        return true;
    else
        return false;
}

bool isEscapeCharInString(ifstream &file, char &peek)
{
    char x; // get the next character in stream in this
    // peek and store the next character in stream in this
    if (peek == '\\')
    {
        file.get(x);
        NT += x; // Add the escape backslash to the string token (as per the reference implementation)
        peek = file.peek();
        if (strchr(stringAllowedEscapeCharArray, peek))
        {
            file.get(x);
            NT += x;
            peek = file.peek();
            return true;
        }
        else
        {
            cout << "\n\nERROR! Expected an escape character, but " << peek << " happened! Parser will DIE now!\n\n";
            throw exception();
        }
    }
    else
        return false;
}

void readStringToken(ifstream &file)
{
    if (checkIfEOF(file))
    {
        // cout << "\n\nEOF reached unexpectedly without correct parsing through grammar! Will die now!!\n\n";
        exit(0);
    }
    nextTokenType = STRING_TOKEN;
    char x;                  // get the next character in stream in this
    char peek = file.peek(); // peek and store the next character in stream in this

    if (peek == '\'')
    { // check for the single quote to start the string
        file.get(x);
        NT += x; // Add quotes to the token to separate the string from non-string literals with same value
        peek = file.peek();
    }
    else
    {
        // cout << "\n\nERROR! Expected start of string, but " << peek << " happened! Parser will DIE now!\n\n";
        throw exception();
    }
    while (isStringAllowedChar(peek) || (isEscapeCharInString(file, peek) && isStringAllowedChar(peek)))
    {
        file.get(x);
        NT += x;
        peek = file.peek();
    }
    if (peek == '\'')
    { // check for the single quote to close the string
        file.get(x);
        NT += x; // Add quotes to the token to separate the string from non-string literals with same value
    }
    else
    {
        // cout << "\n\nERROR! Expected close of string, but " << peek << " happened! Parser will DIE now!\n\n";
        throw exception();
    }
    // cout << "\n" << nextTokenType << ": " << NT << "\n";
}

void scan(ifstream &file);

void resolveIfCommentOrOperator(ifstream &file)
{
    char x;
    file.get(x); // Move past the first '/'
    char peek = file.peek();
    if (peek == '/')
    {
        // This means it's a comment line, so keep reading/updating file stream pointer without "tokenizing" (adding to NT) until an eol.
        while (!strchr(eolCharArray, peek))
        {
            file.get(x); // move past the whitespaces until an eol
            peek = file.peek();
        }
        file.get(x); // Move past the EOL
        // cout << "\nComment ignored";
        // cout << "\nGoing to scan!";
        scan(file); // call scan to get the next token
    }
    else
    {
        // this means it's an operator sequence
        NT += '/'; // Add the first '/' that we moved past to the operator token
        readOperatorToken(file);
    }
}

void scan(ifstream &file)
{
    if (checkIfEOF(file))
    {
        // cout << "\n\nEOF reached !\n\n";
        return;
    }
    nextTokenType = UNDEFINED_TOKEN;

    char peek = file.peek(); // peek and store the next character in stream in this
    // cout << "\nIn scan, peek= '" << peek << "'";
    NT.clear(); // clear NT to get the next token in file

    if (isalpha(peek))
    {
        readIdentifierToken(file);
    }
    else if (isdigit(peek))
    {
        readIntegerToken(file);
    }
    else if (peek == '/')
    {
        resolveIfCommentOrOperator(file);
    }
    else if (isOperator(peek))
    {
        readOperatorToken(file);
    }
    else if (peek == '\'')
    { // Start of a string
        readStringToken(file);
    }
    else if (iswspace(peek))
    { // ignore whiteSpace chars (space, horTab, newLine, carrReturn, verTab)
        char x;
        file.get(x); // further the file pointer and ignore the whitespace character (no need to tokenize it)
        NT += x;
        // cout << "\nGoing to scan!";
        scan(file); // call scan to get the next token
    }
    else if (isPunctuation(peek))
    {
        readPunctuationChar(file);
    }
}
