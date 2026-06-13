#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SCOPE 64
#define MAX_VARS_PER_SCOPE 64

static char *strdup_str(const char *s);

typedef struct {
  char *names[MAX_VARS_PER_SCOPE];
  int count;
} Scope;

typedef struct {
  Scope scopes[MAX_SCOPE];
  int depth;
} SymTable;

static void sym_init(SymTable *st) { st->depth = 0; st->scopes[0].count = 0; }
static void sym_push(SymTable *st) { st->depth++; st->scopes[st->depth].count = 0; }
static void sym_pop(SymTable *st) { st->depth--; }

static int sym_lookup(SymTable *st, const char *name) {
  for (int d = st->depth; d >= 0; d--)
    for (int i = 0; i < st->scopes[d].count; i++)
      if (strcmp(st->scopes[d].names[i], name) == 0) return 1;
  return 0;
}

static int sym_define(SymTable *st, const char *name) {
  Scope *s = &st->scopes[st->depth];
  for (int i = 0; i < s->count; i++)
    if (strcmp(s->names[i], name) == 0) return 0;
  s->names[s->count++] = strdup_str(name);
  return 1;
}

typedef enum {
  TOKEN_EOF,
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_LBRACKET,
  TOKEN_RBRACKET,
  TOKEN_SEMICOLON,
  TOKEN_COMMA,
  TOKEN_COLON,
  TOKEN_DOT,
  TOKEN_EQUAL,
  TOKEN_LESS,
  TOKEN_GREATER,
  TOKEN_BANG,
  TOKEN_PIPE,
  TOKEN_AMPERSAND,
  TOKEN_ARROW,
  TOKEN_FAT_ARROW,
  TOKEN_EQUAL_EQUAL,
  TOKEN_BANG_EQUAL,
  TOKEN_LESS_EQUAL,
  TOKEN_GREATER_EQUAL,
  TOKEN_LET,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_FN,
  TOKEN_INT,
  TOKEN_STR,
  TOKEN_ERROR,
} TokenType;

typedef struct {
  TokenType type;
  const char *start;
  int length;
  int line;
  int column;
} Token;

typedef struct {
  const char *start;
  const char *current;
  int line;
  int column;
} Lexer;

static const char *token_name(TokenType t) {
  switch (t) {
    case TOKEN_EOF:            return "EOF";
    case TOKEN_NUMBER:         return "NUMBER";
    case TOKEN_IDENTIFIER:     return "IDENTIFIER";
    case TOKEN_STRING:         return "STRING";
    case TOKEN_PLUS:           return "PLUS";
    case TOKEN_MINUS:          return "MINUS";
    case TOKEN_STAR:           return "STAR";
    case TOKEN_SLASH:          return "SLASH";
    case TOKEN_LPAREN:         return "LPAREN";
    case TOKEN_RPAREN:         return "RPAREN";
    case TOKEN_LBRACE:         return "LBRACE";
    case TOKEN_RBRACE:         return "RBRACE";
    case TOKEN_LBRACKET:       return "LBRACKET";
    case TOKEN_RBRACKET:       return "RBRACKET";
    case TOKEN_SEMICOLON:      return "SEMICOLON";
    case TOKEN_COMMA:          return "COMMA";
    case TOKEN_COLON:          return "COLON";
    case TOKEN_DOT:            return "DOT";
    case TOKEN_EQUAL:          return "EQUAL";
    case TOKEN_LESS:           return "LESS";
    case TOKEN_GREATER:        return "GREATER";
    case TOKEN_BANG:           return "BANG";
    case TOKEN_PIPE:           return "PIPE";
    case TOKEN_AMPERSAND:      return "AMPERSAND";
    case TOKEN_ARROW:          return "ARROW";
    case TOKEN_FAT_ARROW:      return "FAT_ARROW";
    case TOKEN_EQUAL_EQUAL:    return "EQUAL_EQUAL";
    case TOKEN_BANG_EQUAL:     return "BANG_EQUAL";
    case TOKEN_LESS_EQUAL:     return "LESS_EQUAL";
    case TOKEN_GREATER_EQUAL:  return "GREATER_EQUAL";
    case TOKEN_LET:            return "LET";
    case TOKEN_IF:             return "IF";
    case TOKEN_ELSE:           return "ELSE";
    case TOKEN_WHILE:          return "WHILE";
    case TOKEN_FN:             return "FN";
    case TOKEN_INT:            return "INT";
    case TOKEN_STR:            return "STR";
    case TOKEN_ERROR:          return "ERROR";
  }
  return "?";
}

static void lexer_init(Lexer *l, const char *src) {
  l->start = src;
  l->current = src;
  l->line = 1;
  l->column = 1;
}

static char lexer_peek(Lexer *l) {
  return *l->current;
}

static char lexer_advance(Lexer *l) {
  char c = *l->current++;
  if (c == '\n') { l->line++; l->column = 1; }
  else { l->column++; }
  return c;
}

static int lexer_match(Lexer *l, char expected) {
  if (lexer_peek(l) != expected) return 0;
  lexer_advance(l);
  return 1;
}

static void lexer_skip_whitespace(Lexer *l) {
  for (;;) {
    char c = lexer_peek(l);
    switch (c) {
      case ' ': case '\t': case '\n': case '\r':
        lexer_advance(l);
        break;
      case '/':
        if (l->current[1] == '/') {
          while (lexer_peek(l) != '\n' && lexer_peek(l) != '\0')
            lexer_advance(l);
        } else if (l->current[1] == '*') {
          lexer_advance(l); lexer_advance(l);
          while (!(lexer_peek(l) == '*' && l->current[1] == '/')) {
            if (lexer_peek(l) == '\0') return;
            lexer_advance(l);
          }
          lexer_advance(l); lexer_advance(l);
        } else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

static Token lexer_make_token(Lexer *l, TokenType type) {
  Token t;
  t.type = type;
  t.start = l->start;
  t.length = (int)(l->current - l->start);
  t.line = l->line;
  t.column = l->column;
  return t;
}

static Token lexer_error_token(Lexer *l, const char *msg) {
  Token t;
  t.type = TOKEN_ERROR;
  t.start = msg;
  t.length = (int)strlen(msg);
  t.line = l->line;
  t.column = l->column;
  return t;
}

static Token lexer_number(Lexer *l) {
  while (isdigit(lexer_peek(l))) lexer_advance(l);
  return lexer_make_token(l, TOKEN_NUMBER);
}

static TokenType lexer_keyword(const char *start, int len) {
  if (len == 3 && memcmp(start, "let", 3) == 0) return TOKEN_LET;
  if (len == 2 && memcmp(start, "if", 2) == 0) return TOKEN_IF;
  if (len == 4 && memcmp(start, "else", 4) == 0) return TOKEN_ELSE;
  if (len == 5 && memcmp(start, "while", 5) == 0) return TOKEN_WHILE;
  if (len == 2 && memcmp(start, "fn", 2) == 0) return TOKEN_FN;
  if (len == 3 && memcmp(start, "int", 3) == 0) return TOKEN_INT;
  if (len == 3 && memcmp(start, "str", 3) == 0) return TOKEN_STR;
  return TOKEN_IDENTIFIER;
}

static Token lexer_identifier(Lexer *l) {
  while (isalnum(lexer_peek(l)) || lexer_peek(l) == '_')
    lexer_advance(l);
  TokenType type = lexer_keyword(l->start, (int)(l->current - l->start));
  return lexer_make_token(l, type);
}

static Token lexer_string(Lexer *l) {
  while (lexer_peek(l) != '"' && lexer_peek(l) != '\0') {
    if (lexer_peek(l) == '\\') lexer_advance(l);
    lexer_advance(l);
  }
  if (lexer_peek(l) == '\0')
    return lexer_error_token(l, "Unterminated string");
  lexer_advance(l);
  return lexer_make_token(l, TOKEN_STRING);
}

static Token lexer_next(Lexer *l) {
  lexer_skip_whitespace(l);
  l->start = l->current;
  char c = lexer_peek(l);
  if (c == '\0') return lexer_make_token(l, TOKEN_EOF);
  if (isdigit(c)) return lexer_number(l);
  if (isalpha(c) || c == '_') return lexer_identifier(l);
  switch (lexer_advance(l)) {
    case '+': return lexer_make_token(l, TOKEN_PLUS);
    case '-':
      if (lexer_match(l, '>')) return lexer_make_token(l, TOKEN_ARROW);
      return lexer_make_token(l, TOKEN_MINUS);
    case '*': return lexer_make_token(l, TOKEN_STAR);
    case '/': return lexer_make_token(l, TOKEN_SLASH);
    case '(': return lexer_make_token(l, TOKEN_LPAREN);
    case ')': return lexer_make_token(l, TOKEN_RPAREN);
    case '{': return lexer_make_token(l, TOKEN_LBRACE);
    case '}': return lexer_make_token(l, TOKEN_RBRACE);
    case '[': return lexer_make_token(l, TOKEN_LBRACKET);
    case ']': return lexer_make_token(l, TOKEN_RBRACKET);
    case ';': return lexer_make_token(l, TOKEN_SEMICOLON);
    case ',': return lexer_make_token(l, TOKEN_COMMA);
    case ':': return lexer_make_token(l, TOKEN_COLON);
    case '.': return lexer_make_token(l, TOKEN_DOT);
    case '=':
      if (lexer_match(l, '=')) return lexer_make_token(l, TOKEN_EQUAL_EQUAL);
      return lexer_make_token(l, TOKEN_EQUAL);
    case '<':
      if (lexer_match(l, '=')) return lexer_make_token(l, TOKEN_LESS_EQUAL);
      return lexer_make_token(l, TOKEN_LESS);
    case '>':
      if (lexer_match(l, '=')) return lexer_make_token(l, TOKEN_GREATER_EQUAL);
      return lexer_make_token(l, TOKEN_GREATER);
    case '!':
      if (lexer_match(l, '=')) return lexer_make_token(l, TOKEN_BANG_EQUAL);
      return lexer_make_token(l, TOKEN_BANG);
    case '|':
      if (lexer_match(l, '=')) {
        if (lexer_match(l, '>')) return lexer_make_token(l, TOKEN_FAT_ARROW);
      }
      return lexer_make_token(l, TOKEN_PIPE);
    case '&': return lexer_make_token(l, TOKEN_AMPERSAND);
    case '"': return lexer_string(l);
    default: {
      char msg[2] = {c, '\0'};
      return lexer_error_token(l, msg);
    }
  }
}

static void token_print(Token t) {
  printf("%d:%d: %s", t.line, t.column, token_name(t.type));
  if (t.type == TOKEN_NUMBER || t.type == TOKEN_IDENTIFIER ||
      t.type == TOKEN_STRING || t.type == TOKEN_ERROR) {
    printf(" %.*s", t.length, t.start);
  }
  printf("\n");
}

typedef enum {
  TYPE_VOID,
  TYPE_INT,
  TYPE_STR,
  TYPE_ARRAY,
} TypeKind;

static const char *type_name(TypeKind t) {
  switch (t) {
    case TYPE_INT:   return "int";
    case TYPE_STR:   return "str";
    case TYPE_ARRAY: return "array";
    default:         return "void";
  }
}

typedef enum {
  NODE_PROGRAM,
  NODE_LET,
  NODE_VARIABLE,
  NODE_NUMBER,
  NODE_UNARY,
  NODE_BINARY,
  NODE_IF,
  NODE_WHILE,
  NODE_BLOCK,
  NODE_FN,
  NODE_CALL,
  NODE_STRING,
  NODE_ARRAY,
  NODE_INDEX,
} NodeType;

typedef struct Node {
  NodeType type;
  struct Node *next;
  union {
    int number;
    char *string;
    struct { char *name; TypeKind type; TypeKind elem_type; struct Node *value; } let;
    struct { char *name; TypeKind type; TypeKind elem_type; } variable;
    struct { struct Node *stmts; } program;
    struct { struct Node *stmts; } block;
    struct { struct Node *cond; struct Node *then_block; struct Node *else_block; } iff;
    struct { struct Node *cond; struct Node *body; } whilee;
    struct { char *name; TypeKind ret_type; TypeKind ret_elem_type; struct Node *params; struct Node *body; } fn;
    struct { char *name; struct Node *args; } call;
    struct { TypeKind elem_type; struct Node *elements; } array;
    struct { struct Node *array; struct Node *index; TypeKind elem_type; } indexx;
    struct { TokenType op; struct Node *right; } unary;
    struct { TokenType op; TypeKind type; struct Node *left; struct Node *right; } binary;
  } as;
} Node;

static Node *node_new(NodeType type) {
  Node *n = calloc(1, sizeof(Node));
  if (!n) { fprintf(stderr, "Out of memory\n"); exit(1); }
  n->type = type;
  return n;
}

static int temp_count = 0;

static const char *op_to_c(TokenType t) {
  switch (t) {
    case TOKEN_PLUS: return "+";
    case TOKEN_MINUS: return "-";
    case TOKEN_STAR: return "*";
    case TOKEN_SLASH: return "/";
    case TOKEN_EQUAL_EQUAL: return "==";
    case TOKEN_BANG_EQUAL: return "!=";
    case TOKEN_LESS: return "<";
    case TOKEN_GREATER: return ">";
    case TOKEN_LESS_EQUAL: return "<=";
    case TOKEN_GREATER_EQUAL: return ">=";
    default: return "?";
  }
}

static void codegen_expr(FILE *out, Node *n);
static void codegen_block_stmts(FILE *out, Node *n);
static void codegen_block_all_stmts(FILE *out, Node *n);
static void codegen_block_expr(FILE *out, Node *n, int zero_if_empty);

static void codegen_expr(FILE *out, Node *n) {
  switch (n->type) {
    case NODE_NUMBER:
      fprintf(out, "%d", n->as.number);
      break;
    case NODE_VARIABLE:
      fprintf(out, "%s", n->as.variable.name);
      break;
    case NODE_UNARY:
      fprintf(out, "-");
      codegen_expr(out, n->as.unary.right);
      break;
    case NODE_BINARY:
      if (n->as.binary.type == TYPE_STR) {
        int t = temp_count++;
        fprintf(out, "({char _s%d[256]; strcpy(_s%d, ", t, t);
        codegen_expr(out, n->as.binary.left);
        fprintf(out, "); strcat(_s%d, ", t);
        codegen_expr(out, n->as.binary.right);
        fprintf(out, "); _s%d; })", t);
      } else {
        fprintf(out, "(");
        codegen_expr(out, n->as.binary.left);
        fprintf(out, " %s ", op_to_c(n->as.binary.op));
        codegen_expr(out, n->as.binary.right);
        fprintf(out, ")");
      }
      break;
    case NODE_IF: {
      int id = temp_count++;
      fprintf(out, "({int _t%d; if (", id);
      codegen_expr(out, n->as.iff.cond);
      fprintf(out, ") {");
      codegen_block_stmts(out, n->as.iff.then_block);
      fprintf(out, " _t%d = ", id);
      codegen_block_expr(out, n->as.iff.then_block, 0);
      fprintf(out, "; } else {");
      codegen_block_stmts(out, n->as.iff.else_block);
      fprintf(out, " _t%d = ", id);
      codegen_block_expr(out, n->as.iff.else_block, 0);
      fprintf(out, "; } _t%d; })", id);
      break;
    }
    case NODE_BLOCK: {
      int id = temp_count++;
      fprintf(out, "({int _t%d; ", id);
      Node *s;
      for (s = n->as.block.stmts; s && s->next; s = s->next) {
        if (s->type == NODE_LET) {
          fprintf(out, "int %s = ", s->as.let.name);
          codegen_expr(out, s->as.let.value);
          fprintf(out, "; ");
        } else {
          codegen_expr(out, s);
          fprintf(out, "; ");
        }
      }
      if (s) {
        if (s->type == NODE_LET) {
          fprintf(out, "int %s = ", s->as.let.name);
          codegen_expr(out, s->as.let.value);
          fprintf(out, "; _t%d = ", id);
          codegen_expr(out, s->as.let.value);
        } else {
          fprintf(out, "_t%d = ", id);
          codegen_expr(out, s);
        }
        fprintf(out, ";");
      } else {
        fprintf(out, "_t%d = 0;", id);
      }
      fprintf(out, " _t%d; })", id);
      break;
    }
    case NODE_STRING:
      fprintf(out, "\"%s\"", n->as.string);
      break;
    case NODE_ARRAY: {
      fprintf(out, "{");
      for (Node *e = n->as.array.elements; e; e = e->next) {
        if (e != n->as.array.elements) fprintf(out, ", ");
        codegen_expr(out, e);
      }
      fprintf(out, "}");
      break;
    }
    case NODE_INDEX:
      fprintf(out, "(");
      codegen_expr(out, n->as.indexx.array);
      fprintf(out, ")[");
      codegen_expr(out, n->as.indexx.index);
      fprintf(out, "]");
      break;
    case NODE_CALL:
      fprintf(out, "%s(", n->as.call.name);
      for (Node *a = n->as.call.args; a; a = a->next) {
        if (a != n->as.call.args) fprintf(out, ", ");
        codegen_expr(out, a);
      }
      fprintf(out, ")");
      break;
    case NODE_WHILE:
      fprintf(out, "({while (");
      codegen_expr(out, n->as.whilee.cond);
      fprintf(out, ") {");
      codegen_block_all_stmts(out, n->as.whilee.body);
      fprintf(out, " } 0; })");
      break;
    default:
      break;
  }
}

static void codegen_block_stmts(FILE *out, Node *n) {
  for (Node *s = n->as.block.stmts; s; s = s->next) {
    if (s->type == NODE_LET) {
      fprintf(out, " int %s = ", s->as.let.name);
      codegen_expr(out, s->as.let.value);
      fprintf(out, ";");
    }
  }
}

static void codegen_block_all_stmts(FILE *out, Node *n) {
  for (Node *s = n->as.block.stmts; s; s = s->next) {
    if (s->type == NODE_LET) {
      fprintf(out, " int %s = ", s->as.let.name);
      codegen_expr(out, s->as.let.value);
      fprintf(out, ";");
    } else {
      fprintf(out, " ");
      codegen_expr(out, s);
      fprintf(out, ";");
    }
  }
}

static void codegen_block_expr(FILE *out, Node *n, int zero_if_empty) {
  Node *s = n->as.block.stmts;
  if (!s) { if (zero_if_empty) fprintf(out, "0"); return; }
  while (s->next) s = s->next;
  if (s->type == NODE_LET)
    codegen_expr(out, s->as.let.value);
  else
    codegen_expr(out, s);
}

static void codegen_stmt(FILE *out, Node *n) {
  switch (n->type) {
    case NODE_LET:
      if (n->as.let.type == TYPE_ARRAY) {
        Node *val = n->as.let.value;
        int count = 0;
        if (val && val->type == NODE_ARRAY)
          for (Node *e = val->as.array.elements; e; e = e->next) count++;
        const char *ct = n->as.let.elem_type == TYPE_STR ? "char*" : "int";
        fprintf(out, "    %s %s[%d] = ", ct, n->as.let.name, count);
        codegen_expr(out, n->as.let.value);
        fprintf(out, ";\n");
      } else {
        fprintf(out, "    %s %s = ", n->as.let.type == TYPE_STR ? "char*" : "int", n->as.let.name);
        codegen_expr(out, n->as.let.value);
        fprintf(out, ";\n");
      }
      break;
    case NODE_IF:
      codegen_expr(out, n);
      fprintf(out, ";\n");
      break;
    case NODE_WHILE:
      fprintf(out, "    while (");
      codegen_expr(out, n->as.whilee.cond);
      fprintf(out, ") {");
      codegen_block_all_stmts(out, n->as.whilee.body);
      fprintf(out, " }\n");
      break;
    case NODE_CALL:
      fprintf(out, "    ");
      codegen_expr(out, n);
      fprintf(out, ";\n");
      break;
    case NODE_FN:
      fprintf(out, "%s %s(",
              n->as.fn.ret_type == TYPE_STR ? "char*" :
              n->as.fn.ret_type == TYPE_ARRAY ? "int*" : "int",
              n->as.fn.name);
      Node *p;
      for (p = n->as.fn.params; p; p = p->next) {
        if (p != n->as.fn.params) fprintf(out, ", ");
        const char *ct = p->as.variable.type == TYPE_STR ? "char*" :
                         p->as.variable.type == TYPE_ARRAY ? "int" : "int";
        fprintf(out, "%s %s", ct, p->as.variable.name);
        if (p->as.variable.type == TYPE_ARRAY) fprintf(out, "[]");
      }
      fprintf(out, ") {\n");
      codegen_block_stmts(out, n->as.fn.body);
      fprintf(out, "    return ");
      codegen_block_expr(out, n->as.fn.body, 1);
      fprintf(out, ";\n}\n");
      break;
    default:
      fprintf(out, "    ");
      codegen_expr(out, n);
      fprintf(out, ";\n");
      break;
  }
}

static void codegen(FILE *out, Node *n) {
  if (n->type == NODE_PROGRAM) {
    fprintf(out, "#include <string.h>\n");
    for (Node *s = n->as.program.stmts; s; s = s->next)
      if (s->type == NODE_FN) codegen_stmt(out, s);
    fprintf(out, "int main(void) {\n");
    Node *last = NULL;
    for (Node *s = n->as.program.stmts; s; s = s->next) {
      last = s;
      if (s->type == NODE_LET) {
        codegen_stmt(out, s);
      } else if (s->type != NODE_FN && s->next) {
        codegen_stmt(out, s);
      }
    }
    fprintf(out, "    return ");
    if (last && last->type != NODE_LET && last->type != NODE_FN)
      codegen_expr(out, last);
    else
      fprintf(out, "0");
    fprintf(out, ";\n}\n");
  }
}

static void node_print(FILE *out, Node *n) {
  if (!n) { fprintf(out, "()"); return; }
  switch (n->type) {
    case NODE_PROGRAM:
      fprintf(out, "(PROGRAM");
      for (Node *s = n->as.program.stmts; s; s = s->next) {
        fprintf(out, " ");
        node_print(out, s);
      }
      fprintf(out, ")");
      break;
    case NODE_BLOCK:
      fprintf(out, "(BLOCK");
      for (Node *s = n->as.block.stmts; s; s = s->next) {
        fprintf(out, " ");
        node_print(out, s);
      }
      fprintf(out, ")");
      break;
    case NODE_WHILE:
      fprintf(out, "(WHILE ");
      node_print(out, n->as.whilee.cond);
      fprintf(out, " ");
      node_print(out, n->as.whilee.body);
      fprintf(out, ")");
      break;
    case NODE_IF:
      fprintf(out, "(IF ");
      node_print(out, n->as.iff.cond);
      fprintf(out, " ");
      node_print(out, n->as.iff.then_block);
      if (n->as.iff.else_block) {
        fprintf(out, " ");
        node_print(out, n->as.iff.else_block);
      }
      fprintf(out, ")");
      break;
    case NODE_CALL:
      fprintf(out, "(CALL %s", n->as.call.name);
      for (Node *a = n->as.call.args; a; a = a->next) {
        fprintf(out, " ");
        node_print(out, a);
      }
      fprintf(out, ")");
      break;
    case NODE_FN:
      fprintf(out, "(FN %s", n->as.fn.name);
      for (Node *p = n->as.fn.params; p; p = p->next) {
        fprintf(out, " (PARAM %s", p->as.variable.name);
        if (p->as.variable.type == TYPE_ARRAY)
          fprintf(out, " :[%s]", type_name(p->as.variable.elem_type));
        else if (p->as.variable.type)
          fprintf(out, " :%s", type_name(p->as.variable.type));
        fprintf(out, ")");
      }
      if (n->as.fn.ret_type == TYPE_ARRAY)
        fprintf(out, " ->[%s]", type_name(n->as.fn.ret_elem_type));
      else if (n->as.fn.ret_type)
        fprintf(out, " ->%s", type_name(n->as.fn.ret_type));
      fprintf(out, " ");
      node_print(out, n->as.fn.body);
      fprintf(out, ")");
      break;
    case NODE_LET:
      fprintf(out, "(LET %s", n->as.let.name);
      if (n->as.let.type == TYPE_ARRAY)
        fprintf(out, " :[%s]", type_name(n->as.let.elem_type));
      else if (n->as.let.type)
        fprintf(out, " :%s", type_name(n->as.let.type));
      fprintf(out, " ");
      node_print(out, n->as.let.value);
      fprintf(out, ")");
      break;
    case NODE_VARIABLE:
      fprintf(out, "(VAR %s", n->as.variable.name);
      if (n->as.variable.type == TYPE_ARRAY)
        fprintf(out, " :[%s]", type_name(n->as.variable.elem_type));
      else if (n->as.variable.type)
        fprintf(out, " :%s", type_name(n->as.variable.type));
      fprintf(out, ")");
      break;
    case NODE_ARRAY:
      fprintf(out, "([");
      for (Node *e = n->as.array.elements; e; e = e->next) {
        if (e != n->as.array.elements) fprintf(out, " ");
        node_print(out, e);
      }
      fprintf(out, "])");
      break;
    case NODE_INDEX:
      fprintf(out, "(INDEX ");
      node_print(out, n->as.indexx.array);
      fprintf(out, " ");
      node_print(out, n->as.indexx.index);
      fprintf(out, ")");
      break;
    case NODE_NUMBER:
      fprintf(out, "%d", n->as.number);
      break;
    case NODE_STRING:
      fprintf(out, "\"%s\"", n->as.string);
      break;
    case NODE_UNARY:
      fprintf(out, "(%s ", token_name(n->as.unary.op));
      node_print(out, n->as.unary.right);
      fprintf(out, ")");
      break;
    case NODE_BINARY:
      fprintf(out, "(%s ", token_name(n->as.binary.op));
      node_print(out, n->as.binary.left);
      fprintf(out, " ");
      node_print(out, n->as.binary.right);
      fprintf(out, ")");
      break;
  }
}

typedef struct {
  Lexer *lexer;
  Token current;
  Token previous;
  int panic;
  SymTable sym;
} Parser;

static void parser_init(Parser *p, Lexer *l) {
  p->lexer = l;
  p->panic = 0;
  p->current = lexer_next(l);
  sym_init(&p->sym);
}

static void parser_advance(Parser *p) {
  p->previous = p->current;
  p->current = lexer_next(p->lexer);
}

static int parser_check(Parser *p, TokenType type) {
  return p->current.type == type;
}

static int parser_match(Parser *p, TokenType type) {
  if (!parser_check(p, type)) return 0;
  parser_advance(p);
  return 1;
}

static void parser_error(Parser *p, const char *msg) {
  fprintf(stderr, "error:%d:%d: %s\n", p->previous.line, p->previous.column, msg);
  p->panic = 1;
}

static char *strdup_str(const char *s) {
  size_t len = strlen(s);
  char *d = malloc(len + 1);
  if (d) memcpy(d, s, len + 1);
  return d;
}

static char *strdup_token(const Token *t) {
  char *s = malloc((size_t)t->length + 1);
  memcpy(s, t->start, (size_t)t->length);
  s[t->length] = '\0';
  return s;
}

static Node *parse_expr(Parser *p);
static Node *parse_comparison(Parser *p);
static Node *parse_term(Parser *p);
static Node *parse_factor(Parser *p);
static Node *parse_statement(Parser *p);
static Node *parse_block(Parser *p);

static Node *parse_program(Parser *p) {
  Node *head = NULL, *tail = NULL;
  while (!parser_check(p, TOKEN_EOF)) {
    Node *stmt = parse_statement(p);
    if (!stmt) break;
    if (p->panic) return node_new(NODE_PROGRAM);
    if (!head) head = stmt;
    else tail->next = stmt;
    tail = stmt;
  }
  Node *prog = node_new(NODE_PROGRAM);
  prog->as.program.stmts = head;
  return prog;
}

static TypeKind parse_type(Parser *p, TypeKind *elem_type) {
  if (parser_match(p, TOKEN_INT)) return TYPE_INT;
  if (parser_match(p, TOKEN_STR)) return TYPE_STR;
  if (parser_match(p, TOKEN_LBRACKET)) {
    TypeKind et = parse_type(p, NULL);
    if (!parser_match(p, TOKEN_RBRACKET))
      parser_error(p, "Expected ']'");
    if (elem_type) *elem_type = et;
    return TYPE_ARRAY;
  }
  parser_error(p, "Expected type (int, str, or [type])");
  return TYPE_VOID;
}

static Node *parse_fn(Parser *p);

static Node *parse_statement(Parser *p) {
  if (parser_match(p, TOKEN_FN)) {
    Node *n = parse_fn(p);
    if (n) parser_match(p, TOKEN_SEMICOLON);
    return n;
  }
  if (parser_match(p, TOKEN_LET)) {
    if (!parser_check(p, TOKEN_IDENTIFIER)) {
      parser_error(p, "Expected variable name");
      return NULL;
    }
    Token name_tok = p->current;
    char *name = strdup_token(&p->current);
    parser_advance(p);
    if (!sym_define(&p->sym, name)) {
      fprintf(stderr, "error:%d:%d: Duplicate variable '%s'\n",
              name_tok.line, name_tok.column, name);
      p->panic = 1;
      free(name);
      return NULL;
    }
    Node *n = node_new(NODE_LET);
    n->as.let.name = name;
    n->as.let.type = TYPE_INT;
    n->as.let.elem_type = TYPE_VOID;
    if (parser_match(p, TOKEN_COLON))
      n->as.let.type = parse_type(p, &n->as.let.elem_type);
    if (!parser_match(p, TOKEN_EQUAL)) {
      parser_error(p, "Expected '='");
      return NULL;
    }
    n->as.let.value = parse_expr(p);
    if (!parser_match(p, TOKEN_SEMICOLON))
      parser_error(p, "Expected ';'");
    return n;
  }
  Node *expr = parse_expr(p);
  if (expr && expr->type != NODE_IF && expr->type != NODE_WHILE && expr->type != NODE_BLOCK)
    parser_match(p, TOKEN_SEMICOLON);
  return expr;
}

static Node *parse_fn(Parser *p) {
  if (!parser_check(p, TOKEN_IDENTIFIER)) {
    parser_error(p, "Expected function name");
    return NULL;
  }
  char *name = strdup_token(&p->current);
  parser_advance(p);
  if (!sym_define(&p->sym, name)) {
    fprintf(stderr, "error:%d:%d: Duplicate function '%s'\n",
            p->previous.line, p->previous.column, name);
    p->panic = 1;
    free(name);
    return NULL;
  }
  if (!parser_match(p, TOKEN_LPAREN)) {
    parser_error(p, "Expected '('");
    return NULL;
  }
  sym_push(&p->sym);
  Node *params_head = NULL, *params_tail = NULL;
  if (!parser_check(p, TOKEN_RPAREN)) {
    do {
      if (!parser_check(p, TOKEN_IDENTIFIER)) {
        parser_error(p, "Expected parameter name");
        return NULL;
      }
      char *pname = strdup_token(&p->current);
      parser_advance(p);
      if (!sym_define(&p->sym, pname)) {
        fprintf(stderr, "error:%d:%d: Duplicate parameter '%s'\n",
                p->previous.line, p->previous.column, pname);
        p->panic = 1;
        free(pname);
        return NULL;
      }
      TypeKind pt = TYPE_INT;
      TypeKind pet = TYPE_VOID;
      if (parser_match(p, TOKEN_COLON))
        pt = parse_type(p, &pet);
      Node *pn = node_new(NODE_VARIABLE);
      pn->as.variable.name = pname;
      pn->as.variable.type = pt;
      pn->as.variable.elem_type = pet;
      if (!params_head) params_head = pn;
      else params_tail->next = pn;
      params_tail = pn;
    } while (parser_match(p, TOKEN_COMMA));
  }
  if (!parser_match(p, TOKEN_RPAREN)) {
    parser_error(p, "Expected ')'");
    sym_pop(&p->sym);
    return NULL;
  }
  TypeKind ret_type = TYPE_INT;
  TypeKind ret_elem_type = TYPE_VOID;
  if (parser_match(p, TOKEN_ARROW))
    ret_type = parse_type(p, &ret_elem_type);
  Node *body = parse_block(p);
  sym_pop(&p->sym);
  Node *n = node_new(NODE_FN);
  n->as.fn.name = name;
  n->as.fn.ret_type = ret_type;
  n->as.fn.ret_elem_type = ret_elem_type;
  n->as.fn.params = params_head;
  n->as.fn.body = body;
  return n;
}

static Node *parse_expr(Parser *p) {
  Node *left = parse_comparison(p);
  while (parser_match(p, TOKEN_PLUS) || parser_match(p, TOKEN_MINUS)) {
    TokenType op = p->previous.type;
    Node *right = parse_comparison(p);
    Node *n = node_new(NODE_BINARY);
    n->as.binary.op = op;
    n->as.binary.left = left;
    n->as.binary.right = right;
    left = n;
  }
  return left;
}

static Node *parse_comparison(Parser *p) {
  Node *left = parse_term(p);
  while (parser_match(p, TOKEN_EQUAL_EQUAL) || parser_match(p, TOKEN_BANG_EQUAL) ||
         parser_match(p, TOKEN_LESS) || parser_match(p, TOKEN_GREATER) ||
         parser_match(p, TOKEN_LESS_EQUAL) || parser_match(p, TOKEN_GREATER_EQUAL)) {
    TokenType op = p->previous.type;
    Node *right = parse_term(p);
    Node *n = node_new(NODE_BINARY);
    n->as.binary.op = op;
    n->as.binary.left = left;
    n->as.binary.right = right;
    left = n;
  }
  return left;
}

static Node *parse_index(Parser *p);

static Node *parse_term(Parser *p) {
  Node *left = parse_index(p);
  while (parser_match(p, TOKEN_STAR) || parser_match(p, TOKEN_SLASH)) {
    TokenType op = p->previous.type;
    Node *right = parse_index(p);
    Node *n = node_new(NODE_BINARY);
    n->as.binary.op = op;
    n->as.binary.left = left;
    n->as.binary.right = right;
    left = n;
  }
  return left;
}

static Node *parse_index(Parser *p) {
  Node *n = parse_factor(p);
  while (parser_match(p, TOKEN_LBRACKET)) {
    Node *idx = parse_expr(p);
    if (!parser_match(p, TOKEN_RBRACKET))
      parser_error(p, "Expected ']'");
    Node *in = node_new(NODE_INDEX);
    in->as.indexx.array = n;
    in->as.indexx.index = idx;
    n = in;
  }
  return n;
}

static Node *parse_block_body(Parser *p);

static Node *parse_factor(Parser *p) {
  if (parser_match(p, TOKEN_LBRACKET)) {
    Node *head = NULL, *tail = NULL;
    if (!parser_check(p, TOKEN_RBRACKET)) {
      do {
        Node *elem = parse_expr(p);
        if (!elem || p->panic) break;
        if (!head) head = elem;
        else tail->next = elem;
        tail = elem;
      } while (parser_match(p, TOKEN_COMMA));
    }
    if (!parser_match(p, TOKEN_RBRACKET))
      parser_error(p, "Expected ']'");
    Node *n = node_new(NODE_ARRAY);
    n->as.array.elements = head;
    return n;
  }
  if (parser_match(p, TOKEN_LBRACE))
    return parse_block_body(p);
  if (parser_match(p, TOKEN_LBRACE))
    return parse_block_body(p);
  if (parser_match(p, TOKEN_WHILE)) {
    Node *cond = parse_expr(p);
    Node *body = parse_block(p);
    Node *n = node_new(NODE_WHILE);
    n->as.whilee.cond = cond;
    n->as.whilee.body = body;
    return n;
  }
  if (parser_match(p, TOKEN_IF)) {
    Node *cond = parse_expr(p);
    Node *then_block = parse_block(p);
    Node *n = node_new(NODE_IF);
    n->as.iff.cond = cond;
    n->as.iff.then_block = then_block;
    if (parser_match(p, TOKEN_ELSE))
      n->as.iff.else_block = parse_block(p);
    else
      n->as.iff.else_block = NULL;
    return n;
  }
  if (parser_match(p, TOKEN_MINUS)) {
    Node *n = node_new(NODE_UNARY);
    n->as.unary.op = TOKEN_MINUS;
    n->as.unary.right = parse_index(p);
    return n;
  }
  if (parser_match(p, TOKEN_LPAREN)) {
    Node *expr = parse_expr(p);
    if (!parser_match(p, TOKEN_RPAREN))
      parser_error(p, "Expected ')'");
    return expr;
  }
  if (parser_match(p, TOKEN_NUMBER)) {
    Node *n = node_new(NODE_NUMBER);
    n->as.number = atoi(p->previous.start);
    return n;
  }
  if (parser_match(p, TOKEN_STRING)) {
    Node *n = node_new(NODE_STRING);
    int len = p->previous.length - 2;
    n->as.string = malloc(len + 1);
    memcpy(n->as.string, p->previous.start + 1, len);
    n->as.string[len] = '\0';
    return n;
  }
  if (parser_match(p, TOKEN_IDENTIFIER)) {
    char *name = strdup_token(&p->previous);
    if (parser_match(p, TOKEN_LPAREN)) {
      Node *args_head = NULL, *args_tail = NULL;
      if (!parser_check(p, TOKEN_RPAREN)) {
        do {
          Node *arg = parse_expr(p);
          if (!arg || p->panic) break;
          if (!args_head) args_head = arg;
          else args_tail->next = arg;
          args_tail = arg;
        } while (parser_match(p, TOKEN_COMMA));
      }
      if (!parser_match(p, TOKEN_RPAREN))
        parser_error(p, "Expected ')'");
      Node *n = node_new(NODE_CALL);
      n->as.call.name = name;
      n->as.call.args = args_head;
      return n;
    }
    if (!sym_lookup(&p->sym, name)) {
      fprintf(stderr, "error:%d:%d: Undefined variable '%s'\n",
              p->previous.line, p->previous.column, name);
      p->panic = 1;
      free(name);
      return NULL;
    }
    Node *n = node_new(NODE_VARIABLE);
    n->as.variable.name = name;
    return n;
  }
  parser_error(p, "Expected expression");
  return NULL;
}

static Node *parse_block_body(Parser *p) {
  Node *n = node_new(NODE_BLOCK);
  sym_push(&p->sym);
  Node *head = NULL, *tail = NULL;
  while (!parser_check(p, TOKEN_RBRACE) && !parser_check(p, TOKEN_EOF)) {
    Node *stmt = parse_statement(p);
    if (p->panic) { sym_pop(&p->sym); n->as.block.stmts = head; return n; }
    if (!stmt) break;
    if (!head) head = stmt;
    else tail->next = stmt;
    tail = stmt;
  }
  sym_pop(&p->sym);
  if (!parser_match(p, TOKEN_RBRACE))
    parser_error(p, "Expected '}'");
  n->as.block.stmts = head;
  return n;
}

static Node *parse_block(Parser *p) {
  if (!parser_match(p, TOKEN_LBRACE)) {
    parser_error(p, "Expected '{'");
    return NULL;
  }
  return parse_block_body(p);
}

static Node *parse(Parser *p) {
  return parse_program(p);
}

static char *read_file(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) { fprintf(stderr, "Could not open %s\n", path); exit(1); }
  size_t cap = 8192, len = 0;
  char *buf = malloc(cap);
  size_t n;
  while ((n = fread(buf + len, 1, cap - len, f)) > 0) {
    len += n;
    if (len >= cap) { cap *= 2; buf = realloc(buf, cap); }
  }
  fclose(f);
  buf[len] = '\n';
  buf[len + 1] = '\0';
  return buf;
}

// --- Type Checker ---

typedef struct { char *name; TypeKind type; TypeKind elem_type; } TyEntry;
typedef struct { TyEntry entries[MAX_VARS_PER_SCOPE]; int count; } TyScope;
typedef struct {
  TyScope scopes[MAX_SCOPE];
  int depth;
} TyEnv;

typedef struct {
  char *name;
  TypeKind ret_type;
  TypeKind param_types[MAX_VARS_PER_SCOPE];
  int param_count;
} FnSig;

static FnSig fn_sigs[MAX_VARS_PER_SCOPE];
static int fn_sig_count = 0;

static void ty_env_init(TyEnv *env) { env->depth = 0; env->scopes[0].count = 0; }
static void ty_env_push(TyEnv *env) { env->depth++; env->scopes[env->depth].count = 0; }
static void ty_env_pop(TyEnv *env) { env->depth--; }

static void ty_env_set(TyEnv *env, const char *name, TypeKind t, TypeKind elem) {
  TyScope *s = &env->scopes[env->depth];
  s->entries[s->count].name = strdup_str(name);
  s->entries[s->count].type = t;
  s->entries[s->count].elem_type = elem;
  s->count++;
}

static TypeKind ty_env_get(TyEnv *env, const char *name) {
  for (int d = env->depth; d >= 0; d--)
    for (int i = 0; i < env->scopes[d].count; i++)
      if (strcmp(env->scopes[d].entries[i].name, name) == 0)
        return env->scopes[d].entries[i].type;
  return TYPE_VOID;
}

static TypeKind ty_env_get_elem(TyEnv *env, const char *name) {
  for (int d = env->depth; d >= 0; d--)
    for (int i = 0; i < env->scopes[d].count; i++)
      if (strcmp(env->scopes[d].entries[i].name, name) == 0)
        return env->scopes[d].entries[i].elem_type;
  return TYPE_VOID;
}

static int type_errors = 0;

static TypeKind type_check_block(TyEnv *env, Node *n);

static FnSig *find_fn_sig(const char *name) {
  for (int i = 0; i < fn_sig_count; i++)
    if (strcmp(fn_sigs[i].name, name) == 0) return &fn_sigs[i];
  return NULL;
}

static TypeKind type_check_expr(TyEnv *env, Node *n) {
  if (!n) return TYPE_VOID;
  switch (n->type) {
    case NODE_NUMBER:
      return TYPE_INT;
    case NODE_STRING:
      return TYPE_STR;
    case NODE_ARRAY: {
      TypeKind et = TYPE_INT;
      for (Node *e = n->as.array.elements; e; e = e->next) {
        TypeKind t = type_check_expr(env, e);
        if (e == n->as.array.elements) {
          et = t;
        } else if (t != et) {
          type_errors++;
          fprintf(stderr, "type error: array element type mismatch, expected %s got %s\n",
                  type_name(et), type_name(t));
        }
      }
      n->as.array.elem_type = et;
      return TYPE_ARRAY;
    }
    case NODE_INDEX: {
      Node *arr = n->as.indexx.array;
      TypeKind at = type_check_expr(env, arr);
      if (at != TYPE_ARRAY) {
        type_errors++;
        fprintf(stderr, "type error: index target must be array, got %s\n", type_name(at));
        return TYPE_VOID;
      }
      TypeKind it = type_check_expr(env, n->as.indexx.index);
      if (it != TYPE_INT) {
        type_errors++;
        fprintf(stderr, "type error: index must be int, got %s\n", type_name(it));
      }
      TypeKind et = TYPE_INT;
      if (arr->type == NODE_VARIABLE)
        et = ty_env_get_elem(env, arr->as.variable.name);
      else if (arr->type == NODE_ARRAY)
        et = arr->as.array.elem_type;
      else if (arr->type == NODE_INDEX)
        et = arr->as.indexx.elem_type;
      n->as.indexx.elem_type = et;
      return et;
    }
    case NODE_VARIABLE: {
      TypeKind t = ty_env_get(env, n->as.variable.name);
      if (t == TYPE_VOID) {
        type_errors++; 
        fprintf(stderr, "type error: undeclared variable '%s'\n", n->as.variable.name);
        return TYPE_VOID;
      }
      if (t == TYPE_ARRAY)
        n->as.variable.elem_type = ty_env_get_elem(env, n->as.variable.name);
      return t;
    }
    case NODE_UNARY: {
      TypeKind t = type_check_expr(env, n->as.unary.right);
      if (t != TYPE_INT) {
        type_errors++; 
        fprintf(stderr, "type error: unary '%s' requires int operand, got %s\n",
                token_name(n->as.unary.op), type_name(t));
      }
      return TYPE_INT;
    }
    case NODE_BINARY: {
      TypeKind lt = type_check_expr(env, n->as.binary.left);
      TypeKind rt = type_check_expr(env, n->as.binary.right);
      TypeKind result = TYPE_VOID;
      if (n->as.binary.op == TOKEN_PLUS) {
        if (lt == TYPE_STR && rt == TYPE_STR) result = TYPE_STR;
        else if (lt == TYPE_INT && rt == TYPE_INT) result = TYPE_INT;
        else {
          type_errors++;
          fprintf(stderr, "type error: '+' requires both int or both str, got %s and %s\n",
                  type_name(lt), type_name(rt));
        }
      } else {
        if (lt != TYPE_INT || rt != TYPE_INT) {
          type_errors++;
          fprintf(stderr, "type error: binary '%s' requires int operands, got %s and %s\n",
                  token_name(n->as.binary.op), type_name(lt), type_name(rt));
        }
        result = TYPE_INT;
      }
      n->as.binary.type = result;
      return result;
    }
    case NODE_IF: {
      TypeKind ct = type_check_expr(env, n->as.iff.cond);
      if (ct != TYPE_INT) {
        type_errors++; 
        fprintf(stderr, "type error: if condition must be int, got %s\n", type_name(ct));
      }
      ty_env_push(env);
      TypeKind tt = type_check_block(env, n->as.iff.then_block);
      ty_env_pop(env);
      ty_env_push(env);
      TypeKind et = n->as.iff.else_block ? type_check_block(env, n->as.iff.else_block) : TYPE_INT;
      ty_env_pop(env);
      if (tt != et) {
        type_errors++; 
        fprintf(stderr, "type error: if branches must have same type, got %s and %s\n",
                type_name(tt), type_name(et));
      }
      return tt;
    }
    case NODE_WHILE: {
      TypeKind ct = type_check_expr(env, n->as.whilee.cond);
      if (ct != TYPE_INT) {
        type_errors++; 
        fprintf(stderr, "type error: while condition must be int, got %s\n", type_name(ct));
      }
      ty_env_push(env);
      type_check_block(env, n->as.whilee.body);
      ty_env_pop(env);
      return TYPE_INT;
    }
    case NODE_BLOCK:
      return type_check_block(env, n);
    case NODE_CALL: {
      FnSig *fsig = find_fn_sig(n->as.call.name);
      if (!fsig) {
        type_errors++; 
        fprintf(stderr, "type error: undefined function '%s'\n", n->as.call.name);
        return TYPE_VOID;
      }
      int i = 0;
      for (Node *a = n->as.call.args; a; a = a->next, i++) {
        TypeKind at = type_check_expr(env, a);
        if (i >= fsig->param_count) {
          type_errors++; 
          fprintf(stderr, "type error: too many arguments to '%s'\n", n->as.call.name);
          break;
        }
        if (at != fsig->param_types[i]) {
          type_errors++; 
          fprintf(stderr, "type error: argument %d to '%s' expected %s, got %s\n",
                  i + 1, n->as.call.name, type_name(fsig->param_types[i]), type_name(at));
        }
      }
      if (i < fsig->param_count) {
        type_errors++; 
        fprintf(stderr, "type error: too few arguments to '%s' (expected %d, got %d)\n",
                n->as.call.name, fsig->param_count, i);
      }
      return fsig->ret_type;
    }
    case NODE_LET:
    case NODE_FN:
    case NODE_PROGRAM:
    default:
      return TYPE_VOID;
  }
}

static TypeKind type_check_block(TyEnv *env, Node *n) {
  if (!n || n->type != NODE_BLOCK) return TYPE_VOID;
  TypeKind last_type = TYPE_VOID;
  for (Node *s = n->as.block.stmts; s; s = s->next) {
    if (s->type == NODE_LET) {
      TypeKind vt = type_check_expr(env, s->as.let.value);
      if (s->as.let.type != TYPE_VOID && vt != s->as.let.type) {
        type_errors++; 
        fprintf(stderr, "type error: let '%s' declared as %s but initializer is %s\n",
                s->as.let.name, type_name(s->as.let.type), type_name(vt));
      }
      ty_env_set(env, s->as.let.name, vt, s->as.let.elem_type);
      last_type = vt;
    } else {
      last_type = type_check_expr(env, s);
    }
  }
  return last_type;
}

static void type_check(Node *ast) {
  type_errors = 0;
  if (!ast || ast->type != NODE_PROGRAM) return;
  fn_sig_count = 0;
  // First pass: collect function signatures
  for (Node *s = ast->as.program.stmts; s; s = s->next) {
    if (s->type != NODE_FN) continue;
    FnSig *sig = &fn_sigs[fn_sig_count++];
    sig->name = s->as.fn.name;
    sig->ret_type = s->as.fn.ret_type;
    sig->param_count = 0;
    for (Node *p = s->as.fn.params; p; p = p->next) {
      sig->param_types[sig->param_count++] = p->as.variable.type;
    }
  }
  // Second pass: check function bodies and main body
  for (Node *s = ast->as.program.stmts; s; s = s->next) {
    if (s->type != NODE_FN) continue;
    TyEnv env;
    ty_env_init(&env);
    for (Node *p = s->as.fn.params; p; p = p->next) {
      ty_env_set(&env, p->as.variable.name, p->as.variable.type, p->as.variable.elem_type);
    }
    TypeKind body_type = type_check_block(&env, s->as.fn.body);
    if (s->as.fn.ret_type != TYPE_VOID && body_type != s->as.fn.ret_type) {
      type_errors++; 
      fprintf(stderr, "type error: function '%s' returns %s but declared as %s\n",
              s->as.fn.name, type_name(body_type), type_name(s->as.fn.ret_type));
    }
  }
  // Check main body (top-level expressions not in a function)
  {
    TyEnv env;
    ty_env_init(&env);
    for (Node *s = ast->as.program.stmts; s; s = s->next) {
      if (s->type == NODE_LET) {
        TypeKind vt = type_check_expr(&env, s->as.let.value);
        if (s->as.let.type != TYPE_VOID && vt != s->as.let.type) {
          type_errors++; 
          fprintf(stderr, "type error: let '%s' declared as %s but initializer is %s\n",
                  s->as.let.name, type_name(s->as.let.type), type_name(vt));
        }
        ty_env_set(&env, s->as.let.name, vt, s->as.let.elem_type);
      } else if (s->type != NODE_FN) {
        type_check_expr(&env, s);
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: packc [--tokens|--emit-c] <file.pack>\n");
    return 1;
  }
  int mode = 0; // 0 = ast, 1 = tokens, 2 = emit-c
  const char *path = argv[1];
  if (argc >= 3) {
    if (strcmp(argv[1], "--tokens") == 0) { mode = 1; path = argv[2]; }
    if (strcmp(argv[1], "--emit-c") == 0) { mode = 2; path = argv[2]; }
  }
  char *src = read_file(path);
  Lexer lexer;
  lexer_init(&lexer, src);
  if (mode == 1) {
    Token t;
    int errors = 0;
    for (;;) {
      t = lexer_next(&lexer);
      token_print(t);
      if (t.type == TOKEN_EOF) break;
      if (t.type == TOKEN_ERROR) errors++;
    }
    free(src);
    return errors > 0 ? 1 : 0;
  }
  Parser parser;
  parser_init(&parser, &lexer);
  Node *ast = parse(&parser);
  if (parser.panic) { free(src); return 1; }
  type_check(ast);
  if (type_errors > 0) {
    free(src);
    return 1;
  }
  if (mode == 2) {
    codegen(stdout, ast);
  } else {
    node_print(stdout, ast);
    printf("\n");
  }
  free(src);
  return 0;
}
