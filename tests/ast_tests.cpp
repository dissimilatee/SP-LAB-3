#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "../ast.h"

namespace {

struct OutputCapture {
    std::streambuf* old = nullptr;
    std::ostringstream buffer;

    OutputCapture() {
        old = std::cout.rdbuf(buffer.rdbuf());
    }

    ~OutputCapture() {
        std::cout.rdbuf(old);
    }

    std::string str() const { return buffer.str(); }
};

struct ASTFixture {
    std::vector<ASTNode*> allocated;

    ASTNode* track(ASTNode* n) {
        allocated.push_back(n);
        return n;
    }

    ASTNode* num(int v) { return track(new_num(v)); }
    ASTNode* var(const std::string& n) { return track(new_var(n)); }
    ASTNode* bin(char op, ASTNode* l, ASTNode* r) { return track(new_binop(op, l, r)); }
    ASTNode* assign(const std::string& n, ASTNode* expr) { return track(new_assign(n, expr)); }
    ASTNode* print(ASTNode* expr) { return track(new_print(expr)); }
    ASTNode* if_node(ASTNode* cond, ASTNode* t, ASTNode* e) { return track(new_if(cond, t, e)); }
    ASTNode* while_node(ASTNode* cond, ASTNode* body) { return track(new_while(cond, body)); }
    ASTNode* seq(ASTNode* a, ASTNode* b) { return track(new_seq(a, b)); }

    ~ASTFixture() {
        if (!allocated.empty()) {
            free_ast(allocated.back());
            allocated.clear();
        }
    }
};

int parse_bool(const std::string& value, int fallback) {
    if (value == "true" || value == "1") return 1;
    if (value == "false" || value == "0") return 0;
    return fallback;
}

void apply_config_from_file(doctest::Context& ctx, const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        const auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        const std::string key = line.substr(0, pos);
        const std::string value = line.substr(pos + 1);

        if (key == "order-by") {
            ctx.setOption("order-by", value.c_str());
        } else if (key == "success") {
            ctx.setOption("success", parse_bool(value, 0));
        } else if (key == "no-version") {
            ctx.setOption("no-version", parse_bool(value, 1));
        }
    }
}

int must_throw_on_negative(int v) {
    if (v < 0) throw std::invalid_argument("negative value");
    return v;
}

} // namespace

TEST_SUITE("AST constructors") {

TEST_CASE_FIXTURE(ASTFixture, "create number and variable nodes") {
    auto* n1 = num(42);
    auto* n2 = var("alpha");

    CHECK_EQ(n1->type, N_NUM);
    CHECK_EQ(n1->value, 42);
    CHECK_EQ(n2->type, N_VAR);
    CHECK_EQ(n2->name, "alpha");
}

TEST_CASE_FIXTURE(ASTFixture, "create complex nodes") {
    auto* add = bin('+', num(2), num(3));
    auto* asg = assign("x_ctor", add);
    auto* pr = print(var("x_ctor"));
    auto* root = seq(asg, pr);

    CHECK_EQ(add->type, N_BINOP);
    CHECK_EQ(asg->type, N_ASSIGN);
    CHECK_EQ(pr->type, N_PRINT);
    CHECK_EQ(root->type, N_SEQ);
}

} // TEST_SUITE

TEST_SUITE("AST evaluation") {

TEST_CASE_FIXTURE(ASTFixture, "parameterized arithmetic operations") {
    struct OpCase { char op; int lhs; int rhs; int expected; };
    const std::vector<OpCase> cases = {
        {'+', 7, 5, 12}, {'-', 7, 5, 2}, {'*', 7, 5, 35}, {'/', 8, 4, 2}, {'/', 8, 0, 0}
    };

    for (const auto& c : cases) {
        SUBCASE(std::string("op_") + c.op) {
            OutputCapture cap;
            auto* expr = bin(c.op, num(c.lhs), num(c.rhs));
            auto* root = print(expr);
            eval_program(root);
            CHECK_EQ(cap.str(), std::to_string(c.expected) + "\n");
        }
    }
}

TEST_CASE_FIXTURE(ASTFixture, "if and relational operators") {
    OutputCapture cap;
    auto* condition = bin('L', num(5), num(5));
    auto* thenBranch = print(num(100));
    auto* elseBranch = print(num(200));
    auto* root = if_node(condition, thenBranch, elseBranch);

    eval_program(root);
    CHECK_NE(cap.str().find("100\n"), std::string::npos);
    CHECK_EQ(cap.str().find("200"), std::string::npos);
}

TEST_CASE_FIXTURE(ASTFixture, "while loop increments variable") {
    OutputCapture cap;

    auto* init = assign("i_loop", num(0));
    auto* condition = bin('<', var("i_loop"), num(3));
    auto* step = assign("i_loop", bin('+', var("i_loop"), num(1)));
    auto* printStep = print(var("i_loop"));
    auto* body = seq(step, printStep);
    auto* loop = while_node(condition, body);
    auto* root = seq(init, loop);

    eval_program(root);

    CHECK_EQ(cap.str(), "1\n2\n3\n");
}

TEST_CASE_FIXTURE(ASTFixture, "sequence and unknown binop path") {
    OutputCapture cap;
    auto* a = assign("unknown_op", bin('?', num(1), num(2)));
    auto* b = print(var("unknown_op"));
    auto* root = seq(a, b);

    eval_program(root);
    CHECK_EQ(cap.str(), "0\n");
}

TEST_CASE("exception test for invalid runtime argument") {
    REQUIRE_THROWS_AS(must_throw_on_negative(-10), std::invalid_argument);
    CHECK_NOTHROW(must_throw_on_negative(10));
}

TEST_CASE("dynamic skip example") {
    const char* env = std::getenv("RUN_EXTRA_ASSERTS");
    if (!env) {
        MESSAGE("Dynamic skip: set RUN_EXTRA_ASSERTS=1 to execute this optional assertion");
        return;
    }
    CHECK_EQ(std::string(env), "1");
}

} // TEST_SUITE

TEST_SUITE("DOT and memory") {

TEST_CASE_FIXTURE(ASTFixture, "dot output contains expected labels") {
    auto* root = seq(assign("x_dot", num(5)), print(var("x_dot")));
    const std::string file = "tests/tmp_ast.dot";
    ast_print_dot(root, file);

    std::ifstream in(file);
    REQUIRE(in.is_open());

    std::stringstream ss;
    ss << in.rdbuf();
    const std::string dot = ss.str();

    CHECK_NE(dot.find("digraph AST"), std::string::npos);
    CHECK_NE(dot.find("label=\";\""), std::string::npos);
    CHECK_NE(dot.find("label=\"print\""), std::string::npos);
    CHECK_NE(dot.find("label=\"x_dot\""), std::string::npos);
}

TEST_CASE_FIXTURE(ASTFixture, "ast_free handles null") {
    CHECK_NOTHROW(free_ast(nullptr));
}

} // TEST_SUITE

int main(int argc, char** argv) {
    doctest::Context context;
    apply_config_from_file(context, "tests/doctest.ini");
    context.applyCommandLine(argc, argv);
    return context.run();
}
