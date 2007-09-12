#ifndef _CXCART_H_
#define _CXCART_H_

#include "inlines.h"
//#include "cx.h"

void sort_fallenidx_byvar32f( int[], int len, CxClassifierVar* );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxReleaseClassifierSample
//    Purpose: releases sample
//    Context:
//    Parameters:
//      sample - sample
//    Returns:
//    Notes:
//F*/
CART_API void cxReleaseClassifierSample( CxClassifierSample **sample );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCreateClassifierSampleHeader
//    Purpose: creates CxClassifierSample in its simplest form
//    Context:
//    Parameters:
//      train_data - matrix of size SxF or FxS where F is number of features
//            and S is number of samples
//      tflag - if tflag is zero train_data has size of FxS.
//      train_responses - row or column matrix full with responses for corresponding samples
//      type_mask - row or column matrix of type CX_32SC1 and of length F+1.
//            Its members correspond to types of features
//            and (the last member) the type of response.
//            Types are coded as follows:
//            CX_CLASSIFIER_VAR_TYPE_NUMERIC_32F - numeric float
//            CX_CLASSIFIER_VAR_TYPE_CATEGORIC_32F( k ) - categoric in range 0..k-1 coded as
//                                                        a float with zero fractional part
//            CX_CLASSIFIER_VAR_TYPE_CATEGORIC_TRANSCENDENT - categoric that may take
//                                                            arbitrary float values
//            ..for complete list please see section with corresponding #defines
//      missed_mask - matrix of type CX_8UC1 and sized equally with train_data.
//            nonzero values indicate missing of corresponding features.
//            pointer to this matrix may be zero -- it defaults to zero matrix
//      indices_of_interest - row or column matrix of type CX_32SC1.
//            It will hold sequence of numbers of samples that are to process.
//            pointer to this matrix may be zero -- it defaults to
//            all samples to be of interest
//    Returns:
//      tuned header CxClassifierSample
//    Notes:
//F*/
CART_API CxClassifierSample* cxCreateClassifierSampleHeader( CvMat *train_data, int tflag,
                                                             CvMat *train_responses,
                                                             CvMat *type_mask,
                                                             CvMat *missed_mask,
                                                             int* samples_of_interest,
															 int num_samples_of_interest );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxReleaseClassifier
//    Purpose: releases classifier
//    Context:
//    Parameters:
//      cfer - classifier
//    Returns:
//    Notes:
//F*/
CART_API void cxReleaseClassifier( CxClassifier** cfer );


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCARTTrainParams
//    Purpose: creates CxCARTTrainParams structure
//    Context:
//    Parameters:
//      features_of_interest - row or column matrix of type CX_32SC1.
//            It is to hold sequence of numbers of features that are to process.
//            pointer to this matrix may be zero -- it defaults
//            to all features to be of interest
//      response_classes - not mandatory row or column matrix of type CX_32FC1
//            It is to hold possible response classes
//      priors - not mandatory row or column matrix of type CX_32FC1 holding prior weights
//            prescribed to response classes
//      splitting_rule - splitting rule
//      num_competitors - maximal number of competitors to be computed for each node
//            if zero -- suppress computation of competitors
//      competitor_threshold - only splits which 'goodness' exceed
//            this fraction of 'goodness' of the best split are accepted
//      num_surrogates - maximal number of surrogates to be computed for each node
//            if zero -- suppress computation of surrogates
//      surrogate_threshold - only surrogates which 'goodness' exceed
//            this fraction are accepted (goodness varies in diapason 0..1)
//      tree_max_depth - maximal depth of classification and regression tree
//            zero stands for unbounded
//      split_min_points - nodes with number of samples less than this threshold
//            may not be splitted any further
//      split_max_points - use not more than that amount of points
//            zero defaults to unbounded behavior
//    Returns:
//      CxCARTTrainParams structure
//    Notes:
//F*/
CART_API CxCARTTrainParams* cxCARTTrainParams( CvMat *features_of_interest,
                                               CvMat *priors,
                                               CxCARTSplitCriterion splitting_rule,
                                               int num_competitors,
                                               float competitor_threshold,
                                               int num_surrogates,
                                               float surrogate_threshold,
                                               int tree_max_depth,
                                               int split_max_points,
											   int split_min_points,
											   int max_clusters,
											   int mem_economy_mode,
                                               int size = sizeof (CxCARTTrainParams) );

//////// Frees train params ///////////////////////////////////
CART_API void cxReleaseTrainParams( CxCARTTrainParams* params);


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCARTSplit
//    Purpose: creates CxCARTSplit header
//    Context:
//    Parameters:
//      cart - main CART structure
//      feature_idx - index of split feature
//      use_calc_storage - whether to allocate from split buffer in CART calculation storage.
//    Returns:
//      CxCARTSplit structure
//    Notes:
//F*/
CART_API CxCARTSplit* cxCARTSplit( struct CxCART* cart, 
								   int feature_idx, BOOL use_calc_storage = TRUE );

CART_API CxCARTSplit* cxCARTBaseSplit( struct CxCARTBase* cart, 
								       int feature_idx );

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxSortFallenByResponse
//    Purpose: Sorts node's fallen_idx strings for categoric variables
//             on response in classification
//    Parameters:
//      cart - CART
//      node - a node (root only)
///////////////////////////////////////////////////////////////////////////////////////
CART_API void cxSortFallenByResponse(CxCART* cart, CxCARTNode* node);


///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxExpandNodeIdx
//    Purpose: copies a node
//    Parameters:
//      cart - CART
//      node - a node to copy
//      use_calc_storage - whether to use CART internal storage
//        for node internal arrays
///////////////////////////////////////////////////////////////////////////////////////
CART_API BOOL cxExpandNodeIdx(CxCART* cart, CxCARTNode* node , 
							  BOOL use_calc_storage = TRUE);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxShrinkNodeIdx
//    Purpose: copies a node
//    Parameters:
//      cart - CART
//      node - a node to copy
//      use_calc_storage - whether CART internal storage
//        was used to allocate node internal arrays
///////////////////////////////////////////////////////////////////////////////////////
CART_API void cxShrinkNodeIdx(CxCART* cart, CxCARTNode* node, 
							  BOOL use_calc_storage = TRUE);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCopyNode
//    Purpose: copies a node
//    Parameters:
//      node - a node to copy
//    Returns :
//      New copied node 
///////////////////////////////////////////////////////////////////////////////////////
CART_API CxCARTNode* cxCopyNode(CxCARTNode* node);


///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxMergeClusters
//    Purpose: merges two clusters
//    Parameters:
//      cluster1 - first cluster 
//      cluster2 - first cluster 
//      num_resp - number of response classes
//    Returns:
//
///////////////////////////////////////////////////////////////////////////////////////
CART_API void cxMergeClusters(CxVarCategoryCluster* cluster1, 
							  CxVarCategoryCluster* cluster2, 
							  int num_resp);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxClusterizeFeatureAgglomerative
//    Purpose: clusterizes a feature using agglomerative clustering
//    Parameters:
//      cart - main CART structure
//      node - node 
//      feature_idx - index of split feature
//      max_clusters - maximum number of clusters
//    Returns :
//      pointer to CxVarCategoryCluster - first of desired clusters for feature.
//      they are connected to a double-linked list
//    Notes : after calling this function you must call cxFreeClusters 
//
///////////////////////////////////////////////////////////////////////////////////////
CART_API CxVarCategoryCluster* cxClusterizeFeatureAgglomerative(CxCART* cart,
											              	    CxCARTNode* node, 
										                        int feature_idx, 
												                int max_clusters);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxClusterizeFeatureKMeans
//    Purpose: clusterizes a feature using K-Means clustering
//    Parameters:
//      cart - main CART structure
//      node - node 
//      feature_idx - index of split feature
//      max_clusters - maximum number of clusters
//      transform_freq - flag indicating whether to transform response frequencies
//         for better resolution.
//      beta - used when transform_freq = TRUE , transformation parameter
//    Returns :
//      pointer to CxVarCategoryCluster - first of desired clusters for feature.
//      they are connected to a double-linked list
//    Notes : after calling this function you must call cxFreeClusters 
//
///////////////////////////////////////////////////////////////////////////////////////
CART_API CxVarCategoryCluster* cxClusterizeFeatureKMeans(CxCARTBase* cart,
											             int* cat_proportions,  
										                 int feature_idx, 
												         int max_clusters,
														 BOOL transform_freq = FALSE,
														 float beta = 2.0);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcCatProportions
//    Purpose: calculates proportions for categories ( how much strings fall
//      into given category of given categoric variable.
//    Parameters:
//      clusters - cluster array
//      cart - main CART structure
//      node - node 
//      feature_idx - index of split feature
//    Returns :
//    Notes : 
///////////////////////////////////////////////////////////////////////////////////////
CART_API void cxCalcCatProportions(int* cat_proportions, 
							       CxCART* cart, CxCARTNode* node, 
								   int feature_idx);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcFrequencies
//    Purpose: calculates frequencies for categories
//    Parameters:
//      clusters - cluster array
//      cart - main CART structure
//      node - node 
//      feature_idx - index of split feature
//    Returns :
//    Notes : 
///////////////////////////////////////////////////////////////////////////////////////
CART_API void cxCalcFrequencies(int* frequencies, 
								CxCART* cart, CxCARTNode* node, 
								int feature_idx);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFreeClusters
//    Purpose: frees cluster list : cluster->first_cat,
//    frequencies for all clusters and clusters itself
//    Parameters:
//      clusters - pointer to first cluster 
//    Returns:
///////////////////////////////////////////////////////////////////////////////////////
CART_API void cxFreeClusters(CxVarCategoryCluster* clusters);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    DumpClusters
//    Purpose: dumps cluster list info to file
//    Parameters:
//      clusters - pointer to first cluster 
//    Returns:
CART_API void DumpClusters(FILE* file, CxVarCategoryCluster* clusters, int num_resp);

///// Determines if node is split or terminal /////////////
CART_INLINE BOOL icxIsNodeSplit(CxCARTNode* node ) 
{
	assert(node);
	return (node->child_left != 0); 
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxBuildCART
//    Purpose: builds Classification And Regression Tree
//    Context:
//    Parameters:
//      sample - train sample
//      params - train params
//      ext_info - external info about table, helps to avoid sorting
//    Returns:
//      CxCART structure
//    Notes:
//F*/
CART_API CxCART* cxBuildCART( CxClassifierSample* sample, 
							  CxCARTTrainParams* params,
                              CxExtCARTInfo* ext_info = NULL);


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxBuildVirginCART
//    Purpose: builds trivial Classification And Regression Tree with one node
//    Context:
//    Parameters:
//      sample - train sample
//      params - train params
//      ext_info - external info about table, helps to avoid sorting
//    Returns:
//      CxCART structure
//    Notes:
//F*/
CART_API CxCART* cxBuildVirginCART( CxClassifierSample* sample,
								    CxCARTTrainParams* params,
                         			CxExtCARTInfo* ext_info = NULL);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxInitCARTBase
//    Purpose: creates empty CART and initializes its base internal data
//    Context:
//    Parameters:
//      num_features - number of features
//      feature_type - feature types array
//		response_type - response type
//      params - train params for construction
//      cart - if NULL , new CxCART structure is allocated,
//        otherwise this parameter is initialized and returned
//      mem_economy_mode - memory economy mode , if TRUE , numeric
//        feature values are not copied to each node.
//    Returns:
//      CxCART structure
//    Notes:
//F*/
CART_API BOOL cxInitCARTBase(CxCARTBase* cart,
							 int num_features, 
			  				 int* feature_type,
				     		 int response_type,
							 CxCARTTrainParams* params);
				
/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCreateCART
//    Purpose: creates empty CART and initializes its internal data
//    Context:
//    Parameters:
//      num_features - number of features
//      feature_type - feature types array
//		response_type - response type
//      params - train params for construction
//      cart - if NULL , new CxCART structure is allocated,
//        otherwise this parameter is initialized and returned
//      mem_economy_mode - memory economy mode , if TRUE , numeric
//        feature values are not copied to each node.
//    Returns:
//      CxCART structure
//    Notes:
//F*/
CART_API CxCART* cxCreateCART(int num_features, 
					          int* feature_type,
				     	      int response_type,
					          CxCARTTrainParams* params,
					          CxCART* cart = NULL);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxReleaseCART
//    Purpose: releases Classification And Regression Tree
//    Context:
//    Parameters:
//      cart - CART
//    Returns:
//    Notes:
//F*/
CART_API void cxReleaseCART( CxCART** cart );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxBuildCARTSubj
//    Purpose: builds internal stuff for CART node
//    Context:
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree, zero defaults to root
//      sample - train sample
//      expand_idx - whether to build expanded fallen_idx in node
//      copy_features - whether we should copy features from sample to internal buffers
//      max_num_samples - upper limit for number of sample to use
//      gulp_chunk - whether we should check max_num_samples only on chunk boundaries
//    Returns:
//      FALSE if out of memory , TRUE otherwise
//    Notes:
//F*/
CART_API BOOL cxBuildCARTSubj( CxCART* cart, CxCARTNode* node , 
                               CxClassifierSample* sample,
							   BOOL expand_idx = TRUE,
							   BOOL use_calc_storage = TRUE , 
                               int copy_features = 0,
                               int max_num_samples = 16000,
                               int gulp_chunk = 1);
//###
CART_API void cxBuildCARTNodeFallenStats( CxCARTBase* cart, CxForestNode* node );

CART_INLINE void icxBuildCARTNodeFallenStats( CxCART* cart, CxCARTNode* node )
{
	cxBuildCARTNodeFallenStats((CxCARTBase*)cart, (CxForestNode*)node);
}

//###
CART_API void cxCreatePriorsCostMult(CxCARTBase* cart, int* root_stats, int root_fallens);
//###
CART_API void cxInitSubjBody(CxCARTSubj* subj, 
							 CxClassifierSample* sample, 
							 int copy_features = 0,
                             int max_num_samples = 16000,
                             int gulp_chunk = 1);


#ifdef _DEBUG
CART_API void cxCheckNodeFallenIdx(CxCART* cart, CxCARTNode* node);
#endif

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxBuildNodeFallenIdxFromExtraInfo
//    Purpose: builds fallen_idx array for a node from external info
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree, zero defaults to root
//F*/
CART_API void cxBuildNodeFallenIdxFromExtraInfo( CxCART* cart,
											     CxCARTNode* node);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    icxBuildCARTNodeMasterFallenIdx
//    Purpose: builds fallen_idx array for a node from subj ( takes longer )
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree, zero defaults to root
//F*/
CART_API void cxBuildNodeFallenIdxFromScratch( CxCART* cart,
			  							       CxCARTNode* node);


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    icxBuildCARTNodeMasterFallenIdx
//    Purpose: builds fallen_idx array for a node
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree, zero defaults to root
//      expand_idx - create with expanded idx array ( for best split and surrogate search)
//      use_calc_storage  -whether to use CART internal storage to allocate
//         node internal arrays (needed for MART).
//F*/
CART_API void cxBuildCARTNodeMasterFallenIdx( CxCART* cart,
			  							      CxCARTNode* node, 
											  BOOL expand_idx = TRUE,
											  BOOL use_calc_storage = TRUE);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxPropagateCARTSubj
//    Purpose: propagates internal stuff from parent to childs
//    Context:
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      shrink_idx - whether to shrink fallen_idx arrays on propagation 
//      use_calc_storage - whether to use CART internal storage to allocate
//         node internal arrays (needed for MART).
//    Returns:
//    Notes:
//F*/
CART_API void cxPropagateCARTSubj( CxCART* cart, 
								   CxCARTNode* node,
								   BOOL shrink_idx = FALSE ,
								   BOOL use_calc_storage = TRUE);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxBuildCARTNodeResponse
//    Purpose: propagates internal stuff from parent to childs
//    Context:
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//////////////////////////////////////////////////////////////////////////////////////////
CART_API void cxBuildCARTNodeResponse( CxRootedCARTBase* cart, CxForestNode* node );

CART_INLINE void cxBuildCARTNodeResponse( CxCART* cart, CxCARTNode* node )
{
	cxBuildCARTNodeResponse( (CxRootedCARTBase*)cart, (CxForestNode*)node);
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcNodeError
//    Purpose: Calculates node error, R(t) = r(t) * p(t)
//    Parameters:
//      cart - classification and regression tree
//      node - node
//    Returns:
//      Node error
//F*/
CART_API double cxCalcNodeError(CxCART* cart, CxCARTNode *node);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxSplitCARTLeaf
//    Purpose: make elementary split at a leaf
//    Context:
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      split - optional split, zero defaults to the best split found
//      not_shrink_idx - flac indicating whether to shrink node 
//       internal arrays or not (used bu MART).
//    Returns:
//      TRUE if found a split
//    Notes:
//F*/
CART_API BOOL cxSplitCARTLeaf( CxCART* cart,
							   CxCARTNode* node,
                               CxCARTSplit* split = NULL,
							   BOOL not_shrink_idx = FALSE);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxSplitCARTLeafFull
//    Purpose: recursively splits node  
//    Parameters:
//      cart - classification and regression tree
//      node - node to split
//      split - split to perform ; NULL means best split 
//      not_shrink_idx - whether to shrink root node's internal array
//        (used bu MART)
//    Returns:
//      TRUE if it did anyfhing
//F*/

CART_API BOOL cxSplitCARTLeafFull (CxCART* cart, 
								   CxCARTNode* node, 
								   CxCARTSplit* split = NULL,
								   BOOL not_shrink_idx = FALSE);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxTrySplitLeaf
//    Purpose: makes "trial" split at a node
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      split - optional split . Must be created and initialized with cxCARTSplit.
//    Returns:
//      Temporary created node with two children
//    Notes:
//       Delete this node afterwards!
//F*/
CART_API CxCARTNode* cxTrySplitLeaf( CxCART* cart,
					        	     CxCARTNode* node,
									 CxCARTSplit* split);


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindCARTNodeGoodSplits
//    Purpose: computes list of good splits for a given node
//    Context:
//    Parameters:
//      cart - classification and regression tree
//      node - node of a tree
//      num_splits - maximal number of requested splits, zero suppresses computation
//      threshold - only splits which 'goodness' exceed
//            this fraction of 'goodness' of the best split are accepted
//    Returns:
//      Linked list of splits
//    Notes:
//      Stores found split in node competitor split storage 
//      
//F*/
CART_API CxCARTSplit* cxFindCARTNodeGoodSplits( CxCART* cart,
											    CxCARTNode* node,
                                                int num_splits = 1,
                                                float threshold = 0.8f);


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcBranchErrorAndComplexity
//    Purpose: recursively calculates branch and node errors and complexities for all 
//	  nodes and subbranches of a given branch
//    Parameters:
//      cart - classification and regression tree
//      sample - arbitrary sample compatible with given classifier
//      branch_error - error for given branch
//      terminal_nodes - terminal nodes in branch
//      prune_step - pruned subtree number, 0 means full tree
//    Returns:
//F*/

CART_API void  cxCalcBranchErrorAndComplexity(CxCART *cart, 
							  				  CxCARTNode *node, 
											  double& branch_error,
											  int& terminal_nodes,
											  int prune_step = 0);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcSampleNodeErrorAndDeviation
//    Purpose: Calculates sample node error and error deviation for all nodes
//    Parameters:
//      cart - classification and regression tree
//      sample - arbitrary sample compatible with given classifier
//    Returns:
//      
//F*/
CART_API void  cxCalcSampleNodeErrorAndDeviation(CxCART *cart, 
												 CxClassifierSample *sample);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcSampleBranchErrorAndDeviation
//    Purpose: Calculates sample branch error and error deviation for all nodes
//    Parameters:
//      cart - classification and regression tree
//      node - root node of the branch
//    Returns:
//      
//F*/
CART_API void  cxCalcSampleBranchErrorAndDeviation(CxCART *cart, CxCARTNode* node);


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    icxGetNextNode
//    Purpose: Iterates all nodes , parent always before child
//    Parameters:
//      node - node
//      prune_step - number of pruned subtree
//    Returns:
//      next node , parent node is always passed before child
//F*/
CART_API CxCARTNode* cxGetNextNode(CxCARTNode* node, int prune_step = 0);
CART_API CxForestNode* cxGetNextForestNode(CxForestNode* node); 

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    icxInitNode
//    Purpose: Initializes the node
//    Parameters:
//      node - node
//    Returns:
//F*/
CART_API void icxInitNode(CxCARTNode *node);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindWeakestLinks
//    Purpose: Finds weakest links in CART
//    Parameters:
//      cart - CART
//      weakest_links_list - weakest links list to fill, must be created before call
//      num_nodes - number of found weakest links is stored here
//      prune_step - pruning step number
//    Returns:
//      alpha (g) value
//F*/
CART_API float cxFindWeakestLinks(CxCART *cart, 
								  CxCARTNode** weakest_links_list,
								  int& num_nodes,
							      int prune_step = 0);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    icxPruneData
//    Purpose: Initializes the pruning data structure
//    Parameters:
//      data - pruning data
//      step - pruning step
//      alpha - complexity penalty
//      reestimation_error - reestimation error
//      terminal_nodes - number of terminal nodes
//      num_pruned_nodes - number of pruned nodes
//      pruned_seq - pruned nodes sequence
//    Returns:
//F*/
CART_API void  icxInitPruneData(CxPruningData* data,
								int step,                 
								float alpha,			
								double reestimation_error,
								int terminal_nodes,
								int num_pruned_nodes , 
								CxCARTNode** pruned_seq);


/*F/////////////////////////////////////////////////////////////////////////
//    Name:    cxPrePruneTree
//    Purpose: Prunes branches with no decrease of error (i.e node error = branch error)
//    Parameters:
//      cart - classification and regression tree
//      nodes_seq - sequence of nodes to prune
//    Returns:
//      Number of pruned nodes
//F*/
CART_API int cxPrePruneTree(CxCART *cart, CxCARTNode** nodes_seq);


/*F/////////////////////////////////////////////////////////////////////////
//    Name:    cxPruneNodes
//    Purpose: marks nodes as pruned and recalculates branch errors and
//    terminal nodes number
//    Parameters:
//      nodes_seq - sequence of nodes to prune
//      num_nodes - number of nodes in nodes_seq
//      prune_step - current pruning step
//    Returns:
//F*/
CART_API void cxPruneNodes(CxCARTNode** nodes_seq,
						   int num_nodes , 
						   int prune_step);

/*F/////////////////////////////////////////////////////////////////////////
//    Name:    cxPruneNodes
//    Purpose: recalculates branch sample errors and error deviation 
//    when performing tree ascention. 
//    Parameters:
//      data - pruning data
//F*/
CART_API void cxRecalcSampleErrors( CxPruningData* data);

/*F/////////////////////////////////////////////////////////////////////////
//    Name:    cxPruneCART
//    Purpose: prunes CART , creating sequence of subtrees
//    Context:
//    Parameters:
//      cart - classification and regression tree
//F*/
CART_API void cxPruneCART(CxCART *cart );

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCARTEvalFeatures
//    Purpose: Gives response of a pruned subtree on a string of features
//    Parameters:
//      cart - classification and regression tree
//      features - pointer to features
//      pruning_step - pruned subtree number
//      step - optional step of features disposition
//      missed_mask - optional pointer to array-mask of missed parameters
//            nonzero values indicate miss
//      missed_mask_step - optional step of missed_mask disposition
//    Returns:
//      Response
//////////////////////////////////////////////////////////////////////////////////////
CART_API CxClassifierVar cxCARTEvalFeatures( CxCART* cart, 
                                             void* features,
										     int pruning_step,
                                             int features_step CV_DEFAULT(sizeof( CxClassifierVar )),
                                             char* missed_mask CV_DEFAULT(0),
                                             int missed_mask_step CV_DEFAULT(1) );

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcSampleErrorAndPrediction
//    Purpose: Calculates prediction error for given sample and predictions for it.
//    Parameters:
//      cart - classification and regression tree
//      sample - sample for error estimation
//      pruning_step - pruned subtree number
//      prediction - receives predictions if not NULL
//      err_deviation - receives rror deviation if not NULL
//    Returns:
//      Prediction error
//F*/
CART_API double cxCalcCARTSampleErrorAndPrediction( CxCART *cart, 
											    CxClassifierSample* sample,
											    int pruning_step,
											    float* prediction = NULL,
											    double* err_deviation = NULL);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcTestErrorsAndPredictions
//    Purpose: Calculates prediction error for given sample and predictions for it
//             on each pruning step.
//    Parameters:
//      cart - classification and regression tree
//      sample - sample for error estimation
//      prediction - receives predictions if not NULL
//    Returns:
//      Prediction error
//F*/
CART_API void cxCalcCARTTestErrorsAndPredictions( CxCART* cart, 
			  			     				      CxClassifierSample* sample,
							    			      float* prediction,
												  BOOL calc_err = TRUE);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcCARTTestErrors
//    Purpose: Calculates test errors for pruned subtrees  
//    Parameters:
//      cart - classification and regression tree
//      sample - sample for error calculation
//    Returns:
//F*/
CART_API void cxCalcCARTTestErrors( CxCART *cart , CxClassifierSample* sample);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcCrossValidationErrors
//    Purpose:  Calculates cross - validation errors for pruned subtrees 
//    Parameters:
//      cart - classification and regression tree
//      sample - sample for error calculation
//    Returns:
//F*/
CART_API void cxCalcCrossValidationErrors( CxCART *cart , CxClassifierSample* sample , int V = 10);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindOptimalData
//    Purpose: Finds optimal pruned subtree when test errors are already calculated
//    Parameters:
//      pruning_storage - pruning storage that holds alpha - error sequence.
//      use_1SE_rule - use 1 SE Rule for selecting optimal subtree or not
//    Returns:
//      Pruned best subtree data
//F*/
CART_API CxPruningData* cxFindOptimalData(CxCARTPruningStorage& pruning_storage,
										  BOOL use_1SE_rule = TRUE);
//// Find optimal step for any classifier
CART_API CxProgressData* cxFindOptimalStep(CxClassifier* cfer,
									       BOOL use_1SE_rule);


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindBestSubtree
//    Purpose: Finds best pruned subtree 
//    Parameters:
//      cart - classification and regression tree
//      sample - sample for error calculation
//      use_1SE_rule - use 1 SE Rule for selecting optimal subtree or not
//    Returns:
//      Pruned best subtree data
//F*/
CART_API CxPruningData* cxFindBestSubtree( CxCART *cart ,
										   CxClassifierSample* sample, 
										   BOOL use_1SE_rule = TRUE);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindBestSubtreeCrossValidation
//    Purpose: Finds best pruned subtree with cross - validation 
//    Parameters:
//      cart - classification and regression tree
//      sample - sample for error calculation
//      V - number of cross - validation folds
//      use_1SE_rule - use 1 SE Rule for selecting optimal subtree or not
//    Returns:
//      Pruned best subtree data
//F*/
CART_API CxPruningData* cxFindBestSubtreeCrossValidation( CxCART *cart ,
														  CxClassifierSample* sample , 
														  int V = 10,
														  BOOL use_1SE_rule = TRUE);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcImpurityChangeAny
//    Purpose: computes inpurity change for any given split, not using expanded idx
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      split - split
//    Returns:
//      Impurity change
//F*/
CART_API float  cxCalcImpurityChangeAny(CxRootedCARTBase* cart, 
										CxForestNode* node,
										CxCARTSplit* split);	

CART_INLINE float  cxCalcImpurityChangeAny(CxCART* cart, 
										   CxCARTNode* node,
										   CxCARTSplit* split)
{
	return cxCalcImpurityChangeAny((CxRootedCARTBase*)cart, (CxForestNode*)node, split);
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindNumericSplitProportion
//    Purpose: computes number of strings going left on a given numeric split
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      split - split
//      left_nonmissing - if not NULL, function stores here the number of strings 
//         with non-missing feature going left
//    Returns:
//      Number of strings going left
//F*/
CART_API int cxFindNumericSplitProportion(CxCART *cart, 
			   							  CxCARTNode *node,
										  CxCARTSplit* split,
										  int* left_nonmissing = NULL);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindCategoricSplitProportion
//    Purpose: computes number of strings going left on a given categoric split
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      split - split
//      left_nonmissing - if not NULL, stores number of strings with nonmissed feature going left
//    Returns:
//      Number of strings going left
//F*/
CART_API int cxFindCategoricSplitProportion(CxCART *cart, 
			   				   			    CxCARTNode *node,
										    CxCARTSplit* split,
											int* left_nonmissing = NULL);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindAnySplitLeftNonmissing
//    Purpose: computes number of strings going left on a given split with non-missed split 
//             feature, acts with shrunk fallen_idx. Slower than two above functions
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      split - split
//      missed_total - receives total number of strings with missing
//         split feature, if not NULL
//      cur_missed_mask - array of size subj->size that is filled by 
//         0 and 1's , 1 when split feature is missing, if not NULL
//    Returns:
//      Number of strings going left
//F*/
CART_API int cxFindAnySplitLeftNonmissing(CxCARTBase* cart, 
			   				   		      CxForestNode* node,
									      CxCARTSplit* split,
										  int* not_missed_total = NULL,
										  char* cur_missed_mask = NULL);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindAnySplitProportion
//    Purpose: computes number of strings going left on a given split, acts with shrunk fallen_idx.
//             Slower than two above functions
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      split - split
//      left_nonmissing - if not NULL, stores number of strings with nonmissed feature going left
//    Returns:
//      Number of strings going left
//F*/
CART_API int cxFindAnySplitProportion(CxCART *cart, 
			   				   		  CxCARTNode *node,
									  CxCARTSplit* split,
									  int* left_nonmissing = NULL);


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindSplitAssoc
//    Purpose: computes association between two splits in a given node
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      split1 - first split
//      split2 - second split
//    Returns:
//      Predictive measure of association between splits
//F*/
CART_API float cxFindSplitAssoc(CxCART *cart, 
			   					CxCARTNode *node,
								CxCARTSplit* split1,
							  	CxCARTSplit* split2);


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindCARTNodeNumericSurrogate
//    Purpose: computes surrogate numeric split on a given feature for a given node
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      feature - feature to compute a surrogate for
//      split - split to store the result , must be created before.
//    Returns:
//      Predictive association of the surrogate stored in split.
//F*/
CART_API float cxFindCARTNodeNumericSurrogate( CxCART *cart,
											   CxCARTNode *node,
                                     		   int feature,
											   CxCARTSplit* split);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindCARTNodeCategoricSurrogate
//    Purpose: computes surrogate categoric split on a given feature for a given node
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      feature - feature to compute a surrogate for
//      split - split to store the result , must be created before.
//    Returns:
//      Predictive association of the surrogate stored in split.
//F*/
CART_API float cxFindCARTNodeCategoricSurrogate( CxCART *cart,
											     CxCARTNode *node,
                                     		     int feature,
											     CxCARTSplit* split);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxSelectNodeSurrogate
//    Purpose: selects a surrogate on given feature in the node if there are any
//    Parameters:
//      node - node of the tree
//      feature - feature to compute a surrogate for
//    Returns:
//      Selected surrogate in the node
//F*/
CART_API CxCARTSplit* cxSelectNodeSurrogate( CxCARTNode *node,
											 int feature);

									
/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindCARTNodeSurrogates
//    Purpose: computes surrogates for a given node
//    Parameters:
//      cart - classification and regression tree
//      node - node to calculate surrogates
//      num_surrogates - maximal number of surrogates to be computed
//            zero suppresses computation of surrogates
//      threshold - only surrogates which 'goodness' exceed
//            this fraction are accepted (goodness varies in diapason 0..1)
//    Returns:
//F*/
CART_API void cxFindCARTNodeSurrogates( CxCART *cart, 
									    CxCARTNode *node,
                                        int num_surrogates CV_DEFAULT(3),
                                        float threshold CV_DEFAULT(.6f));



/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcCARTVariableImportance
//    Purpose: computes variable importance for a CART
//    Parameters:
//      cart - classification and regression tree
//      feature - real feature to calculate variable importance for
//      pruning_step - on what pruning subtree variable importance is 
//          calculated, 0 means full tree. Usually optimal subtree must be used.
//    Returns:
//      Variable importance
//F*/
CART_API float cxCalcCARTVariableImportance(CxCART* cart,
										int feature, 
										int pruning_step = 0);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcVARTVariableImportances
//    Purpose: computes variable importances for a CART
//    Parameters:
//      cart - classification and regression tree
//      feature - real feature to calculate variable importance for
//      pruning_step - on what pruning subtree variable importance is 
//          calculated, 0 means full tree. Usually optimal subtree must be used.
//    Returns:
//F*/
CART_API void cxCalcCARTVariableImportances(CxCART* cart,
											float* imps,	
										    int pruning_step = 0);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindNodeNumericBestSplitRegression
//    Purpose: computes best numeric split on a given feature 
//             for a given node in regression mode
//    Parameters:
//      cart - classification and regression tree.
//      node - node of the tree.
//      feature - feature to compute the best split for.
//      split - split to store the result, must be created before
//              calling this function
//    Returns:
//      Weight of best split stored in split
//F*/
CART_API float cxFindNodeNumericBestSplitRegression( CxCART *cart,
													 CxCARTNode *node,
                                     				 int feature,
													 CxCARTSplit* split);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindNodeNumericBestSplit
//    Purpose: computes best numeric split on a given feature 
//             for a given node in classification mode
//    Parameters:
//      cart - classification and regression tree.
//      node - node of the tree.
//      feature - feature to compute the best split for.
//      split - split to store the result, must be created before
//              calling this function
//    Returns:
//      Weight of best split stored in split
//F*/
CART_API float cxFindNodeNumericBestSplitClassification( CxCART *cart,
													     CxCARTNode *node,
                                     				     int feature,
													     CxCARTSplit* split);
//###
CART_API int  cxScanNumericBestSplitGiniIndex( CxRootedCARTBase* cart,
#ifdef _DEBUG
											   CxForestNode* node, 
											   int feature, 
#endif
											   int* fallen_idx,
											   CxClassifierVar* feature_vals,
											   int num_not_missed,
											   int* fallen_stats_child,
											   double& best_weight);

CART_API int  cxScanNumericBestSplitEntropy( CxRootedCARTBase* cart,
										     int* fallen_idx,
										     CxClassifierVar* feature_vals,
										     int num_not_missed,
										     int* fallen_stats_child,
										     double& best_weight);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindNodeCategoricBestSplitRegression
//    Purpose: computes best categoric split on a given feature
//             for a given node for regression
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      feature - feature to compute a surrogate for
//      split - split to store the result, must be created before
//              calling this function
//    Returns:
//      Weight of best split stored in split
//F*/
CART_API float cxFindNodeCategoricBestSplitRegression( CxCART *cart,
													   CxCARTNode *node,
                                     				   int feature,
													   CxCARTSplit* split);

//###
CART_API float cxScanNodeCategoricBestSplitRegression( CxRootedCARTBase* cart,
													   int num_not_missed,
													   int* cat_proportion,
													   double* sum_proportions,
													   double sum_sq,
													   char* ptr, int n);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindNodeCategoricBestSplitClassification
//    Purpose: computes best categoric split on a given
//             feature for a given node for classification
//    Parameters:
//      cart - classification and regression tree
//      node - node of the tree
//      feature - feature to compute a surrogate for
//      split - split to store the result, must be created before
//              calling this function
//    Returns:
//      Weight of best split stored in split
//F*/
CART_API float cxFindNodeCategoricBestSplitClassification( CxCART *cart,
														   CxCARTNode *node,
                                     					   int feature,
														   CxCARTSplit* split);
//###
CART_API float cxScanNodeCategoricBestSplitClassification( CxRootedCARTBase* cart,
														   int* fallen_stats_proportions,
														   char* ptr, int n);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFindNodeCategoricBestSplit
//    Purpose: computes best categoric split on a given feature for a given node
//    Parameters:
//      cart - classification and regression tree
//		clusters - clusterization data
//      num_clusters - number of clusters
//      node - node of the tree
//      feature - feature to compute a surrogate for
//      split - split to store the result, must be created before
//              calling this function
//    Returns:
//      Weight of best split stored in split
//F*/
CART_API float cxFindNodeCategoricClusteringBestSplit( CxRootedCARTBase* cart,
													   CxVarCategoryCluster* clusters,
													   int num_clusters , 
													   int num_not_missed,
                                     		           int feature,
											           CxCARTSplit* split);

////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxGetBranchWeight
//    Purpose: calculates the number of nodes in a branch
//    Parameters:
//		node - root node of the branch
//    Returns:
//      Number of nodes in the branch
//    Notes:
//      Recursive
////////////////////////////////////////////////////////////////////////////////////
CART_API int cxGetBranchWeight(CxCARTNode* node);

/*F////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxAllocNodeCatProportions
//    Purpose: allocates node internal cat_proportions arrays
//    Parameters:
//      cart - classification and regression tree
//		node - node to allocate
//    Returns:
//      TRUE if successful ( no memory allocation error ) 
//F*/
CART_API BOOL cxAllocNodeCatProportions(CxCART* cart, CxCARTNode* node);

/*F////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxAllocNodeInternals
//    Purpose: allocates node internal arrays
//    Parameters:
//      cart - classification and regression tree
//		node - node to allocate
//      expanded_idx - whether to allocate axpanded fallen_idx array ( for all effective features )
//      use_calc_storage - whether to use storage used by CART
//F*/

CART_API void cxAllocNodeInternals(CxCART* cart, CxCARTNode* node, 
								   BOOL expanded_idx = TRUE,
								   BOOL use_calc_storage = TRUE);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFreeNodeInternals
//    Purpose: frees given node internal arrays
//    Parameters:
//      node - node of the tree
///////////////////////////////////////////////////////////////////////////////////////
CART_API void cxFreeNodeInternals( CxCARTNode* node);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFreeSplit
//    Purpose: frees a split
//    Parameters:
//      split - split to free
///////////////////////////////////////////////////////////////////////////////////////
CART_API void cxFreeSplit(CxCARTSplit* split);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFreeNode
//    Purpose: frees given node
//    Parameters:
//      node - node of the tree
///////////////////////////////////////////////////////////////////////////////////////
CART_API void cxFreeNode( CxCARTNode* node);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxFreeBranch
//    Purpose: frees all branch rooted at given node
//    Parameters:
//      node - node of the tree
//      free_root - delete root node if TRUE.
////////////////////////////////////////////////////////////////
CART_API void cxFreeBranch( CxCARTNode* node, BOOL free_root = TRUE);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcAverageResponse
//    Purpose: Calculates estimated average response for the subtree rooted at given
//      node in regression mode.
//    Parameters:
//      cart - CART
//      feature_status - status of features, 1 - fixed , 0 - unknown
//      feature_values - feature values, for fixed features only.
//      pruning_step - pruned subtree number  
//      node - any tree node , NULL means all tree ( root node )
//    Return value:
//      Estimated average response 
////////////////////////////////////////////////////////////////
CART_API float cxCalcCARTAverageResponse(CxCART* cart, char* feature_status ,
						    			 CxClassifierVar* feature_values, 
							    		 int pruning_step = 0, CxCARTNode* node = NULL);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcClassProbability
//    Purpose: Calculates estimated class probability for given class for the 
//      subtree rooted at given node in classification mode.
//    Parameters:
//      cart - CART
//      feature_status - status of features, 1 - fixed , 0 - unknown
//      feature_values - feature values, for fixed features only.
//      num_class - response class to predict
//      pruning_step - pruned subtree number  
//      node - any tree node , NULL means all tree ( root node )
//    Return value:
//      Estimated average response 
////////////////////////////////////////////////////////////////
CART_API float cxCalcCARTClassProbability(CxCART* cart, char* feature_status ,
									      CxClassifierVar* feature_values, int num_class,
									      int pruning_step = 0, CxCARTNode* node = NULL);

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcCARTProbabilities
//    Purpose: Calculates estimated class probability for all classes for the 
//      subtree rooted at given node in classification mode.
//    Parameters:
//      cart - CART
//      class_prob - output array for class probabilities
//      feature_status - status of features, 1 - fixed , 0 - unknown
//      feature_values - feature values, for fixed features only.
//      num_class - response class to predict
//      pruning_step - pruned subtree number  
//      node - any tree node , NULL means all tree ( root node )
//    Return value:
//      Estimated average response 
////////////////////////////////////////////////////////////////
CART_API void cxCalcCARTProbabilities(CxCART* cart, float* class_prob, char* feature_status ,
								      CxClassifierVar* feature_values, int pruning_step = 0, 
									  CxCARTNode* node = NULL);

////////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxClassifierEval
//    Purpose: Gives response of a classifier on a string of features
//    Parameters:
//      cart - classification and regression tree
//      features - pointer to features
//      pruning_step - pruned subtree number
//      step - optional step of features disposition
//      missed_mask - optional pointer to array-mask of missed parameters
//                    nonzero values indicate miss
//      missed_mask_step - optional step of missed_mask disposition
//    Returns:
//      Response
//////////////////////////////////////////////////////////////////////////////////////
CART_INLINE CxClassifierVar cxClassifierEval( CxClassifier* classifier, 
                                              void* features,
										      int pruning_step,
                                              int features_step ,
                                              char* missed_mask ,
                                              int missed_mask_step  )
{
	assert(classifier && classifier->vftbl && classifier->vftbl->eval);
	return (*(classifier->vftbl->eval))(classifier,features, 
										pruning_step, features_step, 
										missed_mask , missed_mask_step);
}

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxClassifierPredictRegression
//    Purpose: Calculates estimated average response for the given classifier
//      node in regression mode.
//    Parameters:
//      cart - CART
//      feature_status - status of features, 1 - fixed , 0 - unknown
//      feature_values - feature values, for fixed features only.
//      pruning_step - pruned subtree number
//    Return value:
//      Estimated average response 
////////////////////////////////////////////////////////////////
CART_INLINE float cxClassifierPredictRegression( CxClassifier* classifier, 
												 char* feature_status ,
									             CxClassifierVar* feature_values,
									             int pruning_step)
{
	assert(classifier && classifier->vftbl && classifier->vftbl->predict_regression);
	return (*(classifier->vftbl->predict_regression))(classifier,feature_status, feature_values, pruning_step, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxClassifierPredictClassification
//    Purpose: Calculates estimated class probability for given class for the 
//      classifier in classification mode.
//    Parameters:
//      cart - CART
//      feature_status - status of features, 1 - fixed , 0 - unknown
//      feature_values - feature values, for fixed features only.
//      num_class - response class to predict
//      pruning_step - pruned subtree number  
//    Return value:
//      Estimated average response 
////////////////////////////////////////////////////////////////
CART_INLINE void cxClassifierPredictClassification(CxClassifier* classifier, float* class_prob, 
												   char* feature_status ,CxClassifierVar* feature_values, 
												   int pruning_step = 0)

{
	assert(classifier && classifier->vftbl && 
		    classifier->vftbl->predict_classification && class_prob);
	(*classifier->vftbl->predict_classification)(classifier, class_prob,feature_status,
	                                             feature_values, pruning_step, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxDumpClassifier
//    Purpose: Dumps classifier info to ofstream
//      node in regression mode.
//    Parameters:
//      cart - CART
//      file - output stream ( must be attached to file)
//      pruning_step - number of pruned subtree
////////////////////////////////////////////////////////////////
CART_INLINE void cxDumpClassifier(FILE* file, CxClassifier* classifier, int pruning_step)
{
	assert(classifier && classifier->vftbl && classifier->vftbl->dump);
	(*(classifier->vftbl->dump))(file, classifier, pruning_step);
}


/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcClassifierTestErrors
//    Purpose: Calculates test errors for any classifier 
//    Parameters:
//      cfer - classifier
//      sample - sample for error calculation
//    Returns:
//F*/
CART_INLINE void cxCalcClassifierTestErrors( CxClassifier* cfer , CxClassifierSample* sample)
{
	assert(cfer && cfer->vftbl && cfer->vftbl->calc_test_errors);
	(*(cfer->vftbl->calc_test_errors))(cfer,sample);
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcClassifierTestErrorsAndPredictions
//    Purpose: Calculates test errors and predictions for all strings 
//             for samples for any classifier. 
//    Parameters:
//      cfer - classifier
//      sample - sample for error calculation and prediction
//      predictions - [output] array of size
//          cfer->progress_info->progress_data_size x sample->num_samples_of_interest.
//      calc_err - whether to calculate test sample errors accordind to input sample
//          and rewrite them in progress data.
//    Returns:
//F*/
CART_INLINE void cxCalcClassifierTestErrorsAndPredictions( CxClassifier* cfer , 
											 CxClassifierSample* sample,
											 float* predictions,
											 BOOL calc_err = TRUE)
{
	assert(cfer && cfer->vftbl && cfer->vftbl->calc_test_errors_pred);
	(*cfer->vftbl->calc_test_errors_pred)(cfer, sample, predictions, calc_err);
}

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxCalcClassifierTestErrors
//    Purpose: Calculates test errors for any classifier 
//    Parameters:
//      cfer - classifier
//      sample - sample for error calculation
//    Returns:
//F*/
CART_INLINE void cxGetClassifierVarImp( CxClassifier* cfer , float* imps , int mcp = 0)
{
	assert(cfer && cfer->vftbl && cfer->vftbl->get_var_imps);
	(*(cfer->vftbl->get_var_imps))(cfer,imps, mcp);
}

CART_INLINE void cxWriteNode(FILE* file, CxClassifier* cfer,  CxForestNode* node, int indent)
{
	assert(cfer && cfer->vftbl && cfer->vftbl->write_node);
	(*(cfer->vftbl->write_node))(file, cfer,node, indent);
}

//////////////////////////////////////////////////////////////////////////
//																		  
//    Name : _DumpSplit		                                          
//																		  		
//	  Purpose : Output of split info to stream  					          
//	  Parameters : 																		  
//		file - ofstream connected with file for output
//		split - Split
//		split_type - split type
//                                                                        
//////////////////////////////////////////////////////////////////////////
CART_API void _DumpSplit(FILE* file,CxCARTBase* cart, CxCARTSplit* split,  ESplitType split_type);


//////////////////////////////////////////////////////////////////////////
//																		  
//    Name : _DumpNode			                                          
//																		  		
//	  Purpose : Output of node info to stream  					          
//	  Parameters : 																		  
//		cart - CART
//		node - node to dump
//		file - ofstream connected with file for output
//                                                                        
//////////////////////////////////////////////////////////////////////////
CART_API void _DumpNode(FILE* file, CxCARTBase* cart, CxCARTNode *node);

//////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxDumpNode			                                          
//																		  		
//    Purpose : Output of all branch info to stream (recursive)		      
//    Parameters :  																		  
//	    cart - CART
//      node - node to dump
//      file - ofstream connected with file for output
//      pruning_step - pruned subtree number
//                                                                      
//////////////////////////////////////////////////////////////////////////
CART_API void cxDumpNode(FILE* file, CxCARTBase* cart,  CxCARTNode *node, int pruning_step = 0);

//////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxDumpCART			                                          
//																		  		
//    Purpose : Output of all cart info to stream            		          
//    Parameters :  																		  
//      cart - CART
//	    file - ofstream connected with file for output	
//      pruning_step - pruned subtree number
//																  
//////////////////////////////////////////////////////////////////////////
CART_API void cxDumpCART(FILE* file, CxCART* cart, int pruning_step = 0);

CART_API BOOL cxReadTag(FILE* file, char* tag_name, int len);
CART_API BOOL cxReadToDelim(FILE* file, char delim);
CART_API BOOL cxReadSplit(FILE* file, CxCARTBase* cart, CxCARTSplit*& split, ESplitType& type,  char* buf = NULL);
CART_API BOOL cxReadNode(FILE* file, CxCART* cart, CxCARTNode*& node, char* buf = NULL);
CART_API BOOL cxReadAllNodes(FILE* file, CxCART* cart, char* buf = NULL);
CART_API BOOL cxReadPruningData(FILE* file, CxPruningData*& data, char* buf = NULL);
CART_API BOOL cxReadErrorSeq(FILE* file, CxCART* cart, char* buf = NULL);

__inline BOOL icxReadCARTTrainParam(FILE* file, CxCARTTrainParams* params, int num_resp, char* buf);
CART_API BOOL cxReadCARTTrainParams(FILE* file,
						  		    CxCARTTrainParams*& params, 
								    int num_response_classes, 
								    char* buf = NULL);

CART_API BOOL cxReadFeatureTypes(FILE* file, int*& types,
								 int& response_type,
								 int& num_features, char* buf = NULL);
CART_API BOOL cxReadCART(FILE* file, CxCART*& cart, char* buf = NULL);


CART_API void cxWriteTag(FILE* file, const char* tag_name, int indent = 0 , BOOL section = FALSE);
CART_API void cxWriteEndTag(FILE* file, int indent = 0 , const char* tag_name = NULL);

CART_API void cxWriteCARTNode(FILE* file, CxCART* cart, CxCARTNode* node,int indent);

CART_API void cxWriteAllTreeNodes(FILE* file, CxRootedCARTBase* cart, int indent,
								  CxForestNode* root = NULL, int num_nodes = 0);

CART_API void cxWriteTrainParams(FILE* file, CxCARTTrainParams* params, int indent);

CART_API void _cxWriteCARTTrainParams(FILE* file, CxCARTBase* cart, int indent);
CART_API void  cxWriteCARTTrainParams(FILE* file, CxCARTBase* cart, int indent);

CART_API void cxWriteCART(FILE* file, CxCART* cart, int indent);
CART_API void cxWriteFeatureTypes(FILE* file, CxCARTBase* cart, int indent);
CART_API void cxWriteSplit(FILE* file, CxCARTBase* cart, CxCARTSplit*& split, ESplitType type, int indent);
CART_API void cxWriteErrorSeq(FILE* file, CxCART* cart, int indent);
CART_API void cxWritePruningData(FILE* file,  CxPruningData* data, int indent);

CART_API BOOL cxSkipBegin(FILE* file, char* buf);
CART_API BOOL cxSkipSection(FILE* file, char* buf);
CART_API BOOL cxFindTag(FILE* file, const char* tag_name, char* buf = NULL);

CART_API void GetCARTBuildTime(char* str);

CART_API FILE* fopen_cart(char* name, char* attr);
CART_API int fclose_cart(FILE* file);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxWriteClassifier
//    Purpose: Writes classifier to text stream
//    Parameters:
//      file - output stream
//      cfer - classifier
//      indent - indentation
//    Returns:
//F*/
CART_API void cxWriteClassifier(FILE* file, struct CxClassifier* cfer, int indent = 0);

/*F///////////////////////////////////////////////////////////////////////////////////////
//
//    Name:    cxReadClassifier
//    Purpose: Reads classifier from text stream
//    Parameters:
//      istr - input stream
//      cfer - classifier
//      buf  - temporary buffer for reading.
//    Returns:
//F*/
CART_API BOOL cxReadClassifier(FILE* file, struct CxClassifier*& cfer, char* buf = NULL);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxInitNodeStorage			                                          
//																		  		
//    Purpose : Assigns storage to cart , initializes node storage
//              in storage, using inner storage if storage = NULL 	      
//    Parameters :  																		  
//	    cart - CART
//      storage - new storage that is assigned to cart.
//    Notes :                                                                
//		Can be called after cxCreateCART	
//////////////////////////////////////////////////////////////////////////
CART_API BOOL cxInitNodeStorage(CxCART* cart, CxCARTStorage* storage = NULL);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : icxAllocNode			                                          
//																		  		
//    Purpose : Allocates a node from cart's node storage,
//              sets internall array ointers for it.	      
//    Parameters :  																		  
//	    cart - CART
//      id - node id
//      depth - node depth
//      clear - whether to memset the node to 0
//    Return :
//		New node pointer
//    Notes :          
//      Each node is assigned to unique place in storage, 
//      Determined by its id and depth.                                                      
//////////////////////////////////////////////////////////////////////////
CART_IMPL CxCARTNode* cxAllocNode(CxCART* cart , int id ,int depth, BOOL clear = TRUE);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxGrowCARTBranch			                                          
//																		  		
//    Purpose : Grows CART new branch , allocating storages if nessesary 
//    Parameters :  																		  
//	    cart - CART
//      node - branch root node
//      split - split to make, NULL means optimal split
//      not_shrink_idx - whether to shrink root node idx afterwards.
//    Return :
//		FALSE on memory error or same as cxSplitCARTLeafFull
//    Notes :          
//      Shrinks node storage afterwards , clears calculation storage.                                                   
//////////////////////////////////////////////////////////////////////////
CART_API BOOL cxGrowCARTBranch (CxCART* cart, 
		 					    CxCARTNode* node, 
							    CxCARTSplit* split = NULL,
							    BOOL not_shrink_idx = FALSE);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxSetNodeStorageDepth			                                          
//																		  		
//    Purpose : Initialize node storage, allocating memory for tree of given depth 
//    Parameters :  																		  
//	    cart - CART
//      num_samples - number of samples in sample used to build this CART.
//      depth - new maximum depth.
//    Return :
//		FALSE on memory error, otherwise TRUE.
//    Notes :          
//      Shrinks node storage if new depth is less then current maximal depth,
//      or expands if new depth is greater then current maximal depth
//////////////////////////////////////////////////////////////////////////
CART_API BOOL cxSetNodeStorageDepth(CxCART* cart, int num_samples , int depth);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxCompactNodeStorage		                                          
//																		  		
//    Purpose : Compacts node storage, destroying fallen_idx
//    Parameters :  																		  
//	    storage - CART storage
//		root  - root for CART.
//////////////////////////////////////////////////////////////////////////
CART_API BOOL cxCompactNodeStorage(CxCARTStorage* storage, CxCARTNode* root);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxInitSplitBuffer			                                          
//																		  		
//    Purpose : Initializes split buffer to accomodate eff_num_features
//              splits on all effective features.
//    Parameters :  																		  
//	    cart - CART
//      buf - buffer to allocate.
//    Return :
//		none
//    Notes :          
//      Used internally to fill calculation storage split buffer,
//      buf must be allocated befor calling this function.
//////////////////////////////////////////////////////////////////////////
CART_API void cxInitSplitBuffer(CxCART* cart, void* buf);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxAddNodeSplits			                                          
//																		  		
//    Purpose : Copies split of given type to node's split storage
//    Parameters :  																		  
//	    cart - CART
//      node - node of CART
//      splits - array of pointers to splits
//      num_splits - number of elements in splits.
//      split_type - split type 
//      cur_ptr - insertion location, NULL means insertion
//          to beginning of split area of given type.
//    Return :
//		none
//    Notes :          
//      Replaces pointers in splits with new pointers to allocated splits,
//      advances *cur_ptr if cur_ptr is not NULL to point to end of added split
//////////////////////////////////////////////////////////////////////////
CART_API void cxAddNodeSplits(CxCART* cart , CxCARTNode* node , 
							  CxCARTSplit** splits , int num_splits, 
							  int split_type, char** cur_ptr = NULL);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxCreateCalcStorage			                                          
//																		  		
//    Purpose : Create calculation storage for growing new branch
//    Parameters :  																		  
//	    cart - CART
//      num_samples - number of samples in branch root node.
//		start_depth - depth of branch root node.
//      offset_root - offset of the root node.
//    Returns :
//      TRUE if memory was allocated successfully, FALSE otherwise
//    Notes :          
//      Allocates memory for calculations
//////////////////////////////////////////////////////////////////////////
CART_API BOOL cxCreateCalcStorage(CxCART* cart, int num_samples, 
								  int start_depth = 0, int offset_root = 0);


///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxAllocNodeInternalsFromStorage			                                          
//																		  		
//    Purpose : Allocate node internal stuff from calc storage
//    Parameters :  																		  
//	    cart - CART
//      node - node of CART
//    Notes :          
//      Doesn't allocate memory, uses calc_storage
//////////////////////////////////////////////////////////////////////////
CART_API void cxAllocNodeInternalsFromStorage(CxCART* cart , CxCARTNode* node, BOOL clear = TRUE);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxCreatePruningStorage			                                          
//																		  		
//    Purpose : Creates pruning storage
//    Parameters :  																		  
//	    cart - CART
//    Return :
//      TRUE if memory was allocated successfully, FALSE otherwise
//    Notes :          
//      Allocates memory 
//////////////////////////////////////////////////////////////////////////
CART_API BOOL cxCreatePruningStorage(CxCART* cart);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxCreateCrossValStorage			                                          
//																		  		
//    Purpose : Creates cross - validation storage
//    Parameters :  																		  
//	    cart - CART
//      V - number of cross - validation folds
//    Return :
//      TRUE if memory was allocated successfully, FALSE otherwise
//    Notes :          
//      Allocates memory 
//////////////////////////////////////////////////////////////////////////
CART_API BOOL cxCreateCrossValStorage(CxCART* cart , int V);

///////////////////////////////////////////////////////////////////////////
//																		  
//    Name : cxCopyCART			                                          
//																		  		
//    Purpose : Creates cross - validation storage
//    Parameters :  																		  
//	    cart - CART
//    Return :
//      New (identicel) CART on suceess, NULL on failure.
//    Notes :          
//      Allocates memory 
//////////////////////////////////////////////////////////////////////////
CART_API CxCART* cxCopyCART(CxCART* cart);

/// Iterates CART nodes
CART_API int cxCalcNodeNum( CxCARTNode* node);

#ifdef CART_WITH_GUI
CART_API void InitCARTCommObj(HANDLE hStopEvent , HWND hwndMain);
#endif

#ifdef _DEBUG
//// Debug function, asserts node validity
	CART_API void cxAssertNodeValid(CxCART* cart , CxCARTNode* node );
#endif

#endif
