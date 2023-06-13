#ifndef LEXICON_HPP
#define LEXICON_HPP
#include <cstring>
#include <fstream>

using namespace std;

extern string NT; //NextToken
extern const string IDENTIFIER_TOKEN;
extern const string INTEGER_TOKEN;
extern const string STRING_TOKEN;
extern const string OPERATOR_TOKEN;
extern const string KEYWORD_TOKEN;
extern const string UNDEFINED_TOKEN;
extern const string PUNCTUATION_TOKEN;

extern const string FCN_FORM_LABEL;

extern const string GAMMA_STD_LABEL;
extern const string LAMBDA_STD_LABEL;

extern const char *operatorArray;

extern const char *stringAllowedCharArray;

extern const char *stringAllowedEscapeCharArray;

extern const char *eolCharArray;

extern const char *punctuationArray;

extern string nextTokenType;

void scan(ifstream &file);
bool checkIfEOF(ifstream &file);
void readIdentifierToken(ifstream &file);
void readIntegerToken(ifstream &file);
bool isOperator(char c);
void readOperatorToken(ifstream &file);
bool isPunctuation(char c);
void readPunctuationChar(ifstream &file);
bool isStringAllowedChar(char c);
void readStringToken(ifstream &file);
void resolveIfCommentOrOperator(ifstream &file);


#endif //RPAL_INTERPRETER_LEXICON_H
