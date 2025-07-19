#include <stdbool.h>
#include <stdio.h>

#define DYNAMIC_STRING_IMPLEMENTATION
#include "DynamicString.h"

#define DYNAMIC_ARRAY_DEFAULT_SIZE (10)
#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "DynamicArray.h"

// #define CONSTANT_STRING_IMPLEMENTATION
// #include "ConstantString.h"

#define DEBUG_PRINTING

#define UNREACHABLE(msg)                              \
    do {                                              \
        fprintf(stderr, "UNREACHABLE: \"%s\":\n"      \
                        "    file:     |%s|\n"        \
                        "    function: |%s|\n"        \
                        "    line:     |%d|\n",       \
                (msg), __FILE__, __func__, __LINE__); \
        abort();                                      \
    } while (0)

#define ARRAY_LEN(arr) (sizeof((arr)) / sizeof(*(arr)))

#define MACRO_KEYWORD_PREFIX ('#')
#define MACRO_ARGS_START ('(')
#define MACRO_ARGS_SEPARATOR (',')
#define MACRO_ARGS_END (')')

typedef struct {
    const char *data;
    size_t data_len; // the strlen() of data
    size_t current;
    size_t current_line;
} Lexer;

#define LexerCurrent(l) ((l)->data[(l)->current])
#define LexerCurrentLine(l) ((l)->data[(l)->current_line])

typedef struct {
    size_t current;
    size_t current_line;
} LexerMark;

LexerMark GetMark(Lexer *lexer) {
    return (LexerMark){
        .current = lexer->current,
        .current_line = lexer->current_line,
    };
}

void SetMark(Lexer *lexer, LexerMark mark) {
    lexer->current = mark.current;
    lexer->current_line = mark.current_line;
}

typedef enum {
    TokenEnd,
    TokenNone,
    TokenNewline,
    TokenWhitespace,
    TokenText,
    TokenSymbol,
    TokenNumber,
    TokenMacroKeyword,
} TokenType;

typedef struct {
    TokenType type;
    const char *data;
    size_t data_len; // the strlen() of data
} Token;

#define Token_Fmt "%.*s"
#define Token_Arg(t) (int) (t)->data_len, (t)->data

DynamicArrayDef(Tokens, Token);

#define MACRO_KEYWORD(name, t) \
    { .data = name, .data_len = sizeof(name) - 1, .type = t }

Token MacroKeywords[] = {
    MACRO_KEYWORD("macro", TokenMacroKeyword),
};

#undef MACRO_KEYWORD

const char *TokenTypeName(TokenType t) {
    switch (t) {
        case TokenEnd: return "End";
        case TokenNone: return "None";
        case TokenNewline: return "Newline";
        case TokenWhitespace: return "Whitespace";
        // case TokenText: return "Text";
        case TokenSymbol: return "Symbol";
        case TokenNumber: return "Number";
        case TokenMacroKeyword: return "Macro Keyword";
        default: UNREACHABLE("Unknown TokenType");
    }
}

typedef enum {
    MacroValue,
    MacroArgs,
} MacroType;

typedef struct {
    MacroType type;
    Tokens key;
    Tokens value;
} Macro;

DynamicArrayDef(Macros, Macro);

void printMacro(const Macro *macro) {
    printf("( KEY: ");
    DynamicArrayPrint(&macro->key);
    if (macro->value.count > 0) {
        printf(", VALUE: ");
        DynamicArrayPrint(&macro->value);
    }
    printf(")");
}

Macros DefinedMacros = {
    .printFunc = printMacro,
};

bool is_number(char c) {
    return ('0' <= c && c <= '9');
}

bool is_text(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

// made by looking at an ascii table
bool is_symbol(char c) {
    return ('!' <= c && c <= '/') || (':' <= c && c <= '@') || ('[' <= c && c <= '`') || ('{' <= c && c <= '~');
}

// does not include \n intentionally
bool is_space(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f');
}

bool cmpToken(const Token *a, const Token *b) {
    return (a->data_len == b->data_len) && (strncmp(a->data, b->data, a->data_len) == 0);
}

// increment by 1 and check for out of bounds
bool LexerNext(Lexer *lexer) {
    if (lexer->current >= lexer->data_len) return false;
    char cur = LexerCurrent(lexer);
    lexer->current++;
    if (cur == '\n') lexer->current_line = lexer->current;
    return lexer->current < lexer->data_len;
}

Token GetToken(Lexer *lexer) {
    Token token = {
        .data = &lexer->data[lexer->current],
    };

    if (lexer->current >= lexer->data_len) {
        token.type = TokenEnd;
        return token;
    }
    // decide token type
    bool keywordPrefix = false;
    if (LexerCurrent(lexer) == MACRO_KEYWORD_PREFIX) {
        keywordPrefix = true;
        token.data += 1;
        if (!LexerNext(lexer)) {
            token.type = TokenEnd;
            return token;
        }
    }

    char cur = LexerCurrent(lexer);
    if (cur == '\n') {
        token.type = TokenNewline;
        token.data_len += 1;
        LexerNext(lexer);
        return token;
    } else if (is_symbol(cur)) {
        token.type = TokenSymbol;
        token.data_len += 1;
        LexerNext(lexer);
        return token;
    } else if (is_space(cur)) {
        token.type = TokenWhitespace;
        token.data_len += 1;
        LexerNext(lexer);
        return token;
    } else if (is_text(cur)) {
        token.type = TokenText;
    } else if (is_number(cur)) {
        token.type = TokenNumber;
    } else {
        token.type = TokenNone;
    }

    // decide token data
    if (token.type == TokenText || token.type == TokenNumber) {
        do {
            token.data_len += 1;
            if (!LexerNext(lexer)) break;
            cur = LexerCurrent(lexer);
        } while (is_text(cur) || is_number(cur));
    }

    if (keywordPrefix == true) {
        for (size_t i = 0; i < ARRAY_LEN(MacroKeywords); i++) {
            if (cmpToken(&token, &MacroKeywords[i])) {
                token.type = MacroKeywords[i].type;
                break;
            }
        }
    }

    return token;
}

void printToken(const Token *token) {
    if (token->type == TokenNewline || token->type == TokenWhitespace) {
        printf("(%s)", TokenTypeName(token->type));
    } else {
        printf("(%s): \"" Token_Fmt "\"", TokenTypeName(token->type), Token_Arg(token));
    }
}

Token GetTokenAndIgnore(Lexer *lexer, TokenType ignore) {
    Token token = GetToken(lexer);
    while (token.type != TokenEnd) {
        if (token.type != ignore) return token;
        token = GetToken(lexer);
    }
    return token;
}

void MacroError(Lexer *lexer) {
    int line_len = 0;
    while (lexer->data[lexer->current_line + line_len] != '\n' && lexer->data[lexer->current_line + line_len] != '\0') {
        line_len++;
    }
    fprintf(stderr, "\n----- ERROR -----\n");
    fprintf(stderr, "%.*s\n", line_len, &LexerCurrentLine(lexer));
    fprintf(stderr, "%*s\n", (int) (lexer->current - lexer->current_line), "^");
}

#define MacroReportError(lexer, fmt, ...)         \
    do {                                          \
        MacroError((lexer));                      \
        fprintf(stderr, fmt "\n", ##__VA_ARGS__); \
    } while (0)

Macro *FindMatchingMacro(Token token) {
    if (token.type == TokenText) {
        DynamicArrayForeach(Macro, macro, &DefinedMacros) {
            assert(macro->key.data != NULL && macro->key.count > 0);
            Token macroNameToken = macro->key.data[0];
            if (cmpToken(&token, &macroNameToken)) {
                return macro;
            }
        }
    }
    return NULL;
}

int FindMatchingArgToValue(Macro *macro, const Token *value) {
    for (int i = 1; i < macro->key.count; i++) {
        if (cmpToken(&macro->key.data[i], value)) {
            return i;
        }
    }
    return -1;
}

bool MacroDefine(Lexer *lexer) {
    Macro macro = {
        // the printing is for debugging
        .key = {.printFunc = printToken},
        .value = {.printFunc = printToken},
    };

    Token macroNameToken = GetTokenAndIgnore(lexer, TokenWhitespace);
    if (macroNameToken.type != TokenText) {
        MacroReportError(lexer, "Invalid Macro Name");
        return false;
    }
    DynamicArrayAppend(&macro.key, macroNameToken);
    Token macroValueToken = {0};

    Token macroArgToken = GetTokenAndIgnore(lexer, TokenWhitespace);
    if (macroArgToken.type == TokenSymbol && macroArgToken.data[0] == MACRO_ARGS_START) {
        macro.type = MacroArgs;
        macroArgToken = GetTokenAndIgnore(lexer, TokenWhitespace);
        while (macroArgToken.type != TokenEnd && macroArgToken.type != TokenNewline) {
            if (macroArgToken.type != TokenText) {
                MacroReportError(lexer, "Invalid argument for macro \"" Token_Fmt "\"", Token_Arg(&macroNameToken));
                return false;
            }

            DynamicArrayAppend(&macro.key, macroArgToken);
            macroArgToken = GetTokenAndIgnore(lexer, TokenWhitespace);

            if (macroArgToken.type == TokenSymbol) {
                if (macroArgToken.data[0] == MACRO_ARGS_END) {
                    break;
                } else if (macroArgToken.data[0] != MACRO_ARGS_SEPARATOR) {
                    MacroReportError(lexer, "Invalid symbol in macro \"" Token_Fmt "\"", Token_Arg(&macroNameToken));
                    return false;
                }
            }
            macroArgToken = GetTokenAndIgnore(lexer, TokenWhitespace);
        }
        if (macroArgToken.type == TokenEnd || macroArgToken.type == TokenNewline) {
            MacroReportError(lexer, "Invalid macro \"" Token_Fmt "\"", Token_Arg(&macroNameToken));
            return false;
        }
    } else {
        macro.type = MacroValue;
        macroValueToken = macroArgToken;
    }

    if (!macroValueToken.data) macroValueToken = GetTokenAndIgnore(lexer, TokenWhitespace);
    while (macroValueToken.type != TokenEnd && macroValueToken.type != TokenNewline) {
        DynamicArrayAppend(&macro.value, macroValueToken);
        macroValueToken = GetToken(lexer);
    }
    DynamicArrayAppend(&DefinedMacros, macro);
    return true;
}

bool MacroCollectArgs(Lexer *lexer, const Macro *macro, Tokens *macroTokens) {
    LexerMark mark = GetMark(lexer);
    Token macroArgToken = GetToken(lexer);
    if (macroArgToken.type != TokenSymbol || macroArgToken.data[0] != MACRO_ARGS_START) {
        // the token only has the same name but it isnt an activation of the macro
        SetMark(lexer, mark);
        return true;
    }
    DynamicArrayAppend(macroTokens, macroArgToken);
    Token macroNameToken = macro->key.data[0]; // for error reporting
    int arg = 1;

    macroArgToken = GetToken(lexer);
    while (macroArgToken.type != TokenEnd && macroArgToken.type != TokenNewline) {
        if (macroArgToken.type == TokenSymbol && macroArgToken.data[0] == MACRO_ARGS_SEPARATOR) {
            if (arg == macro->key.count - 1) {
                MacroReportError(lexer, "Too many arguments to macro \"" Token_Fmt "\"", Token_Arg(&macroNameToken));
                return false;
            }
            DynamicArrayAppend(macroTokens, macroArgToken);
            arg++;
            goto notArg;
        } else if (macroArgToken.type == TokenSymbol && macroArgToken.data[0] == MACRO_ARGS_END) {
            if (arg < macro->key.count - 1) {
                MacroReportError(lexer, "Too few arguments to macro \"" Token_Fmt "\"", Token_Arg(&macroNameToken));
                return false;
            }
            DynamicArrayAppend(macroTokens, macroArgToken);
            break;
        } else if (macroArgToken.type == TokenText) {
            Macro *nestedMacro = FindMatchingMacro(macroArgToken);
            if (nestedMacro && nestedMacro->type == MacroArgs) {
                DynamicArrayAppend(macroTokens, macroArgToken);
                if (!MacroCollectArgs(lexer, nestedMacro, macroTokens)) return false;
                goto notArg;
            }
        }

        DynamicArrayAppend(macroTokens, macroArgToken);
    notArg:
        macroArgToken = GetToken(lexer);
    }
    if (macroArgToken.type == TokenEnd || macroArgToken.type == TokenNewline) {
        MacroReportError(lexer, "Unfinished use of macro \"" Token_Fmt "\"", Token_Arg(&macroNameToken));
        return false;
    }
    return true;
}

bool MacroExpand(Tokens *tokens, Tokens *expandedTokens) {
    DynamicArrayForeach(Token, token, tokens) {
        Macro *macro = FindMatchingMacro(*token);
        if (!macro) {
        notMacro:
            DynamicArrayAppend(expandedTokens, *token);
        } else if (macro->type == MacroValue) {
            MacroExpand(&macro->value, expandedTokens);
        } else if (macro->type == MacroArgs) {
            token++; // consume the macro names
            if (token >= (tokens)->data + (tokens)->count || token->type != TokenSymbol || token->data[0] != MACRO_ARGS_START) {
                token--;
                goto notMacro;
            }
            token++; // consume MACRO_ARGS_START
            DynamicArrayForeach(Token, macroValueToken, &macro->value) {
                int argIdx = FindMatchingArgToValue(macro, macroValueToken);
                if (argIdx > 0) {
                    Tokens argTokens = {
                        .printFunc = printToken,
                    };

                    for (int start_end = 1; token < (tokens)->data + (tokens)->count; token++) {
                        if (token->type == TokenSymbol) {
                            if (token->data[0] == MACRO_ARGS_START) {
                                start_end++;
                            } else if (token->data[0] == MACRO_ARGS_SEPARATOR && start_end == 1) {
                                token++;
                                break;
                            } else if (token->data[0] == MACRO_ARGS_END) {
                                start_end--;
                            }
                        }
                        if (start_end == 0) break;
                        DynamicArrayAppend(&argTokens, *token);
                    }
                    MacroExpand(&argTokens, expandedTokens);
                    DynamicArrayDestroy(&argTokens);
                } else {
                    DynamicArrayAppend(expandedTokens, *macroValueToken);
                }
            }
        }
    }
    return true;
}

bool MacroLang(Lexer *lexer, DynamicString *output_ds) {
    for (Token token = GetToken(lexer);
         token.type != TokenEnd;
         token = GetToken(lexer)) {
        if (token.type == TokenMacroKeyword) {
            MacroDefine(lexer);
        } else if (token.type == TokenText) {
            Tokens expandedTokens = {
                .printFunc = printToken,
            };
            Macro *macro = FindMatchingMacro(token);
            if (!macro) {
                DynamicArrayAppend(&expandedTokens, token);
            } else if (macro->type == MacroValue) {
                Tokens macroTokens = {
                    .printFunc = printToken,
                };
                DynamicArrayAppend(&macroTokens, token);
                if (!MacroExpand(&macroTokens, &expandedTokens)) return false;
                DynamicArrayDestroy(&macroTokens);
            } else if (macro->type == MacroArgs) {
                Tokens macroTokens = {
                    .printFunc = printToken,
                };
                DynamicArrayAppend(&macroTokens, token);
                if (!MacroCollectArgs(lexer, macro, &macroTokens)) return false;
                if (!MacroExpand(&macroTokens, &expandedTokens)) return false;
                DynamicArrayDestroy(&macroTokens);
            }
            DynamicArrayForeach(Token, expandedToken, &expandedTokens) {
                DynamicStringAppendf(output_ds, Token_Fmt, Token_Arg(expandedToken));
            }
            DynamicArrayDestroy(&expandedTokens);
        } else {
            DynamicStringAppendf(output_ds, Token_Fmt, Token_Arg(&token));
        }
    }

#ifdef DEBUG_PRINTING
    printf("\n----- MACROS -----\n");
    DynamicArrayPrint(&DefinedMacros);
    printf("\n----- MACROS -----\n");
#endif // DEBUG_PRINTING

    return true;
}

#define POP_ARG(arr, c) ((c)--, *(arr)++)

void usage(const char *program_name) {
    fprintf(stderr, "%s <input>\n", program_name);
}

int main(int argc, char **argv) {
#if 0
    const char *program_name = POP_ARG(argv, argc);

    if (argc <= 0) {
        usage(program_name);
        fprintf(stderr, "No Input Provided\n");
        return 1;
    }

    const char *input_file = POP_ARG(argv, argc);
#else
    (void) argc;
    (void) argv;
    // hardcoded for easy debugging in vscode
    const char *input_file = "C:\\Users\\idosw\\C_Projects\\macrolang\\testing.txt";
#endif

    DynamicString input_ds = {0};
    DynamicStringReadFile(&input_ds, input_file);

    DynamicStringRemoveAll(&input_ds, '\r'); // i love windows

#ifdef DEBUG_PRINTING
    printf("\n----- INPUT -----\n");
    DynamicStringPrint(&input_ds);
    printf("\n----- INPUT -----\n");
#endif // DEBUG_PRINTING

    Lexer lexer = {
        .data = input_ds.data,
        .data_len = input_ds.count,
    };
    DynamicString output_ds = {0};
    if (!MacroLang(&lexer, &output_ds)) return 1;

#ifdef DEBUG_PRINTING
    printf("\n----- OUTPUT -----\n");
    DynamicStringPrint(&output_ds);
    printf("\n----- OUTPUT -----\n");
#endif // DEBUG_PRINTING

    return 0;
}