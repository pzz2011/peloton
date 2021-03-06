//===----------------------------------------------------------------------===//
//
//                         PelotonDB
//
// case_expression.h
//
// Identification: src/include/expression/case_expression.h
//
// Copyright (c) 2015-17, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "expression/abstract_expression.h"
#include "expression/comparison_expression.h"
#include "common/sql_node_visitor.h"

namespace peloton {
namespace expression {

//===----------------------------------------------------------------------===//
// CaseExpression
//===----------------------------------------------------------------------===//

class CaseExpression : public AbstractExpression {
 public:
  using AbsExprPtr = std::unique_ptr<AbstractExpression>;
  using WhenClause = std::pair<AbsExprPtr,AbsExprPtr>;

  CaseExpression(type::TypeId type_id,
                 std::vector<WhenClause> &when_clauses,
                 AbsExprPtr default_expr)
      : AbstractExpression(ExpressionType::OPERATOR_CASE_EXPR, type_id),
        clauses_(std::move(when_clauses)),
        default_expr_(std::move(default_expr)) {}

  CaseExpression(type::TypeId type_id, AbsExprPtr argument,
                 std::vector<WhenClause> &when_clauses,
                 AbsExprPtr default_expr)
      : AbstractExpression(ExpressionType::OPERATOR_CASE_EXPR, type_id),
        clauses_(std::move(when_clauses)),
        default_expr_(std::move(default_expr)) {
    for (uint32_t i = 0; i < clauses_.size(); i++)
      clauses_[i].first.reset(new peloton::expression::ComparisonExpression(
          peloton::ExpressionType::COMPARE_EQUAL, argument->Copy(),
          clauses_[i].first->Copy()));
  }

  type::Value Evaluate(const AbstractTuple *tuple1, const AbstractTuple *tuple2,
                 executor::ExecutorContext *context) const override {
    for (auto &clause : clauses_) {
      auto result = clause.first->Evaluate(tuple1, tuple2, context);
      if (result.IsTrue())
        return clause.second->Evaluate(tuple1, tuple2, context);
    }
    if (default_expr_ == nullptr) {
      return type::ValueFactory::GetNullValueByType(return_value_type_);
    } else {
      return default_expr_->Evaluate(tuple1, tuple2, context);
    }
  }

  AbstractExpression* Copy() const override { 
    std::vector<WhenClause> copies;
    for (auto &clause : clauses_)
      copies.push_back(WhenClause(AbsExprPtr(clause.first->Copy()),
                                  AbsExprPtr(clause.second->Copy())));
	return new CaseExpression(return_value_type_, copies,
                              default_expr_ != nullptr ?
                                  AbsExprPtr(default_expr_->Copy()) : nullptr);
  }

  virtual void Accept(SqlNodeVisitor *v) override { v->Visit(this); }

  size_t GetWhenClauseSize() const { return clauses_.size(); }

  const std::vector<WhenClause> &GetWhenClauses() const { return clauses_; }

  AbstractExpression *GetWhenClauseCond(int index) const {
    if (index < 0 || index >= (int)clauses_.size())
      return nullptr;
    return clauses_[index].first.get();
  }

  AbstractExpression *GetWhenClauseResult(int index) const {
    if (index < 0 || index >= (int)clauses_.size())
      return nullptr;
    return clauses_[index].second.get();
  }

  AbstractExpression *GetDefault() const { return default_expr_.get(); };

  // Attribute binding
  void PerformBinding(const std::vector<const planner::BindingContext *> &
                          binding_contexts) override {
    for (auto &clause : clauses_) {
      clause.first->PerformBinding(binding_contexts);
      clause.second->PerformBinding(binding_contexts);
    }
    if (default_expr_ != nullptr) {
      default_expr_->PerformBinding(binding_contexts);
    }
  }

 private:
  // The list of case-when clauses
  std::vector<WhenClause> clauses_;
  // The default value for the case
  AbsExprPtr default_expr_;
};

}  // namespace expression
}  // namespace peloton
