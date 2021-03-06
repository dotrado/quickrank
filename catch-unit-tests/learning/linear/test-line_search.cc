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
 * Contributor:
 *   Salvatore Trani 2016 - salvatore.trani@isti.cnr.it
 *   Claudio Lucchese 2016 - claudio.lucchese@isti.cnr.it
 */
#include "catch/include/catch.hpp"

#include "learning/linear/line_search.h"
#include "metric/ir/ndcg.h"

#include "data/dataset.h"
#include "data/queryresults.h"
#include "io/svml.h"
#include <cmath>
#include <iomanip>

TEST_CASE( "Testing LineSearch", "[linear][learning][linesearch]" ) {

  std::string training_filename =
      "quickranktestdata/msn1/msn1.fold1.train.5k.txt";
  std::string validation_filename =
      "quickranktestdata/msn1/msn1.fold1.vali.5k.txt";
  std::string test_filename = "quickranktestdata/msn1/msn1.fold1.test.5k.txt";
  std::string model_filename = "test-ls-model.xml";

  unsigned int num_samples = 21;
  float window_size = 10.0;
  float reduction_factor = 0.95;
  unsigned int max_iterations = 100;
  unsigned int max_failed_valid = 10;
  bool adaptive = false;
  unsigned int partial_save = -1;
  unsigned int ndcg_cutoff = 10;

  auto ranking_algorithm = std::shared_ptr<quickrank::learning::LTR_Algorithm>(
      new quickrank::learning::linear::LineSearch(num_samples,
                                                  window_size,
                                                  reduction_factor,
                                                  max_iterations,
                                                  max_failed_valid,
                                                  adaptive));

  auto training_metric = std::shared_ptr<quickrank::metric::ir::Metric>(
      new quickrank::metric::ir::Ndcg(ndcg_cutoff));
  auto testing_metric = training_metric;

  //show ranker parameters
  std::cout << "#" << std::endl << *ranking_algorithm;
  std::cout << "#" << std::endl << "# training scorer: " << *training_metric
            << std::endl;

  quickrank::io::Svml reader;
  std::shared_ptr<quickrank::data::Dataset> training_dataset = reader
      .read_horizontal(training_filename);
  std::cout << reader << *training_dataset;

  std::shared_ptr<quickrank::data::Dataset> validation_dataset = reader
      .read_horizontal(validation_filename);
  std::cout << reader << *validation_dataset;

  std::shared_ptr<quickrank::data::Dataset> test_dataset = reader
      .read_horizontal(test_filename);
  std::cout << reader << *test_dataset;

  // run the learning process
  ranking_algorithm->learn(training_dataset, validation_dataset,
                           training_metric, partial_save, model_filename);

  // check again performance on training set
  std::vector<quickrank::Score> train_scores(training_dataset->num_instances());
  ranking_algorithm->score_dataset(training_dataset, &train_scores[0]);
  quickrank::MetricScore training_score = training_metric->evaluate_dataset(
      training_dataset, &train_scores[0]);

  std::cout << *training_metric << " on training data = "
            << std::setprecision(4) << training_score << std::endl;

  // check again performance on validation set
  std::vector<quickrank::Score> valid_scores(
      validation_dataset->num_instances());
  ranking_algorithm->score_dataset(validation_dataset, &valid_scores[0]);
  quickrank::MetricScore validation_score = training_metric->evaluate_dataset(
      validation_dataset, &valid_scores[0]);

  std::cout << *training_metric << " on validation data = "
            << std::setprecision(4) << validation_score << std::endl;

  // check again performance on test set
  std::vector<quickrank::Score> test_scores(test_dataset->num_instances());
  ranking_algorithm->score_dataset(test_dataset, &test_scores[0]);
  quickrank::MetricScore test_score = testing_metric->evaluate_dataset(
      test_dataset, &test_scores[0]);

  std::cout << *testing_metric << " on test data = " << std::setprecision(4)
            << test_score << std::endl;

  // write model on disk
  ranking_algorithm->save(model_filename);

  // reload model from disk
  auto model_reloaded =
      quickrank::learning::LTR_Algorithm::load_model_from_file(model_filename);
  model_reloaded->score_dataset(test_dataset, &test_scores[0]);
  quickrank::MetricScore test_score_reloaded =
      testing_metric->evaluate_dataset(test_dataset, &test_scores[0]);

  std::cout << *testing_metric << " on test data (reloading the model) = " <<
      std::setprecision(4) << test_score_reloaded << std::endl;

  //std::remove(model_filename.c_str());

  REQUIRE( Approx( test_score ) == test_score_reloaded);

  REQUIRE( training_score >= 0.2334);
  REQUIRE( validation_score >= 0.2307);
  REQUIRE( test_score >= 0.2484);
}
