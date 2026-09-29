#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "toyTokenizer.cpp"
using namespace std;

// =====================================================================
// PARSER  — recursive descent following the BNF
//    S -> <A><C><B> | <A>
//  <A> -> a<A> | a
//  <B> -> b<B> | b
//  <C> -> c
// =====================================================================
struct Parser {
    Token*    tokens;
    int       count;
    int       pos;
};

const Token* prCurrent(const Parser &p) { return &p.tokens[p.pos]; }
bool prCheck(const Parser &p, TokenType t) { return prCurrent(p)->tokenType == t; }

void prAdvance(Parser &p) {
    p.pos++;
}

void prError(const Parser &p, string msg) {
    const Token* t = prCurrent(p);
	cerr << "Parser error at line " << t->line << ": column " << t->column << ": " << msg << " (got " << tokenTypeName(t->tokenType) << ")\n";
    exit(1);
}

// check that the current token is t, then move to the next token
void prExpect(Parser &p, TokenType t, string what) {
    if (!prCheck(p, t)) {
		prError(p, "expected " + what);
    }
    prAdvance(p);
}

// -------- <A> -> a<A> | a --------
void parseA(Parser &p) {
    prExpect(p, TOK_A, "a");
    if (prCheck(p, TOK_A)) parseA(p);      // another 'a' follows: a<A>
}

// -------- <B> -> b<B> | b --------
void parseB(Parser &p) {
    prExpect(p, TOK_B, "b");
    if (prCheck(p, TOK_B)) parseB(p);      // another 'b' follows: b<B>
}

// -------- <C> -> c --------
void parseC(Parser &p) {
    prExpect(p, TOK_C, "c");
}

// -------- S -> <A><C><B> | <A> --------
void parseS(Parser &p) {
    parseA(p);
    if (prCheck(p, TOK_C)) {               // S -> <A><C><B>
        parseC(p);
        parseB(p);
    }                                      // otherwise S -> <A>
}

// -------- <program> -> S {S}   (one sentence per line) --------
void parseProgram(Parser &p) {
    while (!prCheck(p, TOK_ENDFILE)) {
        int ln = prCurrent(p)->line;
        parseS(p);
        // the sentence must use the whole line
        if (!prCheck(p, TOK_ENDFILE) && prCurrent(p)->line == ln)
            prError(p, "unexpected token after a complete sentence");
        cout << "Line " << ln << ": sentence accepted\n";
    }
}

// =====================================================================
// MAIN
// =====================================================================
int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return 1;
    }
    // ---- Tokenize ----
    Tokenizer tk;
    tkInit(&tk, fp);

    static Token tokens[MAX_TOKENS];
    int tokenCount = tokenize(&tk, tokens, MAX_TOKENS);

    fclose(fp);

    cout << "Tokens:\n";
    for (int i = 0; i < tokenCount; i++)
        cout << "  " << tokenTypeName(tokens[i].tokenType) << "\tlexeme=" << tokens[i].lexeme
             << "\tline=" << tokens[i].line << "\tcolumn=" << tokens[i].column << "\n";
    cout << "\nParsing:\n";

	Parser parser;
	parser.tokens = tokens;
	parser.count  = tokenCount;
	parser.pos    = 0;

	parseProgram(parser);
	cout << "No errors found\n";

    return 0;
}
