/* Version 1.01.  Mar 11, 2006 */
/* For documentation and updates go to www.DataOnStage.com. */

/* This file is derived from the file models.h provided in 
   directory PNL/high/DEMO of PNL Release 1.0.
   Several functions and comments were added and some corrections were made.
*/

#ifndef __MODELS_H
#define __MODELS_H
#include "pnlHigh.hpp"

PNLW_BEGIN


//--------------------------------------------------------------------
// Models to demonstrate link and connection strengths
//--------------------------------------------------------------------

// Two Node network with parameters for probabilities
BayesNet* TwoNode_example( double a, double b1, double b2 );

// Example of strong/weak link strengths from Figure 1 of manual.
BayesNet* Strong_Weak_example();

// Example with three nodes with different numbers of states: 4, 2, 3
BayesNet* States_4_2_3_example();

// OR function (deterministic function):      D = A or B or C
BayesNet* Deterministic_OR_model();

// Max function (deterministic function):     D = max(A,B).
BayesNet* Deterministic_MAX_model();

//--------------------------------------------------------------------
//  Models from PNLHigh distribution (from PNL/high/DEMO/models.h):
//--------------------------------------------------------------------

BayesNet* AsiaModelCorrected();  // Values corrected for P(Bronchitis|Smoking)
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
