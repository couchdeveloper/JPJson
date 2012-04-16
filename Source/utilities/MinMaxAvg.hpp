//
//  MinMaxAvg.hpp
//  Bench
//
//  Created by Andreas Grosam on 4/2/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef BENCH_MINMAXAVG_HPP
#define BENCH_MINMAXAVG_HPP


#include <algorithm>

namespace utilities {

    template <typename T>
    class MinMaxAvg {
    public:  
        MinMaxAvg() : count_(0), min_(0), max_(0), sum_(0) {};
        T min() const { return min_; }
        T max() const { return max_; }
        double avg() const { return count_ ? sum_/count_ : 0; }
        
        void set(const T& v) {
            if (count_ == 0) {
                sum_ = v;
                min_ = v;
                max_ = v;
            } else {
                sum_ += v;
                min_ = std::min(v, min_);
                max_ = std::max(v, max_);
            }
            ++count_;
        }
        
    private:
        size_t count_;
        T min_;
        T max_;
        T sum_;
    };


    
}
#endif
