/*
 * QuickRank - A C++ suite of Learning to Rank algorithms
 * Webpage: http://quickrank.isti.cnr.it/
 * Contact: quickrank@isti.cnr.it
 *
 * Unless explicitly acquired and licensed from Licensor under another
 * license, the contents of this file are subject to the Reciprocal Public
 * License ("RPL") Version 1.5, or subsequent versions as allowed by the RPL,
 * and You may not copy or use this file in either source code or executable
 * form, except in compliance with the terms and conditions of the RPL.
 *
 * All software distributed under the RPL is provided strictly on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, AND
 * LICENSOR HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, QUIET ENJOYMENT, OR NON-INFRINGEMENT. See the RPL for specific
 * language governing rights and limitations under the RPL.
 *
 * Contributors:
 *  - Salvatore Trani(salvatore.trani@isti.cnr.it)
 */
#pragma once

#include <memory>
#include <set>

#include "data/dataset.h"
#include "metric/ir/metric.h"
#include "learning/ltr_algorithm.h"
#include "learning/linear/line_search.h"
#include "optimization/optimization.h"
#include "optimization/post_learning/post_learning_opt.h"
#include "pugixml/src/pugixml.hpp"

namespace quickrank {
namespace optimization {
namespace post_learning {
namespace pruning {

/// This implements various strategies for pruning ensembles.
class EnsemblePruning : public PostLearningOptimization {

 public:

  enum class PruningMethod {
    RANDOM, LOW_WEIGHTS, SKIP, LAST, QUALITY_LOSS, SCORE_LOSS
  };

  EnsemblePruning(double pruning_rate);

  EnsemblePruning(double pruning_rate,
                  std::shared_ptr<learning::linear::LineSearch> lineSearch);

  EnsemblePruning(const pugi::xml_document& model);

  /// Returns the name of the optimizer.
  std::string name() const {
    return NAME_;
  }

  /// Returns the pruning method of the algorithm.
  virtual PruningMethod pruning_method() const = 0;

  virtual bool line_search_pre_pruning() const = 0;

  virtual void pruning(std::set<unsigned int>& pruned_estimators,
                       std::shared_ptr<data::Dataset> dataset,
                       std::shared_ptr<metric::ir::Metric> scorer) = 0;

  void optimize(std::shared_ptr<quickrank::learning::LTR_Algorithm> algo,
                std::shared_ptr<quickrank::data::Dataset> training_dataset,
                std::shared_ptr<quickrank::data::Dataset> validation_dataset,
                std::shared_ptr<quickrank::metric::ir::Metric> metric,
                size_t partial_save,
                const std::string model_filename);

  /// Process the dataset filtering out features with 0-weight
  virtual std::shared_ptr<data::Dataset> filter_dataset(
      std::shared_ptr<data::Dataset> dataset,
      std::set<unsigned int>& pruned_estimators) const;

  static const std::vector<std::string> pruningMethodNames;

  static PruningMethod getPruningMethod(std::string name) {
    auto i_item = std::find(pruningMethodNames.cbegin(),
                            pruningMethodNames.cend(),
                            name);
    if (i_item != pruningMethodNames.cend()) {

      return PruningMethod(std::distance(pruningMethodNames.cbegin(), i_item));
    }

    // TODO: Fix return value...
    throw std::invalid_argument("pruning method " + name + " is not valid");
//    return NULL;
  }

  static std::string getPruningMethod(PruningMethod pruningMethod) {
    return pruningMethodNames[static_cast<int>(pruningMethod)];
  }

  static const std::string NAME_;

 protected:
  double pruning_rate_;
  unsigned int estimators_to_prune_;
  unsigned int estimators_to_select_;
  std::shared_ptr<learning::linear::LineSearch> lineSearch_;

  std::vector<double> weights_;

  /// The output stream operator.
  friend std::ostream& operator<<(std::ostream& os, const EnsemblePruning& a) {
    return a.put(os);
  }

  /// Prints the description of Algorithm, including its parameters
  std::ostream& put(std::ostream& os) const;

  /// Save the current model in the given output file stream.
  std::ofstream& save_model_to_file(std::ofstream& of) const;

  virtual void score(data::Dataset *dataset, Score *scores) const;

  virtual void import_weights_from_line_search(
      std::set<unsigned int>& pruned_estimators);
};

}  // namespace pruning
}  // namespace post_learning
}  // namespace optimization
}  // namespace quickrank
