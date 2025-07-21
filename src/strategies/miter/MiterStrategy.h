#include <vector>
#include "DNL.h"
#include "BoolExpr.h"

namespace KEPLER_FORMAL {

class MiterStrategy {
 public:
  MiterStrategy() = default;

  void build();

  const std::vector<BoolExpr>& getPOs() const {
    return POs_;
  }

 private:
 
  std::vector<naja::DNL::DNLID> collectInputs();
  std::vector<naja::DNL::DNLID> collectOutputs();

  std::vector<BoolExpr> POs_;
  std::vector<naja::DNL::DNLID> inputs_;
  std::vector<naja::DNL::DNLID> outputs_;
};

}  // namespace KEPLER_FORMAL