#ifndef MAX_H
#define MAX_H

#include "component.h"
#include "reduce.h"

namespace alpha {
namespace protort {
namespace components {

float do_max(std::vector<float> &v)
{
    float max;
    max = v[0];
    for(float val: v)
        max = val > max ? val : max;
    return max;
}

using max = reduce<do_max>;

} // namespace components
} // namespace protort
} // namespace alpha

#endif // MAX_H
