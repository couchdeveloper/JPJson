//
//  bench.hpp
//  Bench
//
//  Created by Andreas Grosam on 22.03.13.
//
//

#ifndef Bench_bench_hpp
#define Bench_bench_hpp

#include <algorithm>
#include <chrono>
#include <sstream>

namespace test {

    template <typename Duration>
    std::string formatted_duration(Duration value)
    {
        std::stringstream ss;
        using std::chrono::duration_cast;
        using std::chrono::hours;
        using std::chrono::minutes;
        using std::chrono::seconds;
        using std::chrono::milliseconds;
        using std::chrono::microseconds;
        using std::chrono::nanoseconds;
        
        if (duration_cast<nanoseconds>(value).count() < 2000) {
            ss <<  duration_cast<nanoseconds>(value).count() << " ns";
        }
        else if (duration_cast<microseconds>(value).count() < 2000) {
            ss <<  duration_cast<nanoseconds>(value).count()/1000.0 << " µs";
        }
        else if (duration_cast<milliseconds>(value).count() < 2000) {
            ss <<  duration_cast<microseconds>(value).count()/1000.0 << " ms";
        }
        else if (duration_cast<seconds>(value).count() < 2000) {
            ss <<  duration_cast<milliseconds>(value).count()/1000.0 << " s";
        }
        else {
            ss <<  duration_cast<minutes>(value).count() << " min";
        }
        return ss.str();
    }
    
    
    
    template <typename Derived, int N = 1000, typename Timer = std::chrono::high_resolution_clock>
    class bench_base
    {
    public:
        typedef Timer timer;
        typedef typename Timer::duration duration;
                
        Derived& derived()              { return static_cast<Derived&>(*this); }
        const Derived& derived() const  { return static_cast<const Derived&>(*this); }
        
        template <typename... Args>
        void        prepare(Args&&... args)  { this->derived().prepare_imp(std::forward<Args>(args)...); }
        void        teardown()  { this->derived().teardown_imp(); }
        duration    bench()    { return this->derived().bench_imp(); }
        void        report(duration min, duration max, duration tot, std::size_t n)   { return this->derived().report_imp(min, max, tot, n); }
        
        template <typename... Args>
        void run(Args&&... args)
        {
            prepare(std::forward<Args>(args)...);
            for (int k = 0; k < N; ++k)
            {
                duration t = bench();
                t_min_ = std::min(t_min_, t);
                t_max_ = std::max(t_max_, t);
                t_tot_ += t;
            }
            teardown();
            report(t_min_, t_max_, t_tot_, N);
        }
        
    private:
        duration t_min_ = duration::max();
        duration t_max_ = duration::zero();
        duration t_tot_ = duration::zero();
    };


}
    
    
#endif
