#include <vector>

namespace KEPLER_FORMAL {
class BoolExpr;

class MiterStrategy {
 public:
  MiterStrategy();

 private:
  std::vector<BoolExpr> POs_;
};

}  // namespace KEPLER_FORMAL