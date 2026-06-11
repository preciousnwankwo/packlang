#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

static Token lexer_identifier(Lexer *l) {
  while (isalnum(lexer_peek(l)) || lexer_peek(l) == '_')
    lexer_advance(l);
  return lexer_make_token(l, TOKEN_IDENTIFIER);
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
  NODE_NUMBER,
  NODE_UNARY,
  NODE_BINARY,
} NodeType;

typedef struct Node {
  NodeType type;
  struct Node *next;
  union {
    int number;
    struct { TokenType op; struct Node *right; } unary;
    struct { TokenType op; struct Node *left; struct Node *right; } binary;
  } as;
} Node;

static Node *node_new(NodeType type) {
  Node *n = calloc(1, sizeof(Node));
  if (!n) { fprintf(stderr, "Out of memory\n"); exit(1); }
  n->type = type;
  return n;
}

static void node_print(FILE *out, Node *n) {
  if (!n) { fprintf(out, "()"); return; }
  switch (n->type) {
    case NODE_NUMBER:
      fprintf(out, "%d", n->as.number);
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
} Parser;

static void parser_init(Parser *p, Lexer *l) {
  p->lexer = l;
  p->panic = 0;
  p->current = lexer_next(l);
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

static Node *parse_expr(Parser *p);
static Node *parse_term(Parser *p);
static Node *parse_factor(Parser *p);

static Node *parse_expr(Parser *p) {
  Node *left = parse_term(p);
  while (parser_match(p, TOKEN_PLUS) || parser_match(p, TOKEN_MINUS)) {
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

static Node *parse_term(Parser *p) {
  Node *left = parse_factor(p);
  while (parser_match(p, TOKEN_STAR) || parser_match(p, TOKEN_SLASH)) {
    TokenType op = p->previous.type;
    Node *right = parse_factor(p);
    Node *n = node_new(NODE_BINARY);
    n->as.binary.op = op;
    n->as.binary.left = left;
    n->as.binary.right = right;
    left = n;
  }
  return left;
}

static Node *parse_factor(Parser *p) {
  if (parser_match(p, TOKEN_MINUS)) {
    Node *n = node_new(NODE_UNARY);
    n->as.unary.op = TOKEN_MINUS;
    n->as.unary.right = parse_factor(p);
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
  parser_error(p, "Expected expression");
  return NULL;
}

static Node *parse(Parser *p) {
  Node *n = parse_expr(p);
  if (!parser_check(p, TOKEN_EOF)) {
    parser_error(p, "Unexpected token");
    while (!parser_check(p, TOKEN_EOF)) parser_advance(p);
  }
  return n;
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

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: packc [--tokens] <file.pack>\n");
    return 1;
  }
  int tokens_only = 0;
  const char *path = argv[1];
  if (argc >= 3 && strcmp(argv[1], "--tokens") == 0) {
    tokens_only = 1;
    path = argv[2];
  }
  char *src = read_file(path);
  Lexer lexer;
  lexer_init(&lexer, src);
  if (tokens_only) {
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
  node_print(stdout, ast);
  printf("\n");
  free(src);
  return 0;
}
