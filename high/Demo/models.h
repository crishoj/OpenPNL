#ifndef __MODELS_H
#define __MODELS_H
#include "pnlHigh.hpp"

PNLW_BEGIN
BayesNet* AsiaModel();
LIMID *PigsModel();
BayesNet* ScalarGaussianBNetModel();
BayesNet *WasteModel();
BayesNet* KjaerulfsBNetModel();

PNLW_END
#endif