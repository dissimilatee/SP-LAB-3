#include "ast.h"
#include <fstream>

using namespace std;

static unordered_map<string,int> symTable;

ASTNode *new_num(int v) {
    auto *n = new ASTNode();
    n->type = N_NUM;
    n->value = v;
    return n;
}

ASTNode *new_var(string v) {
    auto *n = new ASTNode();
    n->type = N_VAR;
    n->name = v;
    return n;
}

ASTNode *new_binop(char op, ASTNode *l, ASTNode *r) {
    auto *n = new ASTNode();
    n->type = N_BINOP;
    n->op = op;
    n->left = l;
    n->right = r;
    return n;
}

ASTNode *new_assign(string var, ASTNode *expr) {
    auto *n = new ASTNode();
    n->type = N_ASSIGN;
    n->name = var;
    n->left = expr;
    return n;
}

ASTNode *new_print(ASTNode *expr) {
    auto *n = new ASTNode();
    n->type = N_PRINT;
    n->left = expr;
    return n;
}

ASTNode *new_if(ASTNode *cond, ASTNode *then_b, ASTNode *else_b) {
    auto *n = new ASTNode();
    n->type = N_IF;
    n->cond = cond;
    n->then_branch = then_b;
    n->else_branch = else_b;
    return n;
}

ASTNode *new_while(ASTNode *cond, ASTNode *body) {
    auto *n = new ASTNode();
    n->type = N_WHILE;
    n->cond = cond;
    n->then_branch = body;
    return n;
}

ASTNode *new_seq(ASTNode *a, ASTNode *b) {
    auto *n = new ASTNode();
    n->type = N_SEQ;
    n->left = a;
    n->right = b;
    return n;
}

static int eval_expr(ASTNode *n) {
    if (!n) return 0;
    switch (n->type) {
        case N_NUM:
            return n->value;
        case N_VAR:
            return symTable[n->name];
        case N_BINOP: {
            int l = eval_expr(n->left);
            int r = eval_expr(n->right);
            switch (n->op) {
                case '+': return l + r;
                case '-': return l - r;
                case '*': return l * r;
                case '/': return r ? l / r : 0;
                case '<': return l < r;
                case '>': return l > r;
                case 'L': return l <= r;
                case 'G': return l >= r;
                case 'E': return l == r;
                case 'N': return l != r;
                default:  return 0;
            }
        }
        default:
            return 0;
    }
}

void eval_program(ASTNode *n) {
    if (!n) return;
    switch (n->type) {
        case N_SEQ:
            eval_program(n->left);
            eval_program(n->right);
            break;
        case N_ASSIGN:
            symTable[n->name] = eval_expr(n->left);
            break;
        case N_PRINT:
            std::cout << eval_expr(n->left) << std::endl;
            break;
        case N_IF:
            if (eval_expr(n->cond))
                eval_program(n->then_branch);
            else
                eval_program(n->else_branch);
            break;
        case N_WHILE:
            while (eval_expr(n->cond))
                eval_program(n->then_branch);
            break;
        default:
            break;
    }
}

// -------- DOT (Graphviz) --------

static int nextId;

static int dump_node(ASTNode *n, ofstream &out) {
    if (!n) return -1;
    int myId = nextId++;
    // label
    switch (n->type) {
        case N_NUM:
            out << "  node" << myId << " [label=\""<< n->value << "\"];\n";
            break;
        case N_VAR:
            out << "  node" << myId << " [label=\""<< n->name << "\"];\n";
            break;
        case N_ASSIGN:
            out << "  node" << myId << " [label=\"=\"];\n";
            break;
        case N_PRINT:
            out << "  node" << myId << " [label=\"print\"];\n";
            break;
        case N_IF:
            out << "  node" << myId << " [label=\"if\"];\n";
            break;
        case N_WHILE:
            out << "  node" << myId << " [label=\"while\"];\n";
            break;
        case N_BINOP:
            out << "  node" << myId << " [label=\""<< n->op << "\"];\n";
            break;
        case N_SEQ:
            out << "  node" << myId << " [label=\";\"];\n";
            break;
    }

    auto connect_child = [&](ASTNode *child) {
        if (!child) return;
        int cid = dump_node(child, out);
        if (cid != -1) {
            out << "  node" << myId << " -> node" << cid << ";\n";
        }
    };

    connect_child(n->left);
    connect_child(n->right);
    connect_child(n->cond);
    connect_child(n->then_branch);
    connect_child(n->else_branch);

    return myId;
}

void ast_print_dot(ASTNode *n, const std::string &fname) {
    ofstream out(fname);
    out << "digraph AST {\n";
    nextId = 0;
    dump_node(n, out);
    out << "}\n";
}

void free_ast(ASTNode *n) {
    if (!n) return;
    free_ast(n->left);
    free_ast(n->right);
    free_ast(n->cond);
    free_ast(n->then_branch);
    free_ast(n->else_branch);
    free_ast(n->next);
    delete n;
}
