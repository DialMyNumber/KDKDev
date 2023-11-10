#include "AABB.h"

#include <float.h>
#include <algorithm>

namespace chag
{

    Aabb make_aabb(const glm::vec3 &min, const glm::vec3 &max)
    {
        Aabb result = { min, max };
        return result;
    }



    Aabb make_inverse_extreme_aabb()
    {
        return make_aabb(glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX), glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX));
    }



    Aabb make_aabb(const glm::vec3 *positions, const size_t numPositions)
    {
        Aabb result = make_inverse_extreme_aabb();

        for (size_t i = 0; i < numPositions; ++i)
        {
            result = combine(result, positions[i]);
        }

        return result;
    }


#if 0
    Aabb operator * (const glm::mat4x4 &tfm, const Aabb &a)
    {
        Aabb result = { tfm[3], tfm[3] };

        for (int i = 1; i < 4; ++i)
        {
            for (int j = 1; j < 4; ++j)
            {
                float e = tfm(i, j) * a.min[j - 1];
                float f = tfm(i, j) * a.max[j - 1];
                if (e < f)
                {
                    result.min[i - 1] += e;
                    result.max[i - 1] += f;
                }
                else
                {
                    result.min[i - 1] += f;
                    result.max[i - 1] += e;
                }
            }
        }
        return result;
    }
#endif


} // namespace chag
