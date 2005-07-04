#ifndef __EXAMPLES_H
#define __EXAMPLES_H
#include "pnlHigh.hpp"

PNLW_BEGIN

std::ostream &operator<<(std::ostream &str, const TokArr &ta);

void AsiaDemo();
void PigsDemo();
void ScalarGaussianBNetDemo();
void WasteDemo();
void KjaerulfsBNetDemo();
void RPSDemo();
void CropDemo();
void Structural();
void FraudDemo();
void DBNDemo();

PNLW_END
#endif