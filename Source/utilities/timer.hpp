//
//  timer.cpp
//  
//
//  Created by Andreas Grosam on 8/25/11.
//  Copyright 2011 Andreas Grosam
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifndef UTILITIES_TIMER_HPP
#define UTILITIES_TIMER_HPP


#include <mach/mach_time.h>
#include <time.h>
#include <stdio.h>


namespace utilities {
    class timer 
    {
    public:
        typedef uint64_t value_type;
        
        timer() : _t0 (-1), _t1(-1), _t (0), _correction (timer::s_correction) {
            mach_timebase_info_data_t tbinfo;
            ::mach_timebase_info(&tbinfo);
            _k = ((long double) tbinfo.numer) / ((long double) tbinfo.denom);
            if (_correction == 0) {
                _correction = evaluateCorrection();
            }
        }
        
        void start() {
            _t1 = 0;
            _t0 = mach_absolute_time();
        }
        void pause() {
            stop();
            _t += _t1 - _t0;
        }
        
        void stop() {
            _t1 = mach_absolute_time();
            if (_t0 == 0)
                _t1 = 0;
            else {
                if (_t1 - _t0 > _correction)
                    _t1 = _t1 - _correction;
                else
                    _t1 = _t0;
            }
        }
        
        void reset() {
            _t0 = _t1 = -1; 
            _t = 0;
        }
        
        uint64_t nanoSeconds() const {
            if (_t1)
                return (_t1 - _t0 + _t) * _k;
            else 
                return (mach_absolute_time() - _t0 + _t) * _k;
        }
        
        double  seconds() const {
            return nanoSeconds() * 1e-9;
        }
        
    private:    
        
        uint64_t evaluateCorrection() {
            uint64_t c = 0;
            for (int i = 0; i < 1000; ++i)
            {
                start();
                stop();
                c += nanoSeconds();
            }
            timer::s_correction = c / 1000;
            return timer::s_correction;
        }
        
    private:
        uint64_t _t0;  // start time
        uint64_t _t1;  // stop time
        uint64_t _t;   // total time
        long double _k;
        uint64_t _correction;   
        static uint64_t s_correction;
    };
    
    
}


#endif  // UTILITIES_TIMER_HPP
