// gist_alex.h


#ifdef __GNUG__
#pragma interface "gist_alex.h"
#endif

#ifndef GIST_ALEX_H
#define GIST_ALEX_H

#include "gist_p.h"		// for keyrec_t
#include "gist_query.h"		// for gist_query_t

// // VCPORT_B
// #ifndef WIN32
// class ostream;
// #endif
// // VCPORT_E
#include <iostream>
using namespace std; 

class gist_p;
class vec_t;
class gist_cursorext_t;

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
	void bulk_load(const int values, int num_keys);
	// void bulk_load(int num_keys) {}

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

private:

    // Finds the first slot on the page with an equal key (and data, if 'data'
    // is valid), or, if none should exist, the rightmost item with a smaller key
    // (or data, if !keyOnly).
    int _binSearch(
        const gist_p& page,
	const void* key,
	const void* data,
	bool keyOnly);


   /* Statistics related to the key domain.
   * The index can hold keys outside the domain, but lookups/inserts on those
   * keys will be inefficient.
   * If enough keys fall outside the key domain, then we expand the key domain.
   */
	// struct InternalStats {
	// 	T key_domain_min_ = std::numeric_limits<T>::max();   // 当前键域的最小值
	// 	T key_domain_max_ = std::numeric_limits<T>::lowest(); // 当前键域的最大值
	// 	int num_keys_above_key_domain = 0;                    // 超出右侧键域的键数量
	// 	int num_keys_below_key_domain = 0;                    // 超出左侧键域的键数量
	// 	int num_keys_at_last_right_domain_resize = 0;         // 上次向右扩展时的总键数
	// 	int num_keys_at_last_left_domain_resize = 0;          // 上次向左扩展时的总键数
	// 	T min_key_in_left = std::numeric_limits<T>::max();    // 左侧扩展后的新最小值
	// 	T max_key_in_right = std::numeric_limits<T>::lowest();// 右侧扩展后的新最大值
	// };

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
