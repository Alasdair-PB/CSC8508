//
// Contributors: Alfie
//

#include <random>

namespace Util {
    template <typename T>
    std::vector<T> RandomiseVector(std::vector<T> const& inVector) {
        static std::random_device rd;
        static std::default_random_engine rng(rd());
        std::vector<T> out(inVector);
        std::ranges::shuffle(out, rng);
        return out;
    }
}
