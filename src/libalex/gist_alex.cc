// gist_alex.cc

#ifdef __GNUG__
#pragma implementation "gist_alex.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <iostream>

using namespace std;


// VCPORT_E

#include "gist_compat.h"	// for MAXINT/MININT
#include "gist_alex.h"
#include "gist_cursorext.h"	// for gist_cursorext_t::*
#include "gist_support.h"	// for print<>, parse<>, etc.
#include <assert.h>
#include "alex.h"
#include "alex_base.h"
#include "alex_fanout_tree.h"


#define DiskSetting 0
#define _Debug 0
#define Profiling 0

DerivedParamss derived_paramss_;


alex_query_t::alex_query_t(bt_oper oper, void *val1, void *val2)
    : oper(oper), val1(val1), val2(val2)
{
}

alex_query_t::~alex_query_t()
{
    if (val1 != NULL) delete val1;
    if (val2 != NULL) delete val2;
}

static int
int_cmp(const void *a, const void *b)
{
    
    // can't simply copy ints, alignment might be wrong
    int x, y;
    (void) memcpy((void *) &x, a, sizeof(x));
    (void) memcpy((void *) &y, b, sizeof(y));
    // 输出比较的内容
    // cout << "正在比较: " << x << " 和 " << y << endl;
    if (x < y) { 
        return -1;
    } else if (x > y) {
        return 1;
    } else {
        return 0;
    }
    // this doesn't work when a is close to -2^31
    //int res = (*((int *) a) - *((int *) b));
}

static int
double_cmp(const void *a, const void *b)
{
    double x = *((double *) a);
    double y = *((double *) b);
    if (x < y) {
        return -1;
    } else if (x > y) {
        return 1;
    } else {
        return 0;
    }
}

static int
str_cmp(const void *a, const void *b)
{
    return (strcmp((const char *) a, (const char *) b));
}


///////////////////////////////////////////////////////////////////////////////
// gist_alex::gist_alex - constructor
//
// Description:
//
///////////////////////////////////////////////////////////////////////////////

alex_ext_t::alex_ext_t(
    gist_ext_t::gist_ext_ids id,
    const char* name,
    PrintPredFct printPred,
    PrintDataFct printData,
    ParseFct parsePred,
    ParseFct parseData,
    ParseQueryFct parseQuery,
    CmpFct keyCmp,
    CmpFct dataCmp,
    SizeFct keySize,
    SizeFct dataSize,
    NegInftyFct negInftyKey,
    NegInftyFct negInftyData) :
    gist_ext_t(id, name, printPred, printData, parsePred, parseData, parseQuery),
    keyCmp(keyCmp), dataCmp(dataCmp), keySize(keySize),
    dataSize(dataSize), negInftyKey(negInftyKey), negInftyData(negInftyData)
{
};

///////////////////////////////////////////////////////////////////////////////
// gist_alex::bulk_load
//
// Description:
//	- Bulk load a dataset
// 
//     
///////////////////////////////////////////////////////////////////////////////

rc_t 
alex_ext_t::bulk_load(gist_p& page,const vector<int>& keys, const vector<int>& values){

    cout << "=== 1. Starting ALEX Bulk Load Process ===" << endl;
    
    int num_keys = keys.size();

    std::pair<int, int>* pairs = new std::pair<int, int>[num_keys];
    for (int i = 0; i < num_keys; i++) {
        pairs[i] = std::make_pair(keys[i], values[i]);
    }


    cout << "=== 2. Sorting key-value pairs ===" << endl;
    std::sort(pairs, pairs + num_keys, 
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first < b.first;
        });

    cout << "=== 3. Initializing learned index root node ===" << endl;
    gistctrl_t new_hdr;
  
    // // Build temporary root model, which outputs a CDF in the range [0, 1]

    int min_key = pairs[0].first;
    int max_key = pairs[num_keys - 1].first;
    cout << "Minimum key: " << min_key << std::endl;
    cout << "Maximum key: " << max_key << std::endl;

    cout << "page 地址: " << &page << endl;
    // new_hdr.root = page.root();
    // new_hdr.level = page.level();
    

    // new_hdr.is_model_node = true;
    // new_hdr.num_children = 0;

    double model_a = 1.0 / (max_key - min_key);
    double model_b = -1.0 * min_key * model_a;

    // 输出计算出的初始模型参数
    cout << "\n=== 4. Initial Model Parameters ===" << endl;
    cout << "model_a (slope): " << model_a << endl;
    cout << "model_b (intercept): " << model_b << endl;

    W_DO(page.set_page_model(model_a, model_b));
    

    // rootalex.root_node_->model_.a_ = 1.0 / (max_key - min_key);
    // rootalex.root_node_->model_.b_ = -1.0 * min_key * rootalex.root_node_->model_.a_;

    // 2. 训练根节点模型
    // alex::LinearModel<int> root_data_node_model;
    // alex::LinearModelBuilder<int> model_builder(&root_data_node_model);
    // for (size_t i = 0; i < keys.size(); ++i) {
    //     model_builder.add(keys[i], i); // 键值映射到位置i
    // }
    // model_builder.build();

    // 测试初始模型
    cout << "\n=== 5. Testing Initial Model ===" << endl;

    // for (size_t i = 0; i < keys.size(); ++i) {
    //     double predicted_pos = root_data_node_model.predict(keys[i]);
    //     double error = abs(predicted_pos - i);

        // 只打印部分结果以避免输出过多
        // if (i < 5 || i > keys.size() - 5 || i == keys.size() / 2) {
        //     cout << "Key: " << keys[i] << ", Actual pos: " << i 
        //          << ", Predicted pos: " << predicted_pos 
        //          << ", Error: " << error << endl;
        // }

    // }


    DataNodeStats stats;

    // // 计算成本
    // double page_cost = compute_expected_cost(
    //     pairs,
    //     num_keys,
    //     gist_p::kInitDensity_,
    //     params_.expected_insert_frac,
    //     &root_data_node_model,
    //     params_.approximate_cost_computation,
    //     &stats
    // );

    // cout << "Page cost: " << page_cost << endl;

    // 设置页面成本
    // page.set_cost(page_cost);
    // 6.4 计算键范围边界
    // int fanout = 2;

    // vector<double> boundaries(fanout + 1);
    // for (int i = 0; i <= fanout; ++i) {
    //     double pos = static_cast<double>(i) / fanout;
    //     boundaries[i] = (pos - model_b) / model_a;
    // }

    // // 6.5 递归构建子页面
    // int cur_pos = 0;
    // for (int i = 0; i < fanout; ++i) {
    //     // 计算当前子页面的键范围
    //     double lower = boundaries[i];
    //     double upper = boundaries[i + 1];
    //     // 收集属于该范围的键值对
    //     vector<int> child_keys, child_values;
    //     while (cur_pos < num_keys && pairs[cur_pos].first < upper) {
    //         child_keys.push_back(pairs[cur_pos].first);
    //         child_values.push_back(pairs[cur_pos].second);
    //         cur_pos++;
    //     }
        // 分配子页面
        // gist_p child_page;
        // W_DO(child_page.format(child_page.pid(), hdr));
    // }

    // hdr->_model = alex::LinearModel<int>(model_a, model_b);
    // stats_.num_model_nodes++;
//       auto model_node = new (model_node_allocator().allocate(1))
//           model_node_type(node->level_, allocator_);
//       if (best_fanout_tree_depth == 0) {
//         // slightly hacky: we assume this means that the node is relatively
//         // uniform but we need to split in
//         // order to satisfy the max node size, so we compute the fanout that
//         // would satisfy that condition
//         // in expectation
//         // 计算扇出数(fanout = 2^depth)
//         best_fanout_tree_depth =
//             static_cast<int>(std::log2(static_cast<double>(num_keys) /
//                                        derived_params_.max_data_node_slots)) +
//             1;
//         used_fanout_tree_nodes.clear();
//         int max_data_node_keys = static_cast<int>(
//             derived_params_.max_data_node_slots * data_node_type::kInitDensity_);
//         fanout_tree::compute_level<T, P>(
//             values, num_keys, node, total_keys, used_fanout_tree_nodes,
//             best_fanout_tree_depth, max_data_node_keys,
//             params_.expected_insert_frac, params_.approximate_model_computation,
//             params_.approximate_cost_computation);
//       }
//       int fanout = 1 << best_fanout_tree_depth;
//       model_node->model_.a_ = node->model_.a_ * fanout;
//       model_node->model_.b_ = node->model_.b_ * fanout;
//       model_node->num_children_ = fanout;
//       model_node->children_ =
//           new (pointer_allocator().allocate(fanout)) AlexNode<T, P>*[fanout];

//       //#if DiskSetting
//       model_node->childrenAddrs = new PhyscialAddr[fanout];
//       //#endif
//       if (hybrid_mode == LEAF_DISK) {
//           model_node->is_leaf = new int[fanout];;
//           for (int i = 0; i < fanout; i++) {
//               // 0 is model 1 is leaf
//               model_node->is_leaf[i] = 0;
//           }
//       }
//       // Instantiate all the child nodes and recurse
//       int cur = 0;
//       for (fanout_tree::FTNode& tree_node : used_fanout_tree_nodes) {
//         auto child_node = new (model_node_allocator().allocate(1))
//             model_node_type(static_cast<short>(node->level_ + 1), allocator_);
//         child_node->cost_ = tree_node.cost;
//         child_node->duplication_factor_ =
//             static_cast<uint8_t>(best_fanout_tree_depth - tree_node.level);
//         int repeats = 1 << child_node->duplication_factor_;
//         double left_value = static_cast<double>(cur) / fanout;
//         double right_value = static_cast<double>(cur + repeats) / fanout;
//         double left_boundary = (left_value - node->model_.b_) / node->model_.a_;
//         double right_boundary =
//             (right_value - node->model_.b_) / node->model_.a_;
//         child_node->model_.a_ = 1.0 / (right_boundary - left_boundary);
//         child_node->model_.b_ = -child_node->model_.a_ * left_boundary;
//         model_node->children_[cur] = child_node;
//         LinearModel<T> child_data_node_model(tree_node.a, tree_node.b);
//         bool is_leaf = bulk_load_node(values + tree_node.left_boundary,
//                        tree_node.right_boundary - tree_node.left_boundary,
//                        model_node->children_[cur], total_keys, // here we write the pointer for model node
//                        &(model_node->childrenAddrs[cur]), // here we write the location for leaf node
//                        static_cast<uint8_t>(best_fanout_tree_depth - tree_node.level),
//                        tree_node.expected_avg_search_iterations,
//                        tree_node.expected_avg_shifts,
//                        &child_data_node_model);
//         model_node->children_[cur]->duplication_factor_ =
//             static_cast<uint8_t>(best_fanout_tree_depth - tree_node.level);
//         if (model_node->children_[cur]->is_leaf_) {
//           static_cast<data_node_type*>(model_node->children_[cur])
//               ->expected_avg_exp_search_iterations_ =
//               tree_node.expected_avg_search_iterations;
//           static_cast<data_node_type*>(model_node->children_[cur])
//               ->expected_avg_shifts_ = tree_node.expected_avg_shifts;
//         }
//         if (is_leaf && hybrid_mode == LEAF_DISK) {
//             model_node->is_leaf[cur] = 1;
//         }
//         // if the returned one is leaf,
//         for (int i = cur + 1; i < cur + repeats; i++) {
//             // set flag
//             if (is_leaf && hybrid_mode == LEAF_DISK) model_node->is_leaf[i] = 1;
//             model_node->children_[i] = model_node->children_[cur];
//           #if DiskSetting
//             model_node->childrenAddrs[i] = model_node->childrenAddrs[cur];
//           #endif
//         }
//         cur += repeats;
//       }

//       delete_node(node);
//       node = model_node;

//       return false;
  }

    // the return flag shows the built node is model or leaf
    rc_t alex_ext_t::bulk_load_node(const std::pair<int,int>* values, int num_keys, gist_p& page,
        int total_keys, PhyscialAddr *r_addr, uint8_t duplication_factor_,
        double expected_avg_exp_search_iterations_, double expected_avg_shifts_,
        const alex::LinearModel<int>* data_node_model = nullptr){

            std::vector<alex::fanout_tree::FTNode> used_fanout_tree_nodes;
            std::pair<int, double> best_fanout_stats;
            best_fanout_stats = alex::fanout_tree::find_best_fanout_top_down<int, int>(
                values, num_keys,page,total_keys,used_fanout_tree_nodes,derived_paramss_.max_fanout, 
                params_.expected_insert_frac,params_.approximate_model_computation,
                params_.approximate_cost_computation);
            int best_fanout_tree_depth = best_fanout_stats.first;
            double best_fanout_tree_cost = best_fanout_stats.second;

             // Decide whether this node should be a model node or data node
            // if (best_fanout_tree_cost < page.get_cost() ||
            //     num_keys > derived_paramss_.max_data_node_slots *
            //                     gist_p::kInitDensity_) {
                // std::cout << "best_fanout_tree_cost: " << best_fanout_tree_cost << std::endl;
                // std::cout << "page.get_cost(): " << page.get_cost() << std::endl;
                // std::cout << "num_keys: " << num_keys << std::endl;
                // std::cout << "derived_paramss_.max_data_node_slots: " << derived_paramss_.max_data_node_slots << std::endl;
                // std::cout << "gist_p::kInitDensity_: " << gist_p::kInitDensity_ << std::endl;
                // std::cout << "max_slots * kInitDensity_: " << derived_paramss_.max_data_node_slots * gist_p::kInitDensity_ << std::endl;
                // Convert to model node based on the output of the fanout tree
                // cout << "Converting to model node:"<< stats_.num_model_nodes << endl;
                // stats_.num_model_nodes++;
                // cout << "Converting to model node:"<< stats_.num_model_nodes << endl;
                // // 1. 创建新的模型节点页面
                // gist_p model_page;
            // }


    }

  ///////////////////////////////////////////////////////////////////////////////
// gist_alex::add compute_expected_cost function
//
// Description:
//	- Computes the expected cost of a data node constructed using the input dense
//  array of keys
//  Assumes existing_model is trained on the dense array of keys
// Return Values:
//      double
///////////////////////////////////////////////////////////////////////////////

  static double alex_ext_t::compute_expected_cost(
    const std::pair<int,int>* values,
    int num_keys,
    double density,
    double expected_insert_frac,
    const  alex::LinearModel<int>* existing_model = nullptr, bool use_sampling = false,
    DataNodeStats* stats = nullptr)
{
    if (use_sampling) {
        return compute_expected_cost_sampling(values, num_keys, density,
                                              expected_insert_frac,
                                              existing_model, stats);
      }
  
      if (num_keys == 0) {
        return 0;
      }
  
      int data_capacity =
          std::max(static_cast<int>(num_keys / density), num_keys + 1);
  
      // Compute what the node's model would be
      alex::LinearModel<int> model;
      if (existing_model == nullptr) {
        build_model(values, num_keys, &model);
      } else {
        model.a_ = existing_model->a_;
        model.b_ = existing_model->b_;
      }
      model.expand(static_cast<double>(data_capacity) / num_keys);
  
      // Compute expected stats in order to compute the expected cost
      double cost = 0;
      double expected_avg_exp_search_iterations = 0;
      double expected_avg_shifts = 0;
      if (expected_insert_frac == 0) {
        alex::ExpectedSearchIterationsAccumulator acc;
        build_node_implicit(values, num_keys, data_capacity, &acc, &model);
        expected_avg_exp_search_iterations = acc.get_stat();
      } else {
        alex::ExpectedIterationsAndShiftsAccumulator acc(data_capacity);
        build_node_implicit(values, num_keys, data_capacity, &acc, &model);
        expected_avg_exp_search_iterations =
            acc.get_expected_num_search_iterations();
        expected_avg_shifts = acc.get_expected_num_shifts();
      }
      cost = alex::kExpSearchIterationsWeight * expected_avg_exp_search_iterations +
      alex::kShiftsWeight * expected_avg_shifts * expected_insert_frac;
  
      if (stats) {
        stats->num_search_iterations = expected_avg_exp_search_iterations;
        stats->num_shifts = expected_avg_shifts;
      }

      return cost;

}

    static void alex_ext_t::build_model (const std::pair<int,int>* values, int num_keys, alex::LinearModel<int>* model,
        bool use_sampling = false) {
        if (use_sampling) {
            build_model_sampling (values,num_keys, model);
            return;
        }
        // build the model using all data points
        // Independent variable (x): The key values[i].first.
        // Dependent variable (y): The index i of the key in the array.
            alex::LinearModelBuilder<int> builder(model);
            for (int i = 0; i < num_keys; i++) {
            builder.add(values[i].first, i);
            }
            builder.build();
    }

    static void alex_ext_t::build_model_sampling (const std::pair<int,int>* values, int num_keys,
        alex::LinearModel<int>* model, bool verbose = false) {
    const static int sample_size_lower_bound = 10;
    // If slope and intercept change by less than this much between samples,
    // return
    const static double rel_change_threshold = 0.01;
    // If intercept changes by less than this much between samples, return
    const static double abs_change_threshold = 0.5;
    // Increase sample size by this many times each iteration
    const static int sample_size_multiplier = 2;

    // If the number of keys is sufficiently small, we do not sample
    if (num_keys <= sample_size_lower_bound * sample_size_multiplier) {
        build_model(values, num_keys, model, false);
        return;
    }

    int step_size = 1;
    double sample_size = num_keys;
    while (sample_size >= sample_size_lower_bound) {
        sample_size /= sample_size_multiplier;
        step_size *= sample_size_multiplier;
    }
    step_size /= sample_size_multiplier;

    // Run with initial step size
    alex::LinearModelBuilder<int> builder(model);
    for (int i = 0; i < num_keys; i += step_size) {
        
    }
        builder.build();
        double prev_a = model->a_;
        double prev_b = model->b_;
        if (verbose) {
            std::cout << "Build index, sample size: " << num_keys / step_size
            << " (a, b): (" << prev_a << ", " << prev_b << ")" << std::endl;
        }

    // Keep decreasing step size (increasing sample size) until model does not
    // change significantly
    while (step_size > 1) {
    step_size /= sample_size_multiplier;
    // Need to avoid processing keys we already processed in previous samples
    int i = 0;
    while (i < num_keys) {
        i += step_size;
        for (int j = 1; (j < sample_size_multiplier) && (i < num_keys);
        j++, i += step_size) {
        builder.add(values[i].first, i);
        }
    }
    builder.build();

    double rel_change_in_a = std::abs((model->a_ - prev_a) / prev_a);
    double abs_change_in_b = std::abs(model->b_ - prev_b);
    double rel_change_in_b = std::abs(abs_change_in_b / prev_b);
    if (verbose) {
    std::cout << "Build index, sample size: " << num_keys / step_size
    << " (a, b): (" << model->a_ << ", " << model->b_ << ") ("
    << rel_change_in_a << ", " << rel_change_in_b << ")"
    << std::endl;
    }
    if (rel_change_in_a < rel_change_threshold &&
    (rel_change_in_b < rel_change_threshold ||
    abs_change_in_b < abs_change_threshold)) {
    return;
    }
    prev_a = model->a_;
    prev_b = model->b_;
    }
}

static double alex_ext_t::compute_expected_cost_sampling(
    const std::pair<int,int>* values, int num_keys, double density,
        double expected_insert_frac,
        const alex::LinearModel<int>* existing_model = nullptr,
        DataNodeStats* stats = nullptr){
  const static int min_sample_size = 25;

  // Stop increasing sample size if relative diff of stats between samples is
  // less than this
  const static double rel_diff_threshold = 0.2;

  // Equivalent threshold in log2-space
  const static double abs_log2_diff_threshold =
      std::log2(1 + rel_diff_threshold);

  // Increase sample size by this many times each iteration
  const static int sample_size_multiplier = 2;

  // If num_keys is below this threshold, we compute entropy exactly
  const static int exact_computation_size_threshold =
      (min_sample_size * sample_size_multiplier * sample_size_multiplier * 2);

  // Target fraction of the keys to use in the initial sample
  const static double init_sample_frac = 0.01;

  // If the number of keys is sufficiently small, we do not sample
  if (num_keys < exact_computation_size_threshold) {
    return compute_expected_cost(values, num_keys, density,
                                 expected_insert_frac, existing_model, false,
                                 stats);
  }

  alex::LinearModel<int> model;  // trained for full dense array
  if (existing_model == nullptr) {
    build_model(values, num_keys, &model);
  } else {
    model.a_ = existing_model->a_;
    model.b_ = existing_model->b_;
  }

  // Compute initial sample size and step size
  // Right now, sample_num_keys holds the target sample num keys
  int sample_num_keys = std::max(
      static_cast<int>(num_keys * init_sample_frac), min_sample_size);
  int step_size = 1;
  double tmp_sample_size =
      num_keys;  // this helps us determine the right sample size
  while (tmp_sample_size >= sample_num_keys) {
    tmp_sample_size /= sample_size_multiplier;
    step_size *= sample_size_multiplier;
  }
  step_size /= sample_size_multiplier;
  sample_num_keys =
      num_keys /
      step_size;  // now sample_num_keys is the actual sample num keys

  std::vector<SampleDataNodeStats>
      sample_stats;  // stats computed usinig each sample
  bool compute_shifts = expected_insert_frac !=
                        0;  // whether we need to compute expected shifts
  double log2_num_keys = std::log2(num_keys);
  double expected_full_search_iters =
      0;  // extrapolated estimate for search iters on the full array
  double expected_full_shifts =
      0;  // extrapolated estimate shifts on the full array
  bool search_iters_computed =
      false;  // set to true when search iters is accurately computed
  bool shifts_computed =
      false;  // set to true when shifts is accurately computed

  // Progressively increase sample size
  while (true) {
    int sample_data_capacity = std::max(
        static_cast<int>(sample_num_keys / density), sample_num_keys + 1);
    alex::LinearModel<int> sample_model(model.a_, model.b_);
    sample_model.expand(static_cast<double>(sample_data_capacity) / num_keys);

    // Compute stats using the sample
    if (expected_insert_frac == 0) {
      alex::ExpectedSearchIterationsAccumulator acc;
      build_node_implicit_sampling(values, num_keys, sample_num_keys,
                                   sample_data_capacity, step_size, &acc,
                                   &sample_model);
      sample_stats.push_back({std::log2(sample_num_keys), acc.get_stat(), 0});
    } else {
      alex::ExpectedIterationsAndShiftsAccumulator acc(sample_data_capacity);
      build_node_implicit_sampling(values, num_keys, sample_num_keys,
                                   sample_data_capacity, step_size, &acc,
                                   &sample_model);
      sample_stats.push_back({std::log2(sample_num_keys),
                              acc.get_expected_num_search_iterations(),
                              std::log2(acc.get_expected_num_shifts())});
    }

    if (sample_stats.size() >= 3) {
      // Check if the diff in stats is sufficiently small
      SampleDataNodeStats& s0 = sample_stats[sample_stats.size() - 3];
      SampleDataNodeStats& s1 = sample_stats[sample_stats.size() - 2];
      SampleDataNodeStats& s2 = sample_stats[sample_stats.size() - 1];
      // (y1 - y0) / (x1 - x0) = (y2 - y1) / (x2 - x1) --> y2 = (y1 - y0) /
      // (x1 - x0) * (x2 - x1) + y1
      double expected_s2_search_iters =
          (s1.num_search_iterations - s0.num_search_iterations) /
              (s1.log2_sample_size - s0.log2_sample_size) *
              (s2.log2_sample_size - s1.log2_sample_size) +
          s1.num_search_iterations;
      double rel_diff =
          std::abs((s2.num_search_iterations - expected_s2_search_iters) /
                   s2.num_search_iterations);
      if (rel_diff <= rel_diff_threshold || num_keys <= 2 * sample_num_keys) {
        search_iters_computed = true;
        expected_full_search_iters =
            (s2.num_search_iterations - s1.num_search_iterations) /
                (s2.log2_sample_size - s1.log2_sample_size) *
                (log2_num_keys - s2.log2_sample_size) +
            s2.num_search_iterations;
      }
      if (compute_shifts) {
        double expected_s2_log2_shifts =
            (s1.log2_num_shifts - s0.log2_num_shifts) /
                (s1.log2_sample_size - s0.log2_sample_size) *
                (s2.log2_sample_size - s1.log2_sample_size) +
            s1.log2_num_shifts;
        double abs_diff =
            std::abs((s2.log2_num_shifts - expected_s2_log2_shifts) /
                     s2.log2_num_shifts);
        if (abs_diff <= abs_log2_diff_threshold ||
            num_keys <= 2 * sample_num_keys) {
          shifts_computed = true;
          double expected_full_log2_shifts =
              (s2.log2_num_shifts - s1.log2_num_shifts) /
                  (s2.log2_sample_size - s1.log2_sample_size) *
                  (log2_num_keys - s2.log2_sample_size) +
              s2.log2_num_shifts;
          expected_full_shifts = std::pow(2, expected_full_log2_shifts);
        }
      }

      // If diff in stats is sufficiently small, return the approximate
      // expected cost
      if ((expected_insert_frac == 0 && search_iters_computed) ||
          (expected_insert_frac > 0 && search_iters_computed &&
           shifts_computed)) {
        double cost =
            alex::kExpSearchIterationsWeight * expected_full_search_iters +
            alex::kShiftsWeight * expected_full_shifts * expected_insert_frac;
        if (stats) {
          stats->num_search_iterations = expected_full_search_iters;
          stats->num_shifts = expected_full_shifts;
        }
        return cost;
      }
    }

    step_size /= sample_size_multiplier;
    sample_num_keys = num_keys / step_size;
  }
}

static void alex_ext_t::build_node_implicit_sampling(const std::pair<int,int>* values,int num_keys,
    int sample_num_keys,
    int sample_data_capacity,
    int step_size, alex::StatAccumulator* ent,
    const alex::LinearModel<int>* sample_model) {
        int last_position = -1;
    int sample_keys_remaining = sample_num_keys;
    for (int i = 0; i < num_keys; i += step_size) {
      int predicted_position =
          std::max(0, std::min(sample_data_capacity - 1,
                               sample_model->predict(values[i].first)));
      int actual_position =
          std::max<int>(predicted_position, last_position + 1);
      int positions_remaining = sample_data_capacity - actual_position;
      if (positions_remaining < sample_keys_remaining) {
        actual_position = sample_data_capacity - sample_keys_remaining;
        for (int j = i; j < num_keys; j += step_size) {
          predicted_position =
              std::max(0, std::min(sample_data_capacity - 1,
                                   sample_model->predict(values[j].first)));
          ent->accumulate(actual_position, predicted_position);
          actual_position++;
        }
        break;
      }
      ent->accumulate(actual_position, predicted_position);
      last_position = actual_position;
      sample_keys_remaining--;
    }
  }

  static void alex_ext_t::build_node_implicit(const std::pair<int,int>* values, int num_keys,
    int data_capacity, alex::StatAccumulator* acc,
    const alex::LinearModel<int>* model){
        int last_position = -1;
    int keys_remaining = num_keys;
    for (int i = 0; i < num_keys; i++) {
      int predicted_position = std::max(
          0, std::min(data_capacity - 1, model->predict(values[i].first)));
      int actual_position =
          std::max<int>(predicted_position, last_position + 1);
      int positions_remaining = data_capacity - actual_position;
      if (positions_remaining < keys_remaining) {
        actual_position = data_capacity - keys_remaining;
        for (int j = i; j < num_keys; j++) {
          predicted_position = std::max(
              0, std::min(data_capacity - 1, model->predict(values[j].first)));
          acc->accumulate(actual_position, predicted_position);
          actual_position++;
        }
        break;
      }
      acc->accumulate(actual_position, predicted_position);
      last_position = actual_position;
      keys_remaining--;
    }
    }

///////////////////////////////////////////////////////////////////////////////
// gist_alex::insert - insert new entry in sort order
//
// Description:
//	- insert after rightmost slot with item <= (key, data)
// Return Values:
//      RCOK
///////////////////////////////////////////////////////////////////////////////
rc_t
alex_ext_t::insert(
    gist_p& page,
    const vec_t& key,
    const vec_t& dataPtr,
    shpid_t child)
{
    const void* data;
    if (page.is_leaf()) {
        data = dataPtr.ptr(0);
    } else {
        // by convention, our key also contains a data pointer (to
        // make the internal node keys unique); we don't want to use
        // this during _binSearch(), so we 'skip' over it.
	data = (const void *) (((char *) key.ptr(0)) + this->keySize(key.ptr(0)));
    }
    int slot = _binSearch(page, key.ptr(0), data, false);
    W_DO(page.insert(key, dataPtr, slot + 1, child));
    return RCOK;
}

///////////////////////////////////////////////////////////////////////////////
// gist_alex::remove - remove number of slots
//
// Description:
// Return Values:
//      RCOK
///////////////////////////////////////////////////////////////////////////////

rc_t 
alex_ext_t::remove(
    gist_p& page,
    const int slots[],
    int numSlots)
{
    for (int i = numSlots - 1; i >= 0; i--) {
        W_DO(page.remove(slots[i]));
    }
    return RCOK;
}


///////////////////////////////////////////////////////////////////////////////
// gist_alex::updateKey - nothing to do
//
// Description:
//	- B-tree partitions the data space, no BPs to update 
//
// Return Values:
//      RCOK
///////////////////////////////////////////////////////////////////////////////

rc_t
alex_ext_t::updateKey(
    gist_p& page,
    int& slot,
    const vec_t& newKey)
{
    return RCOK;
}


///////////////////////////////////////////////////////////////////////////////
// gist_alex::findMinPen - return insertion subtree of (key, data)
//
// Description:
//	- return slof of rightmost item that is <= (key, data)
//
// Return Values:
//      RCOK
///////////////////////////////////////////////////////////////////////////////
void
alex_ext_t::findMinPen(
    const gist_p& page,
    const vec_t& key,
    const vec_t& data,
    int& slot)
{
    slot = _binSearch(page, key.ptr(0), data.ptr(0), false);
    assert(slot != -1);
}


///////////////////////////////////////////////////////////////////////////////
// gist_alex::search - return qualifying slots
//
// Description:
//	- return slof of rightmost item that is <= (key, data)
//
// Return Values:
//      RCOK
///////////////////////////////////////////////////////////////////////////////
void 
alex_ext_t::search(
    gist_p& page,
    const gist_query_t* query,
    int matches[],
    int& numMatches)
{

    const alex_query_t* q = (const alex_query_t *) query;
    int start, end; // start and end slot to scan

    numMatches = 0;
    switch (q->oper) {
    case alex_query_t::bt_nooper:
        start = 0;
	end = page.nrecs() - 1;
	break;
    case alex_query_t::bt_eq:
        start = _binSearch(page, q->val1, NULL, true);
	if (start == -1) {
	    // we're not going to find anything here
	    return;
	}
	// if we're at an internal node and val1 == key[start], there might be 
	// duplicates of val on the child to the left of child[start] (unless
	// we're already at the left boundary)
	if (!page.is_leaf() && start > 0 &&
	    keyCmp(page.rec(start).key(), q->val1) == 0) {

	    start--;
	}

	// The end of the range would be the rightmost slot with the same key,
	// but since _binSearch() can't find that or the position of the next higher
	// key, we pretend our range extends to the end of the page and check the 
	// keys as we go through the slots.
	end = page.nrecs() - 1;
	break;
    case alex_query_t::bt_lt:
    case alex_query_t::bt_le:
        start = 0;
	end = _binSearch(page, q->val1, NULL, true);
	break;
    case alex_query_t::bt_gt:
    case alex_query_t::bt_ge:
        start = _binSearch(page, q->val1, NULL, true);
	if (start == -1) start = 0;
	end = page.nrecs() - 1;
	break;
    case alex_query_t::bt_betw:  // equiv. to >= val1 && <= val2
        start = _binSearch(page, q->val1, NULL, true);
	if (start == -1) start = 0;
        end = _binSearch(page, q->val2, NULL, true);
	break;
    default: // something's wrong
        assert(0);
    };

    bool hit = false;
    bool stop = false; // bt_eq might tell us to stop
    for (int slot = start; slot <= end; slot++) {
        if (page.is_leaf()) {
	    switch (q->oper) {
	    case alex_query_t::bt_nooper:
		hit = true;
		break;
	    case alex_query_t::bt_eq:
	        if (keyCmp(page.rec(slot).key(), q->val1) == 0) {
		    hit = true;
		} else {
		    hit = false;
		    stop = true; // no more equal keys on this page
		}
		break;
	    case alex_query_t::bt_lt:
	        if (slot != end || keyCmp(page.rec(slot).key(), q->val1) < 0) {
		    hit = true;
		}
		break;
	    case alex_query_t::bt_le:
		hit = true;
		break;
	    case alex_query_t::bt_gt:
	        if (slot != start || keyCmp(page.rec(slot).key(), q->val1) > 0) {
		    hit = true;
		}
		break;
	    case alex_query_t::bt_ge:
		// start positioned on rightmost item <= key, we must only return
		// items < key
	        if (slot != start || keyCmp(page.rec(slot).key(), q->val1) >= 0) {
		    hit = true;
		}
		break;
	    case alex_query_t::bt_betw:
	        if (slot != start || keyCmp(page.rec(slot).key(), q->val1) >= 0) {
		    hit = true;
		}
		break;
	    default: // something's fishy
	        assert(0);
	    }

	} else { // internal node
	    switch (q->oper) {
	    case alex_query_t::bt_lt: 
		if (slot == end) {
		    // only goto child if its smallest key < val1; _binSearch()
		    // might have found key == val1)
		    if (keyCmp(page.rec(slot).key(), q->val1) < 0) hit = true;
		} else {
		    hit = true;
		}
		break;
	    case alex_query_t::bt_eq: 
		// stop checking the entries when we hit one that's > val1
	        if (keyCmp(page.rec(slot).key(), q->val1) > 0) {
		    hit = false;
		    stop = true;
		} else {
		    hit = true;
		}
		break;
	    default:
	        hit = true;
	    }
	}
	if (hit) {
	    matches[numMatches] = slot;
	    numMatches++;
	    hit = false;
	}
	if (stop) {
	    break;
	}
    }
}


///////////////////////////////////////////////////////////////////////////////
// gist_alex:getKey - return pointer to key on page
//
// Description:
//
///////////////////////////////////////////////////////////////////////////////

void 
alex_ext_t::getKey(
    const gist_p& page,
    int slot,
    vec_t& key)
{
    const keyrec_t& tup = page.rec(slot);
    key.set(tup.key(), tup.klen());
}


///////////////////////////////////////////////////////////////////////////////
// gist_alex::pickSplit - choose split point that balances size of nodes
//
// Description:
// 	- Chooses a split point that roughly balances the size of the two new
//	  nodes. Also takes sizes of new entries into account.
//
// Preconditions:
// Postconditions:
// Notes:
//
// Return Values:
//      RCOK
///////////////////////////////////////////////////////////////////////////////

rc_t 
alex_ext_t::pickSplit(
    gist_p& page,
    int rightEntries[],
    int& numRight,
    const vec_t& oldBp,
    vec_t& leftBp,
    vec_t& rightBp,
    const vec_t& entry1,
    bool& oneGoesRight,
    const vec_t& entry2,
    bool& twoGoesRight)
{
    // first, count the number of bytes used for all keys (also the new ones)
    int totalBytes = 0;
    int slotCnt = page.nrecs();
    int i;
    for (i = 0; i < slotCnt; i++) {
	// for internal entries, we include the data pointers, because
	// these will also be distributed (and who knows, they might
	// even be varying length)
	totalBytes += page.rec(i).klen();
    }
    totalBytes += entry1.len(0);
    if (entry1.size() != 0) slotCnt++;
    totalBytes += entry2.len(0);
    if (entry2.size() != 0) slotCnt++;

    PosInfo entries[gist_p::max_scnt + 2];
    keyrec_t::hdr_s hdr1, hdr2;
    _loadPosInfo(page, entry1, entry2, hdr1, hdr2, entries);
    // now, accumulate entries until you exceed half the total size
    // (entries[] contains sizes for all entries in key/data order, including new entries)
    int total = 0;
    i = 0;
    while (total < totalBytes/2 && i < slotCnt) {
        total += entries[i].hdr->klen();
	i++;
    }
    assert(i != slotCnt); // can't be...

    // everything from slot i on goes right ...
    numRight = 0;
    oneGoesRight = false;
    twoGoesRight = false;
    for (int j = i; j < slotCnt; j++) {
	if (entries[j].slot > 0) {
	    rightEntries[numRight] = entries[j].slot;
	    numRight++;
	} else if (entries[j].slot == -1) {
	    oneGoesRight = true;
	} else {
	    assert(entries[j].slot == -2);
	    twoGoesRight = true;
	}
    }

    // the BP of the original node stays the same
    if (oldBp.size() != 0) {
	(void) memcpy(leftBp.ptr(0), oldBp.ptr(0), oldBp.len(0));
	const void *leftptr = leftBp.ptr(0);
	leftBp.set(leftptr, oldBp.len(0));
    } else {
        // this is what used to be the root;
	// the BP becomes -infinity
	void *leftptr = leftBp.ptr(0);
	negInftyKey(leftptr);
	int leftsz = this->keySize(leftptr);
	void *dataptr = (void *) ((char *) leftptr + leftsz);
	negInftyData(dataptr);
	leftsz += this->dataSize(dataptr);
	leftBp.set(leftptr, leftsz);
    }

    // the BP of the new right sibling is the item at the split point
    void* rightptr = rightBp.ptr(0);
    int rightlen;
    if (entries[i].slot > 0) {
        // take then BP from the page
	const keyrec_t& tup = page.rec(entries[i].slot);
	(void) memcpy(rightptr, tup.key(), tup.klen());
	rightlen = tup.klen();
	if (page.is_leaf()) {
	    // also copy the data ptr
	    (void) memcpy((void *) ((char *) rightptr + rightlen), tup.elem(), tup.elen());
	    rightlen += tup.elen();
	}
    } else {
        const vec_t* e;
	if (entries[i].slot == -1) {
	    e = &entry1;
	} else {
	    assert(entries[i].slot == -2);
	    e = &entry2;
	}
	(void) memcpy(rightptr, e->ptr(0), e->len(0));
	rightlen = e->len(0);
	if (page.is_leaf()) {
	    (void) memcpy((void *) ((char *) rightptr + (int) rightlen), e->ptr(1), e->len(1));
	    rightlen += e->len(1);
	}
    }
    rightBp.set(rightptr, rightlen);

    return RCOK;
}


/////////////////////////////////////////////////////////////////////////
// alex_ext_t::unionBp - generate BP
//
// Description:
//	- B-trees partition the data space, which means that BPs do not
//	  change when data is added to or delete from a page
//	- unfortunately, we can't generally provide the correct BP 
// 	  for bulk-loading (don't know when to return -\infty)
//
/////////////////////////////////////////////////////////////////////////

void
alex_ext_t::unionBp(
    const gist_p& page, // in
    vec_t& bp, // in/out
    bool bpIsValid, // in
    const vec_t& pred1, // in
    const vec_t& pred2, // in
    bool& bpChanged) // out
{
    bpChanged = false;
}

gist_cursorext_t*
alex_ext_t::queryCursor(
    const gist_query_t* query) const
{
    return gist_cursorext_t::gist_cursorext_list[gist_cursorext_t::cext_stack_id];
}

bool 
alex_ext_t::check(
    const vec_t& bp,
    const vec_t& pred,
    int level)
{
    if (keyCmp(pred.ptr(0), bp.ptr(0)) < 0) return false;
    if (level > 1) {
        // check data contained in predicate
	void* bpData = (char *) bp.ptr(0) + keySize(bp.ptr(0));
	void* predData = (char *) pred.ptr(0) + keySize(pred.ptr(0));
	return (dataCmp(predData, bpData) >= 0);
    }
    return true;
}


int
alex_ext_t::_binSearch(
    const gist_p& page,
    const void* key,
    const void* data,
    bool keyOnly) // true: only compare keys
{
    int hi = page.nrecs() - 1;
    if (hi == -1) {
        // nothing on this page yet
	return -1;
    }
    int lo = 0;
    int mid;
    const void* midkey;
    const void* middata;
    int res;

    for (;;) {
	mid = (hi + lo) / 2;
        const keyrec_t& tup = page.rec(mid);
	midkey = tup.key();
	if (page.is_leaf()) {
	    middata = tup.elem();
	} else {
	    int sz = this->keySize(midkey);
	    middata = (const void *) (((char *) midkey) + sz);
	}
	res = keyCmp(key, midkey);
	if (!keyOnly && res == 0) {
	    res = dataCmp(data, middata);
	}
	if (res < 0) {
	    // key is smaller than midpoint
	    hi = mid - 1;
	} else if (res > 0) {
	    lo = mid + 1;
	} else {
	    // found an exact match, but not sure it's the first one
	    hi = mid; // not mid - 1, we might end up returning mid
	    if (hi == lo) return hi;
	}
	if (hi < lo) return hi;
    }
#if 0 // just for explanatory purposes
    if (res < 0) {
        return mid-1;
	// because mid-1 is our upper bound, but also our lower bound
	// (hi <= lo) 
    } else {
        // res > 0: lo = hi, because mid < hi and lo now = mid + 1
        return hi;
    }
#endif
}

// Determine where the two new entries would go on the page (which slots they
// would occupy). Returns this info through array of PosInfos, two of which will 
// contain info for new entries

void
alex_ext_t::_loadPosInfo(
    gist_p& page,
    const vec_t& entry1,
    const vec_t& entry2,
    keyrec_t::hdr_s& hdr1, // in: hdr_s for 1st entry, needed for entries[]
    keyrec_t::hdr_s& hdr2, // in: hdr_s for 2nd entry, needed for entries[]
    PosInfo entries[]) // out: hdrs of all items of page + new entries, sorted in key/data order
{
    int cnt = page.nrecs();
    int numEntries = cnt;
    int k;
    for (k = 0; k < cnt; k++) {
        entries[k].hdr = &page.rec(k);
	entries[k].slot = k;
    }
    
    if (entry1.size() == 0) {
	// no entries to add to PosInfo, we're done
        return;
    }

    // Figure out where entry1/-2 would go.
    const void *data1, *data2;
    if (page.is_leaf()) {
	data1 = entry1.ptr(1);
	data2 = entry2.ptr(1);
    } else {
	// extract data portion from "key"
	data1 = (((char *) entry1.ptr(0)) + this->keySize(entry1.ptr(0)));
	data2 = (((char *) entry2.ptr(0)) + this->keySize(entry2.ptr(0)));
    }
    int oneSlot = _binSearch(page, entry1.ptr(0), data1, false) + 1;
    int twoSlot = -1;
    const vec_t* firstEntry = NULL; // new entry with "lower" slot index
    const vec_t* secondEntry = NULL; // new entry with "higher" slot index
    if (entry2.size() != 0) {
	twoSlot = _binSearch(page, entry2.ptr(0), data2, false) + 1;
	if (oneSlot == twoSlot) {
	    // we have to determine which one of the entries goes first
	    int res = keyCmp(entry1.ptr(0), entry2.ptr(0));
	    if (res == 0) {
	        res = dataCmp(data1, data2);
	    }
	    if (res < 0) {
	        firstEntry = &entry1;
		secondEntry = &entry2;
	    } else if (res > 0) {
	        firstEntry = &entry2;
		secondEntry = &entry1;
	    } else {
	        // res == 0: something's wrong (we've got perfect duplicates)
		assert(0);
	    }
	} else if (oneSlot < twoSlot) {
	    firstEntry = &entry1;
	    secondEntry = &entry2;
	} else { // oneSlot > twoSlot
	    firstEntry = &entry2;
	    secondEntry = &entry2;
	}
    } else {
        // we only have entry1
	secondEntry = &entry1;
    }
    int firstSlot = (oneSlot > twoSlot ? twoSlot : oneSlot);
    int secondSlot = (oneSlot > twoSlot ? oneSlot : twoSlot);
    bool oneGoesFirst = (firstEntry == &entry1);

    // insert one entry
    hdr1.klen = secondEntry->len(0);
    numEntries++;
    for (k = numEntries-1; k > secondSlot; k--) {
        entries[k] = entries[k-1];
    }
    entries[secondSlot].hdr = (keyrec_t *) &hdr1;
    entries[secondSlot].slot = (oneGoesFirst ? -2 : -1);

    // insert other entry
    if (entry2.size() != 0) {
	hdr2.klen = firstEntry->len(0);
	numEntries++;
	for (k = numEntries-1; k > firstSlot; k--) {
	    entries[k] = entries[k-1];
	}
	entries[firstSlot].hdr = (keyrec_t *) &hdr2;
	entries[firstSlot].slot = (oneGoesFirst ? -1 : -2);
    }
}


static int
int_size(const void *i)
{
    return sizeof(int);
}

static void
int_negInfty(void *i)
{
    // can't use assignment, i might not be aligned properly
    int min = MININT;
    (void) memcpy(i, (void *) &min, sizeof(min));
}


static int
str_size(const void *s)
{
    return strlen((char *) s) + 1;
}

static void
str_negInfty(void *s)
{
    *((char *) s) = '\0';
}




alex_ext_t alex_ext(gist_ext_t::alex_ext_id, "alex_ext",
    gist_support::printIntBtPred, gist_support::printInt,
    gist_support::parseInt, gist_support::parseInt,
    gist_support::parseIntQuery, int_cmp, int_cmp,
    int_size, int_size, int_negInfty, int_negInfty);
