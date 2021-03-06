/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include "ConstantPropagationRuntimeAssert.h"
#include "ConstantPropagationTransform.h"
#include "ConstantPropagationWholeProgramState.h"
#include "Pass.h"

namespace constant_propagation {

namespace interprocedural {

class PassImpl : public Pass {
 public:
  struct Config {
    bool include_virtuals{false};
    bool create_runtime_asserts{false};
    // The maximum number of times we will try to refine the WholeProgramState.
    // Setting this to zero means that all field values and return values will
    // be treated as Top.
    size_t max_heap_analysis_iterations{0};

    intraprocedural::FixpointIterator::Config intraprocedural_analysis;
    Transform::Config transform;
    RuntimeAssertTransform::Config runtime_assert;
  };

  PassImpl(Config config)
      : Pass("InterproceduralConstantPropagationPass"), m_config(config) {}

  PassImpl() : PassImpl(Config()) {}

  void configure_pass(const PassConfig& pc) override {
    pc.get("replace_moves_with_consts",
           false,
           m_config.transform.replace_moves_with_consts);
    pc.get("fold_arithmetic",
           false,
           m_config.intraprocedural_analysis.fold_arithmetic);
    pc.get("include_virtuals", false, m_config.include_virtuals);
    pc.get("create_runtime_asserts", false, m_config.create_runtime_asserts);
    int64_t max_heap_analysis_iterations;
    pc.get("max_heap_analysis_iterations", 0, max_heap_analysis_iterations);
    always_assert(max_heap_analysis_iterations >= 0);
    m_config.max_heap_analysis_iterations =
        static_cast<size_t>(max_heap_analysis_iterations);
  }

  void run_pass(DexStoresVector& stores,
                ConfigFiles& cfg,
                PassManager& mgr) override;

  /*
   * run_pass() takes a PassManager object, making it awkward to call in unit
   * tests. run() is a more direct way to call this pass. The caller is
   * responsible for picking the right Config settings.
   */
  void run(Scope&);

  /*
   * Exposed for testing purposes.
   */
  std::unique_ptr<FixpointIterator> analyze(const Scope&);

 private:
  void compute_analysis_stats(const WholeProgramState&);

  void optimize(const Scope&, const FixpointIterator&);

  struct Stats {
    size_t constant_fields{0};
    size_t constant_methods{0};
  } m_stats;
  Transform::Stats m_transform_stats;
  Config m_config;
};

} // namespace interprocedural

} // namespace constant_propagation

using InterproceduralConstantPropagationPass =
    constant_propagation::interprocedural::PassImpl;
