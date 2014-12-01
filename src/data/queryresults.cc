/*
 * QuickRank - A C++ suite of Learning to Rank algorithms
 * Webpage: http://quickrank.isti.cnr.it/
 * Contact: quickrank@isti.cnr.it
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Contributor:
 *   HPC. Laboratory - ISTI - CNR - http://hpc.isti.cnr.it/
 */
#include "data/queryresults.h"

namespace quickrank {
namespace data {

QueryResults::QueryResults(unsigned int n_results, quickrank::Label* new_labels,
                           quickrank::Feature* new_features) {
  num_results_ = n_results;
  labels_ = new_labels;
  features_ = new_features;
}

QueryResults::~QueryResults() {
}

}  // namespace data
}  // namespace quickrank

