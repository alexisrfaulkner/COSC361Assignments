#include <iostream>
#include <fstream>
using namespace std;

#define MAX_TOKENS   8192
#define MAX_IDENT    64

// Tokens names: dictionary of all legal symbols/words in our mini-language
enum TokenType { TOK_VAR, TOK_NUMBER, 
	TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, 
	TOK_LPAREN, TOK_RPAREN, 
	TOK_COMMA, TOK_PERIOD, TOK_SEMI,
	TOK_INT, TOK_FLOAT, 
	TOK_DECL, TOK_EOF, 
	TOK_UNKNOWN 
};

// declare a token structure
struct Token {
	TokenType tokenType;
	string lexeme;
	int line;
	int column;
};

/******************************************************/
/*   Helping function to display the token as a string */
const char* tokenTypeName(TokenType t) {
	switch (t) {
		case TOK_VAR: return "Variable"; break;
		case TOK_NUMBER: return "Number"; break;
		case TOK_INT: return "int"; break;
		case TOK_FLOAT: return "float"; break;

		case TOK_PLUS: return "Plus"; break;
		case TOK_MINUS: return "Minus"; break;
		case TOK_STAR: return "Star"; break;
		case TOK_SLASH: return "Slash"; break;
		case TOK_ASSIGN: return "Assign"; break;

		case TOK_LPAREN: return "LParen"; break;
		case TOK_RPAREN: return "RParen"; break;

		case TOK_COMMA: return "Comma"; break;
		case TOK_PERIOD: return "Period"; break;
		case TOK_SEMI: return "Semicolon"; break;

		case TOK_EOF: return "EOF"; break;
	}
	return "Unknown";
}

// Tokenizer structure 
struct Tokenizer {
	FILE *infp;
	int line;
	int column;
	int cur;
	int hasCur;
};

// --- Basic char reading with position tracking ---

void tkInit(Tokenizer *tk, FILE * infp) {
	tk->infp = infp;
	tk->line = 1;
	tk->column = 1;
	tk->cur = 0;
	tk->hasCur = 0;
}

/********************************************/
/* errMsg - function to display the error message with the line number of the error detected. */
void errMsg (Tokenizer *tk, string msg) {
	cout << "Tokenizer error at line " << tk->line << " column " << tk->column << ": "<< msg << endl;
	exit(1);
	
}

int getChar(Tokenizer *tk) {
	int c = fgetc(tk->infp);
	return c;
}

int peekChar(Tokenizer *tk) {
	if (!tk->hasCur) {
		tk->cur = getChar(tk);
		tk->hasCur = 1; // peeked
	}
	return tk->cur;
}

void consumeChar(Tokenizer *tk) {
	if (!tk->hasCur) { // char was peeked but not consumed
		tk->cur = getChar(tk);
		tk->hasCur = 1; // peeked
	}
	if (tk->cur == '\n') {
		tk->line++;
		tk->column = 1;
	} else if (tk->cur != EOF) {
		tk->column++;
	}
	tk->hasCur = 0; // so next peekChar will read fresh new char
}

/*****************************************************/
/* skipWhiteSpaces - a function to call peekChar until it
 returns a non-whitespace character */
void skipWhiteSpaces(Tokenizer *tk) {
	while (1) {
		int c = peekChar(tk);
		if (c == EOF) return;
		if (isspace(c)) { consumeChar(tk); continue; }
		return;
	}
}

/*******************************************************************
LookupKeyword - a simple lookup code for keywords in the language: */
TokenType lookupKeywords (int c) {
	// return keyword token
	TokenType token = TOK_UNKNOWN;
	switch (c) {
		case '+': token = TOK_PLUS; break;
		case '-': token = TOK_MINUS; break;
		case '*': token = TOK_STAR; break;
		case '/': token = TOK_SLASH; break;
		case '=': token = TOK_ASSIGN; break;
		case '(': token = TOK_LPAREN; break;
		case ')': token = TOK_RPAREN; break;
		case ',': token = TOK_COMMA; break;
		case '.': token = TOK_PERIOD; break;
		case ';': token = TOK_SEMI; break;
	}
	return token;
}

// <first> -> A..Z | a..z | _
bool isFirst(int c) {
    return isalpha(c) || c == '_';
}

// <rest> -> <first> | <digit>
bool isRest(int c) {
	return isFirst(c) || isdigit(c);
}

void tkReadIdent(Tokenizer *tk, Token *token) {
	while (1) {
		int c = peekChar(tk);
		if (c == EOF || !isRest(c)) { break; }
		token->lexeme += char(c);
		consumeChar(tk);
	}
	token->tokenType = TOK_VAR;
	token->line = tk->line;
	token->column = tk->column;
	if (token->lexeme == "int") token->tokenType = TOK_INT;
	else if (token->lexeme == "float") token->tokenType = TOK_FLOAT;
}

void tkReadNumber(Tokenizer *tk, Token *token) {
	while (1) {
		int c = peekChar(tk);
		if (c == EOF || !isdigit(c)) break;
		token->lexeme += char(c);
		consumeChar(tk);
	}
	token->tokenType = TOK_NUMBER;
	token->line = tk->line;
	token->column = tk->column;
}

/*****************************************************/
/* lex - a simple lexical analyzer for arithmetic 
 expressions */
int tokenize(Tokenizer *tk, Token *tokens, int maxTokens) {
	int count = 0;
	while (1) {
		skipWhiteSpaces(tk);

		if (count >= maxTokens) errMsg(tk, "too many tokens");

		int c = peekChar(tk);
		int ln = tk->line;
		int col = tk->column;
		
		if (c == EOF) {
			tokens[count].tokenType = TOK_EOF;
			tokens[count].lexeme = "\0";
			tokens[count].line = ln;
			tokens[count].column = col;
			return count+1;
		}

		if (isFirst(c)) { // identifier
			tkReadIdent(tk, &tokens[count]); 
			count++;
			continue;
		}
		if (isdigit(c)) { // number
			tkReadNumber(tk, &tokens[count]);
			count++;
			continue;
		}

		// Single symbol: operator or puctuation
		tokens[count].tokenType = lookupKeywords(c);
		tokens[count].lexeme += char(c);
		tokens[count].line = ln;
		tokens[count].column = col;
		if (tokens[count].tokenType == TOK_UNKNOWN) errMsg(tk, "unknown token");
		count++;
		consumeChar(tk);
	}
}


