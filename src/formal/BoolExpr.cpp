#include "BoolExpr.h"
// for ostream
#include <sstream>

using namespace KEPLER_FORMAL;

std::shared_ptr<BoolExpr> BoolExpr::Var(const std::string& name) {
    return std::make_shared<BoolExpr>(Op::VAR, name);
}

std::shared_ptr<BoolExpr> BoolExpr::And(std::shared_ptr<BoolExpr> a, std::shared_ptr<BoolExpr> b) {
    return std::make_shared<BoolExpr>(Op::AND, a, b);
}

std::shared_ptr<BoolExpr> BoolExpr::Or(std::shared_ptr<BoolExpr> a, std::shared_ptr<BoolExpr> b) {
    return std::make_shared<BoolExpr>(Op::OR, a, b);
}

std::shared_ptr<BoolExpr> BoolExpr::Xor(std::shared_ptr<BoolExpr> a, std::shared_ptr<BoolExpr> b) {
    return std::make_shared<BoolExpr>(Op::XOR, a, b);
}

std::shared_ptr<BoolExpr> BoolExpr::Not(std::shared_ptr<BoolExpr> a) {
    return std::make_shared<BoolExpr>(Op::NOT, a);
}

BoolExpr::BoolExpr(Op op, const std::string& name) : op_(op), name_(name) {}

BoolExpr::BoolExpr(Op op, std::shared_ptr<BoolExpr> a) : op_(op), left_(a) {}

BoolExpr::BoolExpr(Op op, std::shared_ptr<BoolExpr> a, std::shared_ptr<BoolExpr> b)
    : op_(op), left_(a), right_(b) {}

std::string BoolExpr::OpToString(Op op) {
    switch (op) {
        case Op::AND: return "∧";
        case Op::OR:  return "∨";
        case Op::XOR: return "⊕";
        default:      return "?";
    }
}

void BoolExpr::Print(std::ostream& out) const {
    switch (op_) {
        case Op::VAR:
            out << name_;
            break;
        case Op::NOT:
            out << "¬(";
            left_->Print(out);
            out << ")";
            break;
        case Op::AND:
        case Op::OR:
        case Op::XOR:
            out << "(";
            left_->Print(out);
            out << " " << OpToString(op_) << " ";
            right_->Print(out);
            out << ")";
            break;
    }
}

std::string BoolExpr::toString() const {
    std::ostringstream oss;
    Print(oss);
    return oss.str();
}
