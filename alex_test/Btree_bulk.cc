#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include "gist.h"
#include "gist_btree.h"
#include "gist_extensions.h"

using namespace std;

const char INDEXFILENAME[] = "btree-index";
static int k = 10000;
static int io = 100;

// 批量加载上下文结构
struct BulkLoadContext {
    const vector<int>& keys;
    const vector<int>& datas;
    size_t cursor;
};

// 批量加载数据流回调函数
static rc_t BulkLoadStream(
    void* key, int& klen,
    void* data, int& dlen,
    shpid_t& child,
    void* param) 
{
    BulkLoadContext* ctx = static_cast<BulkLoadContext*>(param);
    
    if (ctx->cursor >= ctx->keys.size()) {
        return eEOF;
    }

    // 填充键值对
    *static_cast<int*>(key) = ctx->keys[ctx->cursor];
    klen = sizeof(int);

    *static_cast<int*>(data) = ctx->datas[ctx->cursor];
    dlen = sizeof(int);

    child = 0; // 叶节点层级
    
    ctx->cursor++;
    
    return RCOK;
}

static rc_t ReadDataFile(const char * filename, vector<int> &keys, vector<int> &datas){
    ifstream fin(filename);
    if (!fin) {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }
    int number;
    fin >> number;
    if (fin.fail()) {
        cerr << "Error reading data count from file." << endl;
        return 1;
    }
    int key, data;
    for(int i = 0; i < number; i++){
        fin >> key >> data;
        keys.push_back(key);
        datas.push_back(data);
    }
    if (keys.size() != number) {
        cerr << "Warning: Expected " << number << " records, but got " << keys.size() << endl;
    }
    fin.close();
    return 0;
}

static rc_t ReadQueryFile(const char * filename, vector<string> &queries) {
    ifstream fin(filename);
    if (!fin) {
        cerr << "Error opening query file: " << filename << endl;
        return 1;
    }
    
    string s;
    while(getline(fin, s)) {
        if (!s.empty()) {
            queries.push_back(s);
        }
    }
    
    fin.close();
    return 0;
}

static void printDatum(void* key, int klen, void* data, int dlen, gist_ext_t* ext) {
    vec_t keyv(key, klen);
    vec_t datav(data, dlen);
    ext->printPred(cout, keyv, 1);
    cout << ": ";
    ext->printData(cout, datav);
}

static rc_t runQuery(gist& index, const gist_query_t* query, int k, int io, int& numRetrieved) {
    cout << "Running query..." << endl;
    gist_cursor_t cursor;
    cout << "Initializing cursor..." << endl;
    if (index.fetch_init(cursor, query, k, io) != RCOK) {
        cout << "Can't initialize cursor" << endl;
        return eERROR;
    }
    cout << "Cursor initialized" << endl;
    gist_ext_t* ext = index.extension();
    bool eof = false;
    int key, data;
    smsize_t keysz = sizeof(int);
    smsize_t datasz = sizeof(int);
    int cnt = 0;
    cout << "Fetching data..." << endl;
    for (;;) {
        if (index.fetch(cursor, &key, keysz, &data, datasz, eof) != RCOK) {
            cerr << "Fetch error" << endl;
            return eERROR;
        }
        if (eof) break;
        
        printDatum(&key, keysz, &data, datasz, ext);
        cout << endl;
        cnt++;
    }
    
    numRetrieved = cnt;
    return RCOK;
}

static rc_t Select(gist& index, const char* qstr, int k, int io, int& numRetrieved) {
    gist_ext_t* ext = index.extension();
    gist_query_t* query;
    
    if (ext->parseQuery(qstr, query) != RCOK) {
        cerr << "Query parse error: " << qstr << endl;
        return 1;
    }
   
    rc_t ret = runQuery(index, query, k, io, numRetrieved);
    cout << "[INFO] Retrieved " << numRetrieved << " items" << endl;
    return ret;
}

int main() {
    cout << "=== GIST B+Tree Bulk Loading Test ===" << endl;
    gist index;
    // 检查索引文件是否存在
    ifstream indexfile(INDEXFILENAME);
    if (!indexfile.good()) {
        cout << "\nCreating new B+Tree with bulk loading..." << endl;
        
        // 读取并预处理数据
        vector<int> keys, datas;
        if (ReadDataFile("data/btreedata.txt", keys, datas) != 0) {
            cerr << "Aborting due to data load error" << endl;
            return 1;
        }
        
        cout << "\nLoaded " << keys.size() << " key-value pairs" << endl;
        
        // 初始化批量加载上下文
        BulkLoadContext ctx{keys, datas, 0};
        
        // 创建索引（批量加载）
        cout << "\nStarting bulk load process..." << endl;
        clock_t start = clock();
        rc_t status = index.create(
            INDEXFILENAME,
            &bt_int_ext,
            BulkLoadStream,
            &ctx,
            0.75f  // 填充因子
        );
        
        if (status != RCOK) {
            cerr << "\nBulk load failed with error code: " << status << endl;
            return 1;
        }
        
        clock_t end = clock();
        cout << "[SUCCESS] Bulk load completed in "
             << double(end - start)/CLOCKS_PER_SEC << " seconds" << endl;
        
        // 生成索引结构dump
        ofstream fout("dump/insert.dump");
        index.dump(fout, 0);
        fout.close();
    } 
    else {
        cout << "\nOpening existing index file" << endl;
        if (index.open(INDEXFILENAME) != RCOK) {
            cerr << "Failed to open index file" << endl;
            return 1;
        }
    }

    // 执行查询
    vector<string> queries;
    if (ReadQueryFile("data/btreequery.txt", queries) != 0) {
        cerr << "Query load failed" << endl;
        return 1;
    }
    
    cout << "\n\n=== Executing Queries ===" << endl;
    int queryNum = 1;
    for (const auto& q : queries) {
        cout << "\n【Query " << queryNum++ << "】" << q << endl;
        int numResults = 0;
        clock_t start = clock();
        
        if (Select(index, q.c_str(), 20, io, numResults) != RCOK) {
            cerr << "Query execution failed" << endl;
        }
        
        clock_t end = clock();
        cout << "[STAT] Execution time: " 
             << double(end - start)/CLOCKS_PER_SEC << "s" << endl;
        cout << "══════════════════════════════════════" << endl;
    }

    return 0;
}