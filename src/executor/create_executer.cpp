//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// create_executor.cpp
//
// Identification: src/executor/create_executer.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "executor/create_executer.h"


namespace peloton {
namespace executor {


//Constructor for drop executor
CreateExecutor::CreateExecutor(const planner::AbstractPlan *node,
                               ExecutorContext *executor_context)
    : AbstractExecutor(node, executor_context) {
	global_catalog = nullptr;
	context = executor_context;
}

// Initialize executer
bool CreateExecutor::DInit() {
  LOG_INFO("Initializing Create Executer...");
  auto &bootstrapper = catalog::Bootstrapper::GetInstance();
  global_catalog = bootstrapper.bootstrap();
  global_catalog->CreateDatabase("default_database");
  LOG_INFO("Create Executer initialized!");
  return true;
}

bool CreateExecutor::DExecute() {
	LOG_INFO("Executing Create...");
  const planner::CreatePlan &node = GetPlanNode<planner::CreatePlan>();
  std::string table_name = node.GetTableName();
  std::unique_ptr<catalog::Schema> schema(node.GetSchema());
  Result result = global_catalog->CreateTable("default_database", table_name, std::move(schema));
  context->GetTransaction()->SetResult(result);
  LOG_INFO("Create table succeeded!");
  return false;
}

}
}
