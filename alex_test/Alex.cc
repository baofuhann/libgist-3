#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include "gist.h"
#include "gist_alex.h"
#include "gist_extensions.h"

using namespace std;

const char INDEXFILENAME[] = "btree-index";
static int k = 10000;
static int io = 100;

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

static rc_t ReadQueryFile(const char * filename, vector<string> &queries){
    ifstream fin(filename);
    string s;
    while(getline(fin, s)){
      queries.push_back(s);
    }
    fin.close();
    return 0;
  }


static void printDatum(void* key, int klen, void* data, int dlen, gist_ext_t* ext){
    vec_t keyv(key, klen);
    vec_t datav(data, dlen);
    ext->printPred(cout, keyv, 1);
    cout << ": ";
    ext->printData(cout, datav);
}

static rc_t runQuery(gist& index, const gist_query_t* query, int k, int io, int& numRetrieved) {
    gist_cursor_t cursor;
    if (index.fetch_init(cursor, query, k, io) != RCOK) {
        cout << "can't initialize cursor" << endl;
        return(eERROR);
    }
    gist_ext_t* ext = index.extension();

    bool eof = false;
    int key, data;
    smsize_t keysz = sizeof(int);
    smsize_t datasz = sizeof(int);
    int cnt = 0;
    for (;;) {
    if (index.fetch(cursor, reinterpret_cast<void*>(&key), keysz, reinterpret_cast<void*>(&data), datasz, eof) != RCOK) {
            cerr << "Can't fetch from cursor" << endl;
            return(eERROR);
        }
        if (eof) break;
        // print key and data
    printDatum(reinterpret_cast<void*>(&key),keysz, reinterpret_cast<void*>(&data), datasz, ext);
    cout << endl;
        cnt++;
    }
    numRetrieved = cnt;
    return(RCOK);
}

static rc_t Select(gist& index, const char* qstr, int k, int io, int& numRetrieved){
    gist_ext_t* ext = index.extension();
    gist_query_t* query;
    if (ext->parseQuery(qstr, query) != RCOK) {
        cerr << "Error parsing qualification" << endl;
        return 1;
    }

    if (runQuery(index, query, k, io, numRetrieved) != RCOK) {
        return 1;
    }
    cout << "[INFO] Retrieved " << numRetrieved << " items" << endl;
}



// 适用于 B-Tree（整数索引）
static rc_t Insert(gist& index, int key, int data) {
    /*
    * 调用gist.cc中的insert函数 
    * rc_t insert(const void* key, int klen, const void* data, int dlen);
    * gist.insert 内部调用了 gist_btree.cc (gist扩展类型)中的insert函数
    */

    return index.insert((void*)&key, sizeof(int), (void*)&data, sizeof(int));
}

int main(){
  cout << "Test libgist for ALEX" << endl;
  gist index;
  ifstream indexfile(INDEXFILENAME);
  if(!indexfile.good()){
    rc_t status = index.create(INDEXFILENAME, &alex_ext,"data/btreedata.txt");
    if (status != RCOK) {
      cerr << "Error creating " << INDEXFILENAME << endl;
      return 1;
    }

    // 读取数据文件
    vector<int> keys, datas;
    rc_t status_file = ReadDataFile("data/btreedata.txt", keys, datas);
    if (status != 0) {
        cerr << "Failed to read data file. Exiting..." << endl;
        return 1;
    }
    cout << "===== 键值列表 =====" << endl;
    cout << "总计读取 " << keys.size() << " 个键" << endl;
    for (size_t i = 0; i < keys.size(); i++) {
        cout << "keys[" << i << "] = " << keys[i] << endl;
    }

    cout << endl << "===== 数据列表 =====" << endl;
    cout << "总计读取 " << datas.size() << " 个数据项" << endl;
    for (size_t i = 0; i < datas.size(); i++) {
        cout << "datas[" << i << "] = " << datas[i] << endl;
    }

    // 插入数据
    cout << "\n\n=== Insert Operation Start(GIST_ALEX INDEX) ===" << endl;
    clock_t start = clock();
    // printf("Opening GiST database file: %s\n", keys.size());
    for (int i = 0; i < keys.size(); i++) {
        if (Insert(index, keys[i], datas[i]) != RCOK) {
            cerr << "Error inserting key: " << keys[i] << endl;
        }
    }
    clock_t end = clock();
    cout << "[INFO] Insert Time: " << ((double)(end - start)) / CLOCKS_PER_SEC << " seconds" << endl;
    ofstream fout("dump/insert.dump");
    rc_t content = index.dump(fout, 0);
    fout.close();
  }else{
        cout << "Open an existing index file" << endl;
        rc_t status = index.open(INDEXFILENAME);
        if (status != RCOK) {
        cerr << "Error Opening "<< INDEXFILENAME << endl;
        return 1;
        }
    }
    // 读取整数查询
    vector<string> queries;
    ReadQueryFile("data/btreequery.txt", queries);
    // 打印所有查询
    cout << "\n\n**************** Queries Loaded ****************" << endl;
    for (int i = 0; i < queries.size(); i++) {
        cout << "[QUERY] " << queries[i] << endl;
    }
    cout << "\n\n**************** Select Query ****************" << endl;
    int numRetrieved = 0;
    for(int i = 0; i < queries.size(); i++){
        cout << "[INFO] The query is " << queries[i] << endl;
        clock_t start, end;
        start = clock();
        Select(index, queries[i].c_str(), 20, io,numRetrieved);
        end = clock();
        cout << "[INFO] Select Use time: " << ((double)(end - start)) / CLOCKS_PER_SEC << endl;
        cout << "==========================================" << endl;
    }

  return 0;
}