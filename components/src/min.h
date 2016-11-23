#ifndef MIN_H
#define MIN_H

#include "component.h"
#include "reduce.h"

namespace alpha {
namespace protort {
namespace components {

float do_min(std::vector<float> &v)
{
    float min;
    min = v[0];
    for(float val: v)
        min = val < min ? val : min;
    return min;
}

using min = reduce<do_min>;

} // namespace components
} // namespace protort
} // namespace alpha

#endif // MIN_H
