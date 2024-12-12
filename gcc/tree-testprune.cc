/* Test pass
   Chris Tyler, Seneca Polytechnic College, 2024-11 
   Modelled on tree-nrv.cc

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#define INCLUDE_MEMORY
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree.h"
#include "gimple.h"
#include "tree-pass.h"
#include "ssa.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "internal-fn.h"
#include "gimple-pretty-print.h"

// Included for dump_printf:
#include "tree-pretty-print.h"
#include "diagnostic.h"
#include "dumpfile.h"
#include "builtins.h"

// ============================================================= vvv
// Test pass



namespace {

const pass_data pass_data_test_prune =
{
  GIMPLE_PASS, /* type */
  "testprune", /* name */
  OPTGROUP_NONE, /* optinfo_flags */
  TV_NONE, /* tv_id */
  PROP_cfg, /* properties_required */
  0, /* properties_provided */
  0, /* properties_destroyed */
  0, /* todo_flags_start */
  0, /* todo_flags_finish */
};

class pass_testprune : public gimple_opt_pass
{ 
public:
  pass_testprune (gcc::context *ctxt)
    : gimple_opt_pass (pass_data_test_prune, ctxt)
  {}

  /* opt_pass methods: */
  bool gate (function *)  final override { 
  	return 1; // always execute pass
  }
  unsigned int execute (function *) final override;

}; // class pass_testprune

bool compare_gimple_functions(gimple_seq g1, gimple_seq g2) {
    gimple_stmt_iterator gsi1 = gsi_start(g1);
    gimple_stmt_iterator gsi2 = gsi_start(g2);

    while (!gsi_end_p(gsi1) && !gsi_end_p(gsi2)) {
        gimple *stmt1 = gsi_stmt(gsi1);
        gimple *stmt2 = gsi_stmt(gsi2);

        // Check if the statements are equivalent
        if (!stmt1 || !stmt2 || !simple_cst_equal(stmt1, stmt2)) {
            return false;
        }

        gsi_next(&gsi1);
        gsi_next(&gsi2);
    }

    return gsi_end_p(gsi1) == gsi_end_p(gsi2);
}

unsigned int
pass_testprune::execute (function *fun)
  {
     struct cgraph_node *node;

    FOR_EACH_FUNCTION(node) {
        if (!node->analyzed)
            continue;

        std::string function_name = IDENTIFIER_POINTER(DECL_NAME(node->decl));
        if (function_name.find("_1") != std::string::npos) {
            size_t pos = function_name.find_last_of("_");
            std::string base_func_name = function_name.substr(0, pos);

            struct cgraph_node *base_node = node->get_base_function();
            if (base_node && base_node != node) {
                if (compare_gimple_functions(base_node->get_body(), node->get_body())) {
                    fprintf(dump_file, "PRUNE: %s\n", base_func_name.c_str());
                } else {
                    fprintf(dump_file, "NOPRUNE: %s\n", base_func_name.c_str());
                }
            }
        }
    }
    return 0;
    
  }
} // anon namespace

gimple_opt_pass *
make_pass_testprune (gcc::context *ctxt)
{
  return new pass_testprune (ctxt);
}

// ============================================================= ^^^

