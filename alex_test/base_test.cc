#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include "../include/alex_base.h" 

using namespace alex;
using namespace std;

int main() {
    
    LinearModel<int> model(2.0, 10.0); // y = 2x + 10
    cout << "线性模型: y = " << model.a_ << "x + " << model.b_ << endl;
    
    
    int test_key = 5;
    cout << "预测 key=" << test_key << " 的位置: " << model.predict(test_key) << endl;
    cout << "预测 key=" << test_key << " 的浮点位置: " << model.predict_double(test_key) << endl;
    
    
    cout << "\n===== 测试线性模型构建器 =====" << endl;

    
    vector<pair<int, int>> training_data;
    
   
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> noise(-2, 2);
    
    for (int x = 0; x < 100; x += 5) {
        int y = 3 * x + 5 + noise(gen);
        training_data.push_back({x, y});
    }
    
    // 使用模型构建器创建模型
    LinearModel<int> trained_model;
    LinearModelBuilder<int> builder(&trained_model);
    
    // 添加训练数据
    for (const auto& [x, y] : training_data) {
        builder.add(x, y);
    }
    
    // 构建模型
    builder.build();
    cout << "训练后的线性模型: y = " << trained_model.a_ << "x + " << trained_model.b_ << endl;
    
    // 3. 测试位图操作
    cout << "\n===== 测试位图操作 =====" << endl;
    
    uint64_t bitmap = 0b010100100;
    cout << "原始位图: " << bitset<9>(bitmap) << endl;
    cout << "最右侧的1: " << bitset<9>(extract_rightmost_one(bitmap)) << endl;
    cout << "移除最右侧的1: " << bitset<9>(remove_rightmost_one(bitmap)) << endl;
    cout << "1的数量: " << count_ones(bitmap) << endl;
    
    // 4. 测试统计收集器
    cout << "\n===== 测试统计收集器 =====" << endl;
    
    // 创建搜索迭代收集器
    ExpectedSearchIterationsAccumulator search_acc;
    
    // 创建移位收集器
    ExpectedShiftsAccumulator shifts_acc(100);
    
    // 创建组合收集器
    ExpectedIterationsAndShiftsAccumulator combined_acc(100);
    
    // 添加一些数据点（实际位置和预测位置）
    vector<pair<int, int>> test_positions = {
        {10, 12}, {20, 18}, {30, 31}, {40, 35}, {50, 49}
    };
    
    for (const auto& [actual, predicted] : test_positions) {
        search_acc.accumulate(actual, predicted);
        shifts_acc.accumulate(actual, predicted);
        combined_acc.accumulate(actual, predicted);
    }
    
    cout << "预期搜索迭代次数: " << search_acc.get_stat() << endl;
    cout << "预期移位操作次数: " << shifts_acc.get_stat() << endl;
    cout << "组合收集器 - 搜索迭代次数: " << combined_acc.get_expected_num_search_iterations() << endl;
    cout << "组合收集器 - 移位操作次数: " << combined_acc.get_expected_num_shifts() << endl;
    
    return 0;
}