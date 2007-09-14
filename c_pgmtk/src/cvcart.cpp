/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright( C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
//(including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort(including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/****************************************************************************************\

  Generic implementation of Quick Sort algorithm.
  ----------------------------------------------
  It allows to define qsort function for any certain type and
  any certain precedence rule.
  To use it you should define comparsion macro which takes two arguments
  and returns non-zero if first argument should precede the second and zero else.

  Example:

  Suppose that the task is to sort points by ascending of y coordinates and if
  y's are equal x's should ascend.

    The code is:
    ------------------------------------------------------------------------------
	   #define cmp_pts( pt1, pt2 ) \
	       ((pt1).y < (pt2).y || (pt1).y == (pt2).y && (pt1).x < (pt2).x)

	   [static] IPCX_IMPLEMENT_QSORT( icxSortPoints, CxPoint, cmp_pts )
    ------------------------------------------------------------------------------

    This code generates function void icxSortPoints( CxPoint* array, int length )
    which can be used further.

    aux is an additional parameter, which can be used when comparing elements.

\****************************************************************************************/

#if !defined(NDEBUG) && !defined(WIN32)
#define _DEBUG
#endif

#define CX_IMPLEMENT2_QSORT( func_name, T, less_than, user_data_type )  \
void func_name( T* array, int length, user_data_type aux )		\
{									\
    const int bubble_level = 8;						\
									\
    struct								\
    {									\
	int lb, ub;							\
    }									\
    stack[48];								\
									\
    int sp = 0;								\
									\
    T   temp;								\
    T   lb_val;								\
									\
    aux = aux;								\
									\
    stack[0].lb = 0;							\
    stack[0].ub = length - 1;						\
									\
    while( sp >= 0 )							\
    {									\
	int lb = stack[sp].lb;						\
	int ub = stack[sp--].ub;					\
									\
	for(;;)								\
	{								\
	    int diff = ub - lb;						\
	    if( diff < bubble_level )					\
	    {								\
		int i, j;						\
		T* arr = array + lb;					\
									\
		for( i = diff; i > 0; i-- )				\
		{							\
		    int f = 0;						\
		    for( j = 0; j < i; j++ )				\
			if( less_than( arr[j+1], arr[j] ))		\
			{						\
			    temp = arr[j];				\
			    arr[j] = arr[j+1];				\
			    arr[j+1] = temp;				\
			    f = 1;					\
			}						\
		    if( !f ) break;					\
		}							\
		break;							\
	    }								\
	    else							\
	    {								\
		/* select pivot and exchange with 1st element */	\
		int  m = lb + (diff >> 1);				\
		int  i = lb + 1, j = ub;				\
									\
		lb_val = array[m];					\
									\
		array[m]  = array[lb];					\
		array[lb] = lb_val;					\
									\
		/* partition into two segments */			\
		for(;;)							\
		{							\
		    for( ;i < j && less_than(array[i], lb_val); i++ );  \
		    for( ;j >= i && less_than(lb_val, array[j]); j-- ); \
									\
		    if( i >= j ) break;					\
		    temp = array[i];					\
		    array[i++] = array[j];				\
		    array[j--] = temp;					\
		}							\
									\
		/* pivot belongs in A[j] */				\
		array[lb] = array[j];					\
		array[j]  = lb_val;					\
									\
		/* keep processing smallest segment, and stack largest*/\
		if( j - lb <= ub - j )					\
		{							\
		    if( j + 1 < ub )					\
		    {							\
			stack[++sp].lb   = j + 1;			\
			stack[sp].ub = ub;				\
		    }							\
		    ub = j - 1;						\
		}							\
		else							\
		{							\
		    if( j - 1 > lb)					\
		    {							\
			stack[++sp].lb = lb;				\
			stack[sp].ub = j - 1;				\
		    }							\
		    lb = j + 1;						\
		}							\
	    }								\
	}								\
    }									\
}

#define CX_IMPLEMENT_QSORT( func_name, T, less_than )  \
    CX_IMPLEMENT2_QSORT( func_name, T, less_than, int )

#include "cart/cvcart.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>

#define DATA_TO_FILE1 0 // Numeric split resgression
#define DATA_TO_FILE2 0 // Numeric split clasiification
#define DATA_TO_FILE3 0 // Build root fallen idx
#define DATA_TO_FILE4 0 // Propagate subj
#define TEST_MARTVW 0

#ifdef CART_WITH_GUI
HANDLE g_hStopEvent = NULL;
HWND g_hwndMain = NULL;
#endif

#define ENTER_FUNC(x,y)
#define EXIT_FUNC(x)
#define START_FUNC(x)
#define LEAVE_FUNC(x,y,z)
#define FUNC_MEM_WRITE(a,b,c)
#define FUNC_MEM_READ(a,b,c)
#define FUNC_MEM_MODIFY(a,b,c)
#define WRITE_LOG_MESSAGE(x,y)
/***************************************************************************\
		   Classification And Regression Tree

Bibliography:
	Leo Breiman, Jerome Friedman, Richard Olshen, Charles Stone,
		Classification And Regression Trees
	Servane Gey, Elodie Nedelec,
		Model Selection for CART Regression Trees
	Floriana Esposito, Donato Malerba, Giovanni Semerara,
		A Comparative Analysis of Methods for Pruning Decision Trees
	Rajeev Rastogi, Kyuseok Shim,
		PUBLIC: A Decision Tree Classifier that Integrates
		Building and Pruning
	Luis Fernando Rainho Alves Torgo,
		Inductive Learning of Tree Based Regression Models, PhD Thesis
	Salford Systems booklet
		Critical Features of High Perfomance Decision Trees

Contributed and maintained by Denis Lagno <dlagno@msu.nstl.nnov.ru>
					  <dlagno@hotmail.com>
\***************************************************************************/

#define CX_CART_MAKE_LOCAL( T, PTR, VAR ) T VAR = PTR->VAR

CART_IMPL CxClassifierSampleChunk *
cxGetClassifierSampleChunk( CxClassifierSample *sample, int index )
{
    CV_FUNCNAME( "cxGetClassifierSampleChunk" );
    __BEGIN__;
    if ( sample == 0 )
    {
	CV_ERROR( CV_StsNullPtr, "null sample pointer" );
    }
    if ( index >= sample->num_chunks )
    {
	CV_ERROR( CV_StsBadArg, "index out of bounds" );
    }
    return sample->chunk[index]->is_loaded
	? sample->chunk[index] : (*sample->load_chunk)( sample, index );

    __CLEANUP__;
    __END__;
}

CART_IMPL CxClassifierSampleChunk *
cxLockClassifierSampleChunk( CxClassifierSample *sample, int index )
{
    CxClassifierSampleChunk *chunk = 0;

    CV_FUNCNAME( "cxLockClassifierSampleChunk" );
    __BEGIN__;

    if ( sample == 0 )
    {
	CV_ERROR( CV_StsNullPtr, "" );
    }
    if (index >= sample->num_chunks )
    {
	CV_ERROR( CV_StsBadArg, "index out of bounds" );
    }
    if ( sample->lock_chunk )
    {
	return (*sample->lock_chunk)( sample, index );
    }
    chunk = cxGetClassifierSampleChunk( sample, index );
    if ( icxIsChunkLoaded( chunk ) )
    {
	sample->chunk[index]->is_locked = 1;
    }

    __CLEANUP__;
    __END__;
    return chunk;
}

CART_IMPL void cxUnlockClassifierSampleChunk( CxClassifierSample *sample, int index )
{
    CV_FUNCNAME( "cxUnlockClassifierSampleChunk" );
    __BEGIN__;

    CxClassifierSampleChunk *chunk;

    if ( sample == 0 )
    {
	CV_ERROR( CV_StsNullPtr, "" );
    }
    if ( index >= sample->num_chunks )
    {
	CV_ERROR( CV_StsBadArg, "index out of bounds" );
    }
    if ( sample->unlock_chunk )
    {
	(*sample->unlock_chunk)( sample, index );
	return;
    }
    chunk = cxGetClassifierSampleChunk( sample, index );
    if ( icxIsChunkLoaded( chunk ) )
    {
	sample->chunk[index]->is_locked = 0;
    }

    __CLEANUP__;
    __END__;
}

static void cxReleaseSample( CxClassifierSample *sample )
{
    free(sample);
}

CART_IMPL void cxReleaseClassifierSample( CxClassifierSample **sample )
{
    CV_FUNCNAME( "cxReleaseClassifierSample" );
    __BEGIN__;

    if ( sample == 0 || *sample == 0 )
    {
	CV_ERROR( CV_StsNullPtr, "" );
    }
    if ( (*sample)->release )
    {
	(*(*sample)->release)( *sample );
    }
    *sample = 0;

    __CLEANUP__;
    __END__;
}

CART_IMPL void cxReleaseClassifier( CxClassifier **cfer )
{
    CV_FUNCNAME( "cxReleaseClassifier" );
    __BEGIN__;

    if ( cfer == 0 || *cfer == 0 )
    {
	CV_ERROR( CV_StsNullPtr, "" );
    }
    if ( (*cfer)->vftbl->release )
	 (*((*cfer)->vftbl->release))( *cfer );
    *cfer = 0;

    __CLEANUP__;
    __END__;
}



CART_IMPL CxClassifierSample *
cxCreateClassifierSampleHeader(CvMat *train_data, int tflag,
			       CvMat* train_responses, CvMat* type_mask,
			       CvMat* missed_mask, int* samples_of_interest,
			       int num_samples_of_interest)
{
    CxClassifierSample *sample = 0;

    CV_FUNCNAME( "cxCreateClassifierSampleHeader" );
    __BEGIN__;
    assert(num_samples_of_interest >= 0);
    CxClassifierSampleChunk *chunk;
    int num_samples, num_features;
    int type_step = 0;
    CxClassifierVarType *feature_type;
    uchar *ucp;
    int s;

    if ( train_responses == 0 )
    {
	CV_ERROR( CV_StsNullPtr, "" );
    }
    if ( train_data && CV_MAT_TYPE( train_data->type ) != CV_32FC1 )
    {
	CV_ERROR( CV_StsUnsupportedFormat, "train_data must be of type CV_32FC1" );
    }
    if ( CV_MAT_TYPE( train_responses->type ) != CV_32FC1 )
    {
	CV_ERROR( CV_StsUnsupportedFormat, "train_responses must be of type CV_32FC1" );
    }
    if ( missed_mask )
    {
	if ( !CV_IS_MASK_ARR( missed_mask ) )
	{
	    CV_ERROR( CV_StsBadMask, "missed_mask must be of type CX_8UC1" );
	}
	if(!train_data)
	{
	    CV_ERROR( CV_StsNullPtr, "train_data must be non-zero if missed_mask is non-zero" );
	}
	else if (train_data->rows != missed_mask->rows
	     || train_data->cols != missed_mask->cols )
	{
	    CV_ERROR( CV_StsBadSize,
		      "train_data and missed_mask have different sizes" );
	}
    }
    if ( tflag )
    {
	num_features = train_data ? train_data->cols : 0;
    }
    else
    {
	num_features = train_data ? train_data->rows : 0;
    }
    num_samples = train_responses->cols + train_responses->rows - 1;

    if ( train_responses->rows != 1 && train_responses->cols != 1 )
    {
	CV_ERROR( CV_StsBadSize, "train_responses must be either a row or a column" );
    }
    if ( train_responses->cols + train_responses->rows != num_samples + 1 )
    {
	CV_ERROR( CV_StsBadSize,
		  "train_responses must have length equal to number of samples" );
    }
    if ( type_mask )
    {
	if ( CV_MAT_TYPE( type_mask->type ) != CV_32SC1 )
	{
	    CV_ERROR( CV_StsBadMask, "type_mask must be of type CV_32SC1" );
	}
	if ( type_mask->rows != 1 && type_mask->cols != 1 )
	{
	    CV_ERROR( CV_StsBadSize, "type_mask must be either a row or a column" );
	}
	if ( type_mask->rows + type_mask->cols != num_features + 2 )
	{
	    CV_ERROR( CV_StsBadSize,
		      "type_mask must have length equal to number of features plus one" );
	}
	type_step = (type_mask->rows == 1) ? sizeof(int) : type_mask->step;
    }

    s = sizeof( CxClassifierSample ) + sizeof( CxClassifierVarType ) * num_features
	+ sizeof( CxClassifierSampleChunk * ) + sizeof( CxClassifierSampleChunk );
    sample = (CxClassifierSample *) malloc( s );
	memset(sample , 0 , s);
    feature_type = (CxClassifierVarType *) (sample + 1);
    sample->feature_type = feature_type;
    sample->chunk = (CxClassifierSampleChunk **)(sample->feature_type + num_features);
    chunk = (CxClassifierSampleChunk *) (sample->chunk + 1);
    sample->chunk[0] = chunk;
    chunk->is_loaded = 1;
    chunk->num_points = num_samples;
    chunk->is_locked = 0;
    chunk->num_indices_of_interest = num_samples;
    chunk->indices_of_interest = 0;
    if ( samples_of_interest )
    {
	chunk->indices_of_interest = samples_of_interest;
	chunk->num_indices_of_interest = num_samples_of_interest;
    }
    else
    {
//	chunk->indices_of_interest_step = sizeof( int ) ;
//	chunk->num_indices_of_interest = num_samples;
    }
    chunk->features_data = train_data ? (CxClassifierVar *)train_data->data.fl : NULL;
    chunk->missed_data = missed_mask ? (char *) missed_mask->data.ptr : NULL;

    if ( missed_mask )
    {
	if ( tflag )
	{
	    chunk->missed_step[0] = missed_mask->step;
	    chunk->missed_step[1] = 1;
	}
	else
	{
	    chunk->missed_step[0] = 1;
	    chunk->missed_step[1] = missed_mask->step;
	}
    }
    chunk->responses_data = (CxClassifierResponse *) train_responses->data.ptr;
    chunk->weights_data = 0;
    if ( tflag )
    {
	chunk->features_step[0] = train_data->step;
	chunk->features_step[1] = sizeof( float );
    }
    else
    {
	chunk->features_step[0] = sizeof( float );
	chunk->features_step[1] = train_data ? train_data->step : 0;
    }
    chunk->responses_step = train_responses->rows == 1
	? sizeof( float ) : train_responses->step;
    chunk->weights_step = sizeof( float );
    sample->num_chunks = 1;
    sample->is_weighted = 0;
    sample->num_features = num_features;
    if ( type_mask )
	sample->response_type = *(CxClassifierVarType *)(type_mask->data.ptr + type_step * num_features);
    else
	sample->response_type = CX_CLASSIFIER_VAR_TYPE_CATEGORIC_TRANSCENDENT;

    assert(sample->response_type >= 0);
    sample->load_chunk = 0;
    sample->lock_chunk = 0;
    sample->unlock_chunk = 0;
    if ( type_mask )
    {
	ucp = type_mask->data.ptr;
	for ( s = 0; s < num_features; ++s, ucp += type_step )
	{
//            CX_CART_TRACE( tdev << "copy feature_type " << *(CxClassifierVarType *)ucp << nl );
	    feature_type[s] = *(CxClassifierVarType *)ucp;
	    assert( feature_type[s] >= 0);
	}
    }
    else
    {
	for ( s = 0; s < num_features; ++s )
	{
//            CX_CART_TRACE( tdev << "setting feature_type " << CX_CLASSIFIER_VAR_TYPE_NUMERIC_32F << nl );
	    feature_type[s] = CX_CLASSIFIER_VAR_TYPE_NUMERIC_32F;
	}
    }
    sample->release = &cxReleaseSample;
    __CLEANUP__;
    __END__;
    return sample;
}

CART_IMPL void cxReleaseCART( CxCART **cart )
{
    CV_FUNCNAME( "cxReleaseCART" );
    __BEGIN__;
    {
	CxCART* _cart = *cart;
	if ( cart == 0 || _cart == 0 )
	{
	    CV_ERROR( CV_StsNullPtr, "" );
	}
	if (_cart->features_corr)
	{
	    free(_cart->features_corr);
	    _cart->features_corr = NULL;
	}
	if (_cart->root)
	{
	    CxCARTSubj* subj = _cart->root->subj;
	    if (subj && (--(subj->ref_count) == 0))
	    {
		free(subj);
	    }
	}
	if (_cart->params)
	{
	    cxReleaseTrainParams(_cart->params);
	    _cart->params = NULL;
	}

	icxFreePruningStorage(_cart->pruning_storage);
	icxFreeCrossValStorage(_cart->cross_val_storage);
	if (_cart->storage)
	    icxFreeStorage(_cart->storage);
/*#ifdef CART_WITH_GUI
	if (_cart->m_hStopEvent)
		CloseHandle(_cart->m_hStopEvent);
	_cart->m_hStopEvent = NULL;
#endif*/
	free(_cart);
	_cart = NULL;
	}
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxReleaseTrainParams( CxCARTTrainParams* params)
{
    if (params && (--(params->ref_count) == 0))
    {
	if (params->features_of_interest_mat)
	{
	    cvReleaseMat(&(params->features_of_interest_mat));
	}
	if (params->priors_mat)
	{
	    cvReleaseMat(&(params->priors_mat));
	}
	if (params->cost_mat)
	{
	    free(params->cost_mat);
	    params->cost_mat = NULL;
	}
	free(params);
    }
}


static void release_cart( CxClassifier *cart )
{
    cxReleaseCART( (CxCART **)&cart );
}


CART_IMPL CxCARTTrainParams *
cxCARTTrainParams(CvMat *features_of_interest, CvMat *priors,
		  CxCARTSplitCriterion splitting_rule, int num_competitors,
		  float competitor_threshold, int num_surrogates,
		  float surrogate_threshold,  int tree_max_depth,
		  int split_max_points, int split_min_points, int max_clusters,
		  int mem_economy_mode, int size)
{
    CxCARTTrainParams *params = 0;

    CV_FUNCNAME( "cxCARTTrainParams" );
    __BEGIN__;

    assert( mem_economy_mode == 0 || mem_economy_mode == 1);
    assert( tree_max_depth > 0 && tree_max_depth <= 32 );
    assert( max_clusters > 1 && max_clusters < 20 );
    assert( num_competitors >= 0 && num_surrogates >= 0 &&
	split_max_points >= 0 && split_min_points >= 0);
    assert( size >= sizeof(CxCARTTrainParams));
    assert( splitting_rule == CxCARTGiniCriterion ||
	splitting_rule == CxCARTEntropyCriterion);

    if ( features_of_interest )
    {
	if ( CV_MAT_TYPE( features_of_interest->type ) != CV_32SC1 )
	{
	    CV_ERROR( CV_StsUnsupportedFormat,
		      "features_of_interest must be of type CV_32SC1" );
	}
	if ( features_of_interest->rows != 1 && features_of_interest->cols != 1 )
	{
	    CV_ERROR( CV_StsBadSize, "features_of_interest must be either a row or a column" );
	}
    }

    if ( priors )
    {
	if ( CV_MAT_TYPE( priors->type ) != CV_32FC1 )
	{
	    CV_ERROR( CV_StsUnsupportedFormat, "priors must be of type CV_32FC1" );
	}
	if ( priors->rows != 1 && priors->cols != 1 )
	{
	    CV_ERROR( CV_StsBadSize, "priors must be either a row or a column" );
	}
    }
    params = (CxCARTTrainParams *) malloc( size );
	memset( params , 0 , size);
	params->features_of_interest_mat = features_of_interest;
    if ( features_of_interest )
    {
	params->num_features_of_interest = features_of_interest->rows
	    + features_of_interest->cols - 1;
	params->features_of_interest = (int *) features_of_interest->data.ptr;
	params->features_of_interest_step = features_of_interest->rows == 1
	    ? sizeof( int ) : features_of_interest->step;
    }
	params->priors_mat = priors;
	if ( priors )
    {
	params->priors = priors->data.fl;
//        params->priors_step = priors->rows == 1 ? sizeof( float ) : priors->step;
    }
    params->splitting_rule = splitting_rule;
    params->num_competitors = num_competitors;
    params->competitor_threshold = competitor_threshold;
    params->max_clusters = max_clusters;
    params->num_surrogates = num_surrogates;
    params->surrogate_threshold = surrogate_threshold;
    params->tree_max_depth = tree_max_depth;
    params->split_min_points = split_min_points;
    params->split_max_points = split_max_points;
    params->mem_economy_mode = mem_economy_mode;
    params->ref_count = 1;
    __CLEANUP__;
    __END__;
    return params;
}

static inline int less_than( int a, int b )
{
    return a < b;
}

static void icxSort_32s( int[], int len, int = 0 );

CX_IMPLEMENT_QSORT( icxSort_32s, int, less_than )

CART_IMPL CxCARTSplit* cxCARTSplit( CxCART* cart, int feature_idx , BOOL use_calc_storage )
{
    int eff_feature = cart->features_corr[feature_idx];
    int type = cart->feature_type[feature_idx];
    BOOL numeric = icxIsClassifierVarNumeric(type);
    BOOL var32s = icxIsClassifierVar32s(type);
    CxCARTSplit* split = NULL;

     CV_FUNCNAME( "cxCARTSplit" );
    __BEGIN__;

    if ( cart == 0 )//|| cart->root == 0 )
    {
	CV_ERROR( CV_StsNullPtr, "" );
    }
    if ( feature_idx >= cart->num_features )
    {
	CV_ERROR( CV_StsBadSize, "bad feature index" );
    }

    if (use_calc_storage)
    {
	split = ((CxCARTSplit*)cart->storage->calc_storage.buf_splits) + eff_feature;
	assert(split->feature_idx == feature_idx);
    }
    else
    {
	int s = sizeof( CxCARTSplit );
	s += cart->num_classes[feature_idx] * sizeof( char );
	split = (CxCARTSplit *) malloc( s );
	split->feature_idx = feature_idx;
	split->next_competitor = NULL;
	split->next_surrogate = NULL;
	if (numeric)
	{
	    if (var32s)
		split->boundary.value.i = 0;//value.i;
	    else
		split->boundary.value.fl = 0.0;//value.fl;
	}
	else
	    split->boundary.ptr = (split + 1);
    }
    split->revert = FALSE;
    return split;
    __CLEANUP__;
    __END__;
}

CART_IMPL CxCARTSplit* cxCARTBaseSplit( CxCARTBase* cart, int feature_idx )
{
    int type = cart->feature_type[feature_idx];
    BOOL numeric = icxIsClassifierVarNumeric(type);

    assert ( cart != 0 &&
	feature_idx >= 0 &&
	feature_idx < cart->num_features);

    int s = sizeof( CxCARTSplit );
    s += cart->num_classes[feature_idx] * sizeof( char );
    CxCARTSplit* split = (CxCARTSplit*) malloc( s );
    split->feature_idx = feature_idx;
    split->next_competitor = NULL;
    split->next_surrogate = NULL;
    if (numeric)
    {
	split->boundary.value.i = 0;//value.i;
    }
    else
	split->boundary.ptr = (split + 1);

    split->revert = FALSE;
    return split;
}

CART_IMPL void cxBuildCARTNodeResponse( CxRootedCARTBase* cart, CxForestNode *node )
{
    CV_FUNCNAME( "cxBuildCARTNodeResponse" );
    __BEGIN__;
    assert(cart && node && node->subj);

    /*	if (cxCalcNodeNum(node) == 3)
    { int mmm = 1;	}*/

    CxClassifierVarType type = cart->response_type;
    BOOL floating = icxIsClassifierVar32f(type);

    if ((type < 1) || (type > CX_CLASSIFIER_VAR_TYPE_NUMERIC_32F))
    {
	char buf[250];
	sprintf(buf , "Type of response incorrect : %d" , type);
	icxSetCARTError((CxCARTBase*)cart , CART_INVALID_PARAMS_ERROR ,
	    buf , CART_ERROR_STATUS_FATAL);
	return ;
    }

    if ( icxIsClassifierVarNumeric( type ))
    {
	//// Calculate average and deviation
	double sum = 0;
	double sum_sq = 0;
	int num_fallens = node->num_fallens;

	CxCARTSubj* subj = node->subj;
	CxClassifierVar* responses = subj->response;
	int* _idx = node->fallen_idx;
	for (int i = num_fallens; i--  ; _idx++)
	{
	    assert(icxCompareVarsSimple(icxGetSubjResponse(node->subj, *_idx) , responses[*_idx]));
	    float f = icxVarToFloat(responses[*_idx]);
	    sum += f;
	    //		sum_sq += f*f;
	}
	float resp = node->response.fl = (float)(sum / num_fallens);
	_idx = node->fallen_idx;
	for (int i1 = num_fallens; i1--  ; _idx++)
	{
	    float f = icxVarToFloat(responses[*_idx]) - resp;
	    sum_sq += f*f;
	}
	node->error = (float)sum_sq;//(float)(sum_sq - sum * sum / num_fallens);
    }
    else
    {
	int num_resp = cart->num_response_classes;
	int num_resp_aligned = num_resp;
	ALIGN_NUM_RESP( num_resp_aligned );
	node->sum_weighted_fallens = 0.0f;
	int* fallen_stats = node->fallen_stats;
	//// Calculate most frequent class
	int best_i = 0;
	float* cost_mat = cart->params->cost_mat;
	if (!cost_mat)
	{
	    float f_best = -1.0f;
	    //// No cost matrix
	    for (int  i = 0 ; i < num_resp; i++)
	    {
		float f = cart->priors_mult[i] * fallen_stats[i];
		node->sum_weighted_fallens += f;
		if ( f > f_best )
		{
		    best_i = i;
		    f_best = f;
		}
	    }
	    node->error = node->sum_weighted_fallens - f_best;
	}
	else
	{
	    //// Cost matrix
	    float error_best = FLT_MAX;
	    float* _cost_mult = cart->cost_error_mult;
	    for (int  i = 0 ; i < num_resp;
	    i++, _cost_mult += num_resp_aligned)
	    {
		float error = 0.0;
		for (int j = 0 ; j < num_resp; j++)
		    error += _cost_mult[j] * fallen_stats[j];
		if ( error < error_best )
		{
		    best_i = i;
		    error_best = error;
		}
	    }
	    for (int j = 0 ; j < num_resp; j++)
	    {
		float f = cart->priors_mult[j] * fallen_stats[j];
		node->sum_weighted_fallens += f;
	    }
	    node->error = error_best;
	    assert(error_best >= 0.0f && error_best < FLT_MAX/2);
	}
	assert( best_i >= 0 && best_i < num_resp);
	if (floating)
	    node->response.fl = (float)best_i;
	else
	    node->response.i = best_i;

    }
    node->error = node->error/cart->croot->num_fallens;
//	if (node->response.fl >= 1.000001 || node->response.fl <= -1.000001	)
//		int mmm = 1;

    __CLEANUP__;
    __END__;
}

CART_IMPL void cxBuildCARTNodeFallenStats( CxCARTBase* cart, CxForestNode* node )
{
    CV_FUNCNAME( "icxBuildCARTNodeFallenStats" );
    __BEGIN__;
    CxClassifierVarType type = cart->response_type;
    if (icxIsClassifierVarNumeric(type))
    {// Return if regression
	return;
    }
    CX_CART_MAKE_LOCAL( int *, node, fallen_stats );
    CX_CART_MAKE_LOCAL( int, node, num_fallens );
    CxCARTSubj* subj = node->subj;
#ifdef _DEBUG
    CX_CART_MAKE_LOCAL( int, cart, num_response_classes );
    assert(num_response_classes > 0);
    CxCARTAtom* body = subj->body;
    //   for (int i1 = 0; i1 < num_response_classes ; i1++)
    //	assert(fallen_stats[i1] == 0);
#endif
    memset(fallen_stats , 0 , cart->num_response_classes * sizeof(int));
    int* fallen_idx = node->fallen_idx;
    if (icxIsClassifierVarTranscendent( type ))
    {
	assert(0);
//	response = cxMapClassifierTranscendentVar( map, var );
    }
    BOOL floating = icxIsClassifierVar32f( type );
    CxClassifierVar* responses = subj->response;
    for (int i = 0 ;  i < num_fallens; i++ , fallen_idx++)
    {
	CxClassifierVar var = responses[*fallen_idx];
	int response = icxVarToInt(var , floating);
	assert( response >= 0 && response < num_response_classes);
	assert(	icxVarToInt(body[*fallen_idx].response) == response);
	fallen_stats[response]++;
    }

    __CLEANUP__;
    __END__;
}

#define LESS_THAN_NUM_RESP_32F( a, b ) (aux[a].fl < aux[b].fl)
#define LESS_THAN_NUM_RESP_32S( a, b ) (aux[a].i < aux[b].i)
#define LESS_THAN_NUM_32F( a, b ) (aux[a].notch.fl < aux[b].notch.fl)
#define LESS_THAN_NUM_32S( a, b ) (aux[a].notch.i < aux[b].notch.i)
#define LESS_THAN_FLOAT( a, b ) (aux[a] < aux[b])

static void sort_fallenidx_byvar32s( int[], int len, CxClassifierVar* );
static void sort_fallenidx_byatoms32f( int[], int len, CxCARTAtom * );
static void sort_fallenidx_byatoms32s( int[], int len, CxCARTAtom * );
static void sort_float( int[], int len, float* );

CX_IMPLEMENT2_QSORT( sort_fallenidx_byvar32f, int, LESS_THAN_NUM_RESP_32F, CxClassifierVar * )
CX_IMPLEMENT2_QSORT( sort_fallenidx_byvar32s, int, LESS_THAN_NUM_RESP_32S, CxClassifierVar * )
CX_IMPLEMENT2_QSORT( sort_fallenidx_byatoms32f, int, LESS_THAN_NUM_32F, CxCARTAtom * )
CX_IMPLEMENT2_QSORT( sort_fallenidx_byatoms32s, int, LESS_THAN_NUM_32S, CxCARTAtom * )
CX_IMPLEMENT2_QSORT( sort_float, int, LESS_THAN_FLOAT, float* )

#undef LESS_THAN_NUM_32S
#undef LESS_THAN_NUM_32F

CART_IMPL void cxSortFallenByResponse(CxCART* cart, CxCARTNode* node)
{
    assert(icxIsClassifierVarCategoric(cart->response_type));
    CxCARTSubj* subj = node->subj;
    CxClassifierVar* responses = subj->response;
    int num_fallens = node->num_fallens;
    int* fallen_idx = node->fallen_idx;
    int eff_num_features = cart->eff_num_features;
    ///// Sort each categoric feature's indices additionally by response value
    for (int i = 0; i < eff_num_features; i++)
    {
	int cur_feature = cart->features_back_corr[i];
	assert(cur_feature >= 0);
	int categoric = icxIsClassifierVarCategoric(cart->feature_type[cur_feature]);
	if (categoric)
	{
	    int* idx = fallen_idx + i * num_fallens;
	    int num_classes = cart->num_classes[cur_feature];
	    int* num_var_classes = icxGetCatProportions(cart , node , i);
#ifdef _DEBUG
	    //	ofstream file("sort.log");
	    //	file << "Before sorting : \n";
#endif
	    int* _idx = idx;
	    for (int k = 0 ; k < num_classes ; k++)
	    {
		int num_var_class = num_var_classes[k];
#ifdef _DEBUG
		int j;
		for (j = 0 ; j < num_var_class ; j++, _idx++)
		{
		    int sample_idx = (*_idx);
		    assert( fabs(icxGetSubjFeature(subj , sample_idx , cur_feature ).fl - k) < EPS);
		}
		_idx -= num_var_class;
#endif
		_idx += num_var_class;
		sort_fallenidx_byvar32f( _idx - num_var_class, num_var_class, responses );
#ifdef _DEBUG
		//				file << "\nAfter sorting : \n";
		int* old_idx = _idx - num_var_class;
		for (j = 0 ; j < num_var_class ; j++, old_idx++)
		{
		    int sample_idx = (*old_idx);
		    assert( fabs(icxGetSubjFeature(subj , sample_idx , cur_feature ).fl - k) < EPS);
		    //					file << atom->response.fl << ' ';
		}
		//				file.flush();
#endif
	    }
	    free(num_var_classes);
	}
    }
}


CART_IMPL void cxBuildNodeFallenIdxFromExtraInfo(CxCART* cart,
						 CxCARTNode* node)
{
    START_FUNC(cxBuildNodeFallenIdxFromExtraInfo);

    CxExtCARTInfo* info = cart->extra_info;
    int eff_num_features = cart->eff_num_features ;
    assert(node);
    CxCARTSubj* subj = node->subj;
    assert(subj);
#ifdef _DEBUG
    int size = subj->size;
#endif
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned );
    CxClassifierVar* responses = subj->response;

    int num_cases = info->data_mat->cols;
    int num_fallens = node->num_fallens;

    int i = 0;

    /* Array for correspondence from sample to subj string numbering, containing
    corresponding node's fallen_idx subj string numbers, and -1 when sample is
    not in node */
    int* buf_fallen_idx = (regression ? NULL : (int*)malloc(num_fallens * sizeof(int)));
    int* samples_of_interest = info->samples_of_interest;
    int* samples_back_corr = (int*)malloc(num_cases * sizeof(int));
    memset(samples_back_corr, -1 , num_cases * sizeof(int));
    FUNC_MEM_WRITE(cxBuildNodeFallenIdxFromExtraInfo , num_cases , int);

    int* shrunk_fallen_idx = node->shrunk_fallen_idx;

    BOOL is_root = (node == cart->root);
    /*	if (!is_root)
    {int mmm = 1;}*/

    for (i = 0 ; i < num_fallens; i++)
    {
	int subj_idx = (is_root ? i : shrunk_fallen_idx[i]);
	assert( subj_idx >= 0 && subj_idx < size);
	int sample_idx = samples_of_interest ? samples_of_interest[subj_idx] : subj_idx;
	assert(samples_back_corr[sample_idx] < 0);
	samples_back_corr[sample_idx] = subj_idx;
    }
    //// Read samples_of_interest[subj_idx]
    FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo , num_fallens , int);
    //// Write samples_back_corr[sample_idx]
    FUNC_MEM_WRITE(cxBuildNodeFallenIdxFromExtraInfo , num_fallens , int);


    int* sorted_idx = info->sorted_idx;
    int* num_missed = node->num_missed;
    int* fallen_idx = node->fallen_idx;
    assert(fallen_idx);
    //// Create node->fallen_idx from sorted_idx
#if DATA_TO_FILE3
    FILE* file1 = fopen("d:\\CART\\AlamereTest\\build_idx_cat_class.dat", "wb");
    int selected_var = -1;
#endif
    for (i = 0 ; i < eff_num_features ; i++)
    {
	int real_var = cart->features_back_corr[i];
	assert(real_var >= 0 && real_var < cart->num_features );
	int num_not_missed = info->num_not_missed[real_var];
	BOOL categoric = icxIsClassifierVarCategoric(cart->feature_type[real_var]);
	int* cat_proportions = node->cat_proportions[i];
	int* _fallen_idx = fallen_idx + i * num_fallens;
	int* _sorted_idx = sorted_idx + real_var * num_cases ;
	CxClassifierVar* feature_vals_init =
	    (CxClassifierVar*)(info->data_mat->data.ptr +
	    info->data_mat->step * real_var );

	if (cat_proportions)
	{
	    if (categoric)
	    {
		int num_classes = cart->num_classes[real_var];
		assert(num_classes > 0 && num_classes < CX_CLASSIFIER_VAR_TYPE_FLAG_32F);

		int* ext_cat_proportions = info->cat_proportions ? info->cat_proportions[real_var] : NULL;
		if (regression)
		{
		    ////  Fill fallen_idx and category proportions
		    if (ext_cat_proportions) /// Main branch
		    {
			for (int cat = 0 ; cat < num_classes ; cat++)
			{
			    int ext_cat_proportion = ext_cat_proportions[cat];
			    int cat_proportion = 0;
			    for (int j = 0 ; j < ext_cat_proportion ; j++, _sorted_idx++)
			    {
				int idx = *_sorted_idx;
				assert(idx >= 0 && idx < num_cases);
				int subj_idx = samples_back_corr[idx];
				if( subj_idx > -1 )
				{
				    assert( subj_idx >= 0 && subj_idx < size );
				    (*_fallen_idx++) = subj_idx;
				    cat_proportion++;
#ifdef _DEBUG
				    int cat1 = icxVarToInt(feature_vals_init[idx]);
				    assert( cat1 >= 0 && cat1 < num_classes);
				    int feat_val = cvRound(icxGetSubjFeature(subj, subj_idx, real_var).fl);
				    assert( feat_val == cat1 );
				    assert( cat1 == cat );
#endif
				}
			    }
			    cat_proportions[cat] = cat_proportion;
			}
		    }
		    else
		    {
			for (int j = 0 ; j < num_not_missed ; j++, _sorted_idx++)
			{
			    int idx = *_sorted_idx;
			    assert(idx >= 0 && idx < num_cases);
			    int subj_idx = samples_back_corr[idx];
			    if( subj_idx > -1 )
			    {
				assert( subj_idx >= 0 && subj_idx < size );
				(*_fallen_idx++) = subj_idx;
				int cat = icxVarToInt(feature_vals_init[idx]);
				assert( cat >= 0 && cat < num_classes);
#ifdef _DEBUG
				int feat_val = cvRound(icxGetSubjFeature(subj, subj_idx, real_var).fl);
				assert( feat_val == cat );
#endif
				cat_proportions[cat]++;
			    }
			}
			//// Read feature_vals_init[idx]
			FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int);
			//// Write cat_proportions[cat]
			FUNC_MEM_WRITE(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int);

		    }
		}
		else
		{
		/*  Fill fallen_idx , category-response frequency tables and perform sorting
		of fallen_idx by responses. Indices are copied to buf_fallen_idx first
		    to avoid copying fallen_idx */
		    assert(responses);
		    int* _buf_fallen_idx = buf_fallen_idx;
#if DATA_TO_FILE3
		    fwrite(&num_fallens, sizeof(int), 1, file1);
		    fwrite(&num_cases, sizeof(int), 1, file1);
		    fwrite(&num_classes, sizeof(int), 1, file1);
		    fwrite(&num_not_missed, sizeof(int), 1, file1);
		    fwrite(&num_resp, sizeof(int), 1, file1);

		    fwrite(ext_cat_proportions, sizeof(int), num_classes, file1);
		    fwrite(samples_back_corr, sizeof(int), num_cases, file1);
		    fwrite(_sorted_idx, sizeof(int), num_cases, file1);

		    for (int i1 = 0 ; i1 < num_fallens ; i1++)
		    {
			int n = (int)responses[i1].fl;
			fwrite(&n, sizeof(int), 1, file1);
		    }
		    selected_var = i;
		    fflush(file1);
#endif
		    if (ext_cat_proportions) /// Main branch
		    {
			int* _cat_proportions = cat_proportions;
			for (int cat = 0 ; cat < num_classes ; cat++ , _cat_proportions += num_resp_aligned)
			{
			    int ext_cat_proportion = ext_cat_proportions[cat];
			    for (int j = 0 ; j < ext_cat_proportion ; j++, _sorted_idx++)
			    {
				int idx = *_sorted_idx;
				assert(idx >= 0 && idx < num_cases);
				int subj_idx = samples_back_corr[idx];
				if( subj_idx > -1 )
				{
				    (*_buf_fallen_idx++) = subj_idx;
				    int resp = icxVarToInt(responses[subj_idx]);
				    assert( resp >= 0 && resp < num_resp );
#ifdef _DEBUG
				    int cat1 = icxVarToInt(feature_vals_init[idx]);
				    assert( cat1 >= 0 && cat1 < num_classes);
				    assert ( cat1 == cat);
#endif
				    _cat_proportions[resp]++;
				    assert( subj_idx >= 0 && subj_idx < size );
				}
			    }
			}
			//// Read responses[subj_idx]
			FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , CxClassifierVar);
			//// Write cat_proportions[cat * num_resp_aligned + resp]
			FUNC_MEM_WRITE(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int);
		    }
		    else
		    {
			for (int j = 0 ; j < num_not_missed ; j++, _sorted_idx++)
			{
			    int idx = *_sorted_idx;
			    assert(idx >= 0 && idx < num_cases);
			    int subj_idx = samples_back_corr[idx];
			    if( subj_idx > -1 )
			    {
				(*_buf_fallen_idx++) = subj_idx;
				int cat = icxVarToInt(feature_vals_init[idx]);
				int resp = icxVarToInt(responses[subj_idx]);
				assert( resp >= 0 && resp < num_resp );
				assert( cat >= 0 && cat < num_classes);
				cat_proportions[cat * num_resp_aligned + resp]++;
				assert( subj_idx >= 0 && subj_idx < size );
			    }
			}
			//// Read feature_vals_init[idx]
			FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int);
			//// Read responses[subj_idx]
			FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , CxClassifierVar);
			//// Write cat_proportions[cat * num_resp_aligned + resp]
			FUNC_MEM_WRITE(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int);
		    }

		    int** buf = (int**)malloc( num_resp_aligned * num_classes * sizeof(int*) );
		    int* num_proportions = (int*)calloc( num_classes , sizeof(int));
		    int* _num_proportions = num_proportions;
		    int** _buf = buf;
		    int* cur_buf_ptr = _fallen_idx;
		    int* cur_bucket = cat_proportions;

		    /* Setting pointers in fallen_idx to copy new idx values,
		    also we calculate num_proportions here */
		    for (int cat = 0 ; cat < num_classes ; cat++,_num_proportions++)
		    {
			for (int resp = 0 ; resp < num_resp_aligned; resp++, _buf++, cur_bucket++)
			{
			    (*_buf) = cur_buf_ptr ;
			    int bucket_size = (*cur_bucket);
			    cur_buf_ptr += bucket_size;
			    (*_num_proportions) += bucket_size;
			}
		    }
		    //// Write cat_proportions[cat * num_resp_aligned + resp]
		    FUNC_MEM_WRITE(cxBuildNodeFallenIdxFromExtraInfo , num_classes * num_resp_aligned , int*);
		    //// Read cur_bucket
		    FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo ,  num_classes * num_resp_aligned  , int);
		    //// Modify _num_proportions
		    FUNC_MEM_MODIFY(cxBuildNodeFallenIdxFromExtraInfo ,  num_classes * num_resp_aligned  , int);


		    /// Copying new (sorted) idx values to node's fallen_idx
		    _num_proportions = num_proportions;
		    _buf = buf;
		    _buf_fallen_idx = buf_fallen_idx;
		    for (int cat1 = 0 ; cat1 < num_classes ; cat1++, _num_proportions++, _buf += num_resp_aligned)
		    {
			int cur_num_proportion = *_num_proportions;
			for (int j = 0 ; j < cur_num_proportion ; j++, _buf_fallen_idx++)
			{
			    int subj_idx = *_buf_fallen_idx;
			    assert( subj_idx >= 0 && subj_idx < size );
			    int resp = icxVarToInt(responses[subj_idx]);
			    assert( resp >= 0 && resp < num_resp );
			    (*_buf[resp]++) = subj_idx;
			}
			_fallen_idx += cur_num_proportion;
		    }
		    //// Read responses[subj_idx]
		    FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , CxClassifierVar);
		    //// Read _buf_fallen_idx
		    FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int);
		    //// Modify _buf[resp]
		    FUNC_MEM_MODIFY(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int*);
		    //// Write *_buf[resp]
		    FUNC_MEM_WRITE(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int);

		    assert(cur_buf_ptr == buf[ num_resp_aligned * num_classes - 1] );
		    free(num_proportions);
		    free(buf);
				}
			}
			else
			{
			    ////  Fill fallen_idx and feature_vals
			    CxClassifierVar* feature_vals = (CxClassifierVar* )cat_proportions;
			    for (int j = 0 ; j < num_not_missed ; j++, _sorted_idx++)
			    {
				int idx = *_sorted_idx;
				assert(idx >= 0 && idx < num_cases);
				int subj_idx = samples_back_corr[idx];
				if( subj_idx > -1 )
				{
				    assert( subj_idx >= 0 && subj_idx < size );
				    (*_fallen_idx++) = subj_idx;
				    (*feature_vals++) = feature_vals_init[idx];
				}
			    }
			    //// Read feature_vals_init[idx]
			    FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int);
			    //// Write feature_vals
			    FUNC_MEM_WRITE(cxBuildNodeFallenIdxFromExtraInfo , num_not_missed , int);

			}
		}
		else
		{
		    //// Just fill fallen_idx
		    for (int j = 0 ; j < num_not_missed ; j++, _sorted_idx++)
		    {
			int subj_idx = samples_back_corr[*_sorted_idx];
			if( subj_idx > -1 )
			{
			    assert( subj_idx >= 0 && subj_idx < size );
			    (*_fallen_idx++) = subj_idx;
			}
		    }
		}

		int cur_num_missed = 0;
		for (int j = num_not_missed ; j < num_cases ; j++, _sorted_idx++)
		{
		/*  Pass strings with missed i-th effective feature, calculating num_missed,
		    and filling fallen_idx */
		    int subj_idx = samples_back_corr[*_sorted_idx];
		    if( subj_idx > -1 )
		    {
			assert(icxIsSubjFeatureMissed(subj, subj_idx, real_var));
			(*_fallen_idx++) = subj_idx;
			assert( subj_idx >= 0 && subj_idx < size );
			cur_num_missed++;
		    }
		}

#if DATA_TO_FILE3
		if (selected_var > -1)
		{
		    int real_var = cart->features_back_corr[selected_var];

		    fwrite(node->fallen_idx + selected_var * num_fallens, sizeof(int), num_fallens, file1);
		    fwrite(node->cat_proportions[selected_var] , sizeof(int),
			num_resp_aligned * cart->num_classes[real_var], file1);
		    fwrite(num_missed+selected_var, sizeof(int), 1, file1);
		    fclose(file1);

		    cart->error.code = CART_UNKNOWN_ERROR;
		    cart->error.status = CART_ERROR_STATUS_FATAL;
		}
#endif

		//// Read _sorted_idx and samples_back_corr
		FUNC_MEM_READ(cxBuildNodeFallenIdxFromExtraInfo , 2 * num_cases , int);
		//// Write subj_idx -> _fallen_idx
		FUNC_MEM_WRITE(cxBuildNodeFallenIdxFromExtraInfo , num_cases , int);

		num_missed[i] = cur_num_missed;
		assert ( _fallen_idx == node->fallen_idx + (i + 1) * num_fallens);
	}
	free(samples_back_corr);
	if (!regression)
	    free(buf_fallen_idx);
	EXIT_FUNC(cxBuildNodeFallenIdxFromExtraInfo);
}


CART_IMPL void cxBuildNodeFallenIdxFromScratch( CxCART* cart,
					       CxCARTNode* node)
{
    int eff_num_features = cart->eff_num_features ;
    assert(node);
    CxCARTSubj* subj = node->subj;
    assert(subj);
    int size = subj->size;
    CxCARTAtom* body = subj->body;
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned );
    int* fallen_idx = node->fallen_idx;
    assert(fallen_idx);
    CxClassifierVar* responses = subj->response;

    CxClassifierVar* buf_features = (CxClassifierVar*) malloc(size * sizeof(CxClassifierVar));
    int* shrunk_fallen_idx = node->shrunk_fallen_idx;
    if (node == cart->root)
    {
	for (int i = 0 ; i < size ; i++)
	    shrunk_fallen_idx[i] = i;
    }

    //// Do sorting ourselves
    int num_fallens = node->num_fallens;
    for (int i = 0; i < eff_num_features; i++)
    {
	int real_feature = cart->features_back_corr[i];
	int type = cart->feature_type[real_feature];
	assert(type >= 0);
	BOOL numeric = icxIsClassifierVarNumeric( type ) ;
	BOOL floating = icxIsClassifierVar32f( type ) ;
	int* cat_proportions = node->cat_proportions[i];
	int* _fallen_idx = fallen_idx + i * num_fallens;
	int num_classes = cart->num_classes[real_feature];

	CxClassifierVar infinity;
	CxClassifierVar zero;

	if (floating && numeric)
	{
	    infinity.fl = (numeric ? FLT_MAX :
	regression ? num_classes : num_classes * num_resp_aligned);
	zero.fl = 0.0;
	}
	else
	{
	    infinity.i = numeric ? INT_MAX
		:(regression ? num_classes:num_classes*num_resp_aligned);
	    zero.i = 0;
	}

	int shift = real_feature * subj->features_step;
	int shift_missed = real_feature * subj->missed_mask_step;
	int num_missed = 0;
	/* Performing sort of fallen_idxon each effective feature, calculate missing
	values numbers and frequency tables by the way */
	int k;
	for (k = 0 ; k < num_fallens; k++ )
	{
	    int str_num = shrunk_fallen_idx[k];
	    CxCARTAtom* atom = body + str_num;
	    char* missed_mask = atom->missed_mask;
	    /// Calculate missing values
	    if (missed_mask && missed_mask[shift_missed])
	    {
		buf_features[str_num] = infinity;
		num_missed ++;
	    }
	    else
	    {
	    /* Setting value to sort by ( current feature value or
	    combination of feature and response ; missed values
		receive value greater than all others */
		CxClassifierVar var = *(CxClassifierVar *)((char*)atom->features + shift);
		if (!numeric)
		{
		    int cat = icxVarToInt(var);
		    assert( cat >= 0 && cat < num_classes);
		    if (regression)
		    {
			cat_proportions[cat]++;
			buf_features[str_num].i = icxVarToInt(var);
		    }
		    else
		    {
			/// Sort by category and response
			int resp = icxVarToInt(responses[str_num]);
			assert( resp >= 0 && resp < num_resp );
			cat_proportions[cat * num_resp_aligned + resp]++;
			buf_features[str_num].i = cat * num_resp + resp;
		    }
		}
		else buf_features[str_num] = var;
	    }
	}
	node->num_missed[i] = num_missed;
	if (numeric)
	{
	    memcpy(_fallen_idx , shrunk_fallen_idx , num_fallens * sizeof(int));
	    /// Do sorting of _fallen_idx ; copy features if nessesary
	    if ( floating )
		sort_fallenidx_byvar32f( _fallen_idx, num_fallens, buf_features );
	    else
		sort_fallenidx_byvar32s( _fallen_idx, num_fallens, buf_features );
	    if (!cart->params->mem_economy_mode)
	    {
		/// Copy features to cat_proportions
		CxClassifierVar* feature_vals = (CxClassifierVar* )cat_proportions;
		for (k = 0 ; k < num_fallens - num_missed; k++, feature_vals++)
		{
		    assert(_fallen_idx[k] >= 0 && _fallen_idx[k] < size );
		    (*feature_vals) = buf_features[_fallen_idx[k]];
		}

	    }
	}
	else
	{
	    /// Perform quick (one - pass) sorting of categoric features
	    int num_buckets = ( regression ? (num_classes + 1) : (num_resp_aligned * num_classes + 1));
	    int** buf = (int**)malloc( num_buckets * sizeof(int*) );
	    int** _buf = buf;
	    int* cur_buf_ptr = _fallen_idx;
	    int* cur_bucket = cat_proportions;
	    /* Setting pointers in fallen_idx to copy new idx values,
	    also we calculate num_proportions here */
	    for (int bucket = 0 ; bucket < num_buckets - 1; bucket++, _buf++, cur_bucket++)
	    {
		(*_buf) = cur_buf_ptr ;
		cur_buf_ptr += (*cur_bucket);
	    }
	    /// Last bucket with missing values
	    (*_buf) = cur_buf_ptr;
	    /// Copying new (sorted) idx values to node's fallen_idx
	    int* _buf_fallen_idx = shrunk_fallen_idx;
	    for (k = 0 ; k < num_fallens; k++, _buf_fallen_idx++ )
	    {
		int subj_idx = *_buf_fallen_idx;
		assert( subj_idx >= 0 && subj_idx < size );
		int bucket = buf_features[subj_idx].i;
		assert( bucket >= 0 && bucket < num_buckets);
		(*buf[bucket]++) = subj_idx;
	    }
	    assert(buf[num_buckets - 1] == fallen_idx + (i + 1) * num_fallens);
	    free(buf);
	}
	}
	free(buf_features);
}

#ifdef _DEBUG
CART_IMPL void cxCheckNodeFallenIdx(CxCART* cart, CxCARTNode* node)
{
//// Checking node->cat_proportions
    int eff_num_features = cart->eff_num_features ;
    assert(node);
    CxCARTSubj* subj = node->subj;
    assert(subj);
    int size = node->num_fallens;
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned );
    int* fallen_idx = node->fallen_idx;
    assert(fallen_idx);

    assert(node->cat_proportions);
    int i;
    for (i = 0 ; i < eff_num_features ; i++)
    {
	int real_feature = cart->features_back_corr[i];
	int* idx = fallen_idx + i * size;
	BOOL categoric = icxIsClassifierVarCategoric(cart->feature_type[real_feature]);
	int* cat_proportions = node->cat_proportions[i];
	int nonmissed = size - node->num_missed[i];

	if (categoric)
	{
	    int num_classes = cart->num_classes[real_feature];
	    if (regression)
	    {
		//// Checking category proportions
		int* cat_proportions1 =(int*)calloc(num_classes , sizeof(int));
		for (int j = 0 ; j < nonmissed; j++)
		{
		    int feat_val = cvRound(icxGetSubjFeature(subj, idx[j] , real_feature).fl);
		    cat_proportions1[feat_val]++;
		}
		for (int cat = 0 ; cat < num_classes ; cat++)
		    assert(cat_proportions1[cat] == cat_proportions[cat]);
		free(cat_proportions1);
	    }
	    else
	    {
		//// Checking frequency tables
		int* cat_proportions1 =(int*)calloc(num_classes * num_resp_aligned , sizeof(int));
		cxCalcFrequencies(cat_proportions1 , cart , node , real_feature);
		for (int bucket = 0 ; bucket < num_classes * num_resp_aligned; bucket++)
		    assert(cat_proportions1[bucket] == cat_proportions[bucket]);
		free(cat_proportions1);
	    }
	}
	else if (!cart->params->mem_economy_mode)
	{
	    //// Checking feature values
	    int shift = real_feature * subj->features_step;
	    CxCARTAtom* body = subj->body;
	    CxClassifierVar* _cat_proportions = (CxClassifierVar*)cat_proportions;
	    for (int j = 0 ; j < nonmissed; j++)
		assert(_cat_proportions[j].i == (*(CxClassifierVar *)((char*)body[idx[j]].features + shift)).i);//icxGetSubjFeature(subj, idx[j] , real_feature);
	}
    }
    //// Checking sort order
    for (i = 0 ; i < eff_num_features ; i++)
    {
	int real_feature = cart->features_back_corr[i];

	BOOL categoric = icxIsClassifierVarCategoric(cart->feature_type[real_feature]);
	float prev_val = -FLT_MAX;
	int missed = 0;
	for (int j = 0 ; j < size; j++)
	{
	    int idx = fallen_idx[i * size + j];
	    float val = FLT_MAX;
	    if (!icxIsSubjFeatureMissed(node->subj, idx , real_feature))
		val = ((!categoric || regression) ? icxGetSubjFeature(node->subj, idx , real_feature).fl :
	    icxGetSubjFeature(node->subj, idx , real_feature).fl * num_resp + icxGetSubjResponse(subj , idx).fl);
	    else
		missed++;

	    assert(val >= prev_val);
	    prev_val = val;
	}
	assert(node->num_missed[i] == missed);
    }
}
#endif

CART_IMPL void cxBuildCARTNodeMasterFallenIdx( CxCART *cart,
					      CxCARTNode *node,
					      BOOL expand_idx,
					      BOOL use_calc_storage)
{

    CV_FUNCNAME( "cxBuildCARTNodeMasterFallenIdx" );
    __BEGIN__;

    CX_CART_MAKE_LOCAL( CxCARTSubj *, node, subj );
    CX_CART_MAKE_LOCAL( int, subj, size );

    node->num_fallens = size;
    cxAllocNodeInternals(cart,node,expand_idx, use_calc_storage);
    if (cart->error.code > 0)
	return;
    CX_CART_MAKE_LOCAL( int *, node, fallen_idx );
    assert( fallen_idx );

    if (!expand_idx && (node == cart->root))
    {
	for (int i = 0 ; i < size ; i++)
	    fallen_idx[i] = i;
    }
    else
    {
    /* Construct main node fallen_idx and num_missed from external array,
	or do it ourselves */
	if (cart->extra_info)
	{
	    cxBuildNodeFallenIdxFromExtraInfo(cart , node);
	}
	else
	{
	    cxBuildNodeFallenIdxFromScratch(cart , node);
	}
#ifdef _DEBUG
	if (expand_idx)
	{
	    cxCheckNodeFallenIdx(cart , node);
	}
#endif
    }
    icxBuildCARTNodeFallenStats( cart, node );
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxCreatePriorsCostMult(CxCARTBase* cart, int* root_stats, int root_fallens)
{
    int num_resp = cart->num_response_classes;

    float* priors_mult = cart->priors_mult;
    float* priors = cart->params->priors;
    BOOL need_gini_mult = (cart->params->splitting_rule == CxCARTGiniCriterion);
    if (priors)
    {
	for (int i = 0 ; i < num_resp ; i++)
	    priors_mult[i] = (root_stats[i] == 0 ) ? 0.0f : priors[i] * root_fallens / root_stats[i];
    }
    else
    {
	for (int i = 0 ; i < num_resp ; i++)
	    priors_mult[i] = 1.0f;
    }
    float* _cost_mat = cart->params->cost_mat;
    if (_cost_mat)
    {
	int num_resp_aligned = num_resp;
	ALIGN_NUM_RESP( num_resp_aligned );
	float* cost_col_sum = (float*)calloc( num_resp_aligned * 2, sizeof(float));
	float* cost_row_sum = cost_col_sum + num_resp_aligned;
	int i;

	for (i = 0 ; i < num_resp ; i++ , _cost_mat += num_resp_aligned)
	{
	    icxAddVector(cost_col_sum , _cost_mat , num_resp );
	}
	float denom = 1.0f;
	if (priors)
	{
	    denom = icxScalarProd( cost_col_sum , priors, num_resp) ;
	}
	else
	{
	    denom = 0;
	    for (int i1 = 0 ; i1 < num_resp ; i1++ )
		denom += cost_col_sum[i1] * root_stats[i1] / root_fallens;
	}
	float* aux_priors_mult = (float*)malloc(num_resp * sizeof(float));
	denom = 1.0f / denom;
	for (int i1 = 0 ; i1 < num_resp ; i1++ )
	{
	    cost_col_sum[i1] *= denom;
	    aux_priors_mult[i1] = priors_mult[i1] *	cost_col_sum[i1] ;
	}

	float* _cost_error_mult = cart->cost_error_mult;
	float* _cost_gini_mult  = cart->cost_gini_mult;

	_cost_mat = cart->params->cost_mat;
	for (i = 0 ; i < num_resp ; i++,
	    _cost_mat        += num_resp_aligned,
	    _cost_error_mult += num_resp_aligned ,
	    _cost_gini_mult  += num_resp_aligned)
	{
	    float p1 = aux_priors_mult[i];// * (num_resp - 1) * denom;
	    float cost_row_mult = 0.0;
	    for (int j = 0 ; j < num_resp ; j++ )
	    {
		float cost = _cost_mat[j] ;
		float cost_mult = cost * priors_mult[j] ;
		_cost_error_mult[j] = cost_mult;
		if (need_gini_mult)
		    _cost_gini_mult[j]  = cost_mult * p1;
		cost_row_mult += (cost / cost_col_sum[j]);
	    }
	    cost_row_sum[i] = cost_row_mult * (num_resp - 1) * denom * denom;
	}
	memcpy(priors_mult , aux_priors_mult , sizeof(float) * num_resp );
	if (need_gini_mult)
	{
	    _cost_gini_mult  = cart->cost_gini_mult;
	    for (i = 0 ; i < num_resp ; i++,
		_cost_gini_mult  += num_resp_aligned)
	    {
		for (int j = 0 ; j < num_resp ; j++ )
		{
		    _cost_gini_mult[j] *= cost_row_sum[i];
		}
	    }
#ifdef _DEBUG
	    _cost_gini_mult  = cart->cost_gini_mult;
	    _cost_error_mult = cart->cost_error_mult;
	    _cost_mat = cart->params->cost_mat;
	    for (i = 0 ; i < num_resp - 1 ; i++,
		_cost_gini_mult  += (num_resp_aligned + 1),
		_cost_error_mult += (num_resp_aligned + 1),
		_cost_mat        += (num_resp_aligned + 1))
	    {
		//// Must be zeros on diagonal !!!!
		assert (*_cost_error_mult == 0);
		assert (*_cost_gini_mult  == 0);
		assert (*_cost_mat  == 0);
	    }
#endif
	    free(cost_col_sum);
	    free(aux_priors_mult);
	    _cost_gini_mult = cart->cost_gini_mult;
	    float* base = cart->cost_gini_mult;
	    //// Upper triangle += lower triangle ( symmetrize ).
	    for (i = 0 ; i < num_resp - 1; i++ , base += (num_resp_aligned + 1))
	    {
		float* _cost_gini_mult1 = base + num_resp_aligned;
		_cost_gini_mult = base + 1;
		for (int j = i + 1 ; j < num_resp ; j++ ,
		    _cost_gini_mult++ ,
		    _cost_gini_mult1 += num_resp_aligned)
		{
		    *_cost_gini_mult += (*_cost_gini_mult1);
		    *_cost_gini_mult1 = (*_cost_gini_mult);
		}
	    }
	}
    }
}

CART_API void cxInitSubjBody(CxCARTSubj* subj,
			     CxClassifierSample* sample, BOOL copy_features,
			     int max_num_samples, int gulp_chunk)
{
    int cur_offset = 0;
    int features_step[2];
    int num_chunks = copy_features ? 1 : sample->num_chunks;

    subj->markup = (int*)(subj + 1);
    subj->num_chunks = num_chunks;

    assert(!copy_features);

    //// More then one chunk not supported!
    assert(sample->num_chunks == 1);
    int i;
    for (i = 0; i < sample->num_chunks; ++i )
    {
	CxClassifierSampleChunk* chunk = cxGetClassifierSampleChunk( sample, i );
	if ( !icxIsChunkLoaded( chunk ) )
	{
	    if ( !copy_features )
	    {
		subj->markup[i] = cur_offset;
	    }
	    continue;
	}

	CX_CART_MAKE_LOCAL( int, chunk, num_points );
	CX_CART_MAKE_LOCAL( int, chunk, num_indices_of_interest );
	CX_CART_MAKE_LOCAL( int *, chunk, indices_of_interest );
	CX_CART_MAKE_LOCAL( CxClassifierResponse *, chunk, responses_data );
	CX_CART_MAKE_LOCAL( int, chunk, responses_step );

	char* data = (char *) chunk->features_data;
	features_step[0] = chunk->features_step[0];
	features_step[1] = chunk->features_step[1];

	if ( !copy_features )
	{
	    cxLockClassifierSampleChunk( sample, i );
	    int limit = indices_of_interest ? num_indices_of_interest : num_points;
	    CxCARTAtom* atom = subj->body = (CxCARTAtom*)(subj->markup + num_chunks);
	    subj->response = (CxClassifierResponse*)(subj->body + limit);
	    subj->cur_split_directions = (char*)(subj->response + limit);
	    //// Fill subj with data
	    for (int j = 0; j < limit; j++ ,atom++)
	    {
		int indice = indices_of_interest ? indices_of_interest[j] : j ;
		atom->features = (CxClassifierVar*)(data + features_step[0] * indice);
		atom->missed_mask = chunk->missed_data ? (char*)(chunk->missed_data + chunk->missed_step[0] * indice) : NULL;
		CxClassifierVar var = *(CxClassifierResponse *)((char *)responses_data + responses_step * indice);
		atom->response = var;
		subj->response[j] = var;

		cur_offset++;
		if ( !gulp_chunk && (cur_offset == max_num_samples))
		    break;
	    }
	    if ( cur_offset >= max_num_samples )
		break;
	}
	else
	{
	    assert(0);
	}
    }
    subj->size = cur_offset;

    subj->features_step = features_step[1];
    subj->missed_mask_step = sample->chunk[0]->missed_step[1];
    for ( i = 0; i < sample->num_chunks; i++ )
	subj->markup[i] = cur_offset;

    subj->is_valid = 1;
    subj->ref_count = 1;
}


CART_IMPL BOOL cxBuildCARTSubj( CxCART* cart,  CxCARTNode* node ,
			       CxClassifierSample* sample,
			       BOOL expand_idx , BOOL use_calc_storage ,
			       int copy_features, int max_num_samples,
			       int gulp_chunk)
{
    CV_FUNCNAME( "cxBuildCARTSubj" );
    __BEGIN__;
    {

	CxClassifierSampleChunk* chunk = sample->chunk[0];
	int* indices_of_interest = chunk->indices_of_interest;
	int num_indices_of_interest = indices_of_interest ? chunk->num_indices_of_interest : chunk->num_points;


	if ( cart == 0 || sample == 0 )
	{
	    CV_ERROR( CV_StsNullPtr, "" );
	}
	if ( sample->num_features < cart->num_features )
	{
	    CV_ERROR( CV_StsBadSize, "incompatible sample" );
	}

	int num_chunks = copy_features ? 1 : sample->num_chunks;

	int size = sizeof(CxCARTSubj);						    // subj
	size += sizeof(int) * num_chunks;							    // subj->markup
	size += sizeof(CxCARTAtom) * num_indices_of_interest;           // subj->body
	size += sizeof(CxClassifierResponse) * num_indices_of_interest; // subj->response
	size += sizeof(char) * num_indices_of_interest;					// subj->cur_split_directions

	CxCARTSubj* subj = (CxCARTSubj*) malloc(size);
	if (!subj)
	{
	    icxSetCARTError(cart , CART_MEMORY_ERROR ,
		"Memory allocation error" , CART_ERROR_STATUS_FATAL);
	    return FALSE;
	}
	cxInitSubjBody(subj, sample, copy_features, max_num_samples, gulp_chunk);
	node->subj = subj;
	cart->storage->num_samples = subj->size;
	cart->root = node;
	cxBuildCARTNodeMasterFallenIdx( cart, node,expand_idx , use_calc_storage  );
	subj->ref_count = 1;
	if (cart->error.code > 0)
	    return FALSE;
	BOOL classification = icxIsClassifierVarCategoric(cart->response_type);
	//// Calculate priors multipliers ; for MART do it separately.
	if (classification)
	{
	    cxCreatePriorsCostMult((CxCARTBase*)cart,
		node->fallen_stats, node->num_fallens);
	}
	return TRUE;
    }
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxPropagateCARTSubj( CxCART* cart, CxCARTNode* node,
				   BOOL shrink_idx, BOOL use_calc_storage )
				   //int max_depth, int min_num_samples, int replace_valid, int propagate_to_competitors,
{
    CV_FUNCNAME( "cxPropagateCARTSubj" );
    __BEGIN__;


    int i, j;
    CxCARTNode* child[2];
    int *idx;

    if ( cart == 0 || cart->root == 0 )
    {
	//CV_ERROR( CV_StsNullPtr, "" );
	assert(0);
    }

    if ( node == 0 )
	node = cart->root;

    CX_CART_MAKE_LOCAL( int *, node, fallen_stats );
    CX_CART_MAKE_LOCAL( int *, node, fallen_idx );
    CX_CART_MAKE_LOCAL( int, node, num_fallens );
    CxCARTSubj* subj = node->subj;
    CxCARTAtom* body = subj->body;

    child[0] = node->child_left;
    child[1] = node->child_right;
    if ( child[0] == 0 )
	return;

   	START_FUNC(cxPropagateCARTSubj);

	assert(node && node->split);
	CX_CART_MAKE_LOCAL( int, node->split, feature_idx );
	CxClassifierVarType type = cart->feature_type[feature_idx];

	CxCARTSplit* split = node->split;

	type = cart->feature_type[split->feature_idx];

	int num_fallens_sides[2];
	int real_left_fallens = 0;
	int features_step = subj->features_step;
	int missed_mask_step = subj->missed_mask_step;

	char* cur_split_directions = subj->cur_split_directions;
	int eff_feature = cart->features_corr[split->feature_idx];
	assert( eff_feature >= 0);
	int direction = -1;
	if (node->is_idx_expanded)
	{
	    int num_nonmissed = num_fallens - node->num_missed[eff_feature];
	    int right_nonmissed = 0;
	    idx = fallen_idx + eff_feature * num_fallens;

	    //// Calculate the number of strings with nonmissing split feature going left
	    for ( j = 0; j < num_nonmissed; j++ )
	    {
		assert(idx[j]>=0 );
		right_nonmissed += cur_split_directions[idx[j]]; //  0 - left, 1 - right
	    }
	    FUNC_MEM_READ(cxPropagateCARTSubj, num_nonmissed, int);
	    FUNC_MEM_READ(cxPropagateCARTSubj, num_nonmissed, char);

	    //// Direction for strings with all surrogate variables missed
	    direction = (num_nonmissed - right_nonmissed > right_nonmissed) ? -1 : 1; // -1 - left, 1 - right
	    node->direction = direction;

#ifdef _DEBUG
	    for ( j = 0; j < num_nonmissed; j++ )
	    {

		int str_num = idx[j];
		CxCARTAtom* atom = body + str_num;
		assert(cur_split_directions[str_num] == 1 - cxIsStringSplitLeft(cart , node , atom->features ,
		    features_step , atom->missed_mask ,
		    missed_mask_step , split));// , direction));
	    }
	    /*		BOOL err = ((node->id == 14) && (node->depth == 3) && (node->num_fallens == 349) && fabs(split->value.fl - 2740.65) < 0.1);
	    if (err) {	int mmm = 1; }*/
#endif
	    //// Calculate sides for strings with missed split feature
	    int right_missed = 0;
	    for ( j = num_nonmissed; j < num_fallens; j++ )
	    {
		int str_num = idx[j];
		CxCARTAtom* atom = body + str_num;
		assert(cur_split_directions[str_num] == -1);
		cur_split_directions[str_num] = (char)(1 - cxIsStringSplitLeft(cart , node , atom->features ,
		    features_step , atom->missed_mask ,
		    missed_mask_step , split));// , direction);
		right_missed += cur_split_directions[str_num];
	    }
	    /// Read str_num
	    FUNC_MEM_READ(cxPropagateCARTSubj, num_fallens - num_nonmissed , int);
	    /// Read atom
	    FUNC_MEM_READ(cxPropagateCARTSubj, num_fallens - num_nonmissed , CxCARTAtom);
	    /// Write cur_split_directions[str_num]
	    FUNC_MEM_WRITE(cxPropagateCARTSubj, (num_fallens - num_nonmissed), char);
	    /// Read cur_split_directions[str_num]
	    FUNC_MEM_READ(cxPropagateCARTSubj, (num_fallens - num_nonmissed), char);

	    num_fallens_sides[0] = num_fallens - (right_missed + right_nonmissed);
#ifdef _DEBUG
	    int left = (icxIsClassifierVarNumeric( type ) ? cxFindNumericSplitProportion(cart, node,split, &real_left_fallens) :
	    cxFindCategoricSplitProportion(cart,node,split, &real_left_fallens)) ;
	    assert(real_left_fallens == num_nonmissed - right_nonmissed);
	    assert(num_fallens_sides[0] == left);
#endif
	}
	else
	{
	/*  Calculate number of cases that will fall to the left or to the right.
	We do not know it yet. real_left_fallens is number of cases with
	    nonmissing split variabnle that fall to the left  */
	    //num_fallens_sides[0] = cxFindAnySplitProportion(cart,node,split,&real_left_fallens) ;
	    int total_left_fallens = 0;
	    int num_nonmissed = 0;
	    direction = node->direction;
	    if (!direction)
	    {
		//// Direction is unknown
		real_left_fallens =
		    cxFindAnySplitLeftNonmissing((CxCARTBase*)cart,
		    (CxForestNode*)node ,split, &num_nonmissed);
		direction = (real_left_fallens > num_nonmissed - real_left_fallens) ? -1 : 1;
		node->direction = direction;
	    }
#ifdef _DEBUG
	    else
	    {
		real_left_fallens =
		    cxFindAnySplitLeftNonmissing((CxCARTBase*)cart,
		    (CxForestNode*)node ,split, &num_nonmissed);
		int direction1 = (real_left_fallens > num_nonmissed - real_left_fallens) ? -1 : 1;
		assert(direction1 == direction);
	    }
#endif
	    for ( j = 0; j < num_fallens; j++ )
	    {
		int str_num = fallen_idx[j];
		CxCARTAtom* atom = body + str_num;
		int side = cxIsStringSplitLeft(cart , node , atom->features ,
		    features_step , atom->missed_mask ,
		    missed_mask_step , split);// , direction);
		cur_split_directions[str_num] = (char)(1 - side);
		total_left_fallens += side;
	    }
	    num_fallens_sides[0] = total_left_fallens;
	}
	num_fallens_sides[1] = num_fallens - num_fallens_sides[0];
	//	int min_points = MAX(cart->params->split_min_points , 1);
	//	assert( (num_fallens_sides[0] >= min_points) && (num_fallens_sides[1] >= min_points));

	///// Save / load check - that same number of strings goes to the left and to the right ////////
	assert(child[0]->num_fallens == 0 || child[0]->num_fallens == num_fallens_sides[0]);
	assert(child[1]->num_fallens == 0 || child[1]->num_fallens == num_fallens_sides[1]);

	BOOL expand_idx = (!shrink_idx) && node->is_idx_expanded;

	/*  Allocate child nodes internal arrays , right , then left,
	it's important when using storage */

	child[1]->num_fallens = num_fallens_sides[1];
	cxAllocNodeInternals(cart , child[1], expand_idx, use_calc_storage);
	if (cart->error.code > 0)
	{
	    EXIT_FUNC(cxPropagateCARTSubj);
	    return;
	}

	child[0]->num_fallens = num_fallens_sides[0];
	cxAllocNodeInternals(cart , child[0], expand_idx, use_calc_storage);
	if (cart->error.code > 0)
	{
	    EXIT_FUNC(cxPropagateCARTSubj);
	    return;
	}

	child[0]->subj = subj;
	child[1]->subj = subj;

	BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
	int num_resp = icxGetVarNumStates(cart->response_type);
	int num_resp_aligned = num_resp;
	ALIGN_NUM_RESP(num_resp_aligned)

	    int eff_num_features = expand_idx ? cart->eff_num_features : 1;
#if DATA_TO_FILE4
	FILE* file1 = fopen("d:\\CART\\AlamereTest\\prop_idx_cat_class.dat", "wb");
	int selected_var = -1;
#endif
	//// Propagate cases , filling child fallen_idx and num_missed arrays ////////
	for ( i = 0; i < eff_num_features ; i++ )
	{
	    int cur_feature = cart->features_back_corr[i];
	    idx = fallen_idx + i * num_fallens;

	    int num_nonmissed = num_fallens - node->num_missed[i];
	    int* cur_child_fallen_idx[2];
	    cur_child_fallen_idx[0] = child[0]->fallen_idx + i * num_fallens_sides[0];
	    cur_child_fallen_idx[1] = child[1]->fallen_idx + i * num_fallens_sides[1];

	    BOOL categoric = icxIsClassifierVarCategoric(cart->feature_type[cur_feature]);
	    int* cat_proportions = node->cat_proportions ? node->cat_proportions[i] : NULL;
#ifdef _DEBUG
	    if (!expand_idx)
		assert(shrink_idx || !cat_proportions);
#endif
	    if (expand_idx && cat_proportions)
	    {
		int* cur_child_cat_proportions[2];
		assert ( child[0]->cat_proportions && child[1]->cat_proportions &&
		    child[0]->cat_proportions[i] && child[1]->cat_proportions[i] );
		cur_child_cat_proportions[0] = child[0]->cat_proportions[i];
		cur_child_cat_proportions[1] = child[1]->cat_proportions[i];
		if (categoric)
		{
		    int num_classes = cart->num_classes[cur_feature];
		    if (regression)
		    {
			//// Propagate category proportions
			for (int* _idx = idx  , k = 0 ; k < num_classes; k++)
			{
			    int num_var_class = cat_proportions[k];
			    int sum[2] = {0,0};
			    assert(num_var_class >=0);
			    for (j = 0 ; j < num_var_class ; j++, _idx++)
			    {
				int str_num = (*_idx);
				int side = cur_split_directions[str_num];
#ifdef _DEBUG
				CxClassifierVar feat_var = icxGetSubjFeature(subj , str_num , cur_feature);
				assert( cvRound(feat_var.fl) == k);
#endif
				sum[side]++;

				(*(cur_child_fallen_idx[side]++)) = str_num;

			    }
			    cur_child_cat_proportions[0][k] = sum[0];
			    cur_child_cat_proportions[1][k] = sum[1];
			}
			// Read sum[0] and sum[1]
			FUNC_MEM_READ(cxPropagateCARTSubj , 2*num_classes , int);
			// Read cur_child_cat_proportions[0, 1]
			FUNC_MEM_READ(cxPropagateCARTSubj , 2*num_classes , int*);
			// Write sum[0, 1] -> cur_child_cat_proportions[0, 1][k]
			FUNC_MEM_WRITE(cxPropagateCARTSubj , 2*num_classes , int);
		    }
		    else
		    {
#if DATA_TO_FILE4
			fwrite(&num_fallens, sizeof(int), 1, file1);
			fwrite(&num_nonmissed, sizeof(int), 1, file1);
			fwrite(&num_classes, sizeof(int), 1, file1);
			fwrite(&num_resp, sizeof(int), 1, file1);
			fwrite(&cart->root->num_fallens, sizeof(int), 1, file1);

			fwrite(cur_split_directions, sizeof(char), cart->root->num_fallens, file1);
			fwrite(idx, sizeof(int), num_fallens, file1);
			fwrite(cat_proportions, sizeof(int), num_classes * num_resp_aligned, file1);

			selected_var = i;
			fflush(file1);
#endif
			//// Propagate category-response frequency tables
			for (int* _idx = idx , k = 0 ; k < num_classes; k++)
			{
			    for (int resp = 0 ; resp < num_resp; resp++)
			    {
				int group_num = k * num_resp_aligned + resp;
				int group_size = cat_proportions[group_num];
				assert(group_size >=0);
				int sum[2] = {0,0};
				for (j = 0 ; j < group_size ; j++, _idx++)
				{
				    int str_num = (*_idx);
				    int side = cur_split_directions[str_num];
#ifdef _DEBUG
				    CxClassifierVar feat_var = icxGetSubjFeature(subj , str_num , cur_feature);
				    assert( cvRound(feat_var.fl) == k);
				    CxClassifierVar resp_var = icxGetSubjResponse(subj , str_num);
				    assert( cvRound(resp_var.fl) == resp);
#endif
				    sum[side]++;
				    (*(cur_child_fallen_idx[side]++)) = str_num;
				}
				cur_child_cat_proportions[0][group_num] = sum[0];
				cur_child_cat_proportions[1][group_num] = sum[1];
			    }
			}
		    }
		    // Read sum[0] and sum[1]
		    FUNC_MEM_READ(cxPropagateCARTSubj , 2 * num_resp * num_classes , int);
		    // Read cur_child_cat_proportions[0, 1]
		    FUNC_MEM_READ(cxPropagateCARTSubj , 2 * num_resp * num_classes , int*);
		    // Write sum[0, 1] -> cur_child_cat_proportions[0, 1][k]
		    FUNC_MEM_WRITE(cxPropagateCARTSubj , 2 * num_resp * num_classes , int);

		}
		else
		{
		    //// Propagate sorted feature values
		    for ( j = 0; j < num_nonmissed; j++ )
		    {
			int str_num = idx[j];
			int side = cur_split_directions[str_num];
			(*(cur_child_fallen_idx[side]++)) = str_num;
			(*(cur_child_cat_proportions[side]++)) = cat_proportions[j];
		    }
		    // Read cat_proportions[j]
		    FUNC_MEM_READ(cxPropagateCARTSubj , num_nonmissed , int);
		    // Modify cur_child_cat_proportions[side]
		    FUNC_MEM_MODIFY(cxPropagateCARTSubj , num_nonmissed , int);
		    // Write cat_proportions[j]
		    FUNC_MEM_WRITE(cxPropagateCARTSubj , num_nonmissed , int);
		}
		}
		else
		{
		    //// Just propagate fallen_idx
		    for ( j = 0; j < num_nonmissed; j++ )
		    {
			int str_num = idx[j];
			(*(cur_child_fallen_idx[cur_split_directions[str_num]]++)) = str_num;
		    }
		}
		// Read str_num and side.
		FUNC_MEM_READ(cxPropagateCARTSubj , num_nonmissed, char);
		FUNC_MEM_READ(cxPropagateCARTSubj , num_nonmissed, int);
		// Modify cur_child_fallen_idx[side]
		FUNC_MEM_MODIFY(cxPropagateCARTSubj , num_nonmissed , int*);
		// Write str_num
		FUNC_MEM_WRITE(cxPropagateCARTSubj , num_nonmissed , int);


		/*  Propagate strings with missed i-th effective feature, calculating num_missed
		for	children */
		//		assert (expand_idx || (num_nonmissed == num_fallens));
		int child_missed[2] = { 0 , 0 };
		for ( j = num_nonmissed; j < num_fallens; j++ )
		{
		    int str_num = idx[j];
		    int side = cur_split_directions[str_num];
		    (*(cur_child_fallen_idx[side]++)) = str_num;
		    child_missed[side] ++;
		}
		// Read str_num and side.
		FUNC_MEM_READ(cxPropagateCARTSubj , num_fallens - num_nonmissed, char);
		FUNC_MEM_READ(cxPropagateCARTSubj , num_fallens - num_nonmissed, int);
		// Modify cur_child_fallen_idx[side]
		FUNC_MEM_MODIFY(cxPropagateCARTSubj , num_fallens - num_nonmissed , int*);
		// Write str_num
		FUNC_MEM_WRITE(cxPropagateCARTSubj , num_fallens - num_nonmissed , int);


		child[0]->num_missed[i] = child_missed[0];
		child[1]->num_missed[i] = child_missed[1];

		assert( cur_child_fallen_idx[0] - (i + 1) * num_fallens_sides[0] == child[0]->fallen_idx);
		assert( cur_child_fallen_idx[1] - (i + 1) * num_fallens_sides[1] == child[1]->fallen_idx);

    }
    //// Building children fallen stats
    icxBuildCARTNodeFallenStats( cart, child[0] );
    for ( i = num_resp; i--; )
	child[1]->fallen_stats[i] = fallen_stats[i] - child[0]->fallen_stats[i];
    //// Memory corruption tests
    assert((unsigned)node->split->revert < 2);
    //// Building children response
    EXIT_FUNC(cxPropagateCARTSubj);
#if DATA_TO_FILE4
    if (selected_var > -1)
    {
	int real_var = cart->features_back_corr[selected_var];
	int n0 = child[0]->num_fallens;
	int n1 = child[1]->num_fallens;

	int m0 = child[0]->num_missed[selected_var];
	int m1 = child[1]->num_missed[selected_var];

	fwrite(&n0, sizeof(int), 1, file1);
	fwrite(&n1, sizeof(int), 1, file1);

	fwrite(&m0, sizeof(int), 1, file1);
	fwrite(&m1, sizeof(int), 1, file1);

	fwrite(child[0]->fallen_idx + selected_var * n0, sizeof(int), n0, file1);
	fwrite(child[1]->fallen_idx + selected_var * n1, sizeof(int), n1, file1);


	fwrite(child[0]->cat_proportions[selected_var] , sizeof(int),
	    num_resp_aligned * cart->num_classes[real_var], file1);

	fwrite(child[1]->cat_proportions[selected_var] , sizeof(int),
	    num_resp_aligned * cart->num_classes[real_var], file1);

	fclose(file1);

	cart->error.code = CART_UNKNOWN_ERROR;
	cart->error.status = CART_ERROR_STATUS_FATAL;
    }
#endif

    cxPropagateCARTSubj( cart, child[0], shrink_idx , use_calc_storage);
    cxPropagateCARTSubj( cart, child[1], shrink_idx , use_calc_storage);

    __CLEANUP__;
    __END__;
}

#if TEST_MARTVW
#include "cxmart.h"
#endif

CART_IMPL CxCARTSplit* cxFindCARTNodeGoodSplits( CxCART *cart,
						CxCARTNode *node,
						int num_splits,
						float threshold)
{
    CV_FUNCNAME( "cxFindCARTNodeGoodSplits" );
    __BEGIN__;

    int i;
    CxClassifierVarType type;
    assert(node->is_idx_expanded);

    assert ( cart && node && cart->root );

    if ( num_splits <= 0 )
	return 0;
    //	ofstream file ("clustering.log" , ios::app);

    int cur_size = 0;
    num_splits = MIN( num_splits , cart->eff_num_features );
    int eff_num_features = cart->eff_num_features;
    CxCARTSplit** split_array = (CxCARTSplit**)calloc(eff_num_features , sizeof(CxCARTSplit*));
    CxCARTSplit* split = NULL;
    float weight = 0.0;
    float eps_weight = node->parent ? cart->root->split->weight * EPS : EPS;
    int num_good_splits = 0;
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    if (regression)
    {
	for ( i = 0; i < eff_num_features; i++ )
	{
	    int cur_feature = cart->features_back_corr[i];
	    split = cxCARTSplit(cart, cur_feature );
	    type = cart->feature_type[cur_feature];
	    if (icxIsClassifierVarNumeric(type))
	    {
		weight = cxFindNodeNumericBestSplitRegression(cart,node,cur_feature,split) ;
	    }
	    else
	    {
		weight = cxFindNodeCategoricBestSplitRegression(cart,node,cur_feature,split) ;
	    }
	    if (weight > eps_weight)
	    {
		//// Add this split
		split_array[num_good_splits++] = split;
	    }
	}
    }
    else
    {
	int num_fallens = node->num_fallens;
	int* num_missed = node->num_missed;
	int max_clusters = cart->params->max_clusters;
	for ( i = 0; i < eff_num_features; i++ )
	{
	    int cur_feature = cart->features_back_corr[i];
	    split = cxCARTSplit(cart, cur_feature );
	    type = cart->feature_type[cur_feature];
	    if (icxIsClassifierVarNumeric(type))
	    {
		weight = cxFindNodeNumericBestSplitClassification(cart,node,cur_feature,split) ;
	    }
	    else
	    {
		//	CxVarCategoryCluster* clusters = cxClusterizeFeatureAgglomerative(cart , node, cur_feature , cart->params->max_clusters);
#ifdef _DEBUG
		int num_resp = cart->num_response_classes;
		int num_cats = cart->num_classes[cur_feature];
		assert( num_cats >= 0 && num_cats < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);
		int num_resp_aligned = num_resp;
		ALIGN_NUM_RESP( num_resp_aligned );
		int* fsp1 = (int*)icxAlignedCalloc(num_cats * num_resp_aligned, sizeof(int));
		cxCalcFrequencies(fsp1, cart, node, cur_feature);
		assert(memcmp(fsp1 , node->cat_proportions[i] , num_cats * num_resp_aligned * sizeof(int)) == 0);
		icxFreeAligned(fsp1);
#endif

		CxVarCategoryCluster* clusters =
		    cxClusterizeFeatureKMeans((CxCARTBase*)cart , node->cat_proportions[i] ,
		    cur_feature, cart->params->max_clusters);
		if (clusters)
		{
		    weight = cxFindNodeCategoricClusteringBestSplit((CxRootedCARTBase*)cart,clusters,
			max_clusters, num_fallens - num_missed[i],
			cur_feature, split);
		    cxFreeClusters(clusters);
		}
		else
		{
		    weight = cxFindNodeCategoricBestSplitClassification(cart,node,cur_feature,split) ;
		}

		/*		double weight1 = cxFindNodeCategoricBestSplit(cart,node,cur_feature,split);
		if (clusters)
		file << "Feature : " << cur_feature << " Clustering weight : " << weight << " Weight : " << weight1 << '\n';*/

	    }
	    //		file.flush();
	    if (weight > eps_weight)
	    {
		//// Add this split
		split_array[num_good_splits++] = split;
	    }
	}
    }
    split = NULL;

    if (num_good_splits > 0)
    {
	int i;
	assert( num_good_splits <= eff_num_features );

	//// Sort in weight ascending order
	qsort(split_array , num_good_splits , sizeof(CxCARTSplit*) , compare_splits);
	CxCARTSplit* split_prev = NULL;
	cur_size = MIN ( num_good_splits , num_splits);
	float split_thresh = threshold * split_array[0]->weight;
	for ( i = 0; i < cur_size ; i++)
	{
	    if (split_array[i]->weight < split_thresh)
	    {
		cur_size = i;
		break;
	    }
	}
	//// Copy first cur_size splits to node buffer
	cxAddNodeSplits( cart, node , split_array , cur_size , SPLIT_COMPETITOR );
	for ( i = cur_size - 1; i >= 0 ; i-- )
	{
	    split_prev = split;
	    split = split_array[i];
#ifdef _DEBUG
	    //file << "Competitor in node : "<< node->id << ", weight = " << split->weight << '\n';
#endif
	    split->next_competitor = split_prev;
	}
    }
    free(split_array);
    return split;

    __CLEANUP__;
    __END__;
}

CART_IMPL BOOL cxInitCARTBase(CxCARTBase* cart,
			      int num_features,
			      int* feature_type,
			      int response_type,
			      CxCARTTrainParams* params)
{
    CV_FUNCNAME( "cxInitCARTBase" );
    __BEGIN__;
    {
	assert(cart);
	if (!cart)
	    return FALSE;
	if ( num_features < 0 )
	{
	    CV_ERROR( CV_StsBadArg, "invalid number of features" );
	}
	if ( feature_type == 0 )
	{
	    CV_ERROR( CV_StsNullPtr, "" );
	}
	assert( params );
	BOOL classification = icxIsClassifierVarCategoric(response_type);
	int priors_mult_size = 0;
	int cost_mult_size = 0;

	int num_resp = icxGetVarNumStates(response_type);
	int num_resp_aligned = num_resp;
	ALIGN_NUM_RESP(num_resp_aligned )
	    BOOL need_gini_mult = (params->splitting_rule == CxCARTGiniCriterion);

	if (classification)
	{
	    priors_mult_size = num_resp_aligned * sizeof(float) + CART_MEM_ALIGN;
	    if (params->cost_mat )
		cost_mult_size = (need_gini_mult + 1) * num_resp_aligned * num_resp_aligned * sizeof(float) ;
	}

	cart->num_features = num_features;
	cart->features_corr = (int *)malloc( sizeof( int ) * num_features * 4 +
	    priors_mult_size + cost_mult_size);
	if (!cart->features_corr)
	{
	    icxSetCARTError(cart , CART_MEMORY_ERROR ,
		"Memory allocation error" , CART_ERROR_STATUS_FATAL);
	    return FALSE;
	}

	memset(cart->features_corr , -1 , sizeof( int ) * num_features * 3 );
	cart->features_back_corr = cart->features_corr + num_features;
	cart->num_classes = cart->features_back_corr + num_features;
	cart->feature_type = cart->num_classes + num_features;
	if (classification)
	{
	    cart->priors_mult = (float*) icxAlignUp( ((char*)(cart->feature_type + num_features)) , CART_MEM_ALIGN);
	    if (params->cost_mat)
	    {
		cart->cost_error_mult = cart->priors_mult + num_resp_aligned ;
		if (need_gini_mult)
		    cart->cost_gini_mult = cart->cost_error_mult + num_resp_aligned * num_resp_aligned;
	    }
	}

	cart->params = params;
	memcpy( cart->feature_type, feature_type, sizeof( int ) * num_features );
	assert(response_type >= 0);
	cart->response_type = response_type;
	if ( params->features_of_interest )
	{
	    cart->eff_num_features = params->num_features_of_interest;
	    for (int i = 0; i < cart->eff_num_features; ++i )
	    {
		int real_feature = params->features_of_interest[i];
		cart->features_back_corr[i] = real_feature;
		cart->features_corr[real_feature] = i;
	    }
	}
	else
	{
	    cart->eff_num_features = num_features;
	    for (int i = 0; i < cart->eff_num_features; ++i )
	    {
		cart->features_corr[i] = i;
		cart->features_back_corr[i] = i;
	    }
	}
	for (int i = 0; i < num_features; ++i )
	{
	    CxClassifierVarType type = feature_type[i];
	    assert(!icxIsClassifierVarTranscendent( type ));

	    if ((type < 1) || (type > CX_CLASSIFIER_VAR_TYPE_NUMERIC_32F) || icxIsClassifierVarTranscendent( type ))
	    {
		char buf[250];
		sprintf(buf , "Type of feature %d incorrect : %d" , i , type);
		icxSetCARTError(cart , CART_INVALID_PARAMS_ERROR ,
		    buf , CART_ERROR_STATUS_FATAL);
		return FALSE;
	    }
	    if ( icxIsClassifierVarNumeric( type ) )
	    {
		cart->num_classes[i] = 0;
		if (icxGetVarNumStates(feature_type[i]) != 0)
		{
		    char buf[250];
		    sprintf(buf , "Type of feature %d incorrect : %d\n, numeric variable cannot have categories" , i , type);
		    icxSetCARTError(cart, CART_INVALID_PARAMS_ERROR ,
			buf, CART_ERROR_STATUS_FATAL);

		}
	    }
	    else
	    {
		cart->num_classes[i] = icxGetVarNumStates( feature_type[i] );
		assert( cart->num_classes[i] != -1);
	    }
	}
	if ( icxIsClassifierVarNumeric( response_type ) )
	{
	    cart->num_response_classes = 0;
	}
	else
	{
	    cart->num_response_classes = icxGetVarNumStates( response_type );
	}
	return TRUE;
}
__CLEANUP__;
__END__;
}

static CxClassifierFuncTable vftbl_cart_reg =
{
    &release_cart,
	(create_t)&cxCreateCART,
	(eval_t)&cxCARTEvalFeatures,
	(predict_regression_t)&cxCalcCARTAverageResponse,
	(predict_classification_t)NULL,
	(calc_test_errors_t)&cxCalcCARTTestErrors,
	(calc_test_errors_pred_t)&cxCalcCARTTestErrorsAndPredictions,
	(dump_t)&cxDumpCART,
	(get_var_imps_t)&cxCalcCARTVariableImportances,
	(write_text_t)&cxWriteCART,
	(read_text_t)&cxReadCART,
	(write_node_t)&cxWriteCARTNode
};

static CxClassifierFuncTable vftbl_cart_cl =
{
    &release_cart,
	(create_t)&cxCreateCART,
	(eval_t)&cxCARTEvalFeatures,
	(predict_regression_t)NULL,
	(predict_classification_t)&cxCalcCARTProbabilities,
	(calc_test_errors_t)&cxCalcCARTTestErrors,
	(calc_test_errors_pred_t)&cxCalcCARTTestErrorsAndPredictions,
	(dump_t)&cxDumpCART,
	(get_var_imps_t)&cxCalcCARTVariableImportances,
	(write_text_t)&cxWriteCART,
	(read_text_t)&cxReadCART,
	(write_node_t)&cxWriteCARTNode
};

static int vbtbl_cart[] = {1,0};

CART_IMPL CxCART* cxCreateCART(int num_features,
			       int* feature_type,
			       int response_type,
			       CxCARTTrainParams* params,
			       CxCART* cart)
{
    CV_FUNCNAME( "cxCreateCART" );
    __BEGIN__;
    {
	if (!cart)
	    cart = (CxCART*)calloc( 1 , sizeof(CxCART) );
	if (cxInitCARTBase((CxCARTBase*)cart,num_features, feature_type, response_type, params))
	{
	    cart->flags = CX_CLASSIFIER_KIND_CART;
	    cart->progress_info.progress_data_step = sizeof(CxPruningData);

	    if ( icxIsClassifierVarNumeric( response_type ) )
	    {
		cart->vftbl = &vftbl_cart_reg;
	    }
	    else
	    {
		cart->vftbl = &vftbl_cart_cl;
	    }
	    cart->vbtbl = vbtbl_cart;
	}
	return cart;
    }
    __CLEANUP__;
    __END__;
}


CART_IMPL CxCART* cxBuildVirginCART( CxClassifierSample *sample,
				    CxCARTTrainParams *params,
				    CxExtCARTInfo* ext_info)
{
    CV_FUNCNAME( "cxBuildVirginCART" );
    __BEGIN__;
    ENTER_FUNC("CART","cxBuildVirginCART");

    {
	if ( sample == 0 )
	{
	    CV_ERROR( CV_StsNullPtr, "" );
	}

	CxCART* cart = cxCreateCART(sample->num_features, sample->feature_type,
	    sample->response_type, params, NULL);
	if (cart->error.code != 0)
	    return cart;

	cart->extra_info = ext_info;
	if (ext_info)
	    ext_info->samples_of_interest = sample->chunk[0]->indices_of_interest;

	//// Return if categoric response with 0 categories
	if ( icxIsClassifierVarCategoric( cart->response_type ) && (cart->num_response_classes < 1))
	{
	    icxSetCARTError(cart, CART_CALC_ERROR ,
		"Categoric response with less zero values!",
		CART_ERROR_STATUS_NONFATAL);
	    LEAVE_FUNC( "CART", "cxBuildVirginCART" , CX_RETURN_TERMINATE);
	    return cart;
	}

	if ( sample->num_chunks > 1 )
	{
	    assert(0);
	    //     expand maps
	}

	if (!cxInitNodeStorage(cart))
	    return cart;
	int num_samples = icxGetNumSamplesOfInterest(sample);

	WRITE_LOG_MESSAGE ( CART_LOG << "Setting node storage depth", CX_MESSAGE_COMMENT);
	if (!cxSetNodeStorageDepth(cart , num_samples, cart->params->tree_max_depth))
	    return cart;
	WRITE_LOG_MESSAGE ( CART_LOG << "Set node storage depth", CX_MESSAGE_COMMENT);

	CxCARTNode* root = cxAllocNode(cart, 1 , 0);
	cart->root = root;
	cart->num_nodes = 1;
	if (!cxCreateCalcStorage(cart , num_samples , 0 ))
	    return cart;

	if (cxBuildCARTSubj( cart, root, sample, cart->eff_num_features )&& (cart->error.code == 0))
	    cxBuildCARTNodeResponse( cart, root );
	LEAVE_FUNC( "CART", "cxBuildVirginCART" , CX_RETURN_OK);
	return cart;
    }
    __CLEANUP__;
    __END__;
    ///// TTT ///////


}

CART_INLINE void icxCalcNodeSplitDirections( CxCART* cart, CxCARTNode* node )
{
    CxCARTSubj* subj = node->subj;
    int num_fallens = node->num_fallens;
    CxCARTSplit* split = node->split;
    int feature_idx = split->feature_idx;
    int eff_feature = cart->features_corr[feature_idx];
    assert(eff_feature >= 0);
    int* idx = node->fallen_idx + eff_feature * num_fallens;
    int num_nonmissed = num_fallens - node->num_missed[eff_feature];
    int shift = feature_idx * subj->features_step;
    CxCARTAtom* body = subj->body;
    char* cur_split_directions = subj->cur_split_directions;
    int j = 0;
    //// Calculating split directions
    for ( ; j < num_nonmissed; j++ )
    {
	int str_num = idx[j];
	assert(str_num >= 0);
	CxCARTAtom* atom = body + str_num;
	CxClassifierVar var = *(CxClassifierVar *)((char*)atom->features + shift);
	cur_split_directions[str_num] = (char)(1 - icxIsVarSplitLeft(cart , split , var));
    }
    for (; j < num_fallens; j++ )
    {
	cur_split_directions[idx[j]] = -1;
#ifdef _DEBUG
	if (cur_split_directions[idx[j]] == -1)
	    assert(icxIsSubjFeatureMissed(subj , idx[j] , feature_idx));
#endif
    }
}

#if TRACE_MODE
typedef double my_type ;
#endif


CART_IMPL BOOL cxSplitCARTLeaf( CxCART* cart, CxCARTNode* node,
			       CxCARTSplit* split , BOOL not_shrink_idx)
{
    CV_FUNCNAME( "cxSplitCARTLeaf" );
    __BEGIN__;
    //	*((int*)0) = 0;

    if (split && cart->features_corr[split->feature_idx] < 0 || cart->error.code)
	return FALSE;
#ifdef CART_WITH_GUI
    if (g_hStopEvent)//(cart->m_hStopEvent)
    {
	if (::WaitForSingleObject(g_hStopEvent , 0) == WAIT_OBJECT_0)
	{
	    icxSetCARTError(cart , CART_CALC_ERROR ,
		"Calculations aborted by user!" , CART_ERROR_STATUS_FATAL);
	    return FALSE;
	}
    }
#endif

    CxCARTNode *left, *right;
    cxExpandNodeIdx(cart,node);

    if (cart->error.code > 0)
    {
	if (!not_shrink_idx)
	    cxShrinkNodeIdx(cart,node);
	return FALSE;
    }
#if TRACE_MODE
    //	SetTraceMode(TRUE);
#endif
    int num_competitors = cart->params->num_competitors + 1;
    //	if ( !split )	num_competitors++;
    //// Find competitor splits
    CxCARTSplit* best_split = NULL;
    if (split)
	cxAddNodeSplits(cart , node , &split , 1 , SPLIT_MANUAL);

    if (!node->split)
    {
	best_split = cxFindCARTNodeGoodSplits( cart, node, num_competitors ,
	    cart->params->competitor_threshold);
    }
    else
    {
	best_split = (icxIsSplitManual(cart , node) ? node->split->next_competitor : node->split);
	assert(best_split);
    }
#ifdef _DEBUG
    cxAssertNodeValid(cart , node);
#endif
    if (!best_split && !split)
    {
#if TRACE_MODE
    /*		SetTraceMode(FALSE);
    DumpStatistics();
	::MessageBox(NULL , "Tracing complete" , "Message" , MB_OK);*/
#endif
	if (!not_shrink_idx)
	    cxShrinkNodeIdx(cart,node);
	return FALSE;
    }

    //// Delete branch if it exists
    if (node->split)
    {
	int num_deleted = cxGetBranchWeight(node) - 1;
	node->split = NULL;
	cart->num_nodes -= num_deleted;
    }

    //// Create two child nodes
    int id_left = node->id * 2 ;
    int id_right = id_left + 1 ;
    int new_depth = node->depth + 1 ;

    left = cxAllocNode(cart , id_left , new_depth);
    right = cxAllocNode(cart , id_right , new_depth);

    cart->num_nodes += 2;
    node->child_left = left;
    node->child_right = right;
    left->parent = right->parent = node;
#ifdef _DEBUG
    cxAssertNodeValid(cart , node);
#endif

    if (split)
    {
	//		cxAddNodeSplits(cart , node , &split , 1 , SPLIT_MANUAL);
	node->split = split;
	split->next_competitor = best_split;
	//// Calculate impurity change for manual split
	split->weight = cxCalcImpurityChangeAny(cart, node, split) ;
    }
    else
	node->split = best_split;
    if (split && split->weight == 0.0)
    {
	node->split = 0;
	return FALSE;
    }
    icxCalcNodeSplitDirections(cart , node);
    //// Calculate surrogates
#ifdef _DEBUG
    cxAssertNodeValid(cart , node);
#endif
    if (cart->params->num_surrogates > 0)
    {
	cxFindCARTNodeSurrogates(cart,node,cart->params->num_surrogates ,
	    cart->params->surrogate_threshold );
    }
#ifdef _DEBUG
    cxAssertNodeValid(cart , node);
#endif
    //	if (node->id == 12 && node->num_fallens == 4199)
    //	{	int mmm = 1;}
    cxPropagateCARTSubj( cart, node );
    BOOL ok = (cart->error.code == 0);
    //// Build child responses
    if (ok)
    {
	cxBuildCARTNodeResponse( cart, left );
	cxBuildCARTNodeResponse( cart, right );
    }
    if (!not_shrink_idx)
	cxShrinkNodeIdx(cart,node);
#if TRACE_MODE
    SetTraceMode(FALSE);
    DumpStatistics();
    icxSetCARTError(cart , CART_CALC_ERROR ,
	"Tracing complete!" , CART_ERROR_STATUS_FATAL);
    return FALSE;
    //	::MessageBox(NULL , "Tracing complete" , "Message" , MB_OK);
#endif
    /*	time_end = GetTicks();
    char msg[256];
    sprintf(msg , "Ticks : %.3e" , (float)(time_end - time_start));
    ::MessageBox(NULL , msg , "Message" , MB_OK);
    icxSetCARTError(cart , CART_CALC_ERROR ,
    "Tracing complete!" , CART_ERROR_STATUS_FATAL);
    return FALSE;*/

#ifdef _DEBUG
    cxAssertNodeValid(cart , node);
#endif
    return ok;
    __CLEANUP__;
    __END__;

}

CART_IMPL CxCARTNode* cxCopyNode(CxCARTNode* node)
{
    CxCARTNode* copy = (CxCARTNode*)malloc(sizeof(CxCARTNode));
    memcpy(copy, node, sizeof(CxCARTNode));
    copy->is_copy = TRUE;
    copy->child_left = copy->child_right = NULL;
    copy->split = NULL;
    copy->direction = 0;
    return copy;
}

CART_IMPL CxCARTNode* cxTrySplitLeaf( CxCART *cart,
				     CxCARTNode* node,
				     CxCARTSplit* split )

{
    CV_FUNCNAME( "cxTrySplitLeaf" );
    __BEGIN__;

    CxCARTNode* copy = cxCopyNode(node);
    CxCARTNode* left = (CxCARTNode*)malloc(sizeof(CxCARTNode));
    CxCARTNode* right = (CxCARTNode*)malloc(sizeof(CxCARTNode));
   	icxInitNode(left);
	icxInitNode(right);
	copy->child_left = left;
	copy->child_right = right;
	left->parent = right->parent = copy;
	left->depth = right->depth = node->depth + 1;
	copy->split = split;
	cxPropagateCARTSubj( cart, copy , TRUE , FALSE);

	cxBuildCARTNodeResponse( cart, left );
	cxBuildCARTNodeResponse( cart, right );
	//   	assert(fabs(left->error - cxCalcNodeError(cart,left)) < EPS);
	//	assert(fabs(right->error - cxCalcNodeError(cart,right)) < EPS);

	return copy;
	__CLEANUP__;
	__END__;

}

CART_IMPL BOOL cxSplitCARTLeafFull (CxCART *cart,
				    CxCARTNode *node,
				    CxCARTSplit* split,
				    BOOL not_shrink_idx )
{
    CV_FUNCNAME( "cxSplitCARTLeafFull" );
    __BEGIN__;
    ENTER_FUNC( "CART", "cxSplitCARTLeafFull");
    int depth = node->depth;
    if (((node->num_fallens < cart->params->split_max_points) || (depth >= cart->params->tree_max_depth)) && !not_shrink_idx)
    {
	cxShrinkNodeIdx(cart, node);
	LEAVE_FUNC( "CART", "cxSplitCARTLeafFull", CX_RETURN_TERMINATE);
	return FALSE; // Cannot split more!
    }
    if (cxSplitCARTLeaf(cart,node,split, not_shrink_idx))
    {
	//// Left , then right : this is important!
	cxSplitCARTLeafFull(cart,node->child_left);
	cxSplitCARTLeafFull(cart,node->child_right);
    }
    else if (!not_shrink_idx)
	cxShrinkNodeIdx(cart, node);
    LEAVE_FUNC( "CART", "cxSplitCARTLeafFull", CX_RETURN_OK);
    __CLEANUP__;
    __END__;
    return TRUE;
}

CART_IMPL CxCART* cxBuildCART( CxClassifierSample *sample,
			      CxCARTTrainParams *params,
			      CxExtCARTInfo* ext_info)
{


    CV_FUNCNAME( "cxBuildCART" );
    __BEGIN__;
    CxCART* cart = cxBuildVirginCART( sample, params, ext_info);
    if (cart && (cart->error.code == 0) && cart->eff_num_features)
	cxGrowCARTBranch(cart,cart->root);
    return cart;
    __CLEANUP__;
    __END__;
}

CART_IMPL CxCARTNode* cxCopyCARTBranch(CxCART* new_cart,
				       CxCART* old_cart,
				       CxCARTNode* node)
{
    CxCARTNode* new_node = cxAllocNode(new_cart, node->id, node->depth, FALSE);
    int mem_ofs = (char*)new_node - (char*)node;

    CxCARTSplit* old_split = node->split;
    CxCARTSplit* new_split = new_node->split =
	(old_split ? (CxCARTSplit*)((char*)old_split + mem_ofs) : NULL);
    while (old_split)
    {
	old_split = old_split->next_competitor;
	new_split = new_split->next_competitor =
	    (old_split ? (CxCARTSplit*)((char*)old_split + mem_ofs) : NULL);
    }
    old_split = node->split;
    new_split = new_node->split;
    while (old_split)
    {
	old_split = old_split->next_surrogate;
	new_split = new_split->next_surrogate =
	    (old_split ? (CxCARTSplit*)((char*)old_split + mem_ofs) : NULL);
    }
    if (node->split)
    {
	CxCARTNode* left = new_node->child_left = cxCopyCARTBranch(new_cart, old_cart, node->child_left);
	CxCARTNode* right = new_node->child_right = cxCopyCARTBranch(new_cart, old_cart, node->child_right);
	left->parent = right->parent = new_node;
    }
    else
	new_node->child_left = new_node->child_right = NULL;
    icxAllocNodeShrunkIdx(new_cart, new_node);
    new_node->cur_cat_prop_pos = new_node->is_idx_expanded = 0;
    new_node->fallen_idx = new_node->shrunk_fallen_idx;
    return new_node;
}

CART_IMPL CxCART* cxCopyCART(CxCART* cart)
{
    CxCART* new_cart = (CxCART*)malloc( sizeof(CxCART) );
    memcpy(new_cart, cart, sizeof(CxCART));
    CxCARTTrainParams* params = cart->params;
    CxCARTSubj* subj = NULL;
    params->ref_count++;
    if (cart->root)
    {
	subj = cart->root->subj;
	if ( subj )
	    subj->ref_count++;
    }

    int priors_mult_size = 0;
    int cost_mult_size = 0;
    int num_features = cart->num_features;
    int num_resp = icxGetVarNumStates(cart->response_type);
    int num_resp_aligned = num_resp;
    int num_samples = cart->storage->num_samples;
    ALIGN_NUM_RESP(num_resp_aligned )
	BOOL classification = icxIsClassifierVarCategoric(cart->response_type);
    BOOL need_gini_mult = (cart->params->splitting_rule == CxCARTGiniCriterion);

    if (classification)
    {
	priors_mult_size = num_resp_aligned * sizeof(float) + CART_MEM_ALIGN;
	if (params->cost_mat)
	    cost_mult_size = (need_gini_mult + 1) * num_resp_aligned * num_resp_aligned * sizeof(float) ;
    }

    int size = sizeof( int ) * num_features * 4 +
	priors_mult_size + cost_mult_size;
    new_cart->features_corr = (int *)malloc(size);
    if (!cart->features_corr)
    {
	icxSetCARTError(cart , CART_MEMORY_ERROR ,
	    "Memory allocation error" , CART_ERROR_STATUS_FATAL);
	return NULL;
    }
    memcpy(new_cart->features_corr, cart->features_corr , size);
    new_cart->features_back_corr = new_cart->features_corr + num_features;
    new_cart->num_classes = new_cart->features_back_corr + num_features;
    new_cart->feature_type = new_cart->num_classes + num_features;
    if (classification)
    {
	new_cart->priors_mult = (float*) icxAlignUp( ((char*)(new_cart->feature_type + num_features)) , CART_MEM_ALIGN);
	if (params->cost_mat)
	{
	    new_cart->cost_error_mult = new_cart->priors_mult + num_resp_aligned ;
	    if (need_gini_mult)
		new_cart->cost_gini_mult = new_cart->cost_error_mult + num_resp_aligned * num_resp_aligned;
	}
    }
    //// Allocate new cart node storage and copy old nodes to it
    CxCARTStorage* new_storage = new_cart->storage = &new_cart->inner_storage;
    CxCARTNodeStorage& new_node_storage = new_storage->node_storage;
    CxCARTNodeStorage& old_node_storage = cart->storage->node_storage;
    new_node_storage.max_depth = new_node_storage.depth = -1; // No levels
    new_node_storage.num_blocks = 0;
    memset(new_node_storage.blocks, 0, MAX_LEVELS * sizeof(CxCARTLevel));
    if (!cxSetNodeStorageDepth(new_cart, cart->root->num_fallens, old_node_storage.depth))
	return NULL;
    memset( &new_storage->calc_storage , 0 , sizeof(CxCARTCalcStorage));

    CxCARTLevel* _old_level = old_node_storage.levels;
    CxCARTLevel* _new_level = new_node_storage.levels;

    int node_size = new_node_storage.node_size;
    for (int level = 0 ; level <= old_node_storage.depth ; level++, _old_level++, _new_level++)
    {
	int size_nodes = (1 << level) * node_size;
	memcpy(_new_level->buf_nodes , _old_level->buf_nodes, size_nodes);
	memcpy(_new_level->buf_shrunk_idx,
	    _old_level->buf_shrunk_idx, num_samples * sizeof(int) );
    }
    //// Copy nodes
    new_cart->root = cxCopyCARTBranch(new_cart, cart, cart->root);
    //// Copy pruning storae
    size = cart->pruning_storage.max_size * sizeof(CxPruningData);
    new_cart->pruning_storage.buf = (CxPruningData*)malloc(size);
    memcpy(new_cart->pruning_storage.buf, cart->pruning_storage.buf , size);
    new_cart->pruning_storage.pruned_nodes = NULL;
    //// Copy cross-validation storage
    int V = cart->cross_val_storage.V;
    int size_add = (sizeof(void*) + sizeof(int)) * V;
    CxPruningData** seq_ptrs = new_cart->cross_val_storage.seq_ptrs =
	(CxPruningData**)malloc(size_add);
    if (!seq_ptrs)
	return NULL;
    int* sizes = new_cart->cross_val_storage.sizes = (int*) ((char*)seq_ptrs + sizeof(void*) * V );
    memcpy(seq_ptrs , cart->cross_val_storage.seq_ptrs , size_add);
    size = cart->cross_val_storage.cur_size * sizeof(CxPruningData);
    CxPruningData* buf = new_cart->cross_val_storage.buf = (CxPruningData*)malloc(size);
    if (buf == NULL)
	return NULL;
    memcpy(buf, cart->cross_val_storage.buf, size);
    CxPruningData* data = buf;
    for (int i = 0 ; i < V ; i++)
    {
	seq_ptrs[i] = data;
	data += sizes[i];
    }
    return new_cart;
}

CART_IMPL void  cxCalcBranchErrorAndComplexity(CxCART *cart,
					       CxCARTNode *node,
					       double& branch_error,
					       int& terminal_nodes,
					       int prune_step)
{
    CV_FUNCNAME( "cxCalcBranchErrorAndComplexity" );
    __BEGIN__;

    if (!node)
	return;

#ifdef _DEBUG
    double error0 = cxCalcNodeError(cart,node);
#endif
    assert((error0 < EPS && node->error < EPS) || fabs(1.0 - error0/node->error) < 1e-3);

    assert( node->error >= -EPS);
    if (icxIsNodeSplit(node) && (!icxIsNodePruned(node , prune_step)))
    {
	CxCARTNode* right_node = node->child_right;
	CxCARTNode* left_node =  node->child_left;
	double err_right = 0.0;
	double err_left = 0.0;
	int terminal_right = 0;
	int terminal_left = 0;
	cxCalcBranchErrorAndComplexity(cart,
	    right_node,
									   err_right ,
									   terminal_right,
									   prune_step);
	cxCalcBranchErrorAndComplexity(cart,
	    left_node,
									   err_left ,
									   terminal_left,
									   prune_step);
	node->terminal_nodes = terminal_left + terminal_right;
	node->branch_error = err_left + err_right;
    }
    else
    {
	node->branch_error = node->error;
	node->terminal_nodes = 1;
    }
    branch_error = node->branch_error ;
    terminal_nodes = node->terminal_nodes;
    __CLEANUP__;
    __END__;
}

CART_IMPL double  cxCalcNodeError(CxCART *cart,
				  CxCARTNode *node)


{
    CV_FUNCNAME( "cxCalcNodeError" );
    __BEGIN__;

    //	BOOL floating = icxIsClassifierVar32f(cart->response_type);
    double num_total = cart->root->num_fallens;
    if (icxIsClassifierVarNumeric(cart->response_type)) // Numeric - regression
    {
	//// Calculate deviation
	CxCARTSubj* subj = node->subj;
	assert(subj && subj->is_valid );
	double deviation = 0.0;
	int* fallen_idx = node->fallen_idx;
	CxClassifierVar* responses = subj->response;
	assert(fallen_idx);
	for (int i = 0 ; i < node->num_fallens ; i++)
	{
	    int str_num = fallen_idx[i];
	    assert( fabs (subj->body[str_num].response.fl - subj->response[str_num].fl) < EPS);
	    double f = responses[str_num].fl - node->response.fl;
	    deviation += f*f;
	}
	return deviation / num_total;
    }
    else // Categoric
    {
	//// Misclassification rate
	int response = cvRound(node->response.fl);
	int num_resp = cart->num_response_classes;
	assert( response >= 0 && response < num_resp);
	int stat = node->fallen_stats[response];
	assert(stat >= 0 && stat <= node->num_fallens);
	float* cost_mult = cart->cost_error_mult;
	if (cart->params->priors)
	{
	    //// Priors , no cost matrix
	    assert(cart->priors_mult[response] >= -EPS);
	    return (node->sum_weighted_fallens - stat * cart->priors_mult[response]) / num_total;
	}
	else if (!cost_mult)
	{
	    //// Simplest case
	    return (node->num_fallens - stat) / num_total;
	}
	else
	{
	    int num_resp_aligned = num_resp;
	    ALIGN_NUM_RESP(num_resp_aligned)
		float* _cost_mult = cost_mult + num_resp_aligned * response;
	    assert (_cost_mult[response] == 0.0); // zeros on diagonal
	    float error = 0.0;
	    for (int  i = 0 ; i < num_resp; i++)
	    {
		error += _cost_mult[i] * node->fallen_stats[i];
	    }
	    return error / num_total;
	}
    }
    assert( node->error >= -EPS);
    __CLEANUP__;
    __END__;
}


CART_IMPL CxCARTNode* cxGetNextNode(CxCARTNode* node, int prune_step )
{
    CV_FUNCNAME( "cxGetNextNode" );
    __BEGIN__;

    assert (node);
    if ( icxIsNodeSplit(node) && (!icxIsNodePruned(node,prune_step)) )
	// Can go down ?
	return node->child_left;
    else
    {
	CxCARTNode* cur_node = node;
	CxCARTNode* parent_node = cur_node->parent;
	CxCARTNode* right_sibling_node = parent_node ? parent_node->child_right : NULL;
	while (parent_node)
	{
	    right_sibling_node = parent_node->child_right;
	    if (right_sibling_node != cur_node)
		return right_sibling_node;
	    cur_node = cur_node->parent;
	    parent_node = cur_node->parent;
	}
	return NULL;
    }
    __CLEANUP__;
    __END__;
}

CART_IMPL CxForestNode* cxGetNextForestNode(CxForestNode* node )
{
    CV_FUNCNAME( "cxGetNextNode" );
    __BEGIN__;

    assert (node);
    if ( node->split )
	// Can go down ?
	return node->child_left;
    else
    {
	CxForestNode* cur_node = node;
	CxForestNode* parent_node = cur_node->parent;
	CxForestNode* right_sibling_node = parent_node ? parent_node->child_right : NULL;
	while (parent_node)
	{
	    right_sibling_node = parent_node->child_right;
	    if (right_sibling_node != cur_node)
		return right_sibling_node;
	    cur_node = cur_node->parent;
	    parent_node = cur_node->parent;
	}
	return NULL;
    }
    __CLEANUP__;
    __END__;
}

CART_IMPL void icxInitNode(CxCARTNode *node)
{
    CV_FUNCNAME( "icxInitNode" );
    __BEGIN__;
    memset(node,0,sizeof(CxCARTNode));
    __CLEANUP__;
    __END__;
}

CART_IMPL float cxFindWeakestLinks(CxCART *cart,
				   CxCARTNode** weakest_links_list,
				   int& num_nodes,
				   int prune_step)
{
    CV_FUNCNAME( "cxFindWeakestLinks" );
    __BEGIN__;

    assert(weakest_links_list);
    CxCARTNode* node = cart->root;
    double g_min = FLT_MAX;
    CxCARTNode* node_g_min = NULL;
    //////// First pass : finding minimum g-value /////////////
    while (node)
    {
	if (node->terminal_nodes > 1)
	{
	    assert (!icxIsNodePruned(node,prune_step));
	    double g = (node->error - node->branch_error)/(node->terminal_nodes - 1);
	    if (g < g_min)
	    {
		node_g_min = node;
		g_min = g;
	    }
	}
	node = cxGetNextNode(node, prune_step);
    }
    if (!node_g_min)
	return -1.0;
    //////// Second pass : filling weakest links list ////////////
    node = cart->root;
    float eps_weight = node->parent ? cart->root->split->weight * EPS : EPS;
    num_nodes = 0;
    while (node)
    {
	if (node->terminal_nodes > 1)
	{
	    assert (!icxIsNodePruned(node,prune_step));
	    double g = (node->error - node->branch_error)/(node->terminal_nodes - 1);
	    if ( fabs(g - g_min) < eps_weight)
	    {
		//CX_CART_TRACE(tdev << cxCalcNodeNum(node) << ' ');
		node->pruning_step = prune_step;
		weakest_links_list[num_nodes++] = node;
	    }
	}
	node = cxGetNextNode(node, prune_step);
    }

    //	CX_CART_TRACE(tdev << nl << "Found " << weakest_links_list->total << " weakest links"  );
    assert(num_nodes > 0);
    return (float)g_min;
    __CLEANUP__;
    __END__;
}

CART_IMPL int cxPrePruneTree(CxCART *cart,
			     CxCARTNode** nodes_seq)
{
    CV_FUNCNAME( "cxPrePruneTree" );
    __BEGIN__;

    assert (nodes_seq);
    CxCARTNode* node = cart->root;
    //	float eps_weight = cart->root->split ? cart->root->split->weight * EPS  : EPS;
    int num_pruned = 0;
    while (node)
    {
	node->pruning_step = 0;
	if (node->terminal_nodes > 1)
	{
	    if (fabs (node->error - node->branch_error) < EPS)
	    {
		nodes_seq[num_pruned++] = node;
		node->pruning_step = 1;
		//	CX_CART_TRACE(tdev << "Pre-pruned node " << cxCalcNodeNum(node) << ' ');
	    }
	}
	node = cxGetNextNode(node, 1);
    }
    //	CX_CART_TRACE(tdev << nl << "Prepruned " << nodes_seq->total << " nodes" << nl) ;
    if ( num_pruned > 0)
	cxPruneNodes(nodes_seq , num_pruned, 1);
    return num_pruned;
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxPruneNodes(CxCARTNode** nodes_seq,
			    int num_nodes ,
			    int prune_step)
{
    CV_FUNCNAME( "cxPruneNodes" );
    __BEGIN__;

    assert(nodes_seq);
    CxCARTNode* node;
    for (int i = 0 ; i < num_nodes ; i++)
    {
	node = nodes_seq[i];
	node->pruning_step = prune_step;
	int terminal_decrease = node->terminal_nodes - 1;
	double error_increase = node->error - node->branch_error;
	assert ( (terminal_decrease > 0) && (error_increase >= -EPS));
	CxCARTNode* node_cur = node;
	while (node_cur)
	{
	    node_cur->terminal_nodes -= terminal_decrease;
	    node_cur->branch_error += error_increase;
	    node_cur = node_cur->parent;
	}
    }
    //	assert( nodes_seq->total == 0);
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxRecalcSampleErrors(CxPruningData* data)
{
    CV_FUNCNAME( "cxRecalcSampleErrors" );
    __BEGIN__;
    CxCARTNode** nodes_seq = data->nodes_seq;
    assert(nodes_seq);
    int size = data->num_nodes;
    for (int i = 0 ; i < size ; i++)
    {
	CxCARTNode* node = nodes_seq[i];
	double error_increase = node->node_sample_error - node->branch_sample_error;
	double error_dev_increase = node->node_sample_error_dev - node->branch_sample_error_dev;
	//		assert ( (error_increase > - EPS) && (error_dev_increase > -EPS));
	CxCARTNode* node_cur = node;
	while (node_cur)
	{
	    node_cur->branch_sample_error += error_increase;
	    node_cur->branch_sample_error_dev += error_dev_increase;
	    node_cur = node_cur->parent;
	}
    }
    //	assert( nodes_seq->total == 0);
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxPruneCART(CxCART *cart)
{
    CV_FUNCNAME( "cxPruneCART" );
    __BEGIN__;

    double branch_error = 0.0;
    int terminal_nodes = 0;

#ifdef _DEBUG
    assert(cxCreatePruningStorage(cart));
#else
    cxCreatePruningStorage(cart);
#endif
    cxCalcBranchErrorAndComplexity(cart,cart->root, branch_error, terminal_nodes);

    CxCARTNode** nodes_seq = cart->pruning_storage.pruned_nodes;
    CxCARTNode** _nodes_seq = nodes_seq;
    assert(nodes_seq);
    int num_pruned = cxPrePruneTree(cart, nodes_seq);
    int prune_step = 1;
    CxCARTPruningStorage& pruning_storage = cart->pruning_storage;

    CxPruningData* data_seq = pruning_storage.buf;
    CxPruningData* data = data_seq ;
    icxInitPruneData(data , prune_step , 0.0,
	cart->root->branch_error ,
	cart->root->terminal_nodes,
	num_pruned , _nodes_seq);
    _nodes_seq += num_pruned;
    data++;
    pruning_storage.size++;
    assert(pruning_storage.size <= pruning_storage.max_size);

    BOOL proceed = TRUE;
    while (proceed)
    {
	//// Get weakest links ///////
	num_pruned = 0;
	float alpha = cxFindWeakestLinks(cart, _nodes_seq, num_pruned, prune_step);
	proceed = (num_pruned > 0);
	//// Prune subtrees corresponding to weakest links ///////
	if (proceed)
	{
	    prune_step ++;
	    cxPruneNodes(_nodes_seq , num_pruned , prune_step);
	    icxInitPruneData(data, prune_step , alpha,
		cart->root->branch_error ,
		cart->root->terminal_nodes ,
		num_pruned , _nodes_seq);
	    assert(num_pruned > 0);
	    _nodes_seq += num_pruned;
	    data++;
	    pruning_storage.size++;
	    assert(pruning_storage.size <= pruning_storage.max_size);
	}
    }
    cart->progress_info.progress_data_size = pruning_storage.size ;
    __CLEANUP__;
    __END__;
}

CART_IMPL CxClassifierVar cxCARTEvalFeatures( CxCART* cart,
					     void* features,
					     int pruning_step,
					     int features_step ,
					     char* missed_mask ,
					     int missed_mask_step  )
{
    CV_FUNCNAME( "cxCARTEvalFeatures" );
    __BEGIN__;
    CxCARTNode* node = cart->root;
    assert(node);
    while (node && icxIsNodeSplit(node) && (!icxIsNodePruned(node , pruning_step)))
    {
	node = cxIsStringSplitLeft(cart, node ,
	    features , features_step ,
	    missed_mask , missed_mask_step) ? node->child_left : node->child_right;
    }
    return node->response;
    __CLEANUP__;
    __END__;
}

CART_IMPL void  cxCalcSampleNodeErrorAndDeviation(CxCART *cart,
						  CxClassifierSample *sample)
{
    CV_FUNCNAME( "cxCalcSampleNodeErrorAndDeviation" );
    __BEGIN__;
    if (!cart || !cart->root)
	return;
    int i,j;
    int sample_size = 0;
    int type = cart->response_type;
    BOOL floating = icxIsClassifierVar32f(type);

    ///////////// Calculating sample size //////////////////
    for (i = 0; i < sample->num_chunks ; i++)
	sample_size += sample->chunk[i]->num_indices_of_interest;
    if (sample_size < 2 )
    {
	icxSetCARTError(cart, CART_CALC_ERROR ,
	    "Sample contains too small number of cases!",
	    CART_ERROR_STATUS_NONFATAL);
	return;
    }
    BOOL regression = icxIsClassifierVarNumeric(type);
    //////////////// Clearing errors //////////////////
    CxCARTNode* node = cart->root;
    while (node)
    {
	node->branch_sample_error = 0.0;
	node->branch_sample_error_dev = 0.0;
	node->node_sample_error = 0.0;
	node->node_sample_error_dev = 0.0;
	node = cxGetNextNode(node);
    };

    if (!regression)
    {
	int num_resp = cart->num_response_classes;
	float* priors = cart->params->priors;
	int* class_proportion = NULL;
	float* class_mult = (float*)calloc(num_resp, sizeof(float));
	float* cost_mat = cart->params->cost_mat;
	int num_resp_aligned = num_resp;
	ALIGN_NUM_RESP(num_resp_aligned)

	    ///////////// Calculating class proportions if priors exist //////////////
	    if (priors)
	    {
		class_proportion = (int*)calloc(num_resp, sizeof(int));
		for (i = 0; i < sample->num_chunks ; i++)
		{
		    CxClassifierSampleChunk* chunk = sample->chunk[i];
		    for (j = 0 ; j < chunk->num_indices_of_interest ; j++)
		    {
			int string = chunk->indices_of_interest ? chunk->indices_of_interest[j] : j;
			CxClassifierVar var = icxGetChunkResponse(chunk,string);
			int response = icxVarToInt(var , floating);
			assert( response >= 0 && response < num_resp);
			class_proportion[response] ++;
		    }
		}
		for (int r = 0 ; r < num_resp ; r++)
		    class_mult[r] = (priors[r] * sample_size) / class_proportion[r];
	    }
	    else
	    {
		for (int r = 0 ; r < num_resp ; r++)
		    class_mult[r] = 1.0;
	    }
	    //////////////// Throwing all strings into the tree and calculating node errors ////////////
	    for (i = 0; i < sample->num_chunks ; i++)
	    {
		CxClassifierSampleChunk* chunk = sample->chunk[i];
		int feature_step0 = chunk->features_step[0];
		int missed_step0 = chunk->missed_step[0];
		int feature_step1 = chunk->features_step[1];
		int missed_step1 = chunk->missed_step[1];

		for (j = 0 ; j < chunk->num_indices_of_interest ; j++)
		{
		    int string = chunk->indices_of_interest ? chunk->indices_of_interest[j] : j;
		    void* features = ((char*)chunk->features_data) + string * feature_step0;
		    char* missed_mask = chunk->missed_data ? chunk->missed_data + string * missed_step0 : NULL;

		    node = cart->root;
		    while (node)
		    {
			CxClassifierVar var = icxGetChunkResponse(chunk,string);
			int resp_pred = icxVarToInt(node->response , floating );
			int response = icxVarToInt(var, floating );
			if ( resp_pred != response)
			{
			    if (!cost_mat)
				node->node_sample_error += class_mult[response];
			    else
				node->node_sample_error += class_mult[response] *
				cost_mat[ resp_pred * num_resp_aligned];
			}
			//priors ? (priors[resp] * sample_size) / class_proportion[resp] : 1.0;

			node = icxIsNodeSplit(node) ? cxIsStringSplitLeft(cart, node ,
			    features , feature_step1,
			    missed_mask , missed_step1) ?
			    node->child_left : node->child_right : NULL;
		    }
		}
	    }
	    if (class_proportion)
		free(class_proportion);
	    free(class_mult);
    }
    else
    {
	//////////////// Throwing all strings into the tree and calculating node errors ////////////
	for (i = 0; i < sample->num_chunks ; i++)
	{
	    CxClassifierSampleChunk* chunk = sample->chunk[i];
	    int feature_step0 = chunk->features_step[0];
	    int missed_step0 = chunk->missed_step[0];
	    int feature_step1 = chunk->features_step[1];
	    int missed_step1 = chunk->missed_step[1];

	    for (j = 0 ; j < chunk->num_indices_of_interest ; j++)
	    {
		int string = chunk->indices_of_interest ? chunk->indices_of_interest[j] : j;
		void* features = ((char*)chunk->features_data) + string * feature_step0;
		char* missed_mask = chunk->missed_data ? chunk->missed_data + string * missed_step0: NULL;

		node = cart->root;
		while (node)
		{
		    CxClassifierVar var = icxGetChunkResponse(chunk,string);
		    float resp_pred = icxVarToFloat(node->response , floating );
		    float resp_true = icxVarToFloat(var, floating );
		    double diff = resp_true - resp_pred;

		    double sq = diff * diff;
		    node->node_sample_error += sq;
		    node->node_sample_error_dev += sq * sq;

		    node = icxIsNodeSplit(node) ? cxIsStringSplitLeft(cart, node ,
			features ,feature_step1,
			missed_mask , missed_step1) ?
			node->child_left : node->child_right : NULL;
		}
	    }
	}
    }

    __CLEANUP__;
    __END__;

}

CART_IMPL void  cxCalcSampleBranchErrorAndDeviation(CxCART *cart,
						    CxCARTNode* node)
{
    if (icxIsNodeSplit(node))
    {
	cxCalcSampleBranchErrorAndDeviation(cart, node->child_left );
	cxCalcSampleBranchErrorAndDeviation(cart, node->child_right );
	node->branch_sample_error     = node->child_left->branch_sample_error + node->child_right->branch_sample_error;
	node->branch_sample_error_dev = node->child_left->branch_sample_error_dev + node->child_right->branch_sample_error_dev;
    }
    else
    {
	node->branch_sample_error = node->node_sample_error;
	node->branch_sample_error_dev = node->node_sample_error_dev;
    }
}

CART_IMPL double cxCalcCARTSampleErrorAndPrediction( CxCART* cart,
						    CxClassifierSample* sample,
						    int pruning_step,
						    float* prediction,
						    double* err_deviation)
{
    CV_FUNCNAME( "cxCalcSampleErrorAndPrediction" );
    __BEGIN__;
    int i,j;
    int sample_size = 0;
    int type = cart->response_type;
    double sum_sqsq = 0.0;
    BOOL floating = icxIsClassifierVar32f(type);

    for (i = 0; i < sample->num_chunks ; i++)
	sample_size += sample->chunk[i]->num_indices_of_interest;
    if (sample_size < 2)
    {
	icxSetCARTError(cart, CART_CALC_ERROR,
	    "Sample contains too small number of cases!",
	    CART_ERROR_STATUS_NONFATAL);
	return 0.0;
    }
    float* _pred = prediction;
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    double error = 0.0;//priors && (!regression) ? sample_size : 0.0;
    int* class_proportion = NULL;
    if (!regression)
    {
	int num_resp = cart->num_response_classes;
	int num_resp_aligned = num_resp;
	ALIGN_NUM_RESP(num_resp_aligned)

	    float* priors = cart->params->priors;
	float* class_mult = (float*)calloc(num_resp, sizeof(float));

	if (priors)
	{
	    /// Calculate class proportions in the sample ///////
	    class_proportion = (int*)calloc(cart->num_response_classes , sizeof(int));
	    for (i = 0; i < sample->num_chunks ; i++)
	    {
		CxClassifierSampleChunk* chunk = sample->chunk[i];
		for (j = 0 ; j < chunk->num_indices_of_interest ; j++)
		{
		    int string = chunk->indices_of_interest ? chunk->indices_of_interest[j] : j;
		    CxClassifierVar var = icxGetChunkResponse(chunk,string);
		    int resp_true = icxVarToInt(var , floating);
		    assert( resp_true >= 0 && resp_true < cart->num_response_classes);
		    class_proportion[resp_true] ++;
		}
	    }
	    for (int r = 0 ; r < num_resp ; r++)
		class_mult[r] = (priors[r] * sample_size) / class_proportion[r];
	}
	else
	{
	    for (int r = 0 ; r < num_resp ; r++)
		class_mult[r] = 1.0;
	}
	float* cost_mat = cart->params->cost_mat;
	/* Calculate sample error , evaluating each sample and comparing
	predicted response with real one */
	for (i = 0; i < sample->num_chunks ; i++)
	{
	    CxClassifierSampleChunk* chunk = sample->chunk[i];
	    for (j = 0 ; j < chunk->num_indices_of_interest ; j++)
	    {
		int string = chunk->indices_of_interest ? chunk->indices_of_interest[j] : j;
		void* features = ((char*)chunk->features_data) + string * chunk->features_step[0];
		char* missed_mask = chunk->missed_data ? chunk->missed_data + string * chunk->missed_step[0]: NULL;
		CxClassifierVar var = cxCARTEvalFeatures(cart,features ,
		    pruning_step ,
		    chunk->features_step[1],
		    missed_mask,
		    chunk->missed_step[1]);

		int resp_pred = icxVarToInt(var , floating);
		var = icxGetChunkResponse(chunk,string);
		int resp_true = icxVarToInt(var , floating);
		if (prediction)
		    *(_pred++) = (float)resp_pred;
		if ( resp_true != resp_pred)
		{
		    if (!cost_mat)
			error += class_mult[resp_pred];
		    else
			error += class_mult[resp_pred] * cost_mat[ resp_true * num_resp_aligned];
		}
	    }
	}
	double err = error / sample_size;
	if (err_deviation)
	    *err_deviation = sqrt( err * (1.0 - err) / sample_size);

	if (class_proportion)
	    free(class_proportion);
	free(class_mult);
	return err;
    }
    else
    {
    /* Calculate sample error , evaluating each sample and comparing
	predicted response with real one */
	for (i = 0; i < sample->num_chunks ; i++)
	{
	    CxClassifierSampleChunk* chunk = sample->chunk[i];
	    for (j = 0 ; j < chunk->num_indices_of_interest ; j++)
	    {
		int string = chunk->indices_of_interest ? chunk->indices_of_interest[j] : j;
		void* features = ((char*)chunk->features_data) + string * chunk->features_step[0];
		char* missed_mask = chunk->missed_data ? chunk->missed_data + string * chunk->missed_step[0]: NULL;
		CxClassifierVar var = cxCARTEvalFeatures(cart,features ,
		    pruning_step ,
		    chunk->features_step[1],
		    missed_mask,
		    chunk->missed_step[1]);
		float resp_pred = icxVarToFloat(var , floating);
		if (prediction)
		    *(_pred++) = (float)resp_pred;
		var = icxGetChunkResponse(chunk,string);
		float resp_true = icxVarToFloat(var , floating);
		float diff = resp_true - resp_pred;

		double sq = diff * diff;
		error += sq;
		sum_sqsq += sq * sq;
	    }
	}
	double err = error / sample_size;
	if (err_deviation)
	    *err_deviation = sqrt ( (sum_sqsq/sample_size - err * err ) / sample_size) ;
	return err ;
    }
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxCalcCARTTestErrorsAndPredictions( CxCART* cart,
						  CxClassifierSample* sample,
						  float* prediction,
						  BOOL calc_err)
{
    int steps = icxGetProgressCount((CxClassifier*)cart);
    int size = icxGetNumSamplesOfInterest(sample);
#ifdef _INC_WINDOWS
    assert(!IsBadReadPtr(prediction, steps * size * sizeof(float)) &&
	!IsBadWritePtr(prediction, steps * size * sizeof(float)) );
#endif
    float* _pred = prediction;
    CxPruningData* _data = icxGetPruningSeq(cart);
    if (calc_err)
    {
	for (int i = 0 ; i < steps ; _pred += size, i++, _data++)
	{
	    _data->test_sample_error =
		cxCalcCARTSampleErrorAndPrediction(cart, sample, i, _pred,
		&(_data->error_dev));
	}
    }
    else
    {
	for (int i = 0 ; i < steps ; _pred += size, i++, _data++)
	{
	    cxCalcCARTSampleErrorAndPrediction(cart, sample, i, _pred);
	}
    }
}

CART_IMPL void cxCalcCARTTestErrors( CxCART *cart , CxClassifierSample* sample)
{
    CV_FUNCNAME( "cxCalcCARTTestErrors" );
    __BEGIN__;
    int size = icxGetPruningDataCount(cart);
    if (size == 0)
	return;

    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    int num_indices_of_interest = sample->chunk[0]->num_indices_of_interest;

    cxCalcSampleNodeErrorAndDeviation(cart,sample);
    if (cart->error.code > 0)
	return;
    cxCalcSampleBranchErrorAndDeviation(cart , cart->root );
#ifdef _DEBUG
    //	ofstream file("errors.log", ios::app);
#endif
    for (int i = 0 ; i < size; i++)
    {
	CxPruningData* data = icxGetPruningData(cart , i);
	cxRecalcSampleErrors(data);
	double error = cart->root->branch_sample_error / num_indices_of_interest;
	double error_dev = cart->root->branch_sample_error_dev / num_indices_of_interest;
	double test_sample_error_dev = regression ? sqrt ( (error_dev - error * error ) / num_indices_of_interest) :
	sqrt ( error * (1.0 - error) / num_indices_of_interest);
	data->test_sample_error = error ;
	data->error_dev = test_sample_error_dev;

#ifdef _DEBUG
	//		double test_sample_error1 = cxCalcSampleError(cart,sample, data->step , &data->error_dev);
	//		assert(fabs(data->test_sample_error - test_sample_error1) < EPS) ;
	//		file << test_sample_error1 << ' ' << data->test_sample_error << ' '
	//			 << (float)data->error_dev << ' ' << test_sample_error_dev << '\n';
#endif

    }
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxCalcCrossValidationErrors( CxCART *cart , CxClassifierSample* sample , int V)
{
    CV_FUNCNAME( "cxCalcCrossValidationErrors" );
    __BEGIN__;
    int i , j;

    if (cart->error.code > 0)
	return;
    int sample_size = 0;
    for (i = 0; i < sample->num_chunks ; i++)
	sample_size += sample->chunk[i]->num_indices_of_interest;

    BOOL alloc = FALSE;
    int* buf = NULL;
    if (sample_size < 2)
	return;
    if ( sample_size < V )
	V = sample_size;

    CxPruningData* data = NULL;
    CxPruningData* data_next = NULL;
    CxCARTCrossValStorage& crval_storage = cart->cross_val_storage;
    BOOL need_recalc = (V != crval_storage.V);

    //	cxCalcCARTTestErrors(cart , sample);
    icxFinalizePruningStorage( cart->pruning_storage );

    if (need_recalc)
    {
	int*& indices_of_interest = sample->chunk[0]->indices_of_interest;
	BOOL error_occured = FALSE;
	buf = (int*) malloc( sizeof(int) * sample_size);
	//// Copy current indices of interest to buf
	if (indices_of_interest)
	    memcpy ( buf , indices_of_interest , sizeof(int) * sample_size);
	else
	{
	    alloc = TRUE;
	    indices_of_interest = (int*)malloc(sample_size * sizeof (int));
	    for (i = 0 ; i < sample_size ; i++)
	    {
		buf[i] = i;
		indices_of_interest[i] = i;
	    }
	}
	//// Calculate test errors for original tree

	cxCreateCrossValStorage(cart , V);

	//////////////  Building auxiliary trees , calculating errors on them   //////////
	for (i = 0 ; (i < V) && (!error_occured) ; i++)
	{
	    int num = 0;
	    //// Select subsample for building tree
	    for (num = 0, j = 0 ; j < sample_size ; j++)
	    {
		if ((j % V) != i)
		    indices_of_interest[num++] = buf[j];
	    }
	    sample->chunk[0]->num_indices_of_interest = num;
	    CxCART* aux_cart = cxBuildCART( sample, cart->params , cart->extra_info);
#ifdef CART_WITH_GUI
	    if (g_hwndMain)
	    {
		sprintf(strMsg , "Built cross-validation tree %d", i+1);
		::SendMessage(g_hwndMain , WM_DONE_STRING , 0 , (long)strMsg);
	    }
#endif
	    if (aux_cart->error.code == 0)
	    {
		cxPruneCART(aux_cart);
		//// Select subsample for validating tree
		for (num = 0, j = i ; j < sample_size ; j += V)
		    indices_of_interest[num++] = buf[j];
		sample->chunk[0]->num_indices_of_interest = num;

		cxCalcCARTTestErrors(aux_cart , sample);

		icxAddToCrossValStorage(cart , crval_storage , aux_cart->pruning_storage.buf ,
		    aux_cart->pruning_storage.size );

	    }
	    else
	    {
		cart->error.code = aux_cart->error.code;
		strcpy(cart->error.description , aux_cart->error.description);
		cart->error.status = aux_cart->error.status;
		error_occured = TRUE;
	    }
	    aux_cart->params = NULL;
	    cxReleaseCART(&aux_cart);
	}

	//////////////// Restoring indices of interest ////////////////////
	if (alloc)
	{
	    free ( indices_of_interest );
	    indices_of_interest = NULL;
	    sample->chunk[0]->num_indices_of_interest = 0;
	}
	else
	{
	    memcpy ( indices_of_interest ,buf ,sizeof(int) * sample_size);
	    sample->chunk[0]->num_indices_of_interest = sample_size;
	}

	free( buf );
	if (error_occured)
	    return;
	icxFinalizeCrossValStorage(crval_storage);
    }
    /////////// Calculating cross - validation errors for main tree //////////////
    //	ofstream file("crval.log");
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    int size = icxGetPruningDataCount(cart);
    CxPruningData* pruning_seq = icxGetPruningSeq(cart);
    assert(crval_storage.num_pruning_seqs == V);
    for (j = 0 ; j < size ; j++)
    {
	data = pruning_seq + j;
	data_next = (j == size - 1) ? NULL : pruning_seq + (j + 1);
	float alpha = (float)(data_next ? sqrt (data->alpha * data_next->alpha) : data->alpha);
	//		file << "Step " << j << " :  Alpha = " << alpha << '\n';

	double err_sum = 0.0;
	double var_sum = 0.0;
	for (i = 0 ; i < V ; i++)
	{
	    CxPruningData* cur_pruning_seq = crval_storage.seq_ptrs[i];
	    CxPruningData* data_cur =
		icxFindPruningStep (cur_pruning_seq, crval_storage.sizes[i] , alpha);
	    //			file << data_cur->step << " : " << data_cur->test_sample_error << ' ';
	    if (data_cur)
	    {
		err_sum += data_cur->test_sample_error ;
		if (regression)
		    var_sum += data_cur->error_dev * data_cur->error_dev ;
	    }
	}
	//		fputc('\n', file);
	double error = data->test_sample_error = err_sum / V;
	if (regression)
	{
	    data->error_dev = ((var_sum <= 0.0) ? 0.0 : (sqrt(var_sum) / V));
	}
	else
	{
	    data->error_dev = sqrt ( error * (1.0 - error) / sample_size);
	}
    }

    __CLEANUP__;
    __END__;
}

CART_IMPL CxPruningData* cxFindOptimalData(CxCARTPruningStorage& pruning_storage,
					   BOOL use_1SE_rule)
{
    CV_FUNCNAME( "cxFindBestSubtree" );
    __BEGIN__;
    int size = pruning_storage.size;
    if (size == 0)
	return NULL;
    CxPruningData* pruning_seq = pruning_storage.buf;
    CxPruningData* data_best = pruning_seq;
    CxPruningData* data = pruning_seq;

    int i;
    for (i = 0 ; i < size ; i++ , data++)
    {
	if (data->test_sample_error < data_best->test_sample_error)
	    data_best = data;
    }
    if (!use_1SE_rule)
	return data_best;
    double var = data_best->error_dev ;
    data--;
    for (i = size - 1 ; i >= 0 ; i--, data--)
    {
	if (data->test_sample_error <= data_best->test_sample_error + var)
	{
	    //	CX_CART_TRACE(tdev << "optimal data : " << data->step << nl);
	    return data;
	}
    }
    return data_best;
    __CLEANUP__;
    __END__;
}

CART_IMPL CxPruningData* cxFindBestSubtree( CxCART *cart ,
					   CxClassifierSample* sample,
					   BOOL use_1SE_rule )
{

    CV_FUNCNAME( "cxFindBestSubtree" );
    __BEGIN__;
    cxCalcCARTTestErrors(cart , sample);
    icxFinalizePruningStorage( cart->pruning_storage );
    return (cart->error.code == 0) ? cxFindOptimalData ( cart->pruning_storage , use_1SE_rule) : NULL;
    __CLEANUP__;
    __END__;
}

CART_IMPL CxPruningData* cxFindBestSubtreeCrossValidation( CxCART *cart ,
							  CxClassifierSample* sample ,
							  int V,
							  BOOL use_1SE_rule)
{

    CV_FUNCNAME( "cxFindBestSubtreeCrossValidation" );
    __BEGIN__;

    cxCalcCrossValidationErrors(cart , sample , V);
    return (cart->error.code == 0) ? cxFindOptimalData (cart->pruning_storage , use_1SE_rule ) : NULL;
    __CLEANUP__;
    __END__;
}

CART_IMPL void icxInitPruneData(CxPruningData* data,
				int step,
				float alpha,
				double reestimation_error,
				int terminal_nodes,
				int num_pruned_nodes ,
				CxCARTNode** pruned_seq)
{
    CV_FUNCNAME( "icxInitPruneData" );
    __BEGIN__;
    data->step = step;
    data->alpha = alpha;
    data->reestimation_error = reestimation_error;
    data->test_sample_error = 0.0;
    data->terminal_nodes = terminal_nodes;
    data->error_dev = 0.0;
    data->num_nodes = num_pruned_nodes;
    data->nodes_seq = pruned_seq;
    __CLEANUP__;
    __END__;
}


CART_IMPL float cxCalcImpurityChangeAny(CxRootedCARTBase* cart,
					CxForestNode* node,
					CxCARTSplit* split)
{
    CV_FUNCNAME( "cxCalcImpurityChangeCategoric" );
    __BEGIN__;
    assert(cart && node && node->subj);
    if (!node->subj)
	return 0.0;
    int i = 0;
    int feature = split->feature_idx;
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP(num_resp_aligned)

	BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
#ifdef _DEBUG
    int eff_feature = cart->features_corr[feature];
#endif
    int num_fallens = node->num_fallens ;
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );
    assert((long)cart->params->splitting_rule <= CxCARTEntropyCriterion);
    int num_left = 0;
    int num_total = 0;
    int* idx = node->fallen_idx;
    double impurity = 0.0;
    double impurity_right = 0.0;
    double impurity_left = 0.0;
    BOOL floating = icxIsClassifierVar32f(cart->response_type);

    CxCARTSubj* subj = node->subj;
    CxCARTAtom* body = subj->body;
    int shift = feature * subj->features_step;
    int shift_missed = feature * subj->missed_mask_step;

    if (!regression)
    {
	int size =  sizeof(int) * num_resp_aligned;
	int* stats = (int*)icxAlignedCalloc(3 * num_resp_aligned , sizeof(int));
	memset(stats , 0, 3 * size);
	for (i = 0 ; i < num_fallens ; i++)
	{
	    CxCARTAtom* atom = body + idx[i];
	    char* missed_mask = atom->missed_mask;
	    if (!missed_mask || (!missed_mask[shift_missed]))
	    {
		CxClassifierVar var = atom->response;
		int response = icxVarToInt(var , floating);
		var = *(CxClassifierVar *)((char*)atom->features + shift);
		int left = icxIsVarSplitLeft((CxCARTBase*)cart, split , var);
		stats[num_resp_aligned * (1-left) + response]++;
		num_left += left;
		num_total ++;
		stats[2 * num_resp_aligned + response] ++;
	    }
	}
	if ((num_left == 0) || (num_left == num_total))
	{
	    icxFreeAligned(stats);
	    return 0.0;
	}
	if (cart->params->splitting_rule == CxCARTGiniCriterion)
	{
	    float* cost_gini_mult = cart->cost_gini_mult;
	    if (!cost_gini_mult)
	    {
		impurity_left = icxCalcGiniIndexPriors(stats , num_resp, cart->priors_mult);
		impurity_right = icxCalcGiniIndexPriors(stats + num_resp_aligned , num_resp, cart->priors_mult);
		impurity = icxCalcGiniIndexPriors(stats + 2 * num_resp_aligned, num_resp, cart->priors_mult);
	    }
	    else
	    {
		impurity_left = icxCalcGiniIndexWithCosts(stats , num_resp_aligned,
		    cost_gini_mult , cart->priors_mult);
		impurity_right = icxCalcGiniIndexWithCosts(stats +
		    num_resp_aligned , num_resp_aligned,
		    cost_gini_mult , cart->priors_mult);
		impurity = icxCalcGiniIndexWithCosts(stats +
		    2 * num_resp_aligned, num_resp_aligned,
		    cost_gini_mult , cart->priors_mult);
	    }
	}
	else
	{
	    impurity_left = icxCalcEntropyPriors(stats , num_resp, cart->priors_mult);
	    impurity_right = icxCalcEntropyPriors(stats + num_resp_aligned , num_resp, cart->priors_mult);
	    impurity = icxCalcEntropyPriors(stats + 2 * num_resp_aligned, num_resp, cart->priors_mult);
	}
	//	assert ( (1.0 - (impurity_left + impurity_right)/impurity) > -EPS);
	icxFreeAligned(stats);
    }
    else //if regression
    {
	double total = cart->froot->num_fallens;
	double sum[2] = {0.0 , 0.0};
	double sum_sq[2] = {0.0 , 0.0};
	for (i = 0 ; i < num_fallens ; i++)
	{
	    CxCARTAtom* atom = body + idx[i];
	    char* missed_mask = atom->missed_mask;
	    if (!missed_mask || (!missed_mask[shift_missed]))
	    {
		CxClassifierVar var = atom->response;
		float f = icxVarToFloat(var , floating);
		var = *(CxClassifierVar *)((char*)atom->features + shift);
		int side = 1 - icxIsVarSplitLeft((CxCARTBase*)cart, split , var);
		num_left += (1 - side);
		sum[side] += f;
		sum_sq[side] += f*f ;
		num_total++;
	    }
	}
	if ((num_left == 0) || (num_left == num_total))
	    return 0.0;
	impurity_left = (sum_sq[0] - sum[0] * sum[0] / num_left) / total;
	impurity_right = (sum_sq[1] - sum[1] * sum[1] / (num_total - num_left)) / total;
	impurity = (sum_sq[0] + sum_sq[1] - (sum[0] + sum[1])*(sum[0] + sum[1])/num_total )/total;
	//	assert ( (1.0 - (impurity_left + impurity_right)/impurity) > -EPS);

    }
    //	assert ( (1.0 - (impurity_left + impurity_right)/impurity) > -EPS);
    return (float)(impurity - impurity_left - impurity_right);

    __CLEANUP__;
    __END__;
}

//////// Fast float comparison : -1 : f1 < f2 ; 0 - f1 = f2 ; 1 - f1 > f2 /////////
static inline int icxCmpFl(float f1 , float f2)
{
    return (f1 < f2) ? -1 :
    (f1 > f2) ? 1 : 0;
    //	return (!((*(int*)&f1) ^ (*(int*)&f2))-1) & icxSign(f1-f2);
}

//////// Int comparison : -1 : i1 < i2 ; 0 - i1 = i2 ; 1 - i1 > i2 /////////
static inline int icxCmpInt(int i1 , int i2)
{
    return (i1 < i2) ? -1 :
    (i1 > i2) ? 1 : 0;
}

//////// Compares two ClassifierVars ///////////////////////////////////
static inline int icxCompareVars( CxClassifierVar var1, CxClassifierVar var2, int type)
{
    assert( icxIsClassifierVarNumeric (type));
#if FLOAT_ONLY
    UNREFERENCED_PARAMETER(type);
    return icxCmpFl(var1.fl , var2.fl);
#else
    if (icxIsClassifierVar32f(type))
	return icxCmpFl(var1.fl , var2.fl);
    else
	return icxCmpInt(var1.i , var2.i);
#endif
}

CART_IMPL int cxFindNumericSplitProportion(CxCART *cart,
					   CxCARTNode *node,
					   CxCARTSplit* split,
					   int* left_nonmissing)
{
    CV_FUNCNAME( "cxFindNumericSplitProportion" );
    __BEGIN__;
    assert(node->is_idx_expanded);
    int feature = split->feature_idx;
    int type = cart->feature_type[feature];
    assert(icxIsClassifierVarNumeric(type) );
    int eff_feature = cart->features_corr[feature];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );

    int num_not_missed = node->num_fallens - node->num_missed[eff_feature];
    int left = 0;
    int right = num_not_missed - 1 ;
    int* idx = node->fallen_idx + node->num_fallens * eff_feature;
    CxCARTSubj* subj = node->subj;
    int retval = 0;

    int cmp = icxCompareVars( icxGetSubjFeature(subj,idx[left],feature) , split->boundary.value , type);
    if (cmp > 0)
	retval = 0;
    else
    {
	cmp = icxCompareVars(icxGetSubjFeature(subj,idx[right],feature), split->boundary.value , type);
	if (cmp <= 0)
	    retval =  right + 1 ;
	else
	{
	    while ( right - left > 1)
	    {
		int middle = (left + right) >> 1;
		cmp = icxCompareVars(icxGetSubjFeature(subj,idx[middle],feature), split->boundary.value , type);
		if (cmp <= 0)
		    left = middle;
		else
		    right = middle;
	    }
	    retval = right ;
	}
    }
    if (left_nonmissing)
	*left_nonmissing = retval;
    int direction = (retval > num_not_missed - retval) ? -1 : 1; // -1 - left
    for (int  i = num_not_missed ; i < node->num_fallens; i++)
	retval += cxIsAtomSplitLeft(cart,node, idx[i] ,split , direction);
    return retval;
    __CLEANUP__;
    __END__;
}

CART_IMPL int cxFindCategoricSplitProportion(CxCART *cart,
					     CxCARTNode *node,
					     CxCARTSplit* split,
					     int* left_nonmissing)
{
    CV_FUNCNAME( "cxFindCategoricSplitProportion" );
    __BEGIN__;
    assert(node->is_idx_expanded);
#ifdef _DEBUG
    int type = cart->feature_type[split->feature_idx];
#endif
    int feature = split->feature_idx;
    int eff_feature = cart->features_corr[feature];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );
    assert(icxIsClassifierVarCategoric(type) );
    int* idx = node->fallen_idx + node->num_fallens * eff_feature;
    int left = 0;
    int i;
    int num_not_missed = node->num_fallens - node->num_missed[eff_feature];
    for (i = 0 ; i < num_not_missed; i++)
    {
	left += icxIsVarSplitLeft(cart, node->split , icxGetSubjFeature(node->subj , idx[i] , feature) );
    }
    if (left_nonmissing)
	*left_nonmissing = left;
    int direction = (left > num_not_missed - left) ? -1 : 1; // -1 - left
    for ( i = num_not_missed ; i < node->num_fallens; i++)
	left += cxIsAtomSplitLeft(cart,node, idx[i] ,split , direction);
    return left;
    __CLEANUP__;
    __END__;
}

CART_API int cxFindAnySplitLeftNonmissing(CxCARTBase* cart,
					  CxForestNode* node,
					  CxCARTSplit* split ,
					  int* not_missed_total ,
					  char* cur_missed_mask )
{
    int feature = split->feature_idx;
#ifdef _DEBUG
    int eff_feature = cart->features_corr[feature];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );
#endif
    CxCARTSubj* subj = node->subj;
    CxCARTAtom* body = subj->body;
    int* idx = node->fallen_idx;
    int left = 0;
    int features_step = subj->features_step;
    int missed_mask_step = subj->missed_mask_step;

    int shift = feature * features_step;
    int shift_missed = feature * missed_mask_step;
    //// Cycle on non - missing values ////////
    int not_missed = 0;
    for (int i = 0 ; i < node->num_fallens; i++)
    {
	int str_num = idx[i];
	CxCARTAtom* atom = body + str_num;
	char* missed_mask = atom->missed_mask;
	char is_not_missed = (!missed_mask || !missed_mask[shift_missed] ) ;
	if (cur_missed_mask)
	    cur_missed_mask[str_num] = !is_not_missed;
	not_missed += is_not_missed;
	if (is_not_missed)
	{
	    CxClassifierVar var = *(CxClassifierVar *)((char*)atom->features + shift);
	    left += icxIsVarSplitLeft(cart , split , var);
	}
    }

    if (not_missed_total)
	*not_missed_total = not_missed;
    return left;
}


CART_IMPL int cxFindAnySplitProportion(CxCART* cart,
				       CxCARTNode* node,
				       CxCARTSplit* split,
				       int* left_nonmissing)
{
    CV_FUNCNAME( "cxFindAnySplitProportion" );
    __BEGIN__;
#ifdef _DEBUG
    int feature = split->feature_idx;
    int eff_feature = cart->features_corr[feature];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );
#endif*/
    CxCARTSubj* subj = node->subj;
    CxCARTAtom* body = subj->body;
    int* idx = node->fallen_idx;
    int features_step = subj->features_step;
    int missed_mask_step = subj->missed_mask_step;
#ifdef _DEBUG
    int shift_missed = feature * missed_mask_step;
#endif
    char* cur_missed_mask = (char*)malloc(subj->size * sizeof(char));
    int num_missed = 0;
    int left = cxFindAnySplitLeftNonmissing((CxCARTBase*)cart,
	(CxForestNode*)node , split, &num_missed , cur_missed_mask);
    if (left_nonmissing)
	*left_nonmissing = left;
    int direction = (left > num_missed - left) ? -1 : 1; // -1 - left
    //// Cycle on missed values /////////
    for (int i = 0 ; i < node->num_fallens; i++)
    {
	int str_num = idx[i];
	CxCARTAtom* atom = body + idx[i];
	char* missed_mask = atom->missed_mask;
	assert( (missed_mask && (missed_mask[shift_missed]) ) == cur_missed_mask[str_num]);
	if (cur_missed_mask[str_num])
	{
	    left += cxIsStringSplitLeft(cart, node,
		atom->features , features_step,
		missed_mask, missed_mask_step,
		split, direction);
	}
    }
    free(cur_missed_mask);
    return left;
    __CLEANUP__;
    __END__;
}




CART_IMPL float cxFindSplitAssoc(CxCART *cart,
				 CxCARTNode *node,
				 CxCARTSplit* split1,
				 CxCARTSplit* split2)
{
    CV_FUNCNAME( "cxFindSplitAssoc" );
    __BEGIN__;
    int feature1 = split1->feature_idx;
    int feature2 = split2->feature_idx;

    int eff_feature1 = cart->features_corr[feature1];
#ifdef _DEBUG
    int eff_feature2 = cart->features_corr[feature2];
#endif
    assert( eff_feature1 >= 0 && eff_feature1 < cart->eff_num_features );
    assert( eff_feature2 >= 0 && eff_feature2 < cart->eff_num_features );
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    float* left_right = (float*)calloc( 2, sizeof(float));
    int i = 0;
    BOOL floating = icxIsClassifierVar32f(cart->response_type);

    int num_fallen = node->num_fallens ;
    float predicted = 0.0;
    int* idx = node->fallen_idx + num_fallen * eff_feature1;

    for (i = 0 ; i < node->num_fallens - node->num_missed[eff_feature1] ; i++)
    {
	if ( !icxIsSubjFeatureMissed(node->subj,idx[i],feature2) )
	{
	    CxClassifierVar var1 = icxGetSubjFeature(node->subj , idx[i] , feature1);
	    CxClassifierVar var2 = icxGetSubjFeature(node->subj , idx[i] , feature2);

	    int cur_left1 = icxIsVarSplitLeft(cart , split1 , var1);
	    int cur_left2 = icxIsVarSplitLeft(cart , split2 , var2);

	    float mult = 1.0f;
	    if (!regression && cart->params->priors )
	    {
		var1 = icxGetSubjResponse(node->subj,idx[i]);
		int resp = icxVarToInt(var1 , floating);
		mult = cart->priors_mult[resp];
	    }
	    left_right[cur_left1] += mult;
	    predicted += (cur_left1 == cur_left2) * mult;
	}
    }
    float real_num = left_right[0] + left_right[1];
    float min = MIN(left_right[0] , left_right[1]) / real_num;
    float assoc = (min + predicted / real_num - 1.0f) / min;

    free(left_right);
    return (assoc < EPS) ? 0.0f : assoc;
    __CLEANUP__;
    __END__;
}

CART_IMPL float cxFindCARTNodeNumericSurrogate( CxCART *cart,
					       CxCARTNode *node,
					       int feature,
					       CxCARTSplit* split)
{
    CV_FUNCNAME( "cxFindCARTNodeNumericSurrogate" );
    __BEGIN__;
    int i;
    START_FUNC(cxFindCARTNodeNumericSurrogate);

    int type = cart->feature_type[feature];

    assert(node->split);
    split->feature_idx = feature;
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);

    int eff_feature = cart->features_corr[feature];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );

    int feature_old = node->split->feature_idx;
    // If they ask us for surrogate split on the SAME variable, return primary split
    if (feature_old == feature)
    {
	split->boundary = node->split->boundary;
	EXIT_FUNC(cxFindCARTNodeNumericSurrogate);
	return 1.0;
    }
    int* idx = node->fallen_idx + eff_feature * node->num_fallens;
    BOOL floating = icxIsClassifierVar32f(type) ;

    float left_right[2] = {0.0 , 0.0 };
    int num_not_missed = node->num_fallens - node->num_missed[eff_feature];

    CxCARTSubj* subj = node->subj;
    CxCARTAtom* body = subj->body;
    char* cur_split_directions = subj->cur_split_directions;
    int shift = feature * subj->features_step;
#ifdef _DEBUG
    CxCARTSplit* split_old = node->split;
    int shift_old = feature_old * subj->features_step;
    int shift_missed_old = feature_old * subj->missed_mask_step;
#endif
    /*   It's useful to store this in special arrys to reduce random memory access
    in second loop */
    CxClassifierVar* feature_vals_init = (CxClassifierVar*)node->cat_proportions[eff_feature];
    BOOL is_priors_mult = (!regression && cart->params->priors );
    int mem_economy_mode = cart->params->mem_economy_mode;

    CxClassifierVar* responses = subj->response;

    CxClassifierVar value;//feature_vals[0];
    value.fl = -FLT_MAX;
    int best_i = -1;
    int best_i_revert = -1;

    float predicted = 0.0;
    float min_pred_cur = FLT_MAX;
    float max_pred_cur = -FLT_MAX;
    float* priors_mult = cart->priors_mult;

    int* left_right_int = (int*)left_right;
    /* Calculating numbers of cases split to left or right by primary split,
    amongst those with nonmissing split variable and current variable */
    //	is_priors_mult = FALSE; // ##TEST##
    if (is_priors_mult)
    {
	//// Floating - point calculations, because of FP priors.
	for (i = 0; i < num_not_missed ; i++)
	{
	    int str_num = idx[i];
	    int side = cur_split_directions[str_num];
#ifdef _DEBUG
	    CxCARTAtom* atom = body + str_num;
	    char* missed_mask = atom->missed_mask;
	    if (!missed_mask || !(missed_mask[shift_missed_old]) )
		assert(side != -1);
#endif
	    if (side != -1)
	    {
		CxClassifierVar value_new = ((!mem_economy_mode) ?
		    feature_vals_init[i] : *(CxClassifierVar *)((char*)body[str_num].features + shift));
		if (!icxCompareVarsSimple(value , value_new))
		{
		    value = value_new;
		    predicted = left_right[0] - left_right[1] ;
		    //// Good non-reverted surrogate
		    if (predicted > max_pred_cur)
		    {
			max_pred_cur = predicted;
			best_i = i - 1;
		    }
		    //// Good reverted surrogate
		    else if (predicted < min_pred_cur)
		    {
			min_pred_cur = predicted;
			best_i_revert = i - 1;
		    }
		}

#ifdef _DEBUG
		assert (fabs(value_new.fl - (*(CxClassifierVar *)((char*)atom->features + shift)).fl) < EPS);
		CxClassifierVar var = *(CxClassifierVar *)((char*)atom->features + shift_old);
		int cur_left = icxIsVarSplitLeft(cart , split_old , var);
		assert(cur_left == 1-side);
#endif
		//// Multiply everything by priors !
		assert( fabs(atom->response.fl - responses[str_num].fl) < EPS);
		int resp = icxVarToInt(responses[str_num]);
		left_right[side] += priors_mult[resp];
	    }
	}
    }
    else
    {
	//// Integer calculations , much faster
	int predicted_int = 0;
	int min_pred_cur_int = INT_MAX;
	int max_pred_cur_int = -INT_MAX;
	for (i = 0; i < num_not_missed ; i++)
	{
	    int str_num = idx[i];
	    int side = cur_split_directions[str_num];
#ifdef _DEBUG
	    CxCARTAtom* atom = body + str_num;
	    char* missed_mask = atom->missed_mask;
	    if (!missed_mask || !(missed_mask[shift_missed_old]) )
		assert(side != -1);
#endif
	    if (side != -1)
	    {
		CxClassifierVar value_new = ((!mem_economy_mode) ?
		    feature_vals_init[i] : *(CxClassifierVar *)((char*)body[str_num].features + shift));
		FUNC_MEM_READ(cxFindCARTNodeNumericSurrogate, 1 , int);

		if (!icxCompareVarsSimple(value , value_new))
		{
		    value = value_new;
		    predicted_int = left_right_int[0] - left_right_int[1] ;
		    FUNC_MEM_READ(cxFindCARTNodeNumericSurrogate, 2 , int);

		    //// Good non-reverted surrogate
		    if (predicted_int > max_pred_cur_int)
		    {
			max_pred_cur_int = predicted_int;
			best_i = i - 1;
		    }
		    //// Good reverted surrogate
		    else if (predicted_int < min_pred_cur_int)
		    {
			min_pred_cur_int = predicted_int;
			best_i_revert = i - 1;
		    }
		}

#ifdef _DEBUG
		assert (fabs(value_new.fl - (*(CxClassifierVar *)((char*)atom->features + shift)).fl) < EPS);
		CxClassifierVar var = *(CxClassifierVar *)((char*)atom->features + shift_old);
		int cur_left = icxIsVarSplitLeft(cart , split_old , var);
		assert(cur_left == 1-side);
#endif
		left_right_int[side] ++;
		FUNC_MEM_MODIFY(cxFindCARTNodeNumericSurrogate, 1 , int);
	    }
	}
	//// Convert to float
	left_right[0] = (float)left_right_int[0];
	left_right[1] = (float)left_right_int[1];
	predicted = (float)predicted_int;
	min_pred_cur = (float)min_pred_cur_int;
	max_pred_cur = (float)max_pred_cur_int;
    }
    //// Read idx[i] and cur_split_directions[str_num]
    FUNC_MEM_READ(cxFindCARTNodeNumericSurrogate, num_not_missed , int);
    FUNC_MEM_READ(cxFindCARTNodeNumericSurrogate, num_not_missed , char);


    if ( left_right[0] == 0 || left_right[1] == 0 || num_not_missed == 0)
    {
	EXIT_FUNC(cxFindCARTNodeNumericSurrogate);
	return -1.0;
    }
#ifdef _DEBUG
    /*	if ((node->id == 1) && (feature == 2))
    {
    split->value.fl = 6.5;
    split->revert = TRUE;
    //		float assoc1 = cxFindSplitAssoc(cart,node,split_old,split);
	}*/
#endif
    BOOL best_revert = FALSE;
    float best_assoc = EPS;
    // Calculate predictive association ///
    max_pred_cur += left_right[1];
    min_pred_cur += left_right[1];

    float real_num = left_right[0] + left_right[1];
    float max_pred_revert = real_num - min_pred_cur;
    float min_pred_cur0 = MIN(left_right[0] , left_right[1]);

    if (max_pred_cur > max_pred_revert)
	//// Normal split
    {
	best_assoc = ((min_pred_cur0 + max_pred_cur) - real_num) / min_pred_cur0;
    }
    else
    {
	//// Reverted split
	best_assoc = ((min_pred_cur0 + max_pred_revert) - real_num) / min_pred_cur0;
	best_revert = TRUE;
	best_i = best_i_revert;
    }
    //// No good surrogate splits
    if ((best_i < 0) || (best_assoc <= EPS))
    {
	//		split->value = value_zero;
	best_assoc = 0.0;
    }
    else
    {
	// Adjust split value to be average of closest split variable values ////////
	int next_to_best = best_i+1;//MIN( num_not_missed - 1 , best_i+1);
	assert(subj->cur_split_directions[idx[next_to_best]] != -1);
	assert(next_to_best < num_not_missed);
	// Find first previous nonmissing string.
	while ( (subj->cur_split_directions[idx[best_i]] == -1) && best_i)
	{
	    assert(idx[best_i] >= 0 && idx[best_i] < subj->size );
	    best_i--;
	}
	CxClassifierVar val1 = (!mem_economy_mode) ? feature_vals_init[best_i] :
	icxGetSubjFeature(subj , idx[best_i] , feature);
	CxClassifierVar val2 = (!mem_economy_mode) ? feature_vals_init[next_to_best] :
	icxGetSubjFeature(subj , idx[next_to_best] , feature);
	assert(!icxCompareVarsSimple(val1 , val2));
	split->boundary = icxGetHalfwayBetween(val1, val2, floating);
    }
    split->weight = best_assoc; // !!!!!!!!!!!!! For sorting
    split->revert = best_revert;
    EXIT_FUNC(cxFindCARTNodeNumericSurrogate);
    return split->weight ;
    __CLEANUP__;
    __END__;
}

CART_IMPL float cxFindCARTNodeCategoricSurrogate( CxCART *cart,
						 CxCARTNode *node,
						 int feature,
						 CxCARTSplit* split)
{
    CV_FUNCNAME( "cxFindCARTNodeCategoricSurrogate" );
    __BEGIN__;

    int n2 = cart->num_classes[feature];
    assert( n2 > 0 && n2 < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);

    if (n2 < 2)
    {
	return 0.0;
    }
    split->feature_idx = feature;

    int eff_feature = cart->features_corr[feature];
    int* idx = node->fallen_idx + eff_feature * node->num_fallens;
    int feature_old = node->split->feature_idx;

    if (feature_old == feature)
    {
	memcpy(split->boundary.ptr , node->split->boundary.ptr , n2);
	return 1.0;
    }

    START_FUNC(cxFindCARTNodeCategoricSurrogate);

    float* proportion = (float*)calloc( 2 * n2 , sizeof(float));
    float left_right_prob[2] = {0.0,0.0};

    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);

    CxCARTSubj* subj = node->subj;
#ifdef _DEBUG
    int type2 = cart->feature_type[feature];
    assert(icxIsClassifierVarCategoric(type2));
    CxCARTSplit* split_old = node->split;
    CxCARTAtom* body = subj->body;
    int shift_old = feature_old * subj->features_step;
    int shift = feature * subj->features_step;
#endif
    int* cat_proportion = icxGetCatProportions(cart , node , eff_feature);
    CxClassifierVar* responses = subj->response;
    char* cur_split_directions = subj->cur_split_directions;
    BOOL use_priors = (!regression && cart->params->priors);
    float* priors_mult = cart->priors_mult;
    /* Calculate proportions of cases with nonmissing split feature and current feature
    in each category split to left or right by main (primary) split. proportion[0-n2] -
    left , proportion[n2-2*n2-1] - to right*/
    //	use_priors = FALSE; // ##TEST##
    int k = 0;
    if (!use_priors)
    {
	//// Fast integer calculations
	for (int* _idx = idx ; k < n2; k++)
	{
	    int num_var_class = cat_proportion[k];
	    int sum[3] = {0,0,0}; // for side = -1 . 0 . 1 respectively
	    int* _sum = sum + 1;
	    for (int j = 0 ; j < num_var_class ; j++, _idx++)
	    {
		int str_num = (*_idx);
		int side = cur_split_directions[str_num];
#ifdef _DEBUG
		CxCARTAtom* atom = body + str_num;
		if (icxIsSubjFeatureMissed(subj , str_num , feature_old))
		    assert(side == -1);
		if (side != -1) // If not missed
		{
		    assert(side == 0 || side == 1);
		    CxClassifierVar var = *(CxClassifierVar *)((char*)atom->features + shift);
		    int value = icxVarToInt(var);
		    assert( value >=0 && value < n2);
		    assert(value == k);
		    CxClassifierVar var1 = *(CxClassifierVar *)((char*)atom->features + shift_old);
		    int right = 1 - icxIsVarSplitLeft(cart, node->split , var1);
		    assert( right == side );
		    _sum[side] ++;
		}
#else
		_sum[side] ++;
#endif
		//			FUNC_MEM_MODIFY(cxFindCARTNodeCategoricSurrogate, 1 , int);

	    }
	    proportion[k] = (float)_sum[0];
	    proportion[n2 + k] = (float)_sum[1];
	}
    }
    else
    {
	//// FP calculations because of priors
	for (int* _idx = idx ; k < n2; k++)
	{
	    int num_var_class = cat_proportion[k];
	    float sum[2] = {0.0, 0.0};
	    for (int j = 0 ; j < num_var_class ; j++, _idx++)
	    {
		int str_num = (*_idx);
		int side = cur_split_directions[str_num];

#ifdef _DEBUG
		CxCARTAtom* atom = body + str_num;
		if (icxIsSubjFeatureMissed(subj , str_num , feature_old))
		    assert(side == -1);
#endif
		if (side != -1) // If not missed
		{
#ifdef _DEBUG
		    CxClassifierVar var = *(CxClassifierVar *)((char*)atom->features + shift);
		    int value = icxVarToInt(var);
		    assert( value >=0 && value < n2);
		    assert(value == k);
		    CxClassifierVar var1 = *(CxClassifierVar *)((char*)atom->features + shift_old);
		    int right = 1 - icxIsVarSplitLeft(cart, split_old , var1);
		    assert( right == side );
		    assert( fabs(atom->response.fl - responses[str_num].fl) < EPS);
#endif
		    int resp = icxVarToInt(responses[str_num]);
		    sum[side] += priors_mult[resp];
		}
	    }
	    proportion[k] = sum[0];
	    proportion[n2 + k] = sum[1];
	}
    }
    //// Read *_idx and cur_split_directions[str_num]
    FUNC_MEM_READ(cxFindCARTNodeCategoricSurrogate, node->num_fallens - node->num_missed[eff_feature] , int);
    FUNC_MEM_READ(cxFindCARTNodeCategoricSurrogate, node->num_fallens - node->num_missed[eff_feature] , char);
    //// Modify sum[side]
    FUNC_MEM_MODIFY(cxFindCARTNodeCategoricSurrogate, node->num_fallens - node->num_missed[eff_feature] , int);

    if (!regression)
	free(cat_proportion);
	/* Calculate proportions of cases with nonmissing split feature split
    and current feature to left and to right by main split */
    int i;
    for (i = 0 ; i < n2 ; i++)
	for (int j = 0 ; j < 2 ; j++)
	    left_right_prob[j] += proportion[j * n2 + i];

	if ( left_right_prob[0] == 0 || left_right_prob[1] == 0 )
	{
	    free(proportion);
	    EXIT_FUNC(cxFindCARTNodeCategoricSurrogate);

	    return -1.0;
	}
	double real_prob = left_right_prob[0] + left_right_prob[1];
	double inv_real_prob = 1.0f/real_prob;
	double min = ((float)MIN(left_right_prob[0] , left_right_prob[1])) * inv_real_prob;
	double inv_min = 1.0f/min;

	char* ptr = (char*)split->boundary.ptr;

	float predicted = 0.0;
	/*	 Calculate predictive association for split and it's missing categories.
	Categories in which primary splits sends more to the left , are split to the left by
	best surrogate , others are split to the right */
#ifdef _DEBUG
	//	int left0 = 0;
	//	BOOL all_together = TRUE;
	/*	if (node->id == 2 && feature == 3)
	{ int mmm = 1; }*/
#endif
	for (i = 0 ; i < n2 ; i++)
	{
	    float prop_right = proportion[n2 + i] ;
	    float prop_left =  proportion[i];
	    int left = (prop_right < prop_left);//( proportion[n2 + i] < proportion[i] ) ;
#ifdef _DEBUG
	    //		if (i==0) left0 = left;
	    //		else all_together &= ( left == left0 );
#endif
	    int missed = (prop_left + prop_right < EPS );
	    ptr[i] = (char)(missed ? 2 : left) ;
	    predicted += proportion[n2 * (1-left) + i];
	}

	float best_assoc = (float) ((min + predicted * inv_real_prob - 1.0) * inv_min);
#ifdef _DEBUG
	//	if (all_together) assert(best_assoc < EPS);
#endif
	free(proportion);
	split->weight = MAX (best_assoc , 0.0f); // !!!!!!!!!!!!! For sorting
	EXIT_FUNC(cxFindCARTNodeCategoricSurrogate);

	return split->weight;
	__CLEANUP__;
	__END__;
}

CART_IMPL void cxFindCARTNodeSurrogates( CxCART *cart, CxCARTNode *node,
					int num_surrogates ,
					float threshold)
{
    CV_FUNCNAME( "cxFindCARTNodeSurrogates" );
    __BEGIN__;
    {
	int i;
	CxClassifierVarType type;
	assert(node->is_idx_expanded);

	if ( cart == 0 || cart->root == 0 || node == 0 )
	{
	    assert(0);
	    CV_ERROR( CV_StsNullPtr, "" );
	}
	if ( num_surrogates <= 0 )
	    return ;
	assert(node->split);
	int cur_size = 0;
	int eff_num_features = cart->eff_num_features;

	num_surrogates = MIN( num_surrogates , eff_num_features );
	CxCARTSplit** split_array = (CxCARTSplit**)calloc(eff_num_features , sizeof(CxCARTSplit*));
	int num_good_splits = 0;

#ifdef _DEBUG
	//	ofstream file ("cart.log",ios::app);
#endif
	double weight = 0.0;
	CxCARTSplit* split = NULL;

	for ( i = 0; i < eff_num_features; i++ )
	{
	    if ( i == cart->features_corr[node->split->feature_idx] )
		continue;
	    int cur_feature = cart->features_back_corr[i];
	    split = cxCARTSplit(cart, cur_feature);
	    type = cart->feature_type[cur_feature];
#ifdef _DEBUG
	    cxAssertNodeValid(cart , node);
#endif
	    if (icxIsClassifierVarNumeric(type))
		weight = cxFindCARTNodeNumericSurrogate(cart,node,cur_feature,split);
	    else
		weight = cxFindCARTNodeCategoricSurrogate(cart,node,cur_feature,split);
#ifdef _DEBUG
	    cxAssertNodeValid(cart , node);
	    //		file << "Surrogate in node : "<< node->id << ", feature : " << i << " weight = " << split->weight << '\n';
	    //		file.flush();
#endif
	    if (weight > EPS)
	    {
		split_array[num_good_splits++] = split;
	    }
	}
	split = NULL;
	if (num_good_splits > 0)
	{
	    int i;

	    assert( num_good_splits <= eff_num_features );
	    qsort(split_array , num_good_splits , sizeof(CxCARTSplit*) , compare_splits);
	    CxCARTSplit* split_prev = NULL;
	    cur_size = MIN ( num_good_splits , num_surrogates);
	    for ( i = 0; i < cur_size ; i++ )
	    {
		if (split_array[i]->weight < threshold)
		{
		    cur_size = i;
		    break;
		}
	    }
	    //// Copy first cur_size splits to node buffer
	    cxAddNodeSplits(cart, node , split_array , cur_size, SPLIT_SURROGATE);

	    for ( i = cur_size - 1; i >= 0 ; i-- )
	    {
		split_prev = split;
		split = split_array[i];
#ifdef _DEBUG
		//		file << "Surrogate in node : "<< node->id << ", weight = " << split->weight << '\n';
#endif
		split->next_surrogate = split_prev;
	    }
	}
#ifdef _DEBUG
	cxAssertNodeValid(cart , node);
#endif
	node->split->next_surrogate = split;
	free(split_array);
    }
   	__CLEANUP__;
	__END__;
}

CxCARTSplit* cxSelectNodeSurrogate( CxCARTNode *node,
				   int feature)
{
    if (!node || !(node->split))
	return NULL;
    CxCARTSplit* surrogate = node->split;
    while (surrogate)
    {
	if (surrogate->feature_idx == feature)
	    return surrogate;
	surrogate = surrogate->next_surrogate;
    }
    return NULL;
}

CART_IMPL float cxCalcCARTVariableImportance(CxCART* cart, int feature, int pruning_step)
{
    CV_FUNCNAME( "cxCalcCARTVariableImportance" );
    __BEGIN__;

    int eff_feature = cart->features_corr[feature];
    if (eff_feature < 0)
	return 0.0f;

    CxCARTNode* node = cart->root;
    int type = cart->feature_type[feature];
    float importance = 0.0;
    CxCARTSplit* split = cxCARTBaseSplit((CxCARTBase*)cart, feature );
    BOOL numeric = icxIsClassifierVarNumeric(type);
    BOOL calc_surrogates = FALSE;
    // Cannot calculate surrogates here , may be implemented later
    while (node)
    {
	if (icxIsNodeSplit(node) && !icxIsNodePruned(node , pruning_step))
	{
	    if (calc_surrogates)
	    {
		float assoc = 0.0;
		if (numeric)
		    assoc = cxFindCARTNodeNumericSurrogate(cart,node,feature,split);
		else
		    assoc = cxFindCARTNodeCategoricSurrogate(cart,node,feature,split);
		if (assoc > EPS)
		    importance += cxCalcImpurityChangeAny(cart,node,split);
	    }
	    else
	    {
		CxCARTSplit* surrogate = cxSelectNodeSurrogate(node, feature);
		if (surrogate)
		{
		    importance += cxCalcImpurityChangeAny(cart,node,surrogate);
		}
	    }
	}
	node = cxGetNextNode(node, pruning_step);
    }
    free(split);
    return importance;
    __CLEANUP__;
    __END__;
}

CART_API void cxCalcCARTVariableImportances(CxCART* cart,
					    float* imps,
					    int pruning_step )
{
    int num_features = cart->num_features;
#ifdef CART_WITH_GUI
    assert(!IsBadWritePtr(imps, num_features * sizeof(float)));
#endif
    memset(imps, 0, num_features * sizeof(float));
    CxCARTNode* node = cart->root;
    while (node)
    {
	if (icxIsNodeSplit(node) && !icxIsNodePruned(node , pruning_step))
	{
	    CxCARTSplit* surrogate = node->split;
	    while (surrogate)
	    {
		int feature = surrogate->feature_idx;
		imps[feature] += cxCalcImpurityChangeAny(cart,node,surrogate);
		surrogate = surrogate->next_surrogate;
	    }
	}
	node = cxGetNextNode(node, pruning_step);
    }
    /*	float max_imp = 0;
    for (int i = 0; i < num_features ; i++)
    max_imp = __max(max_imp , imps[i]);
    for (int i1 = 0; i1 < num_features ; i1++)
    imps[i] /= max_imp;*/
    return ;
}

#pragma warning(disable : 4725)

CART_IMPL float cxFindNodeNumericBestSplitRegression( CxCART *cart,
						     CxCARTNode *node,
						     int feature,
						     CxCARTSplit* split)
{
    CV_FUNCNAME( "cxFindNodeNumericBestSplitRegression" );
    __BEGIN__;
    assert(icxIsClassifierVarNumeric(cart->feature_type[feature]));
    assert(icxIsClassifierVarNumeric(cart->response_type));

    split->feature_idx = feature;
    int best_i = -1;
    float eps_weight = node->parent ? cart->root->split->weight * EPS : EPS;
    double best_weight = eps_weight;
    int eff_feature = cart->features_corr[feature];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );
    int* idx = node->fallen_idx + node->num_fallens * eff_feature;
    split->boundary.value.i = 0;
    int num_fallens = node->num_fallens;
    //	double sum1 = 0.0;
    double sum2 = 0.0;
    double sum_sq2 = 0.0;
    BOOL floating = icxIsClassifierVar32f(cart->feature_type[feature]);
    assert(cart && cart->root);
    int total = cart->root->num_fallens;
    double inv_total = 1.0/total;
    int num_not_missed = num_fallens - node->num_missed[eff_feature];

    // Exit if unsuficcient number of cases in node to split it.
    int min_points = MAX(cart->params->split_min_points , 1);
    if ( num_not_missed < 2 * min_points )
    {
	return 0.0;
    }

    int mem_economy_mode = cart->params->mem_economy_mode;
    /* We allocate feature_vals in memory economy mode , then we fill it from
    main data matrix , otherwise we just get values that are stored in
    node->cat_proportions */
    CxClassifierVar* feature_vals = (mem_economy_mode ?
	icxAllocFeatureVals(cart) :	(CxClassifierVar*)node->cat_proportions[eff_feature]);
    //  Take buffer of sufficient length from storage.
    float* response_vals = icxAllocFloatBuf(cart);//(float*)malloc(size);
    CxCARTSubj* subj = node->subj;
    CxClassifierVar* responses = subj->response;
    START_FUNC(cxFindNodeNumericBestSplitRegression);


    /* Calculate initial node responses sum and sum of squares. Set left node to be empty,
    right node equal to main (current) node, Also calculate node impurity. It is useful
    to store them in special arrays to reduce random memory access */
    if (!mem_economy_mode)
    {
#if INLINE_ASM_OPTIMIZE
	__asm
	{
	    mov ecx , num_not_missed
		mov edx , idx
		mov esi , responses
		mov edi , response_vals
		fldz
		fldz
loop1:
	    mov ebx , [edx]
		fld dword ptr [esi + 4 * ebx]
		fadd st(1) , st(0)
		fst dword ptr [edi]
		fmul st(0) , st(0)
		faddp st(2) , st(0)
		add edx , 4
		add edi , 4
		dec ecx
		jnz loop1
		fstp qword ptr [sum2]
		fstp qword ptr [sum_sq2]
	}
#else
#ifdef _DEBUG
	CxCARTAtom* body = subj->body;
	int shift = feature * subj->features_step;
#endif
	for (int i = 0 ; i < num_not_missed ; i++)
	{
#ifdef _DEBUG
	    CxCARTAtom* atom = body + idx[i];
	    assert( fabs(feature_vals[i].fl - (*(CxClassifierVar *)((char*)atom->features + shift)).fl) < EPS);
	    assert( fabs (atom->response.fl - responses[idx[i]].fl) < EPS);
#endif
	    float f = icxVarToFloat(responses[idx[i]]);
	    response_vals[i] = f;
	    sum2 += f;
	    sum_sq2 += f*f;
	}
#endif
	/// Read idx[j] and responses[idx[j]]
	FUNC_MEM_READ(cxFindNodeNumericBestSplitRegression , num_not_missed , CxClassifierVar);
	FUNC_MEM_READ(cxFindNodeNumericBestSplitRegression , num_not_missed , int);
	/// Write response_vals[i]
	FUNC_MEM_WRITE(cxFindNodeNumericBestSplitRegression , num_not_missed , int);
    }
    else
    {
	CxCARTAtom* body = subj->body;
	int shift = feature * subj->features_step;

	// Fill feature_vals from main matrix
	for (int i = 0 ; i < num_not_missed ; i++)
	{
	    CxCARTAtom* atom = body + idx[i];
	    feature_vals[i] = *(CxClassifierVar *)((char*)atom->features + shift);
	    float f = icxVarToFloat(atom->response);
	    response_vals[i] = f;
	    sum2 += f;
	    sum_sq2 += f*f;
	}
    }
    double impurity = (sum_sq2 - (sum2 * sum2) / num_not_missed )/ total;
    // Exit if node impurity is too small
    if (impurity < eps_weight)
    {
	EXIT_FUNC ( cxFindNodeNumericBestSplitRegression );
	return 0.0;
    }
    //	CxClassifierVar value = feature_vals[0];
    int upper_limit = num_not_missed - min_points + 1;
    double best_val =  sum2 * sum2 / num_not_missed ;
    double init_val = best_val;
    ///// Pass all cases in given node by increment of split variable ////////
#if INLINE_ASM_OPTIMIZE
#ifdef _DEBUG
    double old_best_val = best_val;
    double old_sum2 = sum2;
#endif
    __asm
    {
	mov ecx , upper_limit   // ecx <- upper_limit - i
	    mov edx , ecx
	    sub edx , min_points    // edx <- upper_limit - min_points
	    mov esi , feature_vals  // esi <- feature vals
	    mov edi , response_vals // edi <- response_vals
	    mov ebx , [esi]			// value = feature_vals[0]
	    fld  qword ptr [best_val]      // st(4) <- best_val
	    fild dword ptr num_not_missed  // st(3) <- num_not_missed
	    fild dword ptr upper_limit     // st(2) <- upper_limit
	    fld qword ptr sum2		// st(1) <- sum2
	    fldz  // sum1 = 0 | sum2 | upper_limit | num_not_missed | best_val
	    push ebp
	    mov ebp , upper_limit
loop2 :
	mov eax , ebx			// eax <- value_new = value;
loop_value_same :
	cmp eax , ebx
	    jne exit_loop_value_same
	    fld dword ptr [edi]  // response | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    add edi , 4
	    fsub st(2) , st(0)
	    faddp st(1) , st(0)  // sum1 + response | sum2 - response | upper_limit | num_not_missed | best_val
	    add esi , 4
	    mov eax , [esi]
	    dec ecx
	    jz exit_loop2
	    jmp loop_value_same
exit_loop_value_same :
	mov ebx , eax // value = value_new;
	    cmp ecx , edx
	    ja  loop2     // i < min_points - repeat main loop
	    push ecx
	    fild dword ptr [esp] // upper_limit - i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    pop ecx
	    fsubr st(0) , st(3)  // i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fld st(1)            // sum1 | i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fmul st(0) , st(2)   // sum1 * sum1 | i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fdiv st(0) , st(1)   // sum1 * sum1/i | i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fxch                 // i | sum1 * sum1/i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fsubr st(0), st(5)   // num_not_missed - i | sum1 * sum1/i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fld st(3)  // sum2 | num_not_missed - i | sum1 * sum1/i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fmul st(0) , st(0)   // sum2 * sum2 | num_not_missed - i | sum1 * sum1/i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fdiv st(0) , st(1)   // sum2 * sum2 / (num_not_missed - i) | num_not_missed - i | sum1 * sum1/i | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    faddp st(2) , st(0)  // num_not_missed - i | cur_val | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fstp st(0)           // cur_val | sum1 | sum2 | upper_limit | num_not_missed | best_val
	    fcomi st(0) , st(5)
	    cmova ebp , ecx
	    fcmovb st(0), st(5)
	    fxch st(5)           // cur_val | sum1 | sum2 | upper_limit | num_not_missed | best_val (updated)
	    fstp st(0)           // sum1 | sum2 | upper_limit | num_not_missed | best_val (updated)
	    jmp loop2
exit_loop2 :
	mov ecx , ebp
	    pop ebp
	    mov eax , upper_limit
	    sub eax , ecx
	    dec eax
	    fcompp
	    fcompp
	    fstp qword ptr [best_val]
	    mov [best_i] , eax
    }
#ifdef _DEBUG
    double new_best_val = old_best_val;
    double sum1 = 0.0;
    sum2 = old_sum2;
    CxClassifierVar value = feature_vals[0];
    int best_i_new = -1;
    for (int i = 0; i < upper_limit; )
    {
	CxClassifierVar value_new = value;
	// While split variable doesn't change ...
	while (icxCompareVarsSimple(value_new , value) & (i < upper_limit))
	{
	    //   Move the case from right to left //////////
	    double f = response_vals[i];
	    FUNC_MEM_READ( cxFindNodeNumericBestSplitRegression , 1 , CxClassifierVar);
	    sum1 += f ;
	    sum2 -= f ;
	    value_new = feature_vals[i+1];
	    FUNC_MEM_READ( cxFindNodeNumericBestSplitRegression , 1 , CxClassifierVar);
	    i++;
	}
	value = value_new;
	/// Calculate split weight - only if not less than min_points goes to each side
	if ((i >= min_points) & (i < upper_limit))
	{
	    double i2 = num_not_missed - i;
	    double cur_val = (sum1 * sum1/i + sum2 * sum2/i2);
	    if (cur_val > new_best_val)
	    {
		best_i_new = i-1;
		new_best_val = cur_val;
	    }
	}
    }
    assert(/* (best_i_new == best_i) && */(fabs (1.0 - best_val/new_best_val ) < EPS));
#endif
#else
    double sum1 = 0.0;
    CxClassifierVar value = feature_vals[0];
    //	float* buf_inv = cart->storage->calc_storage.buf_inv;
    for (int i = 0; i < upper_limit; )
    {
	CxClassifierVar value_new = value;
	// While split variable doesn't change ...
	while (icxCompareVarsSimple(value_new , value) & (i < upper_limit))
	{
	    //   Move the case from right to left //////////
	    double f = response_vals[i];
	    FUNC_MEM_READ( cxFindNodeNumericBestSplitRegression , 1 , CxClassifierVar);
	    sum1 += f ;
	    sum2 -= f ;
	    value_new = feature_vals[i+1];
	    FUNC_MEM_READ( cxFindNodeNumericBestSplitRegression , 1 , CxClassifierVar);
	    i++;
	}
	value = value_new;
	/// Calculate split weight - only if not less than min_points goes to each side
	if ((i >= min_points) & (i < upper_limit))
	{
	    int i2 = num_not_missed - i;
	    double cur_val = (sum1 * sum1/i + sum2 * sum2/i2);
	    //			double cur_val = (sum1 * sum1 * buf_inv[i] + sum2 * sum2 * buf_inv[i2]);
	    if (cur_val > best_val)
	    {
		best_i = i-1;
		best_val = cur_val;
	    }
	}
    }
#endif
    EXIT_FUNC ( cxFindNodeNumericBestSplitRegression );
    best_weight = (best_val - init_val) * inv_total;
    if ((best_weight < eps_weight) || (best_i < 0))
    {
	return 0.0;
    }

    // Adjust split value to be average of closest split variable values ////////
    int next_to_best = MIN( num_not_missed - 1 , best_i+1);
    CxClassifierVar val1 = feature_vals[best_i];
    CxClassifierVar val2 = feature_vals[next_to_best];
    split->boundary = icxGetHalfwayBetween(val1, val2, floating);

    split->weight = (float)best_weight;
#if DATA_TO_FILE1
    FILE* file1 = fopen("d:\\CART\\AlamereTest\\num_split_regressd.dat", "wb");
    FILE* file2 = fopen("d:\\CART\\AlamereTest\\num_split_regressf.dat", "wb");

    //// Write as doubles
    fwrite(&total, sizeof(int), 1, file1);
    fwrite(&num_not_missed, sizeof(int), 1, file1);
    fwrite(idx, sizeof(int), num_not_missed, file1);
    for (int i2 = 0 ; i2 < num_not_missed ; i2++)
    {
	double d = feature_vals[i2].fl;
	fwrite(&d, sizeof(double), 1, file1);
    }
    for (int i1 = 0 ; i1 < subj->size ; i1++)
    {
	double d = responses[i1].fl;
	fwrite(&d, sizeof(double), 1, file1);
    }
    fwrite(&best_weight, sizeof(double), 1, file1);
    fwrite(&best_i, sizeof(int), 1, file1);
    double d = split->boundary.value.fl;
    fwrite(&d, sizeof(double), 1, file1);

    //// Write as floats
    fwrite(&num_fallens, sizeof(int), 1, file2);
    fwrite(&num_not_missed, sizeof(int), 1, file2);
    fwrite(idx, sizeof(int), num_not_missed, file2);
    fwrite(feature_vals, sizeof(float), num_not_missed, file2);
    fwrite(responses, sizeof(float), subj->size, file2);
    fwrite(&best_weight, sizeof(double), 1, file2);
    fwrite(&best_i, sizeof(int), 1, file2);
    fwrite(&(split->boundary.value), sizeof(float), 1, file2);

    fclose(file1);
    fclose(file2);

    cart->error.code = CART_UNKNOWN_ERROR;
    cart->error.status = CART_ERROR_STATUS_FATAL;
    return 0;
#endif
    return (float)best_weight;
    __CLEANUP__;
    __END__;
}
#pragma warning(default : 4725)

inline CxClassifierVar* cxCalcNodeStats( CxCARTBase* cart, CxForestNode* node,
					CxClassifierVar* feature_vals,
					int* idx, int num_not_missed,
					int feature , int* fallen_stats_child )
{
    int num_resp = cart->num_response_classes;
    assert(icxIsClassifierVarCategoric(cart->response_type));
    assert(icxIsClassifierVarNumeric(cart->feature_type[feature]));
#ifdef _DEBUG
    int eff_feature = cart->features_corr[feature];
#endif
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );
    //	int* idx = node->fallen_idx + node->num_fallens * eff_feature;
    int num_fallens = node->num_fallens;
    //	int num_not_missed = num_fallens - node->num_missed[eff_feature];

    START_FUNC( cxCalcNodeStats );
    int mem_economy_mode = cart->params->mem_economy_mode;

    CxCARTSubj* subj = node->subj;
    CxCARTAtom* body = subj->body;
    int shift = feature * subj->features_step;
    CxCARTAtom* atom = body + idx[0];
    CxClassifierVar* responses = subj->response;

#ifdef _DEBUG
    int* fallen_stats_child_test = (int*)calloc( num_resp, sizeof(int) );
#endif
    //	int* _fallen_stats_child = fallen_stats_child + num_resp;
    memcpy(fallen_stats_child , node->fallen_stats , num_resp * sizeof(int));
    if (!mem_economy_mode)
    {
#ifdef _DEBUG
	for (int jj = 0 ; jj < num_not_missed ; jj++)
	{
	    atom = body + idx[jj];
	    assert( fabs(feature_vals[jj].fl - (*(CxClassifierVar *)((char*)atom->features + shift)).fl) < EPS);
	    assert( fabs (atom->response.fl - responses[idx[jj]].fl) < EPS);
	    int r = icxVarToInt(responses[idx[jj]]);
	    fallen_stats_child_test[r] ++ ;
	}
#endif
	for (int j = num_not_missed ; j < num_fallens ; j++)
	{
	    int r = icxVarToInt(responses[idx[j]]);
	    fallen_stats_child[r] -- ;
	}
	/// Read idx[j] and responses[idx[j]]
	FUNC_MEM_READ(cxCalcNodeStats , num_fallens - num_not_missed , CxClassifierVar);
	FUNC_MEM_READ(cxCalcNodeStats , num_fallens - num_not_missed , int);
	/// Modify _fallen_stats_child[r]
	FUNC_MEM_MODIFY(cxCalcNodeStats , num_fallens - num_not_missed , int);
    }
    else
    {
	// Fill feature_vals from main matrix
	int j = 0;
	for (j = 0 ; j < num_not_missed ; j++)
	{
	    atom = body + idx[j];
	    feature_vals[j] = *(CxClassifierVar *)((char*)atom->features + shift);
#ifdef _DEBUG
	    int r = icxVarToInt(atom->response);
	    fallen_stats_child_test[r] ++ ;
#endif
	}
	for (j = num_not_missed ; j < num_fallens ; j++)
	{
	    int r = icxVarToInt(body[idx[j]].response);
	    fallen_stats_child[r] -- ;
	}
    }
#ifdef _DEBUG
    for (int ii = 0 ; ii < num_resp ; ii++)
	assert(fallen_stats_child[ii] == fallen_stats_child_test[ii]);
    free(fallen_stats_child_test);
#endif
    EXIT_FUNC( cxCalcNodeStats );
    return feature_vals;
}
#pragma warning(disable : 4725)

CART_IMPL int  cxScanNumericBestSplitGiniIndex( CxRootedCARTBase* cart,
#ifdef _DEBUG
					       CxForestNode* node,
					       int feature,
#endif
					       int* fallen_idx,
					       CxClassifierVar* feature_vals,
					       int num_not_missed,
					       int* fallen_stats_child,
					       double& best_weight)
{

    START_FUNC(cxScanNumericBestSplitGiniIndex);
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned);

    CxClassifierVar* responses = cart->froot->subj->response;

    float* priors_mult = cart->priors_mult;
    float* cost_gini_mult = cart->cost_gini_mult;
    double impurity = 0.0;
    int* _fallen_stats_child = fallen_stats_child + num_resp_aligned;

    float eps_weight = cart->froot->split ? cart->froot->split->weight * EPS : EPS;

    double sum1 = 0.0;
    double sum_sq1 = 0.0;
    double sum2 = 0.0;
    double sum_sq2 = 0.0;

    int best_i = -1;

#ifdef _DEBUG
    CxCARTSubj* subj = cart->froot->subj;
    CxCARTAtom* body = subj->body;
#endif

    if (!cost_gini_mult)
    {
	//// No cost matrix
	for (int i = 0 ; i < num_resp ; i++)
	{
	    double f = _fallen_stats_child[i] * priors_mult[i];
	    sum2 += f;
	    sum_sq2 += f * f;
	}
	impurity = (sum2 < eps_weight) ? 0.0 : sum2 - sum_sq2 / sum2;
    }
    else
    {
	//// Cost matrix
	for (int i = 0 ; i < num_resp ; i++)
	{
	    double f = _fallen_stats_child[i] * priors_mult[i];
	    sum2 += f;
	}
	float* _cost_gini_mult = cost_gini_mult;
	for (int j = 0 ; j < num_resp  ; j++ ,
	    _cost_gini_mult += num_resp_aligned )
	{
	    for (int i = j + 1 ; i < num_resp ; i++)
		sum_sq2 += _cost_gini_mult[i] * _fallen_stats_child[i] * _fallen_stats_child[j];
	}
	impurity = ((sum2 < eps_weight) ? 0.0 : sum_sq2 / sum2);
    }

    // Exit if node impurity is too small
    if (impurity < eps_weight)
    {
	EXIT_FUNC(cxFindNodeNumericBestSplitClassification);
	return -1;
    }

    CxClassifierVar value = feature_vals[0];
    int min_points = MAX(cart->params->split_min_points , 1);
    int upper_limit = num_not_missed - min_points + 1;
    double best_val = sum_sq2 / sum2;
    double init_val = best_val;

    ///// Pass all cases in given node by increment of split variable ////////
    BOOL no_priors = (BOOL)(!cart->params->priors && !cart->cost_gini_mult);
    //	*((int*)0)=0;
    if (no_priors)
    {
	//// Simplest case - no priors, no cost matrix
	int i_sum2init = cvRound(sum2);
	int i_sum2 = i_sum2init;
#ifdef _DEBUG
	int i_sum1 = 0;
	i_sum1 = i_sum1;
#endif
#if INLINE_ASM_OPTIMIZE
#ifdef _DEBUG
	double best_val_new = best_val;
	double sum2_new = sum2;
	double sum_sq2_new = sum_sq2;
#endif

	int compare_to = upper_limit - min_points;
	int response = 0;
	int tmp = 0;

	best_i = upper_limit ;
	int shift =  4 * num_resp;
	__asm
	{
	    mov ecx , upper_limit   // ecx <- upper_limit - i
		mov esi , feature_vals  // esi <- feature vals
		mov edx , fallen_idx           // edx <- fallen_idx

		fld  qword ptr [best_val]
		fild i_sum2init
		fld  qword ptr sum_sq2
		fld  qword ptr sum_sq1	// sum_sq1 | sum_sq2 | i_sum2init | best_val
		fild i_sum2				// i_sum2 | sum_sq1 | sum_sq2 | i_sum2init | best_val
loop2 :
	    mov ebx , [esi]			// ebx <- value_new = value;
loop_value_same :
	    cmp ebx , [esi]
		jne exit_loop_value_same
		add esi , 4             // feature_vals++
		mov edi , responses     // edi <- responses
		mov eax , [edx]
		add edx , 4             // fallen_idx++
		fld dword ptr [edi + 4 * eax]
		fistp [response]        // Load new response
		mov edi , fallen_stats_child
		mov eax , response
		lea edi , [edi + 4 * eax]
		mov eax , [edi]
		inc eax
		mov [edi] , eax
		lea eax , [2 * eax - 1]
		mov tmp , eax
		fild tmp  // 2 * resp0 + 1 | i_sum2 | sum_sq1 | sum_sq2 | i_sum2init | best_val
		mov eax , shift
		lea edi , [edi + eax]
		mov eax , [edi]
		dec eax
		mov [edi] , eax
		lea eax , [2 * eax + 1]
		mov tmp , eax
		fild tmp // 2 * resp1 - 1 | 2 * resp0 + 1 | i_sum2 | sum_sq1 | sum_sq2 | i_sum2init | best_val
		fsubp st(4) , st(0) // 2 * resp0 + 1 | i_sum2 | sum_sq1 | sum_sq2 (u) | i_sum2init | best_val
		faddp st(2) , st(0) // i_sum2 | sum_sq1 (u)   | sum_sq2 (u) | i_sum2init  | best_val
		fld1
		fsubp st(1) , st(0) // i_sum2 - 1.0 | sum_sq1 (u)   | sum_sq2 (u) | i_sum2init  | best_val
		dec ecx
		jz exit_loop2
		jmp loop_value_same
exit_loop_value_same :
	    cmp ecx , dword ptr [compare_to]
		ja  loop2     // i < min_points - repeat main loop
		fld st(2)     // sum_sq2 | i_sum2 | sum_sq1 | sum_sq2 | i_sum2init  | best_val
		fdiv st(0) , st(1)  // sum_sq2 / i_sum2 | i_sum2 | sum_sq1 | sum_sq2 | i_sum2init  | best_val
		fld st(1)           // i_sum2 | sum_sq2 / i_sum2 | i_sum2 | sum_sq1 | sum_sq2 | i_sum2init  | best_val
		fsubr st(0) , st(5) // i_sum2init - i_sum1 | sum_sq2 / i_sum2 | i_sum2 | sum_sq1 | sum_sq2 | i_sum2init  | best_val
		fdivr st(0) , st(3) // sum_sq2 / (i_sum2init - i_sum1) | sum_sq2 / i_sum2 | i_sum2 | sum_sq1 | sum_sq2 | i_sum2init  | best_val
		faddp st(1) , st(0) // cur_val | i_sum2 | sum_sq1 | sum_sq2 | i_sum2init  | best_val
		fcomi st(0) , st(5)

		mov eax , [best_i]
		cmova eax , ecx
		fcmovb st(0), st(5)
		fxch st(5)
		fstp st(0)       // i_sum2 | sum_sq1 | sum_sq2 | i_sum2init  | best_val
		mov [best_i], eax
		jmp loop2
exit_loop2 :
	    mov eax , upper_limit
		sub eax , [best_i]
		dec eax
		mov [best_i] , eax
		fcompp
		fcompp
		fstp qword ptr [best_val]
	}
#ifdef _DEBUG
	int best_i_new = -1;
	i_sum2 = i_sum2init;
	sum2 = sum2_new;
	sum_sq2 = sum_sq2_new;
	sum1 = 0.0;
	sum_sq1 = 0.0;
	value = feature_vals[0];
	memset (fallen_stats_child , 0 , 8 * num_resp);


	cxCalcNodeStats((CxCARTBase*)cart , node ,
	    feature_vals, fallen_idx,
	    num_not_missed, feature ,
	    fallen_stats_child + num_resp);
	for (int i = 0; i < upper_limit; )
	{
	    CxClassifierVar value_new = value;
	    // While split variable doesn't change ...
	    while (icxCompareVarsSimple(value_new , value) & (i < upper_limit))
	    {
		//   Move the case from right to left //////////
		CxClassifierVar resp = responses[fallen_idx[i]];
		/// Read fallen_idx[i] and responses[fallen_idx[i]]
		FUNC_MEM_READ( cxScanNumericBestSplitGiniIndex , 1 , CxClassifierVar);
		FUNC_MEM_READ( cxScanNumericBestSplitGiniIndex , 1 , int);
		int response = icxVarToInt(resp);
		assert( response >= 0 && response < num_resp);
		int* _fstats0 = fallen_stats_child + response;
		int* _fstats1 = _fstats0 + num_resp;
		i_sum2--;
		sum_sq1 += ((*_fstats0 * 2) + 1);
		sum_sq2 -= ((*_fstats1 * 2) - 1);

		FUNC_MEM_READ( cxScanNumericBestSplitGiniIndex , 2 , int); // Read _fstats0 , _fstats1
		(*_fstats0)++;
		(*_fstats1)--;
		FUNC_MEM_MODIFY( cxScanNumericBestSplitGiniIndex , 2 , int);  // *_fstats0++ , *_fstats1++
		value_new = feature_vals[++i];
		FUNC_MEM_READ( cxScanNumericBestSplitGiniIndex , 1 , CxClassifierVar); // Read value_new
	    }
	    value = value_new;
	    /// Calculate split weight - only if not less than min_points goes to each side
	    if ((i >= min_points) & (i < upper_limit))
	    {
		sum1 = (float)(i_sum2init - i_sum2);
		//				assert(i_sum2init == i_sum2 + i_sum1);
		double cur_val = sum_sq1 / sum1 + sum_sq2 / (double)i_sum2 ;

		if (cur_val > best_val_new)
		{
		    best_i_new = i-1;
		    best_val_new = cur_val;
		}
	    }
	}
	assert((best_i_new == best_i) && (fabs ( 1.0 - best_val_new /best_val ) < EPS));
#endif // #ifdef _DEBUG

#else  // not INLINE_ASM_OPTIMIZE
	for (int i = 0; i < upper_limit; )
	{
	    CxClassifierVar value_new = value;
	    // While split variable doesn't change ...
	    while (icxCompareVarsSimple(value_new , value) & (i < upper_limit))
	    {
		//   Move the case from right to left //////////
		CxClassifierVar resp = responses[fallen_idx[i]];
		/// Read fallen_idx[i] and responses[fallen_idx[i]]
		FUNC_MEM_READ( cxScanNumericBestSplitGiniIndex , 1 , CxClassifierVar);
		FUNC_MEM_READ( cxScanNumericBestSplitGiniIndex , 1 , int);
		int response = icxVarToInt(resp);
		assert( fabs( body[fallen_idx[i]].response.fl - responses[fallen_idx[i]].fl ) < EPS);
		assert( response >= 0 && response < num_resp);
		int* _fstats0 = fallen_stats_child + response;
		int* _fstats1 = _fstats0 + num_resp;

#ifdef _DEBUG
		i_sum1++;
#endif
		i_sum2--;
		sum_sq1 += ((*_fstats0 * 2) + 1);
		sum_sq2 -= ((*_fstats1 * 2) - 1);

		FUNC_MEM_READ( cxScanNumericBestSplitGiniIndex , 2 , int); // Read _fstats0 , _fstats1
		(*_fstats0)++;
		(*_fstats1)--;
		FUNC_MEM_MODIFY( cxScanNumericBestSplitGiniIndex , 2 , int);  // *_fstats0++ , *_fstats1++
		value_new = feature_vals[++i];
		FUNC_MEM_READ( cxScanNumericBestSplitGiniIndex , 1 , CxClassifierVar); // Read value_new
	    }
	    value = value_new;
	    /// Calculate split weight - only if not less than min_points goes to each side
	    if ((i >= min_points) & (i < upper_limit))
	    {
		sum1 = (float)(i_sum2init - i_sum2);
		assert(i_sum2init == i_sum2 + i_sum1);
		double cur_val = sum_sq1 / sum1 + sum_sq2 / (float)i_sum2 ;

		if (cur_val > best_val)
		{
		    best_i = i-1;
		    best_val = cur_val;
		}
	    }
	}
#endif
	best_weight = best_val - init_val;
	}
	else if (!cart->cost_gini_mult)
	{
	    //// Using priors
	    float* priors_mult = cart->priors_mult;
	    for (int i = 0; i < upper_limit; )
	    {
		CxClassifierVar value_new = value;
		// While split variable doesn't change ...
		while (icxCompareVarsSimple(value_new , value) & (i < upper_limit))
		{
		    //   Move the case from right to left //////////
		    CxClassifierVar resp = responses[fallen_idx[i]];
		    int response = icxVarToInt(resp);
		    assert( response >= 0 && response < num_resp);
		    assert( fabs( body[fallen_idx[i]].response.fl - responses[fallen_idx[i]].fl ) < EPS);

		    int* _fstats0 = fallen_stats_child + response;
		    int* _fstats1 = _fstats0 + num_resp_aligned;

		    double mult = priors_mult[response];
		    sum1 += mult;
		    sum2 -= mult;
		    double mult2 = mult * mult;
		    sum_sq1 += mult2 * ( (*_fstats0 * 2) + 1);
		    sum_sq2 -= mult2 * ( (*_fstats1 * 2) - 1);

		    (*_fstats0)++;
		    (*_fstats1)--;
		    value_new = feature_vals[++i];
		}
		value = value_new;
		/// Calculate split weight - only if not less than min_points goes to each side
		if ((i >= min_points) & (i < upper_limit))
		{
		    double cur_val = sum_sq1 / sum1 + sum_sq2 / sum2 ;
		    if (cur_val > best_val)
		    {
			best_i = i-1;
			best_val = cur_val;
		    }
		}
	    }
	    best_weight = best_val - init_val;
	}
	else
	{
	    //// Using cost matrix
	    float* priors_mult = cart->priors_mult;
	    float* cost_gini_mult = cart->cost_gini_mult;
	    int* fallen_stats_child1 = fallen_stats_child + num_resp_aligned;

	    for (int i = 0; i < upper_limit; )
	    {
		CxClassifierVar value_new = value;
		// While split variable doesn't change ...
		while (icxCompareVarsSimple(value_new , value) & (i < upper_limit))
		{
		    //   Move the case from right to left //////////
		    CxClassifierVar resp = responses[fallen_idx[i]];
		    int response = icxVarToInt(resp);
		    assert( response >= 0 && response < num_resp);
		    assert( fabs( body[fallen_idx[i]].response.fl - responses[fallen_idx[i]].fl ) < EPS);

		    double mult = priors_mult[response];
		    sum1 += mult;
		    sum2 -= mult;
		    float* _cost_gini_mult = cost_gini_mult + num_resp_aligned * response;
		    for (int j = 0 ; j < num_resp ; j++)
		    {
			double mult2 = _cost_gini_mult[j];
			sum_sq1 += mult2 * fallen_stats_child[j];
			sum_sq2 -= mult2 * fallen_stats_child1[j];
		    }

		    fallen_stats_child[response]++;
		    fallen_stats_child1[response]--;
		    value_new = feature_vals[++i];
		}
		value = value_new;
		/// Calculate split weight - only if not less than min_points goes to each side
		if ((i >= min_points) & (i < upper_limit))
		{
		    double cur_val = sum_sq1 / sum1 + sum_sq2 / sum2 ;
#ifdef _DEBUG
		    double imp1 = icxCalcGiniIndexWithCosts(fallen_stats_child ,
			num_resp_aligned , cost_gini_mult , priors_mult);
		    double imp2 = icxCalcGiniIndexWithCosts(fallen_stats_child1 ,
			num_resp_aligned , cost_gini_mult , priors_mult);
		    assert( fabs(cur_val - imp1 - imp2) < EPS);
#endif
		    //// Here we search for minimum!
		    if (cur_val < best_val)
		    {
			best_i = i-1;
			best_val = cur_val;
		    }
		}
	    }
	    best_weight = init_val - best_val;
	}
	EXIT_FUNC(cxScanNumericBestSplitGiniIndex);
	return best_i;
}

CART_IMPL int  cxScanNumericBestSplitEntropy(CxRootedCARTBase* cart,
					     int* fallen_idx,
					     CxClassifierVar* feature_vals,
					     int num_not_missed,
					     int* fallen_stats_child,
					     double& best_weight)
{

    START_FUNC(cxScanNumericBestSplitEntropy);
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned);

    CxClassifierVar* responses = cart->froot->subj->response;

    float* priors_mult = cart->priors_mult;
    double impurity = 0.0;
    int* _fallen_stats_child = fallen_stats_child + num_resp_aligned;

    float* log_stats = (float*)calloc( 2 * num_resp_aligned , sizeof(float) );
    float* _log_stats = log_stats + num_resp_aligned;

    float eps_weight = cart->froot->split ? cart->froot->split->weight * EPS : EPS;

    double sum1 = 0.0;
    double sum_log1 = 0.0;
    double sum2 = 0.0;
    double sum_log2 = 0.0;

    int best_i = -1;

#ifdef _DEBUG
    CxCARTSubj* subj = cart->froot->subj;
    CxCARTAtom* body = subj->body;
#endif
    //	memset(log_stats, 0, num_resp_aligned * sizeof(float));
    for (int i = 0 ; i < num_resp ; i++)
    {
	int stat = _fallen_stats_child[i];
	if (stat)
	{
	    double f = stat * priors_mult[i];
	    sum2 += f;
	    double f_log = f * log(f);
	    _log_stats[i] = (float)f_log;
	    sum_log2 += f_log;
	}
    }
    impurity = ( (sum2 > 0) ? sum2 * log(sum2) : 0 ) - sum_log2 ;


    // Exit if node impurity is too small
    if (impurity < eps_weight)
    {
	EXIT_FUNC(cxFindNodeNumericBestSplitClassification);
	free(log_stats);
	return -1;
    }

    CxClassifierVar value = feature_vals[0];
    int min_points = MAX(cart->params->split_min_points , 1);
    int upper_limit = num_not_missed - min_points + 1;
    double best_val = impurity;
    double init_val = best_val;

    ///// Pass all cases in given node by increment of split variable ////////
    BOOL no_priors = (BOOL)(!cart->params->priors && !cart->cost_gini_mult);
    //	*((int*)0)=0;
    if (no_priors)
    {
	//// Simplest case - no priors, no cost matrix
	int i_sum2init = cvRound(sum2);
	int i_sum2 = i_sum2init;

	for (int i = 0; i < upper_limit; )
	{
	    CxClassifierVar value_new = value;
	    // While split variable doesn't change ...
	    while (icxCompareVarsSimple(value_new , value) & (i < upper_limit))
	    {
		//   Move the case from right to left //////////
		CxClassifierVar resp = responses[fallen_idx[i]];
		int response = icxVarToInt(resp);
		assert( fabs( body[fallen_idx[i]].response.fl - responses[fallen_idx[i]].fl ) < EPS);
		assert( response >= 0 && response < num_resp);
		int* _fstats1 = fallen_stats_child + response;
		int* _fstats2 = _fstats1 + num_resp;
		float* _flogstats1 = log_stats + response;
		float* _flogstats2 = _flogstats1 + num_resp;
		int stat1 = ++(*_fstats1);
		int stat2 = --(*_fstats2);
		i_sum2--;
		double flogstat1 = stat1 ? stat1 * log(double(stat1)) : 0;
		double flogstat2 = stat2 ? stat2 * log(double(stat2)) : 0;
		sum_log1 -= (*_flogstats1 - flogstat1);
		sum_log2 -= (*_flogstats2 - flogstat2);
		*_flogstats1 = (float)flogstat1;
		*_flogstats2 = (float)flogstat2;
		value_new = feature_vals[++i];
	    }
	    value = value_new;
	    /// Calculate split weight - only if not less than min_points goes to each side
	    if ((i >= min_points) & (i < upper_limit))
	    {
		sum1 = (float)(i_sum2init - i_sum2);
		double cur_val = ((sum1 > 0) ? sum1 * log(double(sum1)) : 0) +
		    (i_sum2 ? i_sum2 * log(double(i_sum2)) : 0) - sum_log1 - sum_log2;
#ifdef _DEBUG
		double entropy1 = icxCalcEntropy(fallen_stats_child, num_resp);
		double entropy2 = icxCalcEntropy(fallen_stats_child + num_resp_aligned, num_resp);
#endif
		assert(entropy1 == 0 || fabs(sum1 * log(float(sum1)) - sum_log1 - entropy1)/entropy1 < 1e-3);
		assert(entropy2 == 0 || fabs(i_sum2 * log(float(i_sum2)) - sum_log2 - entropy2)/entropy2 < 1e-3);

		if (cur_val < best_val)
		{
		    best_i = i-1;
		    best_val = cur_val;
		}
	    }
	}
	best_weight = init_val - best_val ;
    }
    else
    {
	//// Using priors
	float* priors_mult = cart->priors_mult;
	for (int i = 0; i < upper_limit; )
	{
	    CxClassifierVar value_new = value;
	    // While split variable doesn't change ...
	    while (icxCompareVarsSimple(value_new , value) & (i < upper_limit))
	    {
		//   Move the case from right to left //////////
		CxClassifierVar resp = responses[fallen_idx[i]];
		int response = icxVarToInt(resp);
		assert( response >= 0 && response < num_resp);
		assert( fabs( body[fallen_idx[i]].response.fl - responses[fallen_idx[i]].fl ) < EPS);

		int* _fstats1 = fallen_stats_child + response;
		int* _fstats2 = _fstats1 + num_resp;
		float* _flogstats1 = log_stats + response;
		float* _flogstats2 = _flogstats1 + num_resp;
		double mult = priors_mult[response];
		double stat1 = (++(*_fstats1)) * mult;
		double stat2 = (--(*_fstats2)) * mult;
		sum1 += mult;
		sum2 -= mult;
		double flogstat1 = (stat1 > 0 ) ? stat1 * log(stat1) : 0;
		double flogstat2 = (stat2 > 0 ) ? stat2 * log(stat2) : 0;
		sum_log1 -= (*_flogstats1 - flogstat1);
		sum_log2 -= (*_flogstats2 - flogstat2);
#ifdef _DEBUG
		double entropy1 = icxCalcEntropyPriors(fallen_stats_child, num_resp, priors_mult);
		double entropy2 = icxCalcEntropyPriors(fallen_stats_child + num_resp_aligned, num_resp, priors_mult);
#endif
		assert(entropy1 == 0 || fabs(sum1 * log(sum1) - sum_log1 - entropy1)/entropy1 < 1e-3);
		assert(entropy2 == 0 || fabs(sum2 * log(sum2) - sum_log2 - entropy2)/entropy2 < 1e-3);

		*_flogstats1 = (float)flogstat1;
		*_flogstats2 = (float)flogstat2;
		value_new = feature_vals[++i];
	    }
	    value = value_new;
	    /// Calculate split weight - only if not less than min_points goes to each side
	    if ((i >= min_points) & (i < upper_limit))
	    {
		double cur_val = ((sum1 > 0) ? sum1 * log(sum1) : 0) +
		    ((sum2 > 0) ? sum2 * log(sum2) : 0) - sum_log1 - sum_log2;
		if (cur_val < best_val)
		{
		    best_i = i-1;
		    best_val = cur_val;
		}
	    }
	}
	best_weight = init_val - best_val ;
    }
    EXIT_FUNC(cxScanNumericBestSplitEntropy);
    free(log_stats);
    return best_i;
}

CART_IMPL float cxFindNodeNumericBestSplitClassification( CxCART* cart,
							 CxCARTNode* node,
							 int feature,
							 CxCARTSplit* split)
{
    CV_FUNCNAME( "cxFindNodeNumericBestSplitClassification" );
    __BEGIN__;
    assert(icxIsClassifierVarCategoric(cart->response_type));
    assert(icxIsClassifierVarNumeric(cart->feature_type[feature]));
    split->feature_idx = feature;
    float eps_weight = cart->root->split ? cart->root->split->weight * EPS : EPS;
    double best_weight = eps_weight;
    int eff_feature = cart->features_corr[feature];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features );
    int* idx = node->fallen_idx + node->num_fallens * eff_feature;
    split->boundary.value.i = 0;
    int num_fallens = node->num_fallens;

    BOOL floating = icxIsClassifierVar32f(cart->feature_type[feature]);
    assert((long)cart->params->splitting_rule <= CxCARTEntropyCriterion);

    // Exit if unsuficcient number of cases in node to split it.
    int num_not_missed = num_fallens - node->num_missed[eff_feature];
    int min_points = MAX(cart->params->split_min_points , 1);
    if ( num_not_missed < 2 * min_points )
    {
	return 0.f;
    }

    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned);

    /* Calculate initial node response distribution. Set left node to be empty,
    right node equal to main (current) node, Also calculate node impurity.
    It is useful to store them in special arrays to reduce random memory access */

    int* fallen_stats_child = (int*)calloc( 2 * num_resp, sizeof(int) );
    int* _fallen_stats_child = fallen_stats_child + num_resp_aligned;
    /* We allocate feature_vals in memory economy mode , then we fill it from
    main data matrix , otherwise we just get values that are stored in
    node->cat_proportions */
    int mem_economy_mode = cart->params->mem_economy_mode;
    CxClassifierVar* feature_vals = (mem_economy_mode ?
	icxAllocFeatureVals(cart) :	(CxClassifierVar*)node->cat_proportions[eff_feature]);

    cxCalcNodeStats((CxCARTBase*)cart , (CxForestNode*)node ,
	feature_vals, idx, num_not_missed,
	feature, _fallen_stats_child );
    if (!feature_vals)
    {
	free(fallen_stats_child);
	return 0.0;
    }
    int best_i = (cart->params->splitting_rule == CxCARTGiniCriterion) ?
	cxScanNumericBestSplitGiniIndex((CxRootedCARTBase*)cart,
#ifdef _DEBUG
	(CxForestNode*)node, feature,
#endif
	idx, feature_vals, num_not_missed,
	fallen_stats_child, best_weight) :
    cxScanNumericBestSplitEntropy((CxRootedCARTBase*)cart,
	idx, feature_vals, num_not_missed,
	fallen_stats_child, best_weight) ;

    if ((best_weight < eps_weight) || (best_i < 0))
    {
	free(fallen_stats_child);
	return 0.0;
    }

    // Adjust split value to be average of closest split variable values ////////
    int next_to_best = MIN( num_not_missed - 1 , best_i+1);
    CxClassifierVar val1 = feature_vals[best_i];
    CxClassifierVar val2 = feature_vals[next_to_best];
    split->boundary = icxGetHalfwayBetween(val1, val2, floating);

#if DATA_TO_FILE2
    FILE* file1 = fopen("d:\\CART\\AlamereTest\\num_split_classd.dat", "wb");
    fwrite(&cart->root->num_fallens, sizeof(int), 1, file1);
    fwrite(&num_fallens, sizeof(int), 1, file1);
    fwrite(&num_not_missed, sizeof(int), 1, file1);
    fwrite(&num_resp, sizeof(int), 1, file1);

    fwrite(idx, sizeof(int), num_not_missed, file1);
    for (int i = 0 ; i < num_not_missed ; i++)
    {
	double d = feature_vals[i].fl;
	fwrite(&d, sizeof(double), 1, file1);
    }
    for (int i1 = 0 ; i1 < node->subj->size ; i1++)
    {
	int n = (int)(node->subj->response[i1].fl);
	fwrite(&n, sizeof(int), 1, file1);
    }
    fwrite(node->fallen_stats, sizeof(int), num_resp, file1);
    //// Results
    fwrite(&best_weight, sizeof(double), 1, file1);
    fwrite(&best_i, sizeof(int), 1, file1);
    double d = split->boundary.value.fl;
    fwrite(&d, sizeof(double), 1, file1);
    fclose(file1);

    cart->error.code = CART_UNKNOWN_ERROR;
    cart->error.status = CART_ERROR_STATUS_FATAL;
    return 0;
#endif


    split->weight = (float)best_weight;
    free(fallen_stats_child);
    return (float)best_weight;
    __CLEANUP__;
    __END__;
}
#pragma warning(default : 4725)

CART_IMPL float cxScanNodeCategoricBestSplitRegression( CxRootedCARTBase* cart,
						       int num_not_missed,
						       int* cat_proportion,
						       double* sum_proportions,
						       double sum_sq_proportions,
						       char* ptr, int n)
{
    START_FUNC(cxScanNodeCategoricBestSplitRegression);

#ifdef _DEBUG
    char* best_seq = (char*)calloc(n, sizeof(char));
#endif

    float eps_weight = cart->froot->split ? cart->froot->split->weight * EPS : EPS;
    assert(ptr);
    memset(ptr, 0 , sizeof(char) * n);

    /* For two-class problem or regression sort categories to sort_arr :
    for regression - by average response , for classification - by number of
    responses equal to 1*/
    int k;
    float* sort_index = (float*)calloc(n, sizeof(float));
    int* sort_arr = (int*)malloc(sizeof(int) * n);
    for (k = 0 ; k < n ; k++)
	sort_arr[k] = k;
    for (k = 0 ; k < n; k++ )
	sort_index[k] = (cat_proportion[k] == 0) ? 0 : (float)( sum_proportions[k] / cat_proportion[k] );
    sort_float( sort_arr, n, sort_index );
    free(sort_index);

    /////// Calculate node impurity and find missing categories ////////
    char* missed_cats = (char*)malloc(n * sizeof(char));
    double sum_sides[2] = {0.0 , 0.0};

    double sum = 0.0;

    for (k = 0 ; k < n ; k++)
    {
	int num = cat_proportion[k];
	assert((num >= 0) && (sum_sq_proportions >= 0));
	sum += sum_proportions[k];
	missed_cats[k] = (!num);
    }
    sum_sides[1] = sum;
    double best_val = sum * sum / num_not_missed;
    double impurity = (num_not_missed == 0) ? 0.0 : (sum_sq_proportions - best_val ) / cart->froot->num_fallens;
    int min_points = MAX(cart->params->split_min_points,1);
    float best_weight = eps_weight;

    if (impurity > eps_weight)
    {
    /* Get the path to traverse all binary sequences with last 0, this is
    a sequence of numbers, each number means which cluster to move to
    the other split side (0->1 , 1->0 ). Simple path of length n is
	used in 2-class or regression case.*/
	int path_len = n;
	int* path = (int*)malloc(n * sizeof(int));
	int i;
	for (i = 0; i < n ; i++)
	    path[i] = sort_arr[n-i-1];

	int best_step = -1;
	double init_val = best_val;

#ifdef _DEBUG
	//		ofstream file("seqs.log", ios::app);
#endif
	int num[2];
	num[0] = 0;
	num[1] = num_not_missed;
	for (int step = 0 ; step < path_len - 1 ; step++)
	{
	    /// Get the category which is shifted to the other side
	    int pos = path[step];
	    ptr[pos] ^= 1;
	    int side = ptr[pos];
	    int other_side = 1 - side;
	    /// Recalculate the number of samples going to each side.
	    num[side] -= cat_proportion[pos];
	    num[other_side] += cat_proportion[pos];
	    assert(num[0] >= 0 && num[1] >= 0);

	    /// Update responses sums.
	    sum_sides[side] -= sum_proportions[pos];
	    sum_sides[other_side] += sum_proportions[pos];

	    FUNC_MEM_READ(cxFindNodeCategoricBestSplitRegression , 7 , int);
	    FUNC_MEM_WRITE(cxFindNodeCategoricBestSplitRegression , 4 , int);
#ifdef _DEBUG
	    double sum1[2] = {0.0,0.0};
	    int num1[2] = {0,0};
	    /* Calculate number of cases going to each side , sum of their responses and
	    sum of squares of their responses  */
	    for (i = 0 ; i < n ; i++)
	    {
		int side = 1 - (int)ptr[i];
		num1[side] += cat_proportion[i];
		sum1[side] += sum_proportions[i];
	    }
	    for (side = 0 ; side < 2 ; side ++)
	    {
		assert( num1[side] == num[side]  );
		assert( fabs(sum1[side] - sum_sides[side] ) < EPS  );
	    }

#endif
	    //				assert((sum_sides[0] >= 0) && (sum_sides[1] >= 0));
	    /// Calculate child node's impurity
	    BOOL good_split = ( num[0] >= min_points && num[1] >= min_points);
	    if (good_split)
	    {
		double cur_val = sum_sides[0] * sum_sides[0] / num[0] + sum_sides[1] * sum_sides[1] / num[1]  ;
		FUNC_MEM_READ(cxFindNodeCategoricBestSplitRegression , 6 , int);

#ifdef _DEBUG
		//		assert ( (1.0 - (impurity_left + impurity_right)/impurity) > -EPS);
		/////////////////////////////
		//	float weight1 = cxCalcImpurityChangeCategoric(cart,node,split);
		//	assert(fabs (weight - weight1) < EPS);
		/////////////////////////////
#endif
		if (cur_val > best_val)
		{
#ifdef _DEBUG
		    memcpy(best_seq , ptr , n * sizeof(char));
#endif
		    best_val = cur_val;
		    best_step = step;
		    //			*((int*)0)=0;

		}
	    }
	}
	best_weight = (float)((best_val - init_val) / cart->froot->num_fallens);
	memset (ptr , 0 , n * sizeof(char) );
	for (k = 0 ; k < best_step + 1; k++)
	    ptr[path[k]] = 1;
	free(path);
#ifdef _DEBUG
	for (k = 0 ; k < n ; k++)
	{
	    assert(best_seq[k] == ptr[k]);
	}
#endif
	/* Finally set node split to best split , taking missing categories into
	account */
	for (k = 0 ; k < n; k++ )
	    ptr[k] = (char)(missed_cats[k] ? 2 : ptr[k]);
    }

#ifdef _DEBUG
    /*	if (impurity > EPS)
    {
    float f1 = cxCalcImpurityChangeCategoric(cart,node,split);
    float f2 = cxCalcImpurityChangeAny(cart,node,split);
    if ((f2 != 0) && fabs(1.0 - f1/f2) > EPS )
    {
    assert(0);
    }
	}*/
#endif
	free(missed_cats);
	free(sort_arr);
#ifdef _DEBUG
	free( best_seq );
#endif
	EXIT_FUNC(cxFindNodeCategoricBestSplitRegression);
	return best_weight;
}

CART_IMPL float cxFindNodeCategoricBestSplitRegression( CxCART* cart,
						       CxCARTNode* node,
						       int feature,
						       CxCARTSplit* split)
{
    CV_FUNCNAME( "cxFindNodeCategoricBestSplitRegression" );
    __BEGIN__;

    int n = cart->num_classes[feature];
    assert( n >= 0 && n < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);
    if (n < 2)
    {
	return 0.0;
    }
    int eff_feature = cart->features_corr[feature];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features);
    split->feature_idx = feature;
    char* ptr = (char*)split->boundary.ptr;
    assert(icxIsClassifierVarNumeric(cart->response_type));
    assert(icxIsClassifierVarCategoric(cart->feature_type[feature]));

    //	BOOL floating = icxIsClassifierVar32f( cart->response_type );
    int* fallen_idx = node->fallen_idx + node->num_fallens * eff_feature;

    /// Exit if we cannot do any split
    int num_not_missed = node->num_fallens - node->num_missed[eff_feature];
    int min_points = MAX(cart->params->split_min_points,1);
    if ( num_not_missed < 2 * min_points )
    {
	return 0.0;
    }
    int* cat_proportions = node->cat_proportions[eff_feature];
    /* Calculate number of cases going to each side , sum of their responses and
    sum of squares of their responses for each category */
    double sum_sq_proportions = 0;
    double* sum_proportions = (double*)calloc(n, sizeof(double));
    int k = 0 ;
    CxCARTSubj* subj = node->subj;
    CxClassifierVar* responses = subj->response;
#ifdef _DEBUG
    CxCARTAtom* body = subj->body;
#endif
    int* _idx;
    for (_idx = fallen_idx ; k < n; k++)
    {
	int num_var_class = cat_proportions[k];
	double sum = 0.0;
	for (int j = 0 ; j < num_var_class ; j++, _idx++)
	{
	    int str_num = (*_idx);
	    assert(str_num >= 0);
	    float response = responses[str_num].fl;
#ifdef _DEBUG
	    CxClassifierVar var = icxGetSubjFeature(subj , str_num , feature);
	    assert(fabs(var.fl - k) < EPS);
#endif
	    assert(fabs(response - body[str_num].response.fl) < EPS);
	    sum += response;
	    sum_sq_proportions += response * response;
	}
	sum_proportions[k] = sum;
    }
    //// Read _idx and responses[str_num]
    FUNC_MEM_READ (cxFindNodeCategoricBestSplitRegression , num_not_missed , int);
    FUNC_MEM_READ (cxFindNodeCategoricBestSplitRegression , num_not_missed , CxClassifierVar);

    assert( (fallen_idx + num_not_missed) == _idx);
#ifdef _DEBUG
    int* np = (int*) calloc(n, sizeof(int));
    for (int ii = 0 ; ii < num_not_missed; ii++)
    {
	CxClassifierVar var = icxGetSubjFeature(subj , fallen_idx[ii] , feature);
	np[ cvRound(var.fl) ] ++;
    }
    assert(memcmp(np , cat_proportions , n * sizeof(int)) == 0);
    free(np);
#endif

    float best_weight = cxScanNodeCategoricBestSplitRegression((CxRootedCARTBase*)cart,
	num_not_missed, cat_proportions, sum_proportions,
	sum_sq_proportions, ptr, n);

    free(sum_proportions);
    split->weight = best_weight;
    return best_weight;
    __CLEANUP__;
    __END__;
}

CART_IMPL float cxScanNodeCategoricBestSplitClassification( CxRootedCARTBase* cart,
							   int* fallen_stats_proportions,
							   char* ptr, int n)
{
    assert(ptr);
    memset(ptr, 0 , sizeof(char) * n);

    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned);

    START_FUNC(cxScanNodeCategoricBestSplitClassification);
#ifdef _DEBUG
    char* best_seq = (char*)calloc(n, sizeof(char));
#endif

    // Calculate response frequencies for each category
    int* stats = (int*)icxAlignedCalloc(2 * num_resp_aligned, sizeof(int));
    int* cat_proportion = (int*) calloc(n , sizeof(int));

    int* _fsp = fallen_stats_proportions;
    for (int j = 0 ; j < n ; j++)
    {
	int sum = 0;
	for (int resp = num_resp ; resp-- ; _fsp++)
	    sum += *_fsp;
	cat_proportion[j] = sum;
    }
    int num_not_missed = 0;
    for (int i1 = 0; i1 < n ; i1++)
	num_not_missed += cat_proportion[i1];

	/* For two-class problem or regression sort categories to sort_arr :
	for regression - by average response , for classification - by number of
    responses equal to 1*/
    int* sort_arr = NULL;
    float* priors = cart->params->priors;

    if (cart->num_response_classes == 2)
    {
	float* sort_index = (float*)calloc(n, sizeof(float));
	sort_arr = (int*)malloc(sizeof(int) * n);
	for (int i = 0 ; i < n ; i++)
	    sort_arr[i] = i;

	float prior0 = priors ? priors[0] : 1.0f;
	float prior1 = priors ? priors[1] : 1.0f;
	for (int k = 0 ; k < n; k++ )
	{
	    int* _fsp = fallen_stats_proportions + k * num_resp_aligned;
	    double sum = prior0 * _fsp[0] + prior1 * _fsp[1] ;
	    sort_index[k] = (float)( ( fabs(sum) < EPS) ? 0.0f : _fsp[0] / sum);
	}
	sort_float( sort_arr, n, sort_index );
	free(sort_index);
    }
    float* cost_gini_mult = cart->cost_gini_mult;

    double impurity = 0.0;
    /////// Calculate node impurity and find missing categories ////////
    char* missed_cats = (char*)malloc(n * sizeof(char));

    int* stats1 = stats + num_resp_aligned;
    memset( stats , 0 , sizeof(int) *  num_resp_aligned * 2);
    int* _fallen_stats_proportions = fallen_stats_proportions;
    int k = 0;
    for (k = 0 ; k < n ; k++)
    {
	int* _stats = stats1;
	char cat_missed = 1;
	for (int response = 0 ; response < num_resp_aligned ; response ++, _fallen_stats_proportions++, _stats++)
	{
	    int stat = *_fallen_stats_proportions;//[ i * num_resp + response];
	    (*_stats) += stat;
	    cat_missed &= (!stat);
	}
	missed_cats[k] = cat_missed;
    }
    //// Read *_fallen_stats_proportions
    FUNC_MEM_READ (cxScanNodeCategoricBestSplitClassification , n * num_resp_aligned , int);
    //// Modify *_stats
    FUNC_MEM_MODIFY(cxScanNodeCategoricBestSplitClassification , n * num_resp_aligned , int);
    /*	if (node->num_fallens == 193 && feature == 4)
    {
    int mmm = 1;
    mmm = mmm;
}*/
    BOOL use_gini = (cart->params->splitting_rule == CxCARTGiniCriterion);
    if (use_gini)
    {
	if (!cost_gini_mult)
	{
	    if (priors)
		impurity = icxCalcGiniIndexPriors(stats1 , num_resp, cart->priors_mult);
	    else
		impurity = icxCalcGiniIndex(stats1 , num_resp);
	}
	else
	{
	    impurity = icxCalcGiniIndexWithCosts(stats1 , num_resp_aligned,
		cost_gini_mult ,  cart->priors_mult);
	}
    }
    else
    {
	if (priors)
	    impurity = icxCalcEntropyPriors(stats1 , num_resp, cart->priors_mult);
	else
	    impurity = icxCalcEntropy(stats1 , num_resp);
    }

    //// Read cart->priors_mult and stats1
    FUNC_MEM_READ (cxScanNodeCategoricBestSplitClassification , 2*num_resp , int);

    float eps_weight = cart->froot->split ? cart->froot->split->weight * EPS : EPS;
    int min_points = MAX(cart->params->split_min_points,1);
    float best_weight = 0;

    if (impurity > eps_weight)
    {
	int path_len = 0;
	int* path = NULL;
	BOOL use_priors = (cart->params->priors != NULL);

	/* Get the path to traverse all binary sequences with last 0, this is
	a sequence of numbers, each number means which cluster to move to
	the other split side (0->1 , 1->0 ). Grey codes is used for 'normal'
	case, and simple path of length n is used in 2-class case.*/

	if (cart->num_response_classes == 2)
	{
	    path_len = n;
	    path = (int*)malloc(n * sizeof(int));
	    for (int i = 0; i < n ; i++)
		path[i] = sort_arr[n-i-1];
	}
	else
	{
	    // Grey codes used.
	    path = icxBuildPath(n-1 , path_len);
	}

	int best_step = -1;
	double best_val = -impurity;
	double init_val = best_val;


#ifdef _DEBUG
	//		ofstream file("seqs.log", ios::app);
#endif
	int num[2];
	num[0] = 0;
	num[1] = num_not_missed;
	for (int step = 0 ; step < path_len - 1 ; step++)
	{
	    /// Get the category which is shifted to the other side
	    int pos = path[step];
	    ptr[pos] ^= 1;
	    int side = ptr[pos];
	    int other_side = 1 - side;
	    /// Recalculate the number of samples going to each side.
	    num[side] -= cat_proportion[pos];
	    num[other_side] += cat_proportion[pos];
	    assert(num[0] >= 0 && num[1] >= 0);

	    int* _fallen_stats_proportions = fallen_stats_proportions + pos * num_resp_aligned;
	    //// Recalculate frequency tables
	    icxAddVector(stats + other_side * num_resp_aligned ,
		_fallen_stats_proportions, num_resp);
	    icxSubVector(stats + side * num_resp_aligned ,
		_fallen_stats_proportions, num_resp);
	    //// Read 4 response vectors
	    FUNC_MEM_READ (cxScanNodeCategoricBestSplitClassification , 4 * num_resp_aligned , int);
	    //// Write 2 response vectors
	    FUNC_MEM_WRITE (cxScanNodeCategoricBestSplitClassification , 2 * num_resp_aligned , int);

#ifdef _DEBUG
	    _fallen_stats_proportions = fallen_stats_proportions;
	    /* Create response distribution vector stats by summarizing frequencies
	    according to current ptr value ( that denotes which category goes to which side  */
	    int* stats1 = (int*)icxAlignedCalloc(num_resp_aligned * 2, sizeof(int));
	    int i;
	    for (i = 0 ; i < n ; i++)
	    {
		side = 1 - (int)ptr[i];
		int* _stats = stats1 + side * num_resp_aligned;
		icxAddVector(_stats , _fallen_stats_proportions , num_resp);
		_fallen_stats_proportions += num_resp_aligned;
	    }
	    int num1[2] = {0,0};
	    for (side = 0 ; side < 2 ; side ++)
	    {
		int* _stats = stats1 + side * num_resp_aligned;
		num1[side] = icxSumVector(_stats, num_resp);
	    }
	    for (side = 0 ; side < 2 ; side ++)
	    {
		assert( num1[side] == num[side]  );
		int* _stats1 = stats1 + side * num_resp_aligned;
		int* _stats = stats + side * num_resp_aligned;
		for (i = 0 ; i < num_resp_aligned ; i++)
		{
		    assert( _stats1[i] == _stats[i]);
		}
	    }
	    icxFreeAligned(stats1);
#endif
	    /// Calculate Gini Index for child nodes
	    BOOL good_split = ( num[0] >= min_points && num[1] >= min_points);
	    if (good_split)
	    {
		double cur_val = init_val;
		if (use_gini)
		{
		    if (use_priors)
		    {
			double impurity_left = icxCalcGiniIndexPriors(stats , num_resp, cart->priors_mult);
			double impurity_right = icxCalcGiniIndexPriors(stats + num_resp_aligned, num_resp, cart->priors_mult);
			cur_val = -impurity_left - impurity_right;
		    }
		    else if (!cost_gini_mult)
		    {
			double impurity_left = icxCalcGiniIndex(stats , num_resp);
			double impurity_right = icxCalcGiniIndex(stats + num_resp_aligned, num_resp);
			cur_val = -impurity_left - impurity_right;
		    }
		    else
		    {
			double impurity_left = icxCalcGiniIndexWithCosts(stats ,
			    num_resp_aligned, cost_gini_mult ,
			    cart->priors_mult);
			double impurity_right = icxCalcGiniIndexWithCosts(stats + num_resp_aligned,
			    num_resp_aligned, cost_gini_mult , cart->priors_mult);
			cur_val = -impurity_left - impurity_right;
		    }
		}
		else
		{
		    if (use_priors)
		    {
			double impurity_left = icxCalcEntropyPriors(stats , num_resp, cart->priors_mult);
			double impurity_right = icxCalcEntropyPriors(stats + num_resp_aligned, num_resp, cart->priors_mult);
			cur_val = -impurity_left - impurity_right;
		    }
		    else
		    {
			double impurity_left = icxCalcEntropy(stats , num_resp);
			double impurity_right = icxCalcEntropy(stats + num_resp_aligned, num_resp);
			cur_val = -impurity_left - impurity_right;
		    }
		}

		//// Read stats , cart->priors_mult 2 times
		FUNC_MEM_READ (cxScanNodeCategoricBestSplitClassification , 4 * num_resp , int);

#ifdef _DEBUG
		//		assert ( (1.0 - (impurity_left + impurity_right)/impurity) > -EPS);
		/////////////////////////////
		//	float weight1 = cxCalcImpurityChangeCategoric(cart,node,split);
		//	assert(fabs (weight - weight1) < EPS);
		/////////////////////////////
#endif
		if (cur_val > best_val)
		{
#ifdef _DEBUG
		    memcpy(best_seq , ptr , n * sizeof(char));
#endif
		    best_val = cur_val;
		    best_step = step;
		}
	    }
		}
		best_weight = (float)(best_val - init_val);

		if (cart->num_response_classes == 2)
		{
		    memset (ptr , 0 , n * sizeof(char) );
		    for (int p = 0 ; p < best_step + 1; p++)
			ptr[path[p]] = 1;
		}
		else
		{
		    /// Restore binary sequence by its number.
		    icxGetSeq(ptr , n - 1, best_step);
		}
		free(path);
#ifdef _DEBUG
		for (int p = 0 ; p < n ; p++)
		{
		    assert(best_seq[p] == ptr[p]);
		}
#endif
		/* Finally set node split to best split , taking missing categories into
		account */
		for (k = 0 ; k < n; k++ )
		    ptr[k] = (char)(missed_cats[k] ? 2 : ptr[k]);
	}

#ifdef _DEBUG
	/*	if (impurity > EPS)
	{
	float f1 = cxCalcImpurityChangeCategoric(cart,node,split);
	float f2 = cxCalcImpurityChangeAny(cart,node,split);
	if ((f2 != 0) && fabs(1.0 - f1/f2) > EPS )
	{
	assert(0);
	}
	}*/
#endif
	free(cat_proportion);
	free(missed_cats);
	icxFreeAligned(stats);
	if (sort_arr)
	    free(sort_arr);
#ifdef _DEBUG
	free( best_seq );
#endif
	EXIT_FUNC(cxScanNodeCategoricBestSplitClassification);
	return best_weight;
}

CART_IMPL float cxFindNodeCategoricBestSplitClassification( CxCART* cart,
							   CxCARTNode* node,
							   int feature,
							   CxCARTSplit* split)
{
    CV_FUNCNAME( "cxFindNodeCategoricBestSplitClassification" );
    __BEGIN__;

    //	*((int*)0)=0;
    int n = cart->num_classes[feature];
    assert( n >= 0 && n < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);
    if (n < 2 || n > 25) // 25 categories maximum !!!!
    {
	return 0.0;
    }
    int eff_feature = cart->features_corr[feature];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features);
    split->feature_idx = feature;
    char* ptr = (char*)split->boundary.ptr;
    /// Exit if we cannot do any split
    int num_not_missed = node->num_fallens - node->num_missed[eff_feature];
    int min_points = MAX(cart->params->split_min_points,1);
    if ( num_not_missed < 2 * min_points )
    {
	return 0.0;
    }
    int* fallen_stats_proportions = node->cat_proportions[eff_feature];
#ifdef _DEBUG
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned);

    int* fsp1 = (int*)icxAlignedCalloc(n * num_resp_aligned, sizeof(int));
    cxCalcFrequencies(fsp1,cart,node,feature);
    assert(memcmp(fsp1 , fallen_stats_proportions , n * num_resp_aligned * sizeof(int)) == 0);
    icxFreeAligned(fsp1);
#endif
    float best_weight = cxScanNodeCategoricBestSplitClassification((CxRootedCARTBase*)cart,
	fallen_stats_proportions,	ptr, n);

    split->weight = best_weight;
    return best_weight;
    __CLEANUP__;
    __END__;
}

CART_IMPL float cxFindNodeCategoricClusteringBestSplit( CxRootedCARTBase* cart,
						       CxVarCategoryCluster* clusters,
						       int num_clusters ,
						       int num_not_missed,
						       int feature,
						       CxCARTSplit* split)
{
    double impurity = 0.0;
#ifdef _DEBUG
    int eff_feature = cart->features_corr[feature];
#endif
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features);
    float eps_weight = cart->froot->split ? cart->froot->split->weight * EPS : EPS;
    double best_weight = eps_weight;
    int min_points = cart->params->split_min_points;
    if ( num_not_missed < 2 * min_points )
	return 0.0;
    START_FUNC(cxFindNodeCategoricClusteringBestSplit);

    split->feature_idx = feature;

    int num_resp = cart->num_response_classes;
    assert(num_resp > 2);
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned );

    int* stats = (int*)icxAlignedCalloc(2 * num_resp_aligned, sizeof(int));
    char* ptr = (char*)calloc(num_clusters,  sizeof(char));
#ifdef _DEBUG
    int num_cats = cart->num_classes[feature];
    assert( num_cats >= 0 && num_cats < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);
    char* best_ptr = (char*)calloc(num_clusters, sizeof(char));
    int size = sizeof(char) * num_clusters;
    assert( num_cats > num_clusters);
#endif
    icxArrToInt(clusters->frequencies , num_clusters * num_resp_aligned); // Round values

    //// Arrange clusters to array ( enumerate them )
    CxVarCategoryCluster** cluster_array = (CxVarCategoryCluster**)malloc( num_clusters * sizeof(CxVarCategoryCluster*) );
    CxVarCategoryCluster* cluster = clusters;
    int i = 0;

    while (cluster)
    {
	cluster_array[i++] = cluster;
	cluster = cluster->next_cluster;
    }
    //// Calculating node Gini Index
    int* _stats = stats + num_resp_aligned;
    for (int cluster_num = 0 ; cluster_num < num_clusters ; cluster_num ++)
    {
	int* frequencies = (int*)cluster_array[cluster_num]->frequencies;
	icxAddVector(_stats , frequencies , num_resp);
    }
    BOOL use_gini = (cart->params->splitting_rule == CxCARTGiniCriterion);
    float* cost_gini_mult = 0;

    if (use_gini)
    {
	cost_gini_mult = cart->cost_gini_mult;
	if (!cost_gini_mult)
	{
	    impurity = icxCalcGiniIndexPriors(_stats , num_resp, cart->priors_mult);
	}
	else
	{
	    impurity = icxCalcGiniIndexWithCosts(_stats , num_resp_aligned,
		cost_gini_mult ,  cart->priors_mult);
	}
    }
    else
    {
	impurity = icxCalcEntropyPriors(_stats , num_resp, cart->priors_mult);
    }
    //// Read 2 * num_clusters response vectors
    FUNC_MEM_READ (cxFindNodeCategoricClusteringBestSplit , 2 * num_clusters * num_resp_aligned , int);
    //// Write num_clusters response vectors
    FUNC_MEM_WRITE (cxFindNodeCategoricClusteringBestSplit , num_clusters * num_resp_aligned , int);

#ifdef _DEBUG
    /*	if ((cxCalcNodeNum(node) == 3) && (feature == 4))
    {
    ptr[4] = 1;
    float ff = cxCalcImpurityChangeCategoric(cart,node,split);
}*/
#endif
    BOOL use_priors = (cart->params->priors != NULL);
    if (impurity > eps_weight)
    {
	int path_len;
	/* Get the path to traverse all binary sequences with last 0, the path is
	a sequence of numbers, each number means which cluster to move to
	the other split side (0->1 , 1->0 ) */
	int* path = icxBuildPath(num_clusters - 1 , path_len);
	int best_step = -1;
	int num[2];
	num[0] = 0;
	num[1] = num_not_missed;
	int* cat_proportion = (int*)malloc(num_clusters * sizeof(int));
	for ( i = 0 ; i < num_clusters ; i++)
	{
	    int* v = (int*)cluster_array[i]->frequencies;
	    cat_proportion[i] = icxSumVector(v , num_resp);
	}
	//	icxGetCatProportions(cart , node , eff_feature);

	for (int step = 0 ; step < path_len - 1 ; step++)
	{
	    int pos = path[step];
	    ptr[pos] ^= 1;
	    int side = ptr[pos];
	    int other_side = 1 - side;
	    /// Recalculate numbers of cases fallen to each side
	    num[side] -= cat_proportion[pos];
	    num[other_side] += cat_proportion[pos];
	    assert(num[0] >= 0 && num[1] >= 0);
	    double impurity_left = 0.0;
	    double impurity_right = 0.0;
	    BOOL good_split = TRUE;

	    /* Update response distribution vector stats by recalculating frequencies
	    according to currently shifted cluster */
	    int* frequencies = (int*)cluster_array[pos]->frequencies;
	    icxAddVector(stats + other_side * num_resp_aligned ,
		frequencies, num_resp);
	    icxSubVector(stats + side * num_resp_aligned ,
		frequencies, num_resp);
	    //// Read 4 response vectors
	    FUNC_MEM_READ (cxFindNodeCategoricClusteringBestSplit , 4 * num_resp_aligned , int);
	    //// Write 2 response vectors
	    FUNC_MEM_WRITE (cxFindNodeCategoricClusteringBestSplit , 2 * num_resp_aligned , int);

#ifdef _DEBUG
	    int* stats1 = (int*)icxAlignedCalloc(num_resp_aligned * 2, sizeof(int));
	    for (i = 0 ; i < num_clusters ; i++)
	    {
		side = 1 - char(ptr[i]);
		int* frequencies = (int*)cluster_array[i]->frequencies;
		int* _stats = stats1 + side * num_resp_aligned;
		icxAddVector(_stats , frequencies , num_resp);
	    }
	    int num1[2] = {0,0};
	    num1[0] = icxSumVector(stats1 , num_resp);
	    num1[1] = icxSumVector(stats1 + num_resp_aligned, num_resp);
	    for (side = 0 ; side < 2 ; side ++)
	    {
		assert( num1[side] == num[side]  );
		int* _stats1 = stats1 + side * num_resp_aligned;
		int* _stats = stats + side * num_resp_aligned;
		for (i = 0 ; i < num_resp_aligned ; i++)
		{
		    assert( _stats1[i] == _stats[i]);
		}
	    }
	    icxFreeAligned(stats1);
#endif
	    ///   Calculate Gini Indexes of child nodes
	    good_split = ( num[0] >= min_points && num[1] >= min_points);
	    if (good_split)
	    {
		if (use_gini)
		{
		    if (use_priors)
		    {
			impurity_left = icxCalcGiniIndexPriors(stats , num_resp, cart->priors_mult);
			impurity_right = icxCalcGiniIndexPriors(stats + num_resp_aligned, num_resp, cart->priors_mult);
		    }
		    else if (!cost_gini_mult)
		    {
			//	*((int*)0)=0;
			impurity_left = icxCalcGiniIndex(stats , num_resp);
			impurity_right = icxCalcGiniIndex(stats + num_resp_aligned, num_resp);
		    }
		    else
		    {
			impurity_left = icxCalcGiniIndexWithCosts(stats ,
			    num_resp_aligned, cost_gini_mult ,
			    cart->priors_mult);
			impurity_right = icxCalcGiniIndexWithCosts(stats + num_resp_aligned,
			    num_resp_aligned, cost_gini_mult , cart->priors_mult);
		    }
		}
		else
		{
		    if (use_priors)
		    {
			impurity_left = icxCalcEntropyPriors(stats , num_resp, cart->priors_mult);
			impurity_right = icxCalcEntropyPriors(stats + num_resp_aligned, num_resp, cart->priors_mult);
		    }
		    else if (!cost_gini_mult)
		    {
			impurity_left = icxCalcEntropy(stats , num_resp);
			impurity_right = icxCalcEntropy(stats + num_resp_aligned, num_resp);
		    }
		}
		//	impurity_left = icxCalcGiniIndex(stats , num_resp, cart->priors_mult);
		//	impurity_right = icxCalcGiniIndex(stats + num_resp_aligned, num_resp, cart->priors_mult);
		//// Read stats , cart->priors_mult 2 times
		FUNC_MEM_READ (cxFindNodeCategoricClusteringBestSplit , 4 * num_resp , int);

	    }

	    if (good_split)
	    {
		assert ( (1.0 - (impurity_left + impurity_right)/impurity) > -eps_weight);
		double weight = impurity - impurity_left - impurity_right;
		if (weight > best_weight)
		{
#ifdef _DEBUG
		    memcpy(best_ptr , ptr , size);
#endif
		    best_weight = weight;
		    best_step = step;
		}
	    }
		}
		/// Restore sequence by its number in the path.
		icxGetSeq(ptr , num_clusters - 1, best_step);
		free(path);
#ifdef _DEBUG
		for (int p = 0 ; p < num_clusters ; p++)
		{
		    assert(best_ptr[p] == ptr[p]);
		}
#endif
		///	  Finally set current split to the best split
		char* split_ptr = (char*)split->boundary.ptr;
		for (i = 0 ; i < num_clusters ; i++)
		{
		    CxVarCategory* cat = cluster_array[i]->first_cat;
		    while (cat)
		    {
			split_ptr[cat->category] = (char)((cat->sum_frequencies < EPS) ? 2 : ptr[i]);
			cat = cat->next_cat;
		    }
		}
		free(cat_proportion);
	}
	split->weight = (float)best_weight;

	free(cluster_array);
	icxFreeAligned(stats);
	free(ptr);
#ifdef _DEBUG
	free(best_ptr);
#endif
	EXIT_FUNC(cxFindNodeCategoricClusteringBestSplit);
	return (float)best_weight;
}


CART_IMPL void cxMergeClusters(CxVarCategoryCluster* cluster1, CxVarCategoryCluster* cluster2, int num_resp)
{
    assert(cluster1 && cluster2 && (cluster1 != cluster2));
    CxVarCategoryCluster* _cluster1 = cluster1;
    CxVarCategoryCluster* _cluster2 = cluster2;

    if (cluster1->first_cat->category > cluster2->first_cat->category)
    {
	_cluster1 = cluster2;
	_cluster2 = cluster1;
    }
    if (_cluster2->prev_cluster)
	_cluster2->prev_cluster->next_cluster = _cluster2->next_cluster;
    if (_cluster2->next_cluster)
    {
	_cluster2->next_cluster->prev_cluster = _cluster2->prev_cluster;
	_cluster2->next_cluster = NULL;
    }
    _cluster2->prev_cluster = NULL;
    _cluster1->last_cat->next_cat = _cluster2->first_cat;
    _cluster1->last_cat = _cluster2->last_cat;
    _cluster1->num_cats += _cluster2->num_cats;
    assert (_cluster1->frequencies && _cluster2->frequencies);
    icxAddVector(_cluster1->frequencies , _cluster2->frequencies , num_resp);
    _cluster1->sum_frequencies += _cluster2->sum_frequencies;
    _cluster2->frequencies = NULL;

}

CART_API void cxCalcCatProportions(int* cat_proportions,
				   CxCART* cart, CxCARTNode* node,
				   int feature_idx)
{
    CxCARTSubj* subj = node->subj;
    CxCARTAtom* body = subj->body;
    int eff_feature = cart->features_corr[feature_idx];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features);
    assert(cart->num_response_classes == 0);

    int shift = feature_idx * subj->features_step;
    int* idx = node->fallen_idx + node->num_fallens * eff_feature;
    assert(icxIsClassifierVarCategoric(cart->feature_type[feature_idx]));
    int num_not_missed = node->num_fallens - node->num_missed[eff_feature];
#ifdef _DEBUG
    int num_cats = cart->num_classes[feature_idx];
#endif
    for (int i = 0 ; i < num_not_missed; i++)
    {
	CxCARTAtom* atom = body + idx[i];
	CxClassifierVar var = *(CxClassifierVar *)((char*)atom->features + shift);
	int var_class = icxVarToInt(var);
	assert( var_class >= 0 && var_class < num_cats);
	cat_proportions[var_class]++;
    }

}

CART_IMPL void cxCalcFrequencies(int* frequencies ,
				 CxCART* cart, CxCARTNode* node,
				 int feature_idx)
{
    int eff_feature = cart->features_corr[feature_idx];
    assert( eff_feature >= 0 && eff_feature < cart->eff_num_features);
    int i;
    int num_resp = cart->num_response_classes;
    assert(num_resp > 0);
    assert(icxIsClassifierVarCategoric(cart->feature_type[feature_idx]));

    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned );

#ifdef _DEBUG
    int num_cats = cart->num_classes[feature_idx];
#endif
    int num_not_missed = node->num_fallens - node->num_missed[eff_feature];
    int* idx = node->fallen_idx + node->num_fallens * eff_feature;

    CxCARTSubj* subj = node->subj;
    CxCARTAtom* body = subj->body;
    int shift = feature_idx * subj->features_step;


    for (i = 0 ; i < num_not_missed; i++)
    {
	CxCARTAtom* atom = body + idx[i];
	CxClassifierVar var = *(CxClassifierVar *)((char*)atom->features + shift);
	int var_class = icxVarToInt(var);
	assert( var_class >= 0 && var_class < num_cats);
	int response = icxVarToInt(atom->response);
	assert( response >= 0 && response < num_resp);
	frequencies[var_class * num_resp_aligned + response] ++;
    }

}

CART_IMPL CxVarCategoryCluster* cxClusterizeFeatureAgglomerative(CxCART* cart, CxCARTNode* node,
								 int feature_idx, int max_clusters )
{
    assert (cart && (feature_idx >= 0) && (feature_idx < cart->num_features));
    int eff_feature = cart->features_corr[feature_idx];
    assert(eff_feature >= 0);
    //	CxMemStorage* storage = cart->storage;
    int num_resp = cart->num_response_classes;
    int num_cats = cart->num_classes[feature_idx];
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned );

    if ( (num_cats <= max_clusters) || (num_resp < 3 ) )
	return NULL; // Not clusterize if regression , if num_resp <= 2, or num_cats <= max_clusters
    CxVarCategory* var_cats = (CxVarCategory*)malloc( num_cats * sizeof (CxVarCategory) );
    int i,j;
    //// Creating num_cats variable classes //////
    for (i = 0 ; i < num_cats ; i++)
    {
	CxVarCategory* cat = var_cats + i;
	cat->category = i;
	cat->next_cat = NULL;
    }
    CxVarCategoryCluster* clusters = (CxVarCategoryCluster*)malloc( num_cats * sizeof (CxVarCategoryCluster) );
    CxVarCategoryCluster* prev_cluster = NULL;
    CxVarCategoryCluster* cluster = NULL;
    clusters->frequencies = (float*)icxAlignedCalloc(num_cats * num_resp_aligned, sizeof(float));
    float* _frequencies = clusters->frequencies;
    //// Creating num_cats clusters with one category each
    for (i = 0 ; i < num_cats ; i++, _frequencies += num_resp_aligned)
    {
	cluster = clusters + i;
	cluster->last_cat = cluster->first_cat = var_cats + i;
	if (prev_cluster)
	    prev_cluster->next_cluster = cluster;
	cluster->prev_cluster = prev_cluster;
	prev_cluster = cluster;
	cluster->num_cats = 1;
	cluster->frequencies = _frequencies;
	cluster->sum_frequencies = 0.0f;
    }
    cluster->next_cluster = NULL;

    ///////// Calculate frequencies as inegers , then convert to floating-point
    int* tmp_freq = (int*)clusters->frequencies;
    //	cxCalcFrequencies(tmp_freq , cart, node , feature_idx );
    memcpy(tmp_freq , node->cat_proportions[eff_feature] , num_cats * num_resp_aligned * sizeof(int) );
#ifdef _DEBUG
    int* fsp1 = (int*)icxAlignedCalloc(num_cats * num_resp_aligned, sizeof(int));
    cxCalcFrequencies(fsp1, cart, node, feature_idx);
    assert(memcmp(fsp1 , tmp_freq , num_cats * num_resp_aligned * sizeof(int)) == 0);
    icxFreeAligned(fsp1);
#endif
    icxArrToFloat(tmp_freq , num_resp_aligned * num_cats);

    float* stats_inv = (float*) icxAlignedCalloc(num_resp_aligned,  sizeof(float) );
    for (i = 0; i < num_cats; i++)
    {
	for (j = 0; j < num_resp; j++)
	{
	    float freq = clusters[i].frequencies[j];
	    stats_inv[j] += freq;
	    clusters[i].sum_frequencies += freq * cart->priors_mult[j];
	}
    }
    for (j = 0; j < num_resp; j++)
    {
	stats_inv[j] = (stats_inv[j] == 0.0f) ? 0.0f : 1.0f/stats_inv[j];
	stats_inv[j] *= cart->priors_mult[j];
    }
    int num_clusters = num_cats;

    //////// Computing distances //////////
    float* distances = (float*) malloc( num_cats * num_cats * sizeof(float) );
    CxVarCategoryCluster* cluster1;
    CxVarCategoryCluster* cluster2;
    cluster1 = clusters;
    while (cluster1)
    {
	cluster2 = cluster1->next_cluster;
	while (cluster2)
	{
	    int i1 = cluster1->first_cat->category;
	    int i2 = cluster2->first_cat->category;
	    distances[i1 * num_cats + i2] = cxCalcDistance(cluster1 , cluster2 , stats_inv , num_resp);
	    cluster2 = cluster2->next_cluster;
	}
	cluster1 = cluster1->next_cluster;
    }

    //////// Perform clusterization ////////////
    while (num_clusters > max_clusters)
    {
	CxVarCategoryCluster* best_cluster1 = NULL;
	CxVarCategoryCluster* best_cluster2 = NULL;
	float best_dist = FLT_MAX;
	float dist = 0.0f;
	cluster1 = clusters;
	while (cluster1)
	{
	    cluster2 = cluster1->next_cluster;
	    while (cluster2)
	    {
		int i1 = cluster1->first_cat->category;
		int i2 = cluster2->first_cat->category;
		dist = distances[i1 * num_cats + i2] ;
		if (dist < best_dist)
		{
		    best_dist = dist;
		    best_cluster1 = cluster1;
		    best_cluster2 = cluster2;
		}
		cluster2 = cluster2->next_cluster;
	    }
	    cluster1 = cluster1->next_cluster;
	}
	cxMergeClusters(best_cluster1 , best_cluster2 , num_resp);

	////// Recalculate distances for best_cluster1 /////////
	int i1 = best_cluster1->first_cat->category;
	cluster2 = clusters;
	while (cluster2)
	{
	    if (cluster2 != best_cluster1)
	    {
		int i2 = cluster2->first_cat->category;
		int _i1 = MIN(i1,i2);
		int _i2 = MAX(i1,i2);
		distances[_i1 * num_cats + _i2] = cxCalcDistance(best_cluster1 , cluster2 , stats_inv , num_resp);

	    }
	    cluster2 = cluster2->next_cluster;
	}
	num_clusters--;
    }
    ////// Debug ////////
#ifdef _DEBUG
    /*	cluster1 = clusters;
    while (cluster1)
    {
    cluster2 = clusters;
    while (cluster2)
    {
    int i1 = cluster1->first_cat->category;
    int i2 = cluster2->first_cat->category;
    double dist = (i1==i2) ? 0.0 : distances[MIN(i1,i2) * num_cats + MAX(i1,i2)] ;
    file << dist << ' ';
    cluster2 = cluster2->next_cluster;
    }
    fputc('\n', file);
    cluster1 = cluster1->next_cluster;
	}*/
	/*	ofstream file ("clustering_aggl.log");
	DumpClusters(file , clusters , num_resp);
    file.flush();*/
#endif
    icxFreeAligned(stats_inv);
    free(distances);
    return clusters;
}

#define MAX_KMEANS_ITER 100

CART_IMPL CxVarCategoryCluster* cxClusterizeFeatureKMeans(CxCARTBase* cart, int* cat_proportions,
							  int feature_idx, int max_clusters ,
							  BOOL transform_freq, float beta )
{
    assert (cart && (feature_idx >= 0) && (feature_idx < cart->num_features));
#ifdef _DEBUG
    int eff_feature = cart->features_corr[feature_idx];
#endif
    assert(eff_feature >= 0);
    //	CxMemStorage* storage = cart->storage;
    int num_resp = cart->num_response_classes;
    int num_cats = cart->num_classes[feature_idx];
    assert( num_cats >= 0 && num_cats < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP( num_resp_aligned );

    if ( (num_cats <= max_clusters) || (num_resp < 3 ) )
	return NULL; // Not clusterize if regression , if num_resp <= 2, or num_cats <= max_clusters
    CxVarCategory* var_cats = (CxVarCategory*)malloc( num_cats * sizeof (CxVarCategory) );
    int i,j;

    START_FUNC(cxClusterizeFeatureKMeans);

    //////// Creating num_cats variable classes //////
    float* var_frequencies = (float*)icxAlignedCalloc(num_cats * num_resp_aligned, sizeof(float));
    float* _var_frequencies = var_frequencies;
    for (i = 0 ; i < num_cats ; i++, _var_frequencies += num_resp_aligned)
    {
	CxVarCategory* var = var_cats + i;
	var->category = i;
	var->next_cat = NULL;
	var->frequencies = _var_frequencies;
    }
    //	*((int*)0) = 0;
    CxVarCategoryCluster* clusters = (CxVarCategoryCluster*)malloc( max_clusters * sizeof (CxVarCategoryCluster) );
    CxVarCategoryCluster* prev_cluster = NULL;
    CxVarCategoryCluster* cluster = NULL;
    clusters->frequencies = (float*)icxAlignedCalloc(max_clusters * num_resp_aligned, sizeof(float));

    float* _frequencies = clusters->frequencies;
    //////// Creating max_clusters empty clusters
    for (i = 0 ; i < max_clusters ; i++, _frequencies += num_resp_aligned)
    {
	cluster = clusters + i;
	if (prev_cluster)
	    prev_cluster->next_cluster = cluster;
	cluster->prev_cluster = prev_cluster;
	prev_cluster = cluster;
	cluster->frequencies = _frequencies;
	cluster->sum_frequencies = 0.0f;
	cluster->first_cat = NULL;
	cluster->last_cat = NULL;
	cluster->num_cats = 0;
    }
    cluster->next_cluster = NULL;

    int* tmp_freq = (int*)var_frequencies;
    memcpy(tmp_freq , cat_proportions, num_cats * num_resp_aligned * sizeof(int) );
    FUNC_MEM_WRITE(cxClusterizeFeatureKMeans, num_cats * num_resp_aligned , int);
    icxArrToFloat(tmp_freq , num_resp_aligned * num_cats);
    FUNC_MEM_MODIFY(cxClusterizeFeatureKMeans, num_cats * num_resp_aligned , int);

    //////// Calcculate sums of rows and columns ///////////
    float* priors_mult = cart->priors_mult;
    float* stats_inv = (float*) icxAlignedCalloc(num_resp_aligned,  sizeof(float) );
    for (i = 0; i < num_cats; i++)
    {
	CxVarCategory* _var_cats = var_cats + i;
	float* _frequencies = _var_cats->frequencies;
	_var_cats->sum_frequencies = 0.0f;
	for (j = 0; j < num_resp; j++, _frequencies++)
	{
	    float freq = *_frequencies;
	    stats_inv[j] += freq;
	    if (!transform_freq)
		_var_cats->sum_frequencies += freq * priors_mult[j];
	}
    }
    /// Read cart->priors_mult and _var_cats->frequencies , modify stats_inv[j]
    FUNC_MEM_READ(cxClusterizeFeatureKMeans, 2 * num_cats * num_resp_aligned , float);
    FUNC_MEM_MODIFY(cxClusterizeFeatureKMeans, num_cats * num_resp_aligned , float);
    //	*((int*)0)=0;
    ///////// Transform frequencies ////////////
    float* old_frequencies = NULL;
    if (transform_freq)
    {
	old_frequencies = (float*)icxAlignedCalloc(num_cats * num_resp_aligned, sizeof(float));
	memcpy(old_frequencies, var_frequencies , num_cats * num_resp_aligned * sizeof(float));
	for (i = 0; i < num_cats; i++)
	{
	    for (j = 0; j < num_resp; j++)
	    {
		int pos = num_resp_aligned * i + j;
		float denom = stats_inv[j];
		float p = (denom == .0f) ? .0f : old_frequencies[pos] / denom;
		var_frequencies[pos] = (float)(1.0f - (exp( (1.0f-p) * beta ) - 1.0f) / (exp( (1.0f+p) * beta ) - 1.0f));
	    }
	}
	for (j = 0; j < num_resp; j++)
	    stats_inv[j] = 1.0;
	for (i = 0; i < num_cats; i++)
	{
	    for (j = 0; j < num_resp; j++)
	    {
		float freq = var_cats[i].frequencies[j];
		//	stats_inv[j] += freq;
		var_cats[i].sum_frequencies += freq * cart->priors_mult[j];
	    }
	}
    }
    //////// Invert sums of columns , multilying them with priors /////
    float stats_inv_min = FLT_MAX;
    float* _stats_inv = stats_inv;
    for (j = 0; j < num_resp; j++, _stats_inv++)
    {
	float prior = priors_mult[j];
	float cur_stats_inv = *_stats_inv;
	cur_stats_inv = (cur_stats_inv == 0.0f) ? 0.0f : 1.0f/ cur_stats_inv;
	cur_stats_inv *= prior;
	*_stats_inv = cur_stats_inv;
	if (cur_stats_inv > 0)
	    stats_inv_min = MIN(stats_inv_min , cur_stats_inv);
    }

    /////// Assign variables to clusters ///////
    int* var_to_clusters = (int*)malloc(num_cats * sizeof(int));
    int* var_to_clusters_new = (int*)calloc(num_cats, sizeof(int));
    for (i = 0; i < num_cats; i++)
	var_to_clusters[i] = i * max_clusters / num_cats;

#ifdef _DEBUG
    //	ofstream file("cart.log", ios::app);

    int transfers = 0;
#endif
    int iterations = 0;
    //////// Calculate frequencies /////////////
    for (i = 0; i < num_cats; i++)
    {
	int num_cluster = var_to_clusters[i];
	icxAddVector(clusters[num_cluster].frequencies , var_cats[i].frequencies , num_resp);
	clusters[num_cluster].sum_frequencies += var_cats[i].sum_frequencies;
    }
    /// Read 2 * num_cats vectors
    FUNC_MEM_READ(cxClusterizeFeatureKMeans, 2 * num_cats * num_resp_aligned , float);
    FUNC_MEM_WRITE(cxClusterizeFeatureKMeans, num_cats * num_resp_aligned , float);

    float eps_rel = FLT_EPSILON / stats_inv_min;
    //////// Move categories to closest clusters ///////////
    BOOL go = TRUE;
    while (go && (iterations < MAX_KMEANS_ITER))
    {
	go = FALSE;
	memcpy(var_to_clusters_new , var_to_clusters , num_cats * sizeof(int));
	for (i = 0; i < num_cats; i++)
	{
	    BOOL transfer = FALSE;
	    int old_cluster = var_to_clusters[i];
	    CxVarCategory* cat = var_cats + i;
	    /// Read 3 vectors
	    FUNC_MEM_READ(cxClusterizeFeatureKMeans, 3 * num_resp_aligned , float);
	    float best_dist = cxCalcDistance(clusters + old_cluster, cat, stats_inv, num_resp);
	    for (j = 0; j < max_clusters; j++)
	    {
		if  (j != old_cluster )
		{
		    float dist = cxCalcDistance(clusters + j , cat, stats_inv, num_resp);
		    /// Read 3 vectors
		    FUNC_MEM_READ(cxClusterizeFeatureKMeans, 3 * num_resp_aligned , float);

		    if (dist < best_dist - eps_rel)
		    {
			var_to_clusters_new[i] = j;
			best_dist = dist;
			transfer = TRUE; // There is some improvement
		    }
		}
	    }
	    if (transfer)
	    {
		go = TRUE;
#ifdef _DEBUG
		transfers ++;
#endif
		// Transfer the category to new cluster, if it makes sense
		int new_cluster = var_to_clusters_new[i];

		icxSubVector( clusters[old_cluster].frequencies , cat->frequencies , num_resp);
		icxAddVector( clusters[new_cluster].frequencies , cat->frequencies , num_resp);
		/// Read 4 vectors , write 2
		FUNC_MEM_READ(cxClusterizeFeatureKMeans, 4 * num_resp_aligned , float);
		FUNC_MEM_WRITE(cxClusterizeFeatureKMeans, 2 * num_resp_aligned , float);

		clusters[old_cluster].sum_frequencies -= cat->sum_frequencies;
		clusters[new_cluster].sum_frequencies += cat->sum_frequencies;
	    }
	}
	memcpy(var_to_clusters , var_to_clusters_new , num_cats * sizeof(int));
#ifdef _DEBUG
	/*	float goal = 0.0;
	for (i = 0; i < num_cats; i++)
	{
	cluster = clusters + var_to_clusters_new[i];
	CxVarCategory* cat = var_cats + i;
	float dist = cxCalcDistance(cluster ,cat , stats_inv, num_resp);
	goal += dist * dist * cat->sum_frequencies;
	}
	file << "Goal fun : " << goal << '\n';
	file.flush();*/
#endif
	iterations++;
	assert(iterations < MAX_KMEANS_ITER - 20);
    }

#ifdef _DEBUG
    //	file << "Categories : " << num_cats << " Iterations : " << iterations << " Transfers : " << transfers << '\n';
    //	file.flush();
#endif
    //////// Fill clusters with categories actually //////////
    CxVarCategoryCluster* first_cluster = clusters;
    for (i = 0; i < num_cats; i++)
    {
	int num_cluster = var_to_clusters[i];
	CxVarCategory* cat = var_cats + i;
	cluster = clusters + num_cluster;
	if (i==0)
	    first_cluster = cluster;
	if (!cluster->first_cat)
	    cluster->first_cat = cat;
	if (cluster->last_cat)
	    cluster->last_cat->next_cat = cat;
	cluster->last_cat = cat;
	cluster->num_cats++;
    }
    /* Make first_cluster really first ( in clusters array ) , we need it to
    delete it properly afterwards */
    if (clusters != first_cluster)
    {
	int sz = sizeof(void*);
	swap(&clusters->first_cat, &first_cluster->first_cat, sz);
	swap(&clusters->last_cat, &first_cluster->last_cat, sz);
	swap(&clusters->num_cats, &first_cluster->num_cats, sizeof(int));
	swap(&clusters->sum_frequencies, &first_cluster->sum_frequencies, sizeof(float));
	swap(clusters->frequencies, first_cluster->frequencies, num_resp * sizeof(int));
    }

    icxFreeAligned(stats_inv);
    free(var_to_clusters_new);
    icxFreeAligned(var_frequencies);

    //////// Re-calculate frequencies /////////////
    if (transform_freq)
    {
	memset(	clusters->frequencies, 0 , max_clusters * num_resp_aligned * sizeof(float));
	for (i = 0; i < num_cats; i++)
	{
	    int num_cluster = var_to_clusters[i];
	    cluster = clusters + num_cluster;
	    for (j = 0; j < num_resp; j++)
	    {
		float f = old_frequencies[num_resp_aligned * i + j];
		cluster->frequencies[j] += f;
	    }
	}
	icxFreeAligned(old_frequencies);
    }
    EXIT_FUNC(cxClusterizeFeatureKMeans);

    free(var_to_clusters);
    return clusters;
}

CART_IMPL void cxFreeClusters(CxVarCategoryCluster* clusters)
{
    free (clusters->first_cat);
    icxFreeAligned (clusters->frequencies);
    free(clusters);
}

CART_IMPL void DumpClusters(FILE* file, CxVarCategoryCluster* clusters, int num_resp)
{
    CxVarCategoryCluster* cluster1 = clusters;
    while (cluster1)
    {
	fprintf(file , "Cluster : %d\nSize : %d\nFrequency : %d\nFrequency distribution : \n",
	    (cluster1->first_cat ? cluster1->first_cat->category : -1),
	    cluster1->num_cats, cluster1->sum_frequencies);

	for (int i = 0 ; i < num_resp ; i++)
	    fprintf(file , "%.3f ", cluster1->frequencies[i]);
	fprintf(file, "\nCategories : \n");
	CxVarCategory* cat = cluster1->first_cat;
	while (cat)
	{
	    fprintf(file ,"%d ", cat->category);
	    cat = cat->next_cat;
	}
	fputc('\n', file);
	cluster1 = cluster1->next_cluster;
    }
}

CART_IMPL float cxCalcCARTAverageResponse(CxCART* cart, char* feature_status ,
					  CxClassifierVar* feature_values,
					  int pruning_step, CxCARTNode* node)
{
    assert(icxIsClassifierVarNumeric(cart->response_type));
    if (!node)
	node = cart->root;
    if (icxIsNodeSplit(node) && !icxIsNodePruned(node , pruning_step ) )
    {
	int feature = node->split->feature_idx;
	CxCARTNode* child[2];
	child[1] = node->child_left;
	child[0] = node->child_right;

	if ( feature_status[feature] )
	{
	    int left = icxIsVarSplitLeft(cart, node->split, feature_values[feature]);
	    return cxCalcCARTAverageResponse( cart, feature_status, feature_values, pruning_step, child[left] );
	}
	else
	{
	    float response_left = cxCalcCARTAverageResponse( cart, feature_status, feature_values, pruning_step, child[1] );
	    float response_right = cxCalcCARTAverageResponse( cart, feature_status, feature_values, pruning_step, child[0] );
	    return ( response_left * child[1]->num_fallens + response_right * child[0]->num_fallens ) / node->num_fallens;
	}
    }
    else
	return node->response.fl;
}

CART_IMPL float cxCalcCARTClassProbability(CxCART* cart, char* feature_status ,
					   CxClassifierVar* feature_values, int num_class,
					   int pruning_step, CxCARTNode* node)
{
    assert(icxIsClassifierVarCategoric(cart->response_type));
    assert( 0 <= num_class && num_class < cart->num_response_classes );
    if (!node)
	node = cart->root;
    if (icxIsNodeSplit(node) && !icxIsNodePruned(node , pruning_step ) )
    {
	int feature = node->split->feature_idx;
	CxCARTNode* child[2];
	child[1] = node->child_left;
	child[0] = node->child_right;

	if ( feature_status[feature] )
	{
	    int left = icxIsVarSplitLeft(cart, node->split, feature_values[feature]);
	    return cxCalcCARTClassProbability( cart, feature_status, feature_values,
		num_class, pruning_step, child[left] );
	}
	else
	{
	    float prob_left = cxCalcCARTClassProbability( cart, feature_status, feature_values,
		num_class,  pruning_step, child[1] );
	    float prob_right = cxCalcCARTClassProbability( cart, feature_status, feature_values,
		num_class, pruning_step, child[0] );
	    return ( prob_left * child[1]->num_fallens + prob_right * child[0]->num_fallens ) / node->num_fallens;
	}
    }
    else
    {
	BOOL floating = icxIsClassifierVar32f(cart->response_type);
	float response = floating ? node->response.fl : node->response.i;
	return (response == num_class);
    }
}

CART_IMPL void cxCalcCARTProbabilities(CxCART* cart, float* class_prob, char* feature_status ,
				       CxClassifierVar* feature_values, int pruning_step ,
				       CxCARTNode* node )
{
    assert(class_prob);
    for (int i = 0 ; i < cart->num_response_classes ; i++)
    {
	class_prob[i] = cxCalcCARTClassProbability(cart , feature_status ,
	    feature_values , i ,
	    pruning_step , node );
    }
}

CART_IMPL int cxGetBranchWeight(CxCARTNode* node)
{
    return (!icxIsNodeSplit(node)) ? 1 : cxGetBranchWeight(node->child_left) + cxGetBranchWeight(node->child_right) + 1;
}

CART_IMPL void cxFreeNodeInternals( CxCARTNode* node)
{
    CV_FUNCNAME( "cxFreeNodeInternals" );
    __BEGIN__;

    if (node->fallen_stats)
    {
	free(node->fallen_stats);
	node->fallen_stats = NULL;
    }
    if (node->fallen_idx)
    {
	free(node->fallen_idx);
	node->fallen_idx = NULL;
    }
    if (node->cat_proportions)
    {
	free(node->cat_proportions);
	node->cat_proportions = NULL;

    }
    __CLEANUP__;
    __END__;
}

CART_IMPL BOOL cxExpandNodeIdx(CxCART* cart, CxCARTNode* node, BOOL use_calc_storage)
{
    if (node->is_idx_expanded || node->is_copy)
	return TRUE;
    //// Allocate internal arrays
    if (!use_calc_storage)
    {
	node->fallen_idx = (int*)realloc(node->fallen_idx , sizeof(int) * node->num_fallens * cart->eff_num_features);
	if (!node->fallen_idx)
	    return FALSE;
	assert(!node->cat_proportions);
	cxAllocNodeCatProportions(cart , node);
    }
    else
    {
	assert(cart->storage && cart->storage->calc_storage.buf_fallen_idx);
	cxAllocNodeInternalsFromStorage(cart , node);
	assert(node->shrunk_fallen_idx && node->fallen_idx);
	memcpy (node->fallen_idx , node->shrunk_fallen_idx , sizeof(int) * node->num_fallens );
    }

    node->is_idx_expanded = TRUE;
    /* Restore (fill) internal arrays. If node is close to root, use external info,
    otherwise , if the node contains small number of samples , sort ourselves */
    if (cart->extra_info && (node->num_fallens > cart->root->num_fallens / 8) )
    {
	cxBuildNodeFallenIdxFromExtraInfo(cart , node);
    }
    else
    {
	cxBuildNodeFallenIdxFromScratch(cart , node);
    }
#ifdef _DEBUG
    cxCheckNodeFallenIdx(cart , node);
#endif
    return TRUE;
}

CART_IMPL void cxShrinkNodeIdx(CxCART* cart, CxCARTNode* node, BOOL use_calc_storage)
{
    if (node->is_idx_expanded && !node->is_copy)
    {
	if (!use_calc_storage)
	{
	    node->fallen_idx = (int*)realloc(node->fallen_idx , sizeof(int) * node->num_fallens);
	    assert(node->fallen_idx);
	    free(node->cat_proportions);
	    node->cat_proportions = NULL;
	}
	else
	{
	    assert(cart->storage && cart->storage->calc_storage.buf_fallen_idx && node->shrunk_fallen_idx);
	    memcpy(node->shrunk_fallen_idx , node->fallen_idx , sizeof(int) * node->num_fallens);
	    node->fallen_idx = node->shrunk_fallen_idx;
	    icxFreeCatProportions(cart->storage->calc_storage, node);
	}
	node->is_idx_expanded = FALSE;
    }
}

CART_API BOOL cxAllocNodeCatProportions(CxCART* cart, CxCARTNode* node)
{
    int i;
    int eff_num_features = cart->eff_num_features;
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP(num_resp_aligned )

#if CART_ALIGN_MEMORY
	int add_align = CART_ELEM_ALIGN;
#else
    int add_align = 0;
#endif

    int num_fallens = node->num_fallens;
    int mem_economy_mode = cart->params->mem_economy_mode;

    int sz_cat_prop = eff_num_features * sizeof(int*) ;           // for cat_proportions
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    int sz_add = 0;
    //// Calculate total size /////
    for (i = 0 ; i < eff_num_features ; i++)
    {
	int real_i = cart->features_back_corr[i];
	BOOL categoric = icxIsClassifierVarCategoric(cart->feature_type[real_i]);
	if (categoric)
	{
	    int num_classes = cart->num_classes[real_i];
	    assert( num_classes >= 0 && num_classes < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);
	    if (regression)
		sz_add += num_classes * sizeof(int);
	    else
		sz_add += (add_align + num_classes * num_resp_aligned) * sizeof(int);
	}
	else if (!mem_economy_mode)
	    sz_add += ( (num_fallens+1) * sizeof(CxClassifierVar));
    }
    sz_cat_prop += sz_add;
    node->cat_proportions = (int**)malloc(sz_cat_prop);
    if (!node->cat_proportions)
	return FALSE;
    memset(node->cat_proportions , 0 , sz_cat_prop);
    char* bp = (char*)(node->cat_proportions + eff_num_features);
    //// Set pointers , align them if nessesary
    for (i = 0 ; i < eff_num_features ; i++)
    {
	int real_i = cart->features_back_corr[i];
	BOOL categoric = icxIsClassifierVarCategoric(cart->feature_type[real_i]);
	if (categoric)
	{
	    int* _bp = (int*)bp;
#if CART_ALIGN_MEMORY
	    if (!regression)
	    {
		long int_bp = (long)_bp;
		int_bp = ((int_bp-1) & (-CART_MEM_ALIGN)) + CART_MEM_ALIGN;
		_bp = (int*)int_bp;
	    }
#endif
	    node->cat_proportions[i] = _bp;
	    int num_classes = cart->num_classes[real_i];
	    assert( num_classes >= 0 && num_classes < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);

	    if (regression)
		bp += num_classes * sizeof(int);
	    else
		bp += (add_align + num_classes * num_resp_aligned) * sizeof(int);
	}
	else if (!mem_economy_mode)
	{
	    node->cat_proportions[i] = (int*)bp;
	    bp += ((num_fallens + 1) * sizeof(CxClassifierVar));// - node->num_missed[i];
	}
	else
	    node->cat_proportions[i] = NULL;
    }
#ifdef WIN32
    assert(bp - _msize(node->cat_proportions) == (char*)node->cat_proportions);
#endif
    return TRUE;
}


CART_IMPL void cxAllocNodeInternals(CxCART* cart, CxCARTNode* node,
				    BOOL expanded_idx ,
				    BOOL use_calc_storage )
{
    CV_FUNCNAME( "cxAllocNodeInternals" );
    __BEGIN__;
    int eff_num_features = cart->eff_num_features;
    int num_resp = cart->num_response_classes;
    node->is_idx_expanded = expanded_idx;
    if (!use_calc_storage)
    {
	//		cxFreeNodeInternals(node);
	int s = num_resp * sizeof(int);      // for fallen_stats
	s += eff_num_features * sizeof(int); // for num_missed
	node->fallen_stats = (int *) malloc( s );
	if (!node->fallen_stats)
	{
	    icxSetCARTError(cart, CART_MEMORY_ERROR ,
		"Memory allocation error",
		CART_ERROR_STATUS_FATAL);
	    return;
	}

	node->num_missed = (int*)(node->fallen_stats + num_resp);
	memset(node->num_missed , 0 , sizeof(int) * eff_num_features );

	if (expanded_idx)
	{
	    //// Allocate node's cat_proportions
	    if (!cxAllocNodeCatProportions(cart , node))
	    {
		icxSetCARTError(cart, CART_MEMORY_ERROR ,
		    "Memory allocation error",
		    CART_ERROR_STATUS_FATAL);
		return;
	    }
	}

	//// Allocate node's fallen_idx
	int size_fallen_idx = sizeof(int) * node->num_fallens * (expanded_idx ? cart->eff_num_features : 1);
	node->fallen_idx = (int*)malloc(size_fallen_idx);
	if (!node->fallen_idx)
	{
	    icxSetCARTError(cart, CART_MEMORY_ERROR ,
		"Memory allocation error",
		CART_ERROR_STATUS_FATAL);

	}
    }
    else
    {
	icxAllocNodeShrunkIdx(cart , node);
	if (expanded_idx)
	    cxAllocNodeInternalsFromStorage(cart , node);
	else
	    node->fallen_idx = node->shrunk_fallen_idx;
    }

    __CLEANUP__;
    __END__;
}

CART_IMPL void cxFreeSplit(CxCARTSplit* split)
{
    CV_FUNCNAME( "cxFreeSplit" );
    __BEGIN__;
    if (!split)
	return;
    CxCARTSplit* split_cur = split->next_competitor;
    CxCARTSplit* split_next = NULL;
    while (split_cur)
    {
	split_next = split_cur->next_competitor;
	free(split_cur);
	split_cur = split_next;
    }
    split_cur = split;
    while (split_cur)
    {
	split_next = split_cur->next_surrogate;
	free(split_cur);
	split_cur = split_next;
    }
    //	split = NULL;
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxFreeNode( CxCARTNode* node)
{
    CV_FUNCNAME( "cxFreeNode" );
    __BEGIN__;
    assert(node);
    if (!node->is_copy)
	cxFreeNodeInternals(node);

    cxFreeSplit(node->split);
    free(node);
    __CLEANUP__;
    __END__;
}

CART_IMPL void cxFreeBranch(CxCARTNode* node , BOOL free_root)
{
    CV_FUNCNAME( "cxFreeBranch" );
    __BEGIN__;
    if (icxIsNodeSplit(node))
    {
	cxFreeBranch(node->child_left, TRUE);
	cxFreeBranch(node->child_right , TRUE);
    }
    if (free_root)
	cxFreeNode(node);

    else
    {
	node->child_left = node->child_right = NULL;
	node->split = NULL;
    }
    __CLEANUP__;
    __END__;
}

CART_IMPL void _DumpSplit(FILE* file, CxCARTBase* cart, CxCARTSplit* split, ESplitType split_type)
{
    assert(ferror(file) == 0);
    if (!split)
	return;
    int feature_idx = split->feature_idx;
    int type = cart->feature_type[feature_idx];

    if (icxIsClassifierVarCategoric(type))
    {
	fprintf(file , "split on %d feature at " , feature_idx);
	char* ptr = (char*)split->boundary.ptr;
	for (int i = 0 ; i < cart->num_classes[feature_idx] ; i++)
	    fputc('0' + ptr[i], file);
    }
    else
    {
	if (icxIsClassifierVar32f(type)) 
	{
	    fprintf(file , "split on %d feature at %.10g" , feature_idx, 
		    split->boundary.value.fl); 
	}
	else 
	{
	    fprintf(file, "split on %d feature being <= %d", feature_idx, 
		    split->boundary.value.i);
	}
	assert(ferror(file) == 0);
	clearerr(file);
    }
    if (split_type != SPLIT_SURROGATE)
    {
	fprintf(file ,  " weight : %g\n", split->weight);
    }
    else
    {
	fprintf(file , " predictive association : %g", split->weight);
	if (split->revert)
	    fputs(" revert ", file);
	fputc('\n', file); ;
    }
}

CART_IMPL void _DumpNode(FILE* file, CxCARTBase* cart, CxCARTNode *node)
{
    assert(ferror(file) == 0);
    if (!node)
	return;
    fprintf(file, "Node : %d\nDepth : %d\nStrings in node : %d\nResponse : %g\nNode error : %g"\
	"\nBranch error : %g\nTerminal nodes : %d\nPruning step : %d\nMissing values : \n",
	node->id, node->depth, node->num_fallens, node->response.fl,
	node->error, node->branch_error, node->terminal_nodes, node->pruning_step);

    for (int j = 0; j < cart->eff_num_features ; j++)
	fprintf(file , "%d ", node->num_missed[j]);
    fputc('\n', file);
    assert(ferror(file) == 0);

    /*	file << "Fallen strings : \n";
    for (i = 0 ; i < node->num_fallens ; i++)
    {
    int string_num = node->fallen_idx[i];
    file << string_num << ' ';
    }
    fputc('\n', file);
    if ( node->is_idx_expanded)
    {
    file << "Fallen string feature values : \n";
    for ( j = 0; j < cart->eff_num_features ; j++)
    {
    for (i = 0 ; i < node->num_fallens ; i++)
    {
				int string_num = node->fallen_idx[j * node->num_fallens + i];
				int real_j = cart->features_back_corr[j];
				int type = cart->feature_type[real_j];
				BOOL floating = icxIsClassifierVar32f(type);
				file << string_num << "-" ;
				CxClassifierVar var;
				BOOL missed = icxIsSubjFeatureMissed(node->subj,string_num,real_j);
				if (missed)
				file << "N/A-";
				else
				{
				var = icxGetSubjFeature(node->subj,string_num,real_j);
				if (floating)
				file << var.fl  << "-";
				else
				file << var.i << "-";
				}
				if (floating)
				file << node->subj->body[string_num].response.fl << ' ';
				else
				file << node->subj->body[string_num].response.i << ' ';
				}
				fputc('\n', file);
				}
				}
				file << "\nFallen string responses : \n";
				for (i = 0 ; i < node->num_fallens ; i++)
				{
				int string_num = node->fallen_idx[i];
				file << icxGetSubjResponse(node->subj,string_num).fl << ' ';
				}
    fputc('\n', file);*/

    if (icxIsClassifierVarCategoric(cart->response_type))
    {
	fprintf(file , "Class distribubution : ");
	for (int i = 0; i < cart->num_response_classes ; i++)
	    fprintf(file , "%d ", node->fallen_stats[i]);
	fputc('\n', file);
	assert(ferror(file) == 0);
    }
    if (icxIsNodeSplit( node ))
    {
	//// Dump primary split and all competitors
	CxCARTSplit* split = node->split;
	fprintf(file ,  "Competitor splits : \n");
	while (split)
	{
	    _DumpSplit( file, cart, split, SPLIT_COMPETITOR );
	    assert(split != split->next_competitor);
	    split = split->next_competitor;
	}
	split = node->split->next_surrogate;
	if (split)
	{
	    //// Dump all surrogates
	    fprintf(file , "Surrogate splits : \n");
	    while (split)
	    {
		_DumpSplit(file, cart, split, SPLIT_SURROGATE );
		assert(split != split->next_surrogate);
		split = split->next_surrogate;
	    }
	}
    }
    else
	fputs("Terminal node\n", file);
    fputs("*********************************************************\n", file);
    assert(ferror(file) == 0);
}


CART_IMPL void cxDumpNode( FILE* file, CxCARTBase* cart,  CxCARTNode *node, int pruning_step )
{
    _DumpNode(file, cart, node);
    if (icxIsNodeSplit( node ) &&
	(!icxIsNodePruned(node,pruning_step)) )
    {
	cxDumpNode(file, cart, node->child_left  ,pruning_step);
	cxDumpNode(file, cart, node->child_right ,pruning_step);
    }
}

CART_IMPL void cxDumpCART( FILE* file, CxCART* cart,int pruning_step )
{
    assert(cart && cart->root);
    assert(!ferror(file));
    int i = 0;
    double branch_error = 0.0;
    int terminal_nodes = 0;
    cxCalcBranchErrorAndComplexity(cart,cart->root, branch_error, terminal_nodes);
    //// Dump all nodes
    cxDumpNode(file, (CxCARTBase*)cart, cart->root,  pruning_step);
    //// Dump variable importances
    fprintf(file , "Variable importances :\n");
    for (i = 0 ; i < cart->eff_num_features ; i++)
    {
	float importance = cxCalcCARTVariableImportance(cart,cart->features_back_corr[i]);
	fprintf(file , "%g ", importance);
    }
    //// Dump pruning info
    fprintf(file , "\nnumber of terminal nodes : %d\nPruning info : \n" , cart->root->terminal_nodes );
    int prune_steps = icxGetPruningDataCount(cart);
    CxPruningData* data_seq = icxGetPruningSeq(cart);
    CxPruningData* data = data_seq;
    double last_error = data[prune_steps - 1].reestimation_error;
    if (last_error == 0.0)
	last_error = 1.0;
    double best_error = data->test_sample_error;
    int best_step = data->step;
    for (i = 0 ; i < prune_steps ; i++, data++)
    {
	fprintf(file , "Step : %d\nalpha : %g\nreestimation error : %g\n"\
	    "test error : %g\nerror deviation : %g\nnumber of terminal nodes : %d\n",
	    data->step , data->alpha / last_error, data->reestimation_error / last_error,
	    data->error_dev / last_error, data->terminal_nodes);

	if (data->test_sample_error < best_error)
	{
	    best_error = data->test_sample_error;
	    best_step = data->step;
	}
    }
    fprintf(file , "Best sample error : %g\nBest step : %d\nError normalizer : %g\n",
	best_error, best_step, last_error);
}

const char TAG_START = '[';
const char TAG_END = ']';
const char* PARAM_FEATURE_TYPES = "Feature types";
const char* PARAM_NUM_FEATURES = "Number of features";
const char* PARAM_RESPONSE_TYPE = "Response type";

CART_IMPL BOOL cxReadToDelim(FILE* file, char delim)
{
    char ch = 0;
    assert(!ferror(file) && !feof(file));
    while ((ch != delim) && !feof(file))
	ch = (char)fgetc(file);
    return !ferror(file) && (ch == delim);
}


CART_IMPL BOOL cxReadTag(FILE* file, char* tag_name, int len)
{
    if (ferror(file) || feof(file))
	return FALSE;
    BOOL ok = cxReadToDelim(file, TAG_START);
    if (ok)
    {
	ok = (read_str(file, tag_name, len, TAG_END) > 0);
    }
    assert(ok);
    return !ferror(file) && ok;
}


CART_IMPL void cxWriteTag(FILE* file, const char* tag_name, int indent , BOOL section)
{
    icxWriteIndent(file , indent);
    fputc(TAG_START, file);
    fputs(tag_name, file);
    fputc(TAG_END, file);
    fputc(' ', file);
    if (section)
    {
	fputc('\n', file);
	cxWriteTag(file, SECTION_START, indent);
	fputs(" ## beginning of section : ", file);
	fputs(tag_name, file);
	fputc('\n', file);
    }
}

CART_IMPL void cxWriteEndTag(FILE* file, int indent, const char* tag_name)
{
    cxWriteTag(file, SECTION_END, indent);
    if (tag_name)
    {
	fputs(" ## End of section : ", file );
	fputs(tag_name, file);
	fputc('\n', file);
    }
}

CART_IMPL BOOL cxSkipBegin(FILE* file, char* buf)
{
    if (ferror(file) || feof(file))
	return FALSE;
    if (!cxReadTag(file, buf, BUF_SIZE))
	return FALSE;
    return (!ferror(file) && strcmp(buf, SECTION_START) == 0);
}

CART_IMPL BOOL cxSkipSection(FILE* file, char* buf)
{
    if (ferror(file) || !feof(file))
	return FALSE;
    BOOL ok = TRUE;
    while ( cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0) && ok)
	ok = feof(file);
    return ok;
}

CART_IMPL BOOL cxReadNode(FILE* file, CxCART* cart, CxCARTNode*& node, char* buf)
{
    SKIP_BEGIN();

    CxCARTSplit* last_competitor = NULL;
    CxCARTSplit* last_surrogate = NULL;
    assert(cart->storage);
    CxCARTSplit* split_buf = icxAllocMaxSplitSize(cart->storage);
    char* competitor_buf = NULL;
    char* surrogate_buf = NULL;

    ESplitType type;
    BOOL ok = TRUE;
    int id = -1;
    int depth = -1;
    int num_fallens = -1;

    int stop = FALSE;
    int skipped = FALSE;
    assert(!ferror(file));
    long pos = ftell(file);
    //// Find node id , depth and num_fallens for root node
    while (!stop && cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0))
    {
	FREAD_PARAM_I( PARAM_NUM_FALLENS, num_fallens)
	    ELIF_READ_PARAM_I( PARAM_DEPTH,   depth)
	    ELIF_READ_PARAM_I( PARAM_ID,      id)
	    else
	    skipped = TRUE;
	stop = (id >=0 ) && ( depth >= 0 ) && ( num_fallens >= 0 );
    }

    if (id == 1) // Root node , initialize node storage
    {
	cxSetNodeStorageDepth(cart , num_fallens, cart->params->tree_max_depth);
    }
    else
    {
	assert( cart->storage->num_samples > num_fallens );
	assert( depth <= cart->storage->node_storage.max_depth);
    }
    if (skipped)
    {
	//// Return file pointer to the beginning of node section.
	fseek(file, pos , SEEK_SET);
    }
    assert(id > 0 && depth >= 0 );
    node = cxAllocNode(cart , id , depth );

    node->num_fallens = num_fallens;
    node->id = id;
    node->depth = depth;

    int competitors = 0;
    int surrogates = 0;
    while ( cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0) && ok)
    {
	FREAD_PARAM_I    ( PARAM_NUM_FALLENS,          node->num_fallens)
	    ELIF_READ_PARAM_F( PARAM_NODE_ERROR,           node->error)
	    ELIF_READ_PARAM_F( PARAM_SUM_WEIGHTED_FALLENS, node->sum_weighted_fallens)
	    ELIF_READ_PARAM_F( PARAM_RESPONSE,             node->response.fl)
	    ELIF_READ_PARAM_I( PARAM_NODE_PRUNING_STEP,    node->pruning_step)
	    ELIF_READ_PARAM_I( PARAM_NODE_DIRECTION,       node->direction)
	    else if (strcmp(buf, SECTION_SPLIT) == 0)
	{
	    CxCARTSplit* split = split_buf;
	    ok = cxReadSplit(file, (CxCARTBase*)cart, split, type, buf);
	    if (ok)
	    {
		if (type == SPLIT_MANUAL || type == SPLIT_PRIMARY)
		{
		    cxAddNodeSplits(cart , node , &split , 1 , type, &competitor_buf);
		    last_surrogate = last_competitor = node->split = split;
		}
		else
		{
		    assert( type == SPLIT_COMPETITOR || type == SPLIT_SURROGATE);
		    char** buf = (type == SPLIT_COMPETITOR) ? &competitor_buf : &surrogate_buf ;
		    cxAddNodeSplits(cart , node , &split , 1 , type , buf);
		    if (type == SPLIT_COMPETITOR)
		    {
			competitors++;
			assert(last_competitor);
			last_competitor->next_competitor = split;
			last_competitor = split;
		    }
		    else if (type == SPLIT_SURROGATE)
		    {
			surrogates++;
			assert(last_surrogate);
			last_surrogate->next_surrogate = split;
			last_surrogate = split;
		    }
		}
	    }
	}
	ELSE_SKIP_START();
    }
    assert( competitors <= cart->params->num_competitors );
    assert( surrogates  <= cart->params->num_surrogates  );

    if (ok)
    {
	if (last_surrogate)
	    last_surrogate->next_surrogate = NULL;
	if (last_competitor)
	    last_competitor->next_competitor = NULL;
    }

    free(split_buf);
    //// Memory corruption tests
    assert(!node->split || node->split->weight > -EPS);
    assert(!node->split || ((unsigned)node->split->revert < 2));
    assert(ok);
    return ok && !ferror(file);
}


CART_IMPL void cxWriteCARTNode(FILE* file,CxCART* cart,  CxCARTNode* node, int indent)
{
    BEGIN_SECTION( SECTION_NODE );

    FWRITE_PARAM_I( PARAM_DEPTH,				node->depth);
    FWRITE_PARAM_I( PARAM_ID,					node->id);
    FWRITE_PARAM_I( PARAM_NUM_FALLENS,			node->num_fallens);
    FWRITE_PARAM_F( PARAM_NODE_ERROR,			node->error);
    FWRITE_PARAM_F( PARAM_SUM_WEIGHTED_FALLENS, node->sum_weighted_fallens);
    FWRITE_PARAM_F( PARAM_RESPONSE,				node->response.fl);
    FWRITE_PARAM_I( PARAM_NODE_PRUNING_STEP,	node->pruning_step);
    FWRITE_PARAM_I( PARAM_NODE_DIRECTION,		node->direction);

    CxCARTSplit* split = node->split;
    if (split)
    {
	ESplitType type_primary = ((long)split != (long)(node + 1)) ? //((char*)split == icxGetSplitArea(cart , node , SPLIT_MANUAL) )?
SPLIT_MANUAL : SPLIT_PRIMARY;
	cxWriteSplit(file, (CxCARTBase*)cart, split, type_primary, indent);
	split = split->next_competitor;
	while (split)
	{
	    cxWriteSplit(file, (CxCARTBase*)cart, split, SPLIT_COMPETITOR, indent);
	    split = split->next_competitor;
	}
	split = node->split->next_surrogate;
	while (split)
	{
	    cxWriteSplit(file, (CxCARTBase*)cart, split, SPLIT_SURROGATE, indent);
	    split = split->next_surrogate;
	}
    }
    END_SECTION( SECTION_NODE );
}

const char* PARAM_SPLIT_FEATURE = "Split feature";
const char* PARAM_SPLIT_WEIGHT = "Split weight";
const char* PARAM_SPLIT_REVERT = "Reversed";
const char* PARAM_SPLIT_TYPE = "Split type";
const char* PARAM_SPLIT_VALUE = "Split value";

CART_IMPL BOOL cxReadSplit(FILE* file, CxCARTBase* cart, CxCARTSplit*& split, ESplitType& type, char* buf)
{
    SKIP_BEGIN();

    BOOL ok = TRUE;
    while ( cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0) && ok )
    {
	FREAD_PARAM_I    ( PARAM_SPLIT_TYPE,    type)
	    ELIF_READ_PARAM_I( PARAM_SPLIT_FEATURE, split->feature_idx)
	    ELIF_READ_PARAM_F( PARAM_SPLIT_WEIGHT,  split->weight)
	    ELIF_READ_PARAM_I( PARAM_SPLIT_REVERT,  split->revert)
	    else if (strcmp(buf, PARAM_SPLIT_VALUE) == 0)
	{
	    if (!split)
		return FALSE;
	    int feat_type = cart->feature_type[split->feature_idx];
	    eat_white(file);
	    if (icxIsClassifierVarNumeric(feat_type)) {
		if (icxIsClassifierVar32f(feat_type)) 
		{
		    fscanf(file, "%g", &split->boundary.value.fl);
		} 
		else 
		{
		    fscanf(file, "%d", &split->boundary.value.i);
		}
	    }
	    else
	    {
		split->boundary.ptr = split + 1;
		for (int i = 0 ; i < cart->num_classes[split->feature_idx]; i++)
		{
		    char c = (char)fgetc(file);
		    ((char*)split->boundary.ptr)[i] = (char)((c == 'L') ? 1 :
		    (c == '-') ? 2 : 0);
		}
	    }
	}
	ELSE_SKIP_START();
    }
    assert(ok);
    assert(split->weight >= -EPS);
    assert((unsigned)split->revert < 2);
    return ok && !ferror(file);
}

CART_IMPL void cxWriteSplit(FILE* file, CxCARTBase* cart, CxCARTSplit*& split, ESplitType type, int indent)
{
    BEGIN_SECTION( SECTION_SPLIT );

    FWRITE_PARAM_I( PARAM_SPLIT_TYPE,    type);
    FWRITE_PARAM_I( PARAM_SPLIT_FEATURE, split->feature_idx);
    FWRITE_PARAM_F( PARAM_SPLIT_WEIGHT,  split->weight);
    FWRITE_PARAM_I( PARAM_SPLIT_REVERT,  split->revert);

    cxWriteTag(file , PARAM_SPLIT_VALUE, indent);

    int feat_type = cart->feature_type[split->feature_idx];
    if (icxIsClassifierVarNumeric(feat_type))
    {
	if (icxIsClassifierVar32f(feat_type)) 
	    fprintf(file, "%.10g\n", ((double)split->boundary.value.fl) );
	else 
	    fprintf(file, "%d\n", split->boundary.value.i);
    }
    else
    {
	for (int i = 0 ; i < cart->num_classes[split->feature_idx]; i++)
	{
	    char dir = ((char*)split->boundary.ptr)[i];
	    char c = (dir == 0) ? 'R' : (dir == 1) ? 'L' : '-';
	    fputc(c, file);
	}
	fputc('\n', file);
    }
    END_SECTION( SECTION_SPLIT );
}

CART_IMPL BOOL cxReadCART(FILE* file, CxCART*& cart, char* buf)
{
    BOOL alloc = FALSE;
    if (!buf)
    {
	buf = (char*)calloc( BUF_SIZE, sizeof(char));
	alloc = TRUE;
    }
    if (!cart)
	cart = (CxCART*)calloc(1, sizeof( CxCART ));
    int num_features = -1;
    int response_type = -1;
    int* feature_types = NULL;
    CxCARTTrainParams* params = NULL;
    BOOL ok = cxSkipBegin(file,buf);
    while ( cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0) && ok )
    {
	if (strcmp(buf, SECTION_FEATURE_TYPES) == 0)
	{
	    ok = cxReadFeatureTypes(file, feature_types,response_type,num_features,buf);
	    cart->num_response_classes = icxGetVarNumStates( response_type ) ;
	}
	else if (strcmp(buf, SECTION_TRAIN_PARAMS) == 0)
	{
	    if (response_type >= 0 && num_features > 0 && feature_types)
	    {
		ok = cxReadCARTTrainParams(file, params, cart->num_response_classes, buf);
		if (ok)
		    cxCreateCART(num_features,feature_types,response_type,params,cart);
		free(feature_types);
	    }
	    else
		ok = FALSE;
	}
	else if (strcmp(buf, SECTION_ALL_NODES) == 0)
	{
	    cxInitNodeStorage(cart);
	    ok = cxReadAllNodes(file, cart, buf);
	}
	else if (strcmp(buf, SECTION_ERROR_SEQ) == 0)
	    ok = cxReadErrorSeq(file, cart, buf);
	ELSE_SKIP_START();
    }
    if (alloc)
	free(buf);
    assert(ok);
    return ok && !ferror(file);
}

CART_IMPL void cxWriteCART(FILE* file, CxCART* cart, int indent)
{
    BEGIN_SECTION ( SECTION_CART );
    //	int def_precision = file.precision(10);
    cxWriteFeatureTypes(file, (CxCARTBase*)cart, indent);
    cxWriteCARTTrainParams(file, (CxCARTBase*)cart, indent);
    cxWriteAllTreeNodes(file, (CxRootedCARTBase*)cart, indent);
    cxWriteErrorSeq(file,cart, indent);
    //	file.precision(def_precision);

    END_SECTION ( SECTION_CART );
}

CART_IMPL void cxWriteTrainParams(FILE* file, CxCARTTrainParams* params, int indent)
{
    FWRITE_PARAM_I ( PARAM_NUM_FEATURES_OF_INTEREST, params->num_features_of_interest);
    FWRITE_PARAM_AI( PARAM_FEATURES_OF_INTEREST,	 params->features_of_interest, params->num_features_of_interest);
    FWRITE_PARAM_I ( PARAM_SPLIT_RULE,				 params->splitting_rule);
    FWRITE_PARAM_I ( PARAM_NUM_COMPETITORS,			 params->num_competitors);

    FWRITE_PARAM_F ( PARAM_COMPETITOR_THRESHOLD,	 params->competitor_threshold);
    FWRITE_PARAM_I ( PARAM_NUM_SURROGATES,			 params->num_surrogates);
    FWRITE_PARAM_F ( PARAM_SURROGATE_THRESHOLD,		 params->surrogate_threshold);
    FWRITE_PARAM_I ( PARAM_MAX_DEPTH,			     params->tree_max_depth);

    FWRITE_PARAM_I ( PARAM_SPLIT_MIN_POINTS,		 params->split_min_points);
    FWRITE_PARAM_I ( PARAM_SPLIT_MAX_POINTS,		 params->split_max_points);
    FWRITE_PARAM_I ( PARAM_NUM_CLUSTERS,			 params->max_clusters);
    FWRITE_PARAM_F ( PARAM_ALPHA_STOP,			     params->alpha);
}

CART_IMPL void _cxWriteCARTTrainParams(FILE* file, CxCARTBase* cart , int indent)
{
    BEGIN_SECTION ( SECTION_TRAIN_PARAMS );

    CxCARTTrainParams* params = cart->params;
    cxWriteTrainParams(file , params , indent);
    FWRITE_PARAM_AF( PARAM_PRIORS, params->priors, cart->num_response_classes);
}

CART_IMPL void cxWriteCARTTrainParams(FILE* file, CxCARTBase* cart , int indent)
{
    _cxWriteCARTTrainParams(file, cart, indent);
    END_SECTION( SECTION_TRAIN_PARAMS );
}

__inline BOOL icxReadCARTTrainParam(FILE* file, CxCARTTrainParams* params, int num_resp, char* buf)
{
    assert(!ferror(file) && params);
    FREAD_PARAM_AI( PARAM_FEATURES_OF_INTEREST,
	params->features_of_interest_mat->data.ptr,
				    params->num_features_of_interest)
				    ELIF_READ_PARAM_AF( PARAM_PRIORS,              params->priors_mat->data.fl, num_resp)
				    ELIF_READ_PARAM_I (PARAM_SPLIT_RULE,           params->splitting_rule)
				    ELIF_READ_PARAM_I (PARAM_NUM_COMPETITORS,      params->num_competitors)
				    ELIF_READ_PARAM_F (PARAM_COMPETITOR_THRESHOLD, params->competitor_threshold)
				    ELIF_READ_PARAM_I (PARAM_NUM_SURROGATES,       params->num_surrogates)
				    ELIF_READ_PARAM_F (PARAM_SURROGATE_THRESHOLD,  params->surrogate_threshold)
				    ELIF_READ_PARAM_I (PARAM_MAX_DEPTH,            params->tree_max_depth)
				    ELIF_READ_PARAM_I (PARAM_SPLIT_MIN_POINTS,     params->split_min_points)
				    ELIF_READ_PARAM_I (PARAM_SPLIT_MAX_POINTS,     params->split_max_points)
				    ELIF_READ_PARAM_I (PARAM_NUM_CLUSTERS,         params->max_clusters)
				    ELIF_READ_PARAM_F (PARAM_ALPHA_STOP,           params->alpha)
				    else return FALSE;
    return TRUE;
}

CART_IMPL BOOL cxReadCARTTrainParams(FILE* file,
				     CxCARTTrainParams*& params,
				     int num_response_classes, char* buf)
{
    CvMat* features_of_interest = NULL;
    if (!cxSkipBegin(file,buf))
	return FALSE;

    params = NULL;
    int num_features_of_interest = -1;
    BOOL ok = TRUE;
    CvMat* priors_mat = (num_response_classes > 0) ? cvCreateMat( 1, num_response_classes, CV_32FC1) : NULL;

    while ( cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0) && ok )
    {
	if (strcmp(buf, PARAM_NUM_FEATURES_OF_INTEREST) == 0)
	{
	    fscanf(file , "%d", &num_features_of_interest);
	    if (num_features_of_interest > 0)
		features_of_interest = cvCreateMat( num_features_of_interest , 1, CV_32SC1 );
	    params = cxCARTTrainParams( features_of_interest, /* features_of_interest */
		priors_mat, /*priors */
		CxCARTGiniCriterion,
		5, /* num_competitors */
		.2f, /* competitor_threshold */
		10, /* num_surrogates */
		.0f, /* surrogate_threshold */
		10, /* tree_max_depth */
		5, /* split_min_points, */
		0, /* split_max_points, */
		10, /*number of clusters*/
		0); /* memory economic mode */
	    //										0,	/* Transform frequencies flag */
	    //										2,  /* Default beta value */
	    if (priors_mat)
		params->priors = priors_mat->data.fl;
	}
	else if (!icxReadCARTTrainParam(file, params, num_response_classes, buf))
	{
	    SKIP_START();
	}
    }
    assert(ok);
    return ok && !ferror(file);
}

CART_IMPL void cxWriteFeatureTypes(FILE* file, CxCARTBase* cart, int indent)
{
    BEGIN_SECTION( SECTION_FEATURE_TYPES );

    int response_type = cart->num_response_classes ?
	CX_CLASSIFIER_VAR_TYPE_CATEGORIC_32F(cart->num_response_classes) :
    CX_CLASSIFIER_VAR_TYPE_NUMERIC_32F ;

    FWRITE_PARAM_I ( PARAM_NUM_FEATURES, cart->num_features);
    FWRITE_PARAM_AI( PARAM_FEATURE_TYPES, cart->feature_type, cart->num_features);
    FWRITE_PARAM_I ( PARAM_RESPONSE_TYPE, response_type);

    END_SECTION( SECTION_FEATURE_TYPES );
}

CART_IMPL BOOL cxReadFeatureTypes(FILE* file, int*& types, int& response_type,
				  int& num_features, char* buf)
{
    if (!cxSkipBegin(file,buf))
	return FALSE;
    types = NULL;
    BOOL ok = TRUE;
    while ( cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0) && ok)
    {
	if (strcmp(buf, PARAM_NUM_FEATURES) == 0)
	{
	    fscanf(file , "%d",  &num_features);
	    if (num_features <= 0 )
		return FALSE;
	    else
	    {
		types = (int*)malloc(sizeof(int) * num_features);
		memset(types,-1,sizeof(int) * num_features);
	    }
	}
	ELIF_READ_PARAM_I ( PARAM_RESPONSE_TYPE, response_type)
	    ELIF_READ_PARAM_AI( PARAM_FEATURE_TYPES, types, num_features)
	    ELSE_SKIP_START();
    }
    assert(ok);

    return ok && !ferror(file);
}

CART_IMPL BOOL cxReadAllNodes(FILE* file, CxCART* cart, char* buf)
{
    if (!cxSkipBegin(file,buf))
	return FALSE;
    CxCARTNode* node = NULL;;
    int nodes_read = 0;
    BOOL ok = TRUE;
    while ( cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0) && ok)
    {
	FREAD_PARAM_I(PARAM_NUM_NODES , cart->num_nodes)
	    else if (strcmp(buf, SECTION_NODE) == 0)
	{
	    ok = cxReadNode(file,cart,node,buf);
	    if (ok)
	    {
		nodes_read ++;
		int parent_id = node->id >> 1;
		int parent_depth = node->depth - 1;

		BOOL is_right = node->id - ( parent_id << 1 );
		if (parent_id < 1)
		    cart->root = node;
		else
		{
		    //// Find parent node, it must be loaded already.
		    CxCARTNode* parent = icxGetNodePos(cart->storage->node_storage , parent_id , parent_depth);
		    assert(parent->depth == parent_depth && parent->id == parent_id);
		    if (!parent)
			ok =  FALSE;
		    else
		    {
			assert( parent->depth == parent_depth &&
			    parent->id == parent_id);
			node->parent = parent;
			if (is_right)
			    parent->child_right = node;
			else
			    parent->child_left = node;
#ifdef _DEBUG
			if (parent->child_right && parent->child_left)
			{
			    assert (parent->num_fallens == parent->child_right->num_fallens + parent->child_left->num_fallens);
			}
#endif
		    }
		}
	    }
	}
	ELSE_SKIP_START();
    }
    ok = ok && (nodes_read == cart->num_nodes);
    assert(ok);
    return ok && !ferror(file);
}

CART_IMPL void cxWriteAllTreeNodes(FILE* file, CxRootedCARTBase* cart, int indent,
				   CxForestNode* root, int num_nodes)
{
    BEGIN_SECTION( SECTION_ALL_NODES );
    if (!root)
    {
	root = cart->froot;
	num_nodes = cart->num_nodes;
    }
    FWRITE_PARAM_I( PARAM_NUM_NODES , num_nodes);
    CxForestNode* node = root;
    CxForestNode** all_nodes = (CxForestNode**)calloc(num_nodes , sizeof(CxForestNode*));
    int i = 0;
    while (node)
    {
	all_nodes[i++] = node;
	node = cxGetNextForestNode(node);
    }
    qsort( (void *)all_nodes, num_nodes , sizeof(CxForestNode*),compare_nodes );
    for ( i = 0 ; i < num_nodes; i++)
    {
	(cart->vftbl->write_node)(file, (CxClassifier*)cart,((CxForestNode**)all_nodes)[i], indent);
    }

    free(all_nodes);
    END_SECTION( SECTION_ALL_NODES );
}

CART_IMPL BOOL cxReadErrorSeq(FILE* file, CxCART* cart, char* buf)
{
    SKIP_BEGIN();

    BOOL ok = cxCreatePruningStorage(cart);
    assert(ok);
    CxPruningData* data = icxGetPruningSeq(cart);;
    while ( cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0) && ok)
    {
	if (strcmp(buf, SECTION_PRUNING_DATA) == 0)
	{
	    ok = cxReadPruningData(file , data, buf);
	    if (ok)
	    {
		data++;
		cart->pruning_storage.size++;
		assert( cart->pruning_storage.size < cart->pruning_storage.max_size);
	    }
	}
	ELSE_SKIP_START();
    }
    if (ok)
	cart->progress_info.progress_data_size = cart->pruning_storage.size ;
    assert(ok);
    return ok && !ferror(file);
}

CART_IMPL void cxWriteErrorSeq(FILE* file, CxCART* cart, int indent)
{
    BEGIN_SECTION( SECTION_ERROR_SEQ );

    CxPruningData* data = icxGetPruningSeq(cart);
    int n = icxGetPruningDataCount(cart);
    for (int i = 0 ; i < n ; i++ , data++)
    {
	cxWritePruningData(file , data, indent);
    }

    END_SECTION( SECTION_ERROR_SEQ );
}

CART_IMPL BOOL cxReadPruningData(FILE* file, CxPruningData*& data, char* buf )
{
    SKIP_BEGIN();

    assert(data);
    BOOL ok = TRUE;
    while ( cxReadTag(file, buf, BUF_SIZE) && (strcmp(buf, SECTION_END) != 0) && ok)
    {
	FREAD_PARAM_I     ( PARAM_PRUNING_STEP,       data->step)
	    ELIF_READ_PARAM_I ( PARAM_TERMINAL_NODES,     data->terminal_nodes)
	    ELIF_READ_PARAM_F ( PARAM_ALPHA,              data->alpha)
	    ELIF_READ_PARAM_D ( PARAM_REESTIMATION_ERROR, data->reestimation_error)
	    ELIF_READ_PARAM_D ( PARAM_TEST_SAMPLE_ERROR,  data->test_sample_error)
	    ELIF_READ_PARAM_D ( PARAM_ERROR_DEVIATION,    data->error_dev)
	    ELSE_SKIP_START()	;
    }
    data->nodes_seq = NULL;
    assert(ok);
    return ok && !ferror(file);
}

CART_IMPL void cxWritePruningData(FILE* file,  CxPruningData* data,int indent)
{
    BEGIN_SECTION( SECTION_PRUNING_DATA );

    FWRITE_PARAM_I ( PARAM_PRUNING_STEP,  	     data->step);
    FWRITE_PARAM_I ( PARAM_TERMINAL_NODES, 		 data->terminal_nodes);
    FWRITE_PARAM_F ( PARAM_ALPHA,			 data->alpha);
    FWRITE_PARAM_F ( PARAM_REESTIMATION_ERROR,	 data->reestimation_error);
    FWRITE_PARAM_F ( PARAM_TEST_SAMPLE_ERROR,	 data->test_sample_error);
    FWRITE_PARAM_F ( PARAM_ERROR_DEVIATION,	     data->error_dev);

    END_SECTION( SECTION_PRUNING_DATA );
}

CART_IMPL void GetCARTBuildTime(char* str)
{
    sprintf(str , "CART Build Date : %s , Time : %s" , __DATE__ , __TIME__);
}

CART_IMPL BOOL cxFindTag(FILE* file, const char* tag_name, char* buf )
{
    if (ferror(file) || feof(file))
	return FALSE;
    BOOL alloc = FALSE;
    if (!buf)
    {
	buf = (char*)calloc( BUF_SIZE, sizeof(char));
	alloc = TRUE;
    }
    while ( cxReadTag(file, buf, _MAX_PATH) && (strcmp(buf, tag_name) != 0) && !feof(file))
    {
	;
    }
    BOOL ret = (strcmp(buf, tag_name) == 0);
    if (alloc)
	free(buf);
    return ret;
}

CART_IMPL BOOL cxInitNodeStorage(CxCART* cart, CxCARTStorage* storage)
{
    if (!storage)
	storage = &cart->inner_storage;
    cart->storage = storage;

    CxCARTTrainParams* params = cart->params;
    int* features_of_interest = params->features_of_interest;
    int eff_num_features = cart->eff_num_features;
    int num_categoric_vars = 0;
    int max_num_cats = 0;
    int total_cats = 0;
    int* types = cart->feature_type;//sample->feature_type;

				    /*  Calculating total number of categories , number of
    categoric variables and maximal number of categories */

    for (int i = 0 ; i < eff_num_features ; i++)
    {
	int feature = features_of_interest ? features_of_interest[i] : i;
	int type = types[feature];
	assert (type > 0);
	if (icxIsClassifierVarCategoric(type))
	{
	    num_categoric_vars++;
	    int num_cats = icxGetVarNumStates(type);
	    max_num_cats = MAX( max_num_cats , num_cats );
	    total_cats += num_cats;
	}
    }
    storage->num_categoric_vars = num_categoric_vars;
    storage->total_num_cats = total_cats;
    storage->max_num_cats = max_num_cats;

    int num_resp = icxGetVarNumStates( cart->response_type );
    if ((max_num_cats > 10000) || (num_resp * total_cats > 10000000))
    {
	icxSetCARTError(cart , CART_CALC_ERROR ,
	    "Number of categories is too large!",
	    CART_ERROR_STATUS_FATAL);
	return FALSE;
    }

    int node_size = sizeof(CxCARTNode);

    node_size += num_resp * sizeof(int);          // for fallen_stats
    node_size += eff_num_features * sizeof(int);  // for num_missed
    node_size += eff_num_features * sizeof(int*); // for cat_proportions

    // Space for competititors
    int num_competitors = params->num_competitors + 1;
    int competitors_space = num_competitors * sizeof (CxCARTSplit);
    competitors_space += MIN (max_num_cats * num_competitors  , total_cats) * sizeof(char);

    // Space for surrogates
    int num_surrogates = params->num_surrogates + 1;
    int surrogate_space = num_surrogates * sizeof (CxCARTSplit) ;
    surrogate_space += MIN (max_num_cats * num_surrogates  , total_cats) * sizeof(char);

    // Space for one manual split.
    int manual_split_space = sizeof(CxCARTSplit) + max_num_cats * sizeof(char);

    int split_area_size = competitors_space + surrogate_space + manual_split_space;

    CxCARTNodeStorage& node_storage = storage->node_storage ;
    node_storage.competitors_space = competitors_space;
    node_storage.surrogates_space = surrogate_space;
    node_storage.manual_split_space = manual_split_space;
    node_storage.split_area_size = split_area_size;
    node_size += split_area_size;
    node_storage.node_size = node_size;
    node_storage.max_depth = node_storage.depth = -1; // No levels
    node_storage.split_area_size = split_area_size;
    return TRUE;
}

CART_IMPL CxCARTNode* cxAllocNode(CxCART* cart , int id ,int depth, BOOL clear)
{
    CxCARTStorage* storage = cart->storage;
    CxCARTNodeStorage& node_storage = storage->node_storage;
    assert(depth <= node_storage.max_depth );
    assert(node_storage.num_blocks > 0);

    if ( node_storage.depth < depth)
    {
	assert (depth == node_storage.depth + 1 ); //Cannot grow too fast!
	node_storage.depth = depth;
    }

    int node_size = node_storage.node_size;
    CxCARTNode* node = icxGetNodePos(node_storage , id , depth);
    char* buf = (char*)node;
#ifdef _DEBUG
    char* old_buf = buf;
#endif
    if (clear)
	memset(node , 0 , node_size);
    buf += sizeof(CxCARTNode);
    buf += node_storage.split_area_size;
    int eff_num_features = cart->eff_num_features ;
    int num_resp = icxGetVarNumStates(cart->response_type);
    if (num_resp > 0)
    {
	node->fallen_stats = (int*)buf;
	buf += num_resp * sizeof(int);
    }
    node->num_missed = (int*)buf;
    buf += eff_num_features * sizeof(int);
    node->cat_proportions = (int**)buf;
    buf += eff_num_features * sizeof(int*);
#ifdef _DEBUG
    assert( buf == old_buf + node_size);
#endif
    node->id = id;
    node->depth = depth;
    return node;
}

CART_IMPL BOOL cxGrowCARTBranch (CxCART *cart,
				 CxCARTNode *node,
				 CxCARTSplit* split ,
				 BOOL not_shrink_idx )
{
    assert(cart && node && cart->storage);
    CxCARTStorage* storage = cart->storage;
    CxCARTCalcStorage& calc_storage = storage->calc_storage;
    CxCARTNodeStorage& node_storage = storage->node_storage;
    int max_depth = cart->params->tree_max_depth;
    int new_depth = calc_storage.max_depth - node->depth;
    int num_samples = node->subj->size;
    storage->num_samples = num_samples;
    if (!cxSetNodeStorageDepth(cart , num_samples, max_depth))
	return FALSE;
    BOOL num_reset = (calc_storage.num_root_fallens != node->num_fallens);
    if (new_depth != max_depth || num_reset)
    {
	if ( num_reset )
	{
	/*  Shrink node if number of samples changed.
	    Need to reallocate and recalculate node internals */
	    cxShrinkNodeIdx(cart, node);
	}
	int old_max_depth = calc_storage.max_depth;
	cxCreateCalcStorage(cart , node->num_fallens , node->depth, node->offset);
	if ( max_depth > old_max_depth && node->is_idx_expanded && !num_reset)
	{
	/*  Do not shrink node if number of samples not changed but
	    storage depth increased. Just need to re-calculate pointers */
	    node->cur_cat_prop_pos = 0;
	    cxAllocNodeInternalsFromStorage(cart , node, FALSE);
	}
    }
    BOOL ret = cxSplitCARTLeafFull (cart, node, split , not_shrink_idx);

    icxFreeCalcStorage(calc_storage);
    cxSetNodeStorageDepth(cart , num_samples , node_storage.depth);

    return ret;
}

CART_IMPL BOOL cxCompactNodeStorage(CxCARTStorage* storage, CxCARTNode* root)
{
    CxCARTNodeStorage* node_storage = &storage->node_storage;
    if (node_storage->is_compact)
	return TRUE;
    int num_blocks = node_storage->num_blocks;
    CxCARTLevel* blocks = node_storage->blocks;
    for (int i = 0; i < num_blocks; i++)
    {
	if (blocks[i].buf_shrunk_idx)
	{
	    free(blocks[i].buf_shrunk_idx);
	    blocks[i].buf_shrunk_idx = 0;
	}
    }
    CxCARTNode* node = root;
    while (node)
    {
	node->fallen_idx = node->shrunk_fallen_idx = 0;
	node = cxGetNextNode(node);
    }
    return TRUE;
}

CART_IMPL BOOL cxSetNodeStorageDepth(CxCART* cart, int num_samples , int depth)

{
    CxCARTStorage* storage = cart->storage;
    CxCARTNodeStorage& node_storage = storage->node_storage;
    assert(!node_storage.is_compact);

    int& old_depth = node_storage.max_depth;
    int& num_blocks = node_storage.num_blocks;

    assert(num_samples > 0);
    int node_size = node_storage.node_size;
    storage->num_samples = num_samples;

    CxCARTLevel* blocks = node_storage.blocks;
    int* levels_per_block = node_storage.levels_per_block;

    //// Increase depth , allocating new block for new levels
    if (depth > old_depth)
    {
	int new_node_block_size = icxGetNodeBlockSize(old_depth , depth, node_size);
	int new_idx_block_size = icxGetIdxBlockSize(old_depth , depth , num_samples);

	levels_per_block[num_blocks] = depth - old_depth;

	assert(!blocks[num_blocks].buf_nodes && !blocks[num_blocks].buf_shrunk_idx);
	//// Allocating new block
	char* buf_nodes = (char*)(blocks[num_blocks].buf_nodes = malloc(new_node_block_size));
	int* buf_idx = blocks[num_blocks].buf_shrunk_idx = (int*)malloc(new_idx_block_size);

	if (!buf_nodes || !buf_idx)
	{
	    icxSetCARTError(cart , CART_MEMORY_ERROR ,
		"Memory allocation error" , CART_ERROR_STATUS_FATAL);
	    return FALSE;
	}
	//// Filling new block levels. Indices are at the end of the block, to shrink them if nessesary
	for (int i = old_depth + 1; i <= depth; i++)
	{
	    CxCARTLevel& level = node_storage.levels[i];
#ifdef _DEBUG
	    memset(buf_nodes, 0, (1 << i) * node_size);
	    memset(buf_idx, 0, num_samples * sizeof(int));
#endif
	    level.buf_nodes = buf_nodes;
	    buf_nodes += (1 << i) * node_size;
	    level.buf_shrunk_idx = buf_idx;
	    buf_idx += num_samples;
	    //			buf += level.size;
	}
	assert( buf_nodes - new_node_block_size == blocks[num_blocks].buf_nodes );
	assert( buf_idx - new_idx_block_size/sizeof(int) == blocks[num_blocks].buf_shrunk_idx );
	num_blocks++;
	old_depth = depth;
    }
    //// Decrease depth , deallocating old levels and blocks
    else if (depth < old_depth)
    {
	int* levels_per_block = node_storage.levels_per_block;
	int cur_block = num_blocks - 1;
	int cur_depth = old_depth - levels_per_block[cur_block];
	//// Freeng blocks that are 'higher' than new depth
	while ( cur_depth >= depth)
	{
	    assert(blocks[cur_block].buf_nodes && blocks[cur_block].buf_shrunk_idx);

	    free( blocks[cur_block].buf_nodes );
	    free( blocks[cur_block].buf_shrunk_idx );

	    blocks[cur_block].buf_nodes = NULL;
	    blocks[cur_block].buf_shrunk_idx = NULL;

	    levels_per_block[cur_block--] = 0;
	    assert( cur_block >= 0);
	    cur_depth -= levels_per_block[cur_block];
	}
	node_storage.num_blocks = cur_block + 1;
	assert( depth > cur_depth);
	int new_node_block_size = icxGetNodeBlockSize(cur_depth , depth , node_size);
	int new_idx_block_size = icxGetIdxBlockSize(cur_depth , depth , num_samples);

#ifdef WIN32
	assert(!blocks[cur_block].buf_nodes ||
	    ( (unsigned int)new_node_block_size <= _msize(blocks[cur_block].buf_nodes)) );
	assert(!blocks[cur_block].buf_shrunk_idx ||
	    ( (unsigned int)new_idx_block_size <= _msize(blocks[cur_block].buf_shrunk_idx)) );
#endif

	//// Reallocating lest block that is truncated, but not deleted
	VERIFY(realloc(blocks[cur_block].buf_nodes , new_node_block_size));
	VERIFY(realloc(blocks[cur_block].buf_shrunk_idx, new_idx_block_size));

	levels_per_block[cur_block] = depth - cur_depth;
	//// Zero out deleted levels
	memset (node_storage.levels + depth + 1 , 0 , sizeof(CxCARTLevel) * (old_depth - depth) );

	old_depth = depth;
    }
    return TRUE;
}

CART_IMPL void cxInitSplitBuffer(CxCART* cart, void* buf)
{
    int eff_num_features = cart->eff_num_features;
    CxCARTSplit* split = (CxCARTSplit*)buf;
    char* split_ptr = (char*)(split + eff_num_features);

    for (int i = 0 ; i < eff_num_features ;i++, split++ )
    {
	int real_feature_idx = cart->features_back_corr[i];
	assert(real_feature_idx >= 0);
	split->feature_idx = real_feature_idx;
	split->next_competitor = split->next_surrogate = NULL;

	if (icxIsClassifierVarCategoric(real_feature_idx))
	{
	    split->boundary.ptr = split_ptr;
	    int n = cart->num_classes[real_feature_idx];
	    split_ptr += n;
	}
    }
}

CART_IMPL void cxAddNodeSplits(CxCART* cart , CxCARTNode* node ,
			       CxCARTSplit** splits , int num_splits,
			       int split_type, char** cur_ptr )
{
    char* split_area = (!cur_ptr || !(*cur_ptr)) ?
	icxGetSplitArea(cart , node , split_type) : *cur_ptr;
#ifdef _DEBUG
    char* old_split_area = split_area;
#endif
    assert( (long) split_area >= (long) ( node + 1 ) );
    for (int i = 0 ; i < num_splits ; i++)
    {
	CxCARTSplit* split = splits[i];
	CxCARTSplit* new_split = (CxCARTSplit*)split_area;
	split_area += sizeof(CxCARTSplit) ;
	memcpy( new_split , split , sizeof(CxCARTSplit) );
	int feature_idx = split->feature_idx;
	int n = cart->num_classes[feature_idx];
	assert( n >= 0 && n < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);

	if (icxIsClassifierVarCategoric(cart->feature_type[feature_idx]))
	{
	    new_split->boundary.ptr = new_split + 1;
	    memcpy ( new_split->boundary.ptr , split->boundary.ptr , n);
	    split_area += n;
	}
	new_split->next_competitor = new_split->next_surrogate = NULL;
	splits[i] = new_split;
    }
#ifdef _DEBUG
    if (node->split)
    {
	assert(node->split->weight > -EPS);
	assert((unsigned)node->split->revert < 2);
    }
#endif
    assert( (long)old_split_area + icxGetSplitAreaSize(cart , split_type) >= (long)split_area);
    if (cur_ptr && split_type != SPLIT_MANUAL)
	*cur_ptr = split_area;
}

CART_IMPL BOOL cxCreateCalcStorage(CxCART* cart, int num_samples, int start_depth,
				   int offset_root )
{
    CxCARTStorage* storage = cart->storage;
    int eff_num_features = cart->eff_num_features;

    int max_depth = storage->node_storage.max_depth - start_depth;
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    ALIGN_NUM_RESP(num_resp_aligned )

#if CART_ALIGN_MEMORY
	int add_align = (regression ? 0 : CART_ELEM_ALIGN );
#else
    int add_align = 0;
#endif

    int num_numeric_vars = cart->eff_num_features - storage->num_categoric_vars;
    //// For fallen_idx
    int size_fallen_idx_per_case = eff_num_features * sizeof(int);
    if (!cart->params->mem_economy_mode)
	size_fallen_idx_per_case += num_numeric_vars * sizeof(CxClassifierVar);
    int size_fallen_idx = num_samples * size_fallen_idx_per_case;
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    //// For cat_proportions
    int size_cat_proportions = (regression ? storage->total_num_cats : num_resp_aligned * storage->total_num_cats);
    int total_size_cat_proportions = (( max_depth + 2 ) * size_cat_proportions + 1 ) * sizeof(int) ;

    CxCARTCalcStorage& calc_storage = storage->calc_storage;
    calc_storage.size_cat_proportions = size_cat_proportions;
    calc_storage.size_fallen_idx = size_fallen_idx;
    calc_storage.size_fallen_idx_per_case = size_fallen_idx_per_case;

    int size = 2 * size_fallen_idx + total_size_cat_proportions + add_align;
    int buf_split_size = eff_num_features * sizeof(CxCARTSplit) + storage->total_num_cats * sizeof (char); // For  splits

    size += buf_split_size;
    size += (num_samples + 1) * sizeof(CxClassifierVar) +
	(num_samples + 1) * sizeof(float); // for temporary buffers

    char* buf;
    calc_storage.buf_fallen_idx = buf = (char*)realloc(calc_storage.buf_fallen_idx , size);
#ifdef _DEBUG
    char* oldbuf = buf;
#endif
    if (!buf)
    {
	icxSetCARTError(cart , CART_MEMORY_ERROR ,
	    "Memory allocation error" , CART_ERROR_STATUS_FATAL);
	return FALSE;
    }
    buf += 2 * size_fallen_idx;
#if CART_ALIGN_MEMORY
    buf = icxAlignUp(buf , CART_MEM_ALIGN);
#endif
    calc_storage.buf_cat_proportions = (int*)buf;
    buf += total_size_cat_proportions;

    //// Preparing buffer for splits of any type
    calc_storage.buf_splits = buf;
    cxInitSplitBuffer(cart , buf);
    buf += buf_split_size;
    calc_storage.buf_feat_vals = (CxClassifierVar*)buf;
    calc_storage.buf_resp = (float*)(calc_storage.buf_feat_vals + (num_samples + 1));
    //	calc_storage.buf_inv = calc_storage.buf_resp + (num_samples + 1);
    //// All max_depth + 2 cat_proportion blocks are set to free
    calc_storage.cat_prop_top = 0;
    for (int i = 0 ; i < max_depth + 2 ; i++)
	calc_storage.free_blocks[i] = i + 1;
    calc_storage.max_depth = max_depth;
    calc_storage.num_root_fallens = num_samples;
    calc_storage.offset_root = offset_root;
    calc_storage.mem_economy_mode = cart->params->mem_economy_mode;
#ifdef _DEBUG
    buf += (num_samples + 1) * sizeof(CxClassifierVar) +
	(num_samples + 1) * sizeof(float);
    assert ( buf - size == oldbuf);
#endif
    /*	calc_storage.buf_inv[0] = 0.0;
    for (int i1 = 1 ; i1 < num_samples ; i1++)
    calc_storage.buf_inv[i1] = 1.0f/i1;*/
    return TRUE;
}

CART_IMPL void cxAllocNodeInternalsFromStorage(CxCART* cart , CxCARTNode* node, BOOL clear)
{
    CxCARTCalcStorage& calc_storage = cart->storage->calc_storage;
    int num_fallens = node->num_fallens;
    int eff_num_features = cart->eff_num_features;
    int num_resp = cart->num_response_classes;
    int num_resp_aligned = num_resp;
    BOOL regression = icxIsClassifierVarNumeric(cart->response_type);
    ALIGN_NUM_RESP(num_resp_aligned )

	//// Choose "left" or "right" size of fallen_idx buffer
	char* bp = ((char*)calc_storage.buf_fallen_idx) + (node->depth & 1) * calc_storage.size_fallen_idx ;
    int real_offset = node->offset - calc_storage.offset_root;
    assert( real_offset >= 0 && real_offset <= calc_storage.num_root_fallens ) ;
    assert(node->num_fallens <= calc_storage.num_root_fallens);

    bp += real_offset * (calc_storage.size_fallen_idx_per_case);
#ifdef _DEBUG
    char* old_bp = bp;
#endif
    //// Fallen_idx - from the beginning
    node->fallen_idx = (int*)bp;
    //// Pass by node->fallen_idx , allocate feature values from bp afterwards
    bp += (num_fallens * eff_num_features * sizeof(int));

    //// Must be aligned
    int* cat_proportions = icxAllocateNodeCatProportions(calc_storage , node);
    int* _cat_proportions = cat_proportions;
#if CART_ALIGN_MEMORY
    if (!regression)
	icxAssertAligned( cat_proportions );
#endif
    int mem_economy_mode = cart->params->mem_economy_mode;
    //// Set pointers , alignment is not nessesary nessesary
    for (int i = 0 ; i < eff_num_features ; i++)
    {
	int real_i = cart->features_back_corr[i];
	BOOL categoric = icxIsClassifierVarCategoric(cart->feature_type[real_i]);
	if (categoric)
	{
	    node->cat_proportions[i] = _cat_proportions;
	    int num_classes = cart->num_classes[real_i];
	    assert( num_classes >= 0 && num_classes < CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK);
	    if (regression)
	    {
		_cat_proportions += num_classes;
	    }
	    else
	    {
		_cat_proportions += (num_classes * num_resp_aligned) ;
	    }
	}
	else if (!mem_economy_mode)
	{
	    node->cat_proportions[i] = (int*)bp;
	    bp += num_fallens * sizeof(CxClassifierVar);
	}
	else
	    node->cat_proportions[i] = NULL;

    }

    int size_cat_proportions = calc_storage.size_cat_proportions;

    assert(bp - calc_storage.size_fallen_idx_per_case * num_fallens == old_bp);
    assert(_cat_proportions - size_cat_proportions == cat_proportions);
    //// Clear category proportions or response frequencies
    if (clear)
	memset(	cat_proportions , 0 , size_cat_proportions * sizeof(int));
}

CART_IMPL BOOL cxCreatePruningStorage(CxCART* cart)
{
    CxCARTPruningStorage& pruning_storage = cart->pruning_storage;
    cart->progress_info.progress_data_size = pruning_storage.size = 0;
    int size = pruning_storage.max_size = (cart->num_nodes + 3 ) / 2;
    /*  Pruning storage can accomodate approximately half of number of tree nodes
    of pruning steps and corrsponding pruned nodes pointers */
    cart->progress_info.progress_data =
	(CxProgressData*)(pruning_storage.buf =
	(CxPruningData*)realloc(pruning_storage.buf ,
	size * (sizeof(CxPruningData) + sizeof(CxCARTNode**))));
    if (!pruning_storage.buf)
    {
	icxSetCARTError(cart , CART_MEMORY_ERROR ,
	    "Memory allocation error" , CART_ERROR_STATUS_FATAL);
	return FALSE;
    }
    pruning_storage.pruned_nodes = (CxCARTNode**)(pruning_storage.buf + size);
    return (pruning_storage.buf != NULL);
}

CART_IMPL BOOL cxCreateCrossValStorage(CxCART* cart , int V)
{
    CxCARTPruningStorage& pruning_storage = cart->pruning_storage;
    CxCARTCrossValStorage& crval_storage = cart->cross_val_storage;
    crval_storage.V = V;
    assert(pruning_storage.size > 0 && pruning_storage.max_size >= pruning_storage.size);
    // For pointers to pruning sequences and for their sizes
    int size_add = sizeof(void*) * V + sizeof(int) * V;
    crval_storage.seq_ptrs = (CxPruningData**)realloc(crval_storage.seq_ptrs , size_add);
    crval_storage.sizes = (int*) ((char*)crval_storage.seq_ptrs + sizeof(void*) * V );
    memset ( crval_storage.seq_ptrs  , 0 , size_add);

    /*  Allocating pruning sequences twice longer than for main CART,
    hoping this will be enough */
    crval_storage.max_size = pruning_storage.size * V * 2 ;
    int size = crval_storage.max_size * sizeof(CxPruningData) ;
    crval_storage.buf = (CxPruningData*)realloc(crval_storage.buf , size);
    if (!crval_storage.buf)
    {
	icxSetCARTError(cart , CART_MEMORY_ERROR ,
	    "Memory allocation error" , CART_ERROR_STATUS_FATAL);
	return FALSE;
    }
    return TRUE;
}

CART_IMPL int cxCalcNodeNum( CxCARTNode* node)
{
    assert(node);
    CxCARTNode* parent_node = node->parent;
    if (!parent_node)
	return 1;
    int add = (node == parent_node->child_left) ? 0 : 1;
    return (cxCalcNodeNum(parent_node) << 1) + add;
}

#ifdef CART_WITH_GUI
CART_IMPL void InitCARTCommObj(HANDLE hStopEvent , HWND hwndMain)
{
    g_hStopEvent = hStopEvent;
    g_hwndMain = hwndMain;
}
#endif

#ifdef _DEBUG
CART_IMPL void cxAssertNodeValid(CxCART* cart , CxCARTNode* node )
{
    CxCARTNodeStorage& node_storage = cart->storage->node_storage;
    int depth = node->depth;

    assert(!node->child_left || node->child_left->parent == node);
    assert(!node->child_right || node->child_right->parent == node);
    assert ( icxGetNodePos(node_storage , node->id , depth) == node );
    BOOL is_right = node->id & 1;
    int num_fallens = node->num_fallens;
    if (node->parent == 0)
    {
	assert(cart->root == node);
	assert(node->offset == 0);
    }
    else
    {
	assert(node->parent);
	int offset = node->parent->offset ;
	if (is_right)
	    offset += (node->parent->num_fallens - num_fallens ) ;
	assert( node->offset == offset);
    }
    CxCARTLevel& level = node_storage.levels[depth];

    assert( (uintptr_t)node - (uintptr_t)level.buf_nodes < unsigned( (1 << depth) * node_storage.node_size));
    assert( node->shrunk_fallen_idx == 0 || node->shrunk_fallen_idx == level.buf_shrunk_idx + node->offset );
    int num_resp = icxGetVarNumStates(cart->response_type);
    int total = 0;
    int i;
    for (i = 0 ; i < num_resp ; i++)
    {
	int stat = node->fallen_stats[i];
	assert( stat >= 0 && stat <= num_fallens);
	total += stat;
    }
    //	assert( total == num_fallens);
    for (i = 0 ; i < cart->eff_num_features ; i++)
    {
	int missed = node->num_missed[i];
	assert( missed >= 0 && missed <= num_fallens);
    }
}
#endif
CART_IMPL CxProgressData* cxFindOptimalStep(CxClassifier* cfer,
					    BOOL use_1SE_rule)
{
    CV_FUNCNAME( "cxFindOptimalStep" );
    __BEGIN__;
    int size = cfer->progress_info.progress_data_size;
    int step = cfer->progress_info.progress_data_step;
    char* data = (char*)cfer->progress_info.progress_data;
    if (size == 0)
	return NULL;
    assert(data);
    CxProgressData* data_best = (CxProgressData*)data;
    char* _data = data;
    for (int i = size ; i-- ; _data += step)
    {
	CxProgressData* _data1 = (CxProgressData*)_data;
	if (_data1->test_sample_error < data_best->test_sample_error)
	    data_best = _data1;
    }
    if (!use_1SE_rule)
	return data_best;
    double var = data_best->error_dev ;
    _data -= step;
    for (int i1 = size ; i1-- ; _data -= step)
    {
	CxProgressData* _data1 = (CxProgressData*)_data;
	if (_data1->test_sample_error <= data_best->test_sample_error + var)
	{
	    return _data1;
	}
    }
    return data_best;
    __CLEANUP__;
    __END__;
}

CART_IMPL FILE* fopen_cart(char* name, char* attr)
{ return fopen(name, attr); }

CART_IMPL int close_cart(FILE* file)
{ return fclose(file); }

/* end of file */
