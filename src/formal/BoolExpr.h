#pragma once
#include <iostream>
#include <memory>
#include <string>

namespace KEPLER_FORMAL {

enum class Op { VAR, AND, OR, NOT, XOR };

class BoolExpr : public std::enable_shared_from_this<BoolExpr> {
public:
    // Factory methods
    static std::shared_ptr<BoolExpr> Var(const std::string& name);
    static std::shared_ptr<BoolExpr> And(std::shared_ptr<BoolExpr> a, std::shared_ptr<BoolExpr> b);
    static std::shared_ptr<BoolExpr> Or(std::shared_ptr<BoolExpr> a, std::shared_ptr<BoolExpr> b);
    static std::shared_ptr<BoolExpr> Xor(std::shared_ptr<BoolExpr> a, std::shared_ptr<BoolExpr> b);
    static std::shared_ptr<BoolExpr> Not(std::shared_ptr<BoolExpr> a);

    void Print(std::ostream& out) const;

    std::string toString() const;

    // Public constructors so std::make_shared can access them
    BoolExpr() = default;
    BoolExpr(Op op, const std::string& name);                    // variable
    BoolExpr(Op op, std::shared_ptr<BoolExpr> a);                // unary
    BoolExpr(Op op, std::shared_ptr<BoolExpr> a, std::shared_ptr<BoolExpr> b); // binary

    // Create false
    static std::shared_ptr<BoolExpr> createFalse() {
        return Var("FALSE");
    }
    // Create true
    static std::shared_ptr<BoolExpr> createTrue() {
        return Var("TRUE");
    }

private:
    Op op_;
    std::string name_;
    std::shared_ptr<BoolExpr> left_;
    std::shared_ptr<BoolExpr> right_;

    static std::string OpToString(Op op);
};

}