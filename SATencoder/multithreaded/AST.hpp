#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>

struct VarNode;
struct NotNode;
struct AndNode;
struct OrNode;

// variant that holds any type of node
using Expr = std::variant<VarNode, 
                          std::unique_ptr<NotNode>, 
                          std::unique_ptr<AndNode>, 
                          std::unique_ptr<OrNode>>;
struct VarNode {
    std::string name;
};

struct NotNode {
    Expr child;
};

struct AndNode {
    std::vector<Expr> children;
};

struct OrNode {
    std::vector<Expr> children;
};

// helper functions to create nodes easily
inline Expr makeVar(std::string name) {
    return VarNode{std::move(name)};
}

inline Expr makeNot(Expr e) {
    return std::make_unique<NotNode>(NotNode{std::move(e)});
}

inline Expr makeAnd(std::vector<Expr> es) {
    return std::make_unique<AndNode>(AndNode{std::move(es)});
}

inline Expr makeOr(std::vector<Expr> es) {
    return std::make_unique<OrNode>(OrNode{std::move(es)});
}
