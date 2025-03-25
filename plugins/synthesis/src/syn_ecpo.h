#ifndef FAUDES_ECPO_H
#define FAUDES_ECPO_H

#include "corefaudes.h"
#include <stack>

namespace faudes {

// 声明监督控制函数（根据你的代码推测参数类型）
extern FAUDES_API void applyNaturalProjection(const vGenerator& inputGen, const EventSet& obsEvents, vGenerator& projGen) ;

} // namespace faudes

#endif // FAUDES_ECPO_H
