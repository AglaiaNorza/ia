#pragma once
#include "Encoder.hpp"

class Problem {
public:
    virtual ~Problem() = default;
    
    /**
     * @param tId index of the current thread
     * @param tCount number of threads running
     */
    virtual void generateConstraints(Encoder& encoder, EncodingContext& ctx, 
                                     int tId, int tCount) = 0;
};
