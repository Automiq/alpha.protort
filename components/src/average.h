#ifndef AVERAGE_H
#define AVERAGE_H

#include "component.h"
#include "reduce.h"

namespace alpha {
namespace protort {
namespace components {

float do_average(std::vector<float> &v)
{
    float sum;
    for(float val: v)
        sum += val;
    return sum / v.size();
}

using average = reduce<do_average>;

} // namespace components
} // namespace protort
} // namespace alpha

#endif // MAX_H
