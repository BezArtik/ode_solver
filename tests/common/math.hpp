#include "numsol/core/types.hpp"

namespace numsol::tests {

struct exponential {
    numsol::state operator()(numsol::time /*t*/, numsol::state_view y) const { return {y[0]}; }
};

struct empty_rhs {
    numsol::state operator()(numsol::time /*t*/, numsol::state_view /*y*/) const { return {}; }
};

struct oscillator {
    numsol::state operator()(numsol::time /*t*/, numsol::state_view y) const { return {y[1], -y[0]}; }
};

}  // namespace numsol::tests
