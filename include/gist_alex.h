// gist_alex.h


#ifdef __GNUG__
#pragma interface "gist_alex.h"
#endif

#ifndef GIST_ALEX_H
#define GIST_ALEX_H

#include "gist_p.h"		// for keyrec_t
#include "gist_query.h"		// for gist_query_t
#include "gist_ext.h"		// for gist_ext_t
// #include "alex.h"
// // VCPORT_B
// #ifndef WIN32
// class ostream;
// #endif
// // VCPORT_E
#include <iostream>
#include "alex_base.h"
// #include <alex.h>
using namespace std; 

class gist_p;
class vec_t;
class gist_cursorext_t;

struct DataNodeStats {
    double num_search_iterations = 0; // 平均搜索迭代次数
    double num_shifts = 0;            // 平均移动次数
};

// Used when stats are computed using a sample
struct SampleDataNodeStats {
    double log2_sample_size = 0;
    double num_search_iterations = 0;
    double log2_num_shifts = 0;
};

struct DerivedParamss {
    // The defaults here assume the default max node size of 16MB
    int max_fanout = 1 << 21; // assumes 8-byte pointers
    int max_data_node_slots = (1 << 18) / sizeof(int);
  };


class alex_query_t : public gist_query_t {
public:

    enum bt_oper {
        bt_nooper,
	bt_eq,
	bt_lt,
	bt_le,
	bt_gt,
	bt_ge,
	bt_betw, // SQL BETWEEN operator
	bt_numoper
    };

    // val1 and val2 must have been allocated with new()
    alex_query_t(bt_oper oper, void *val1, void *val2);
    // delete()s val1 and val2
    ~alex_query_t();

    bt_oper oper;
    void *val1;
    void *val2; // only used as upper interval bound for bt_betw
};


/*
 * alex_ext_t:
 *
 * Alex extension class. Leaf items are ordered on the page
 * according to their key/data values (data resolves tie of key order,
 * perfect duplicates not allowed). BPs are stored as the inclusive
 * left boundary of the subtree interval (it's the x of [x, y), and y
 * is the left boundary of the right neighbor BP). Each BP contains
 * the key and data of it's leftmost leaf item. The exceptions are the
 * BPs of the leftmost nodes at each level, which contain (-\infty,
 * -\infty).  */

class alex_ext_t : public gist_ext_t {
public:

    // generic comparison function
    typedef int (*CmpFct)(const void *a, const void *b);
    CmpFct keyCmp; // for keys
    CmpFct dataCmp; // for data pointers

    // returns the size in bytes of a key stored on a disk page;
    // the goal is to get to the data pointer portion of an internal entry,
    // given the pointer to the key portion
    typedef int (*SizeFct)(const void *e);
    SizeFct keySize, dataSize;

    // for printPred()
    typedef void (*PrintFct)(ostream& s, const void *pred);
    PrintFct prPred, prData;

    // for pickSplit() (creating new BPs)
    typedef void (*NegInftyFct)(void *x);
    NegInftyFct negInftyKey, negInftyData;

    struct Params {
        // When bulk loading, Alex can use provided knowledge of the expected
        // fraction of operations that will be inserts
        // For simplicity, operations are either point lookups ("reads") or inserts
        // ("writes)
        // i.e., 0 means we expect a read-only workload, 1 means write-only
        double expected_insert_frac = 1;
        // Maximum node size, in bytes. By default, 16MB.
        // Higher values result in better average throughput, but worse tail/max
        // insert latency
        int max_node_size = 1 << 24;
        // Approximate model computation: bulk load faster by using sampling to
        // train models
        bool approximate_model_computation = true;
        // Approximate cost computation: bulk load faster by using sampling to
        // compute cost
        bool approximate_cost_computation = false;
      };
      // 后置下划线: 表明 params_ 是类 Alex 的成员变量
      Params params_;

    alex_ext_t(
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
	NegInftyFct negInftyData);

	// 添加bulk_load
	// values should be the sorted array of key-payload pairs.
  	// The number of elements should be num_keys.
  	// The index must be empty when calling this method.
	// void bulk_load(const int values, int num_keys);
    rc_t bulk_load(gist_p& page,const vector<int>& keys, const vector<int>& values);
	// void bulk_load(int num_keys) {}
    rc_t bulk_load_node(const std::pair<int,int>* values, int num_keys, gist_p& page,
        int total_keys, PhyscialAddr *r_addr, uint8_t duplication_factor_,
        double expected_avg_exp_search_iterations_, double expected_avg_shifts_,
        const alex::LinearModel<int>* data_node_model = nullptr);

    rc_t insert(
        gist_p& page,
	const vec_t& key,
	const vec_t& data,
	shpid_t child);

    rc_t remove(
        gist_p& page,
	const int slots[],
	int numSlots);

    rc_t updateKey(
        gist_p& page,
	int& slot,
	const vec_t& newKey);

    void findMinPen(
        const gist_p& page,
	const vec_t& key,
	const vec_t& data,
	int& slot);

    void search(
        gist_p& page,
	const gist_query_t* query,
	int matches[],
	int& numMatches);

    void getKey(
        const gist_p& page,
	int slot,
	vec_t& key);

    rc_t pickSplit(
        gist_p& page,
	int rightEntries[],
	int& numRight,
	const vec_t& oldBp,
	vec_t& leftBp,
	vec_t& rightBp,
	const vec_t& entry1,
	bool& oneGoesRight,
	const vec_t& entry2,
	bool& twoGoesRight);

    void unionBp(
        const gist_p& page, // in
	vec_t& bp, // in/out
	bool bpIsValid, // in
	const vec_t& pred1, // in
	const vec_t& pred2, // in
	bool& bpChanged); // out

    gist_cursorext_t* queryCursor(
	const gist_query_t* query) const;

    bool check(
	const vec_t& bp,
	const vec_t& pred,
	int level);

    // add compute_expected_cost
    static double compute_expected_cost( const std::pair<int,int>* values,
        int num_keys,
        double density,
        double expected_insert_frac,
        const  alex::LinearModel<int>* existing_model = nullptr, bool use_sampling = false,
        DataNodeStats* stats = nullptr);

    static void build_model(const std::pair<int,int>* values, int num_keys, alex::LinearModel<int>* model,
        bool use_sampling = false);
    static void build_model_sampling (const std::pair<int,int>* values, int num_keys,
        alex::LinearModel<int>* model,bool verbose = false);
    // Helper function for compute_expected_cost
  // Implicitly build the data node in order to collect the stats
  static void build_node_implicit(const std::pair<int,int>* values, int num_keys,
    int data_capacity, alex::StatAccumulator* acc,
    const alex::LinearModel<int>* model);

static double compute_expected_cost_sampling(
    const std::pair<int,int>* values, int num_keys, double density,
        double expected_insert_frac,
        const alex::LinearModel<int>* existing_model = nullptr,
        DataNodeStats* stats = nullptr);
static void build_node_implicit_sampling(const std::pair<int,int>* values,int num_keys,
    int sample_num_keys,
    int sample_data_capacity,
    int step_size, alex::StatAccumulator* ent,
    const alex::LinearModel<int>* sample_model);

private:

    // Finds the first slot on the page with an equal key (and data, if 'data'
    // is valid), or, if none should exist, the rightmost item with a smaller key
    // (or data, if !keyOnly).
    int _binSearch(
        const gist_p& page,
	const void* key,
	const void* data,
	bool keyOnly);


    struct Stats {
        // 构造函数：在初始化列表中完成所有成员变量初始化
        Stats()
            : num_keys(0),
              num_model_nodes(0),
              num_data_nodes(0),
              num_expand_and_scales(0),
              num_expand_and_retrains(0),
              num_downward_splits(0),
              num_sideways_splits(0),
              num_model_node_expansions(0),
              num_model_node_splits(0),
              num_downward_split_keys(0),
              num_sideways_split_keys(0),
              num_model_node_expansion_pointers(0),
              num_model_node_split_pointers(0),
              num_node_lookups(0),
              num_lookups(0),
              num_inserts(0),
              splitting_time(0),
              cost_computation_time(0) {}
    
        int num_keys;
        int num_model_nodes;
        int num_data_nodes;
        int num_expand_and_scales;
        int num_expand_and_retrains;
        int num_downward_splits;
        int num_sideways_splits;
        int num_model_node_expansions;
        int num_model_node_splits;
        long long num_downward_split_keys;
        long long num_sideways_split_keys;
        long long num_model_node_expansion_pointers;
        long long num_model_node_split_pointers;
        mutable long long num_node_lookups;
        mutable long long num_lookups;
        long long num_inserts;
        double splitting_time;
        double cost_computation_time;
    };

    Stats stats_;
    

    struct PosInfo {
        const keyrec_t* hdr;
	int slot; // slot index on page; or: -1: entry1, -2: entry2
    };

    void _loadPosInfo(
        gist_p& page,
	const vec_t& entry1,
	const vec_t& entry2,
	keyrec_t::hdr_s& hdr1,
	keyrec_t::hdr_s& hdr2,
	PosInfo entries[]);
};

extern alex_ext_t alex_ext;

#endif
