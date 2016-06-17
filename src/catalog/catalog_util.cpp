//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// catalog.cpp
//
// Identification: src/catalog/catalog_util.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "catalog/catalog_util.h"


namespace peloton {

namespace catalog {
/**
 * Inserts a tuple in a table
 */
void InsertTuple(storage::DataTable *table, std::unique_ptr<storage::Tuple> tuple) {
  auto &txn_manager = concurrency::TransactionManagerFactory::GetInstance();
  auto txn = txn_manager.BeginTransaction();
  std::unique_ptr<executor::ExecutorContext> context(
        new executor::ExecutorContext(txn));
  planner::InsertPlan node(table, std::move(tuple));
  executor::InsertExecutor executor(&node, context.get());
  txn_manager.CommitTransaction();
}

void DeleteTuple(storage::DataTable *table, oid_t id){
	  auto txn = peloton::concurrency::current_txn;
	  std::unique_ptr<executor::ExecutorContext> context(
	      new executor::ExecutorContext(txn));

	  std::vector<storage::Tuple *> tuples;

	  // Delete
	  planner::DeletePlan delete_node(table, false);
	  executor::DeleteExecutor delete_executor(&delete_node, context.get());

	  // Predicate
	  // WHERE id_in_table = id
	  expression::TupleValueExpression *tup_val_exp =
	      new expression::TupleValueExpression(VALUE_TYPE_INTEGER, 0, 0);
	  expression::ConstantValueExpression *const_val_exp =
	      new expression::ConstantValueExpression(
	          ValueFactory::GetIntegerValue(id));
	  auto predicate = new expression::ComparisonExpression<expression::CmpEq>(
			  EXPRESSION_TYPE_COMPARE_EQUAL, tup_val_exp, const_val_exp);

	  // Seq scan
	  std::vector<oid_t> column_ids = {0};
	  std::unique_ptr<planner::SeqScanPlan> seq_scan_node(
	      new planner::SeqScanPlan(table, predicate, column_ids));
	  executor::SeqScanExecutor seq_scan_executor(seq_scan_node.get(),
	                                              context.get());

	  // Parent-Child relationship
	  delete_node.AddChild(std::move(seq_scan_node));
	  delete_executor.AddChild(&seq_scan_executor);

	  delete_executor.Init();
	  delete_executor.Execute();
}

/*
  auto &txn_manager = concurrency::TransactionManagerFactory::GetInstance();
  auto txn = txn_manager.BeginTransaction();
  std::unique_ptr<executor::ExecutorContext> context(
      new executor::ExecutorContext(txn));

  std::vector<storage::Tuple *> tuples;

  // Delete
  planner::DeletePlan delete_node(table, false);
  executor::DeleteExecutor delete_executor(&delete_node, context.get());

  // Predicate

  // WHERE ATTR_0 > 60
  expression::TupleValueExpression *tup_val_exp =
      new expression::TupleValueExpression(VALUE_TYPE_INTEGER, 0, 0);
  expression::ConstantValueExpression *const_val_exp =
      new expression::ConstantValueExpression(
          ValueFactory::GetIntegerValue(60));
  auto predicate = new expression::ComparisonExpression<expression::CmpGt>(
      EXPRESSION_TYPE_COMPARE_GREATERTHAN, tup_val_exp, const_val_exp);

  // Seq scan
  std::vector<oid_t> column_ids = {0};
  std::unique_ptr<planner::SeqScanPlan> seq_scan_node(
      new planner::SeqScanPlan(table, predicate, column_ids));
  executor::SeqScanExecutor seq_scan_executor(seq_scan_node.get(),
                                              context.get());

  // Parent-Child relationship
  delete_node.AddChild(std::move(seq_scan_node));
  delete_executor.AddChild(&seq_scan_executor);

  EXPECT_TRUE(delete_executor.Init());
  EXPECT_TRUE(delete_executor.Execute());
  // EXPECT_TRUE(delete_executor.Execute());

  txn_manager.CommitTransaction();
*/

/**
 * Generate a database catalog tuple
 * Input: The database schema, the database id, the database name
 * Returns: The generated tuple
 */
std::unique_ptr<storage::Tuple> GetCatalogTuple(catalog::Schema *schema,
		oid_t id,
		std::string name){
  std::unique_ptr<storage::Tuple> tuple(new storage::Tuple(schema, true));
  Value val1 = ValueFactory::GetIntegerValue(id);
  Value val2 = ValueFactory::GetStringValue(name, nullptr);
  tuple->SetValue(0, val1, nullptr);
  tuple->SetValue(1, val2, nullptr);
  return tuple;
}


}
}
