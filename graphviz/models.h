/* This file is identical to the file models.h provided in 
   directory PNL/high/DEMO of PNL Release 1.0
*/

#ifndef __MODELS_H
#define __MODELS_H
#include "pnlHigh.hpp"

PNLW_BEGIN
BayesNet* AsiaModel();
LIMID *PigsModel();
BayesNet* ScalarGaussianBNetModel();
BayesNet *WasteModel();
DBN* KjaerulfsBNetModel();
BayesNet* RPSModel();
BayesNet* CropModel();
BayesNet* FraudModel();
DBN* DBNModel();

PNLW_END
#endif
