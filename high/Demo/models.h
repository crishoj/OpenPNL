#ifndef __MODELS_H
#define __MODELS_H
#include "pnlHigh.hpp"

PNLW_BEGIN
BayesNet* AsiaModel();
LIMID *PigsModel();
BayesNet* ScalarGaussianBNetModel();
BayesNet *WasteModel();
BayesNet* KjaerulfsBNetModel();
BayesNet* RPSModel();
BayesNet* CropModel();

PNLW_END
#endif