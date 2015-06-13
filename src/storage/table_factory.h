/*-------------------------------------------------------------------------
 *
 * table_factory.h
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 *-------------------------------------------------------------------------
 */

#pragma once

#include "catalog/manager.h"
#include "storage/backend_vm.h"
#include "storage/data_table.h"

#include <string>

namespace nstore {
namespace storage {

/**
 * Magic Table Factory!!
 */
class TableFactory {
    
public:

    /**
     * For a given Schema, instantiate a DataTable object and return it
     */
    static DataTable* GetDataTable(oid_t database_id, catalog::Schema *schema, std::string table_name);

};

} // End storage namespace
} // End nstore namespace


