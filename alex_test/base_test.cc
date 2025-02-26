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
    
    typedef std::vector<std::pair<int, int> >::const_iterator TrainingIterator;
    for (TrainingIterator it = training_data.begin(); it != training_data.end(); ++it) {
        int x = it->first;
        int y = it->second;
        builder.add(x, y);
    }
    
    builder.build();
    cout << "训练后的线性模型: y = " << trained_model.a_ << "x + " << trained_model.b_ << endl;
    
    return 0;
}