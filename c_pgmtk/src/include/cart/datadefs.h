#ifndef _DATADEFS_H_
#define _DATADEFS_H_

#include <float.h>
#include <limits.h>
#include <stdio.h>
#ifdef WIN32
#include <crtdbg.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "cxcore.h"
//#include "cxtypes.h"
//#include "cxerror.h"

#ifdef _MSC_VER
#pragma warning(disable : 4505)

#define CART_WITH_GUI // For stop event handle
#endif

#ifndef CART_EXTERN_C
    #ifdef __cplusplus
        #define CART_EXTERN_C extern "C"
    #else
        #define CART_EXTERN_C
    #endif
#endif

#if defined CART_EXPORTS
    #define CART_API  CART_EXTERN_C __declspec(dllexport) 
#else
    #define CART_API CART_EXTERN_C
#endif

#ifndef CART_INLINE
	#ifdef WIN32
		#define CART_INLINE __declspec(dllexport) __inline 
	#elif defined __cplusplus
		#define CART_INLINE inline 
	#else
		#define CART_INLINE static 
	#endif
#endif

#define CART_IMPL CART_EXTERN_C

#ifndef VERIFY
	#ifdef _DEBUG
		#define VERIFY(f) assert(f)
	#else
		#define VERIFY(f) (f)
	#endif
#endif

#ifdef _MEM_DEBUG 
	#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   calloc(c, s)      _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   _expand(p, s)     _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define   free(p)           _free_dbg(p, _NORMAL_BLOCK)
	#define   _msize(p)         _msize_dbg(p, _NORMAL_BLOCK)
#endif

//#define cxRound(f) ((int)f)

const int BUF_SIZE = 100;

/*************************************/
/* Tag names for saving / loading    */
/*************************************/

//// Section names (global)
static const char* SECTION_START = "BEGIN";
static const char* SECTION_END = "END";

//// Section names (CART)
static const char* SECTION_TRAIN_PARAMS = "Train parameters";
static const char* SECTION_PRIORS = "Priors";
static const char* SECTION_FEATURES_OF_INTEREST = "Features of interest";
static const char* SECTION_FEATURE_TYPES = "Feature types and response type";
static const char* SECTION_ALL_NODES = "All Nodes";
static const char* SECTION_ERROR_SEQ = "Alpha - error sequence";
static const char* SECTION_PRUNING_DATA = "Pruning data";
static const char* SECTION_CART = "CART Parameters";
static const char* SECTION_NODE = "Node Parameters";
static const char* SECTION_SPLIT = "Split";

//// Section names (MART, MCLRT)
static const char* SECTION_ONE_ITERATION = "One iteration data";
static const char* SECTION_MART = "MART Classifier";
static const char* SECTION_MCLRT = "MCLRT Classifier";
static const char* SECTION_FOREST = "Random Forest Classifier";
static const char* SECTION_CLASSIFIER = "Black-box classifier";

//// Section names (For DNutData)
static const char* SECTION_DEP_NET = "Dependency net";
static const char* SECTION_TABLE = "Table data";
static const char* SECTION_ALL_DATA = "All data";
static const char* SECTION_CART_FULL = "Cart with priors and var deps";
static const char* SECTION_CLASSIFIER_FULL = "Classifier with var deps";
static const char* SECTION_BASE_CLASSIFIER = "Classifier with var deps, types, names and levels";

//// Connection string and parameters ( used in DNutData )
static const char* PARAM_CONNECT_STRING = "Connection string";
static const char* PARAM_SQL_CONNECT_STRING = "SQL Connection string";
static const char* PARAM_SOURCE_TYPE = "Source type";
static const char* PARAM_SOURCE_PARAMS = "Source parameters";
static const char* PARAM_VARIABLE_TYPES = "Variable types";
static const char* PARAM_VARIABLE_NAMES = "Variable names";

//// Train params ( CART , MART and MCLRT )
static const char* PARAM_NUM_COMPETITORS = "Number of competitors";
static const char* PARAM_NUM_SURROGATES = "Number of surrogates";
static const char* PARAM_MAX_DEPTH = "Tree max depth";
static const char* PARAM_SPLIT_MIN_POINTS = "Minimal points in terminal node";
static const char* PARAM_SPLIT_MAX_POINTS = "Maximal points in terminal node";
static const char* PARAM_NUM_CLUSTERS = "Maximal number of clusters";
static const char* PARAM_NUM_FEATURES_OF_INTEREST = "Number of features of interest";
static const char* PARAM_FEATURES_OF_INTEREST = "Features of interest";
static const char* PARAM_PRIORS = "Priors";
static const char* PARAM_SPLIT_RULE = "Splitting rule";
static const char* PARAM_COMPETITOR_THRESHOLD = "Competitor threshold";
static const char* PARAM_SURROGATE_THRESHOLD = "Surrogate threshold";
static const char* PARAM_ALPHA_STOP = "Complexity parameter";

//// Train params (MART and MCLRT )
static const char* PARAM_MAX_ITER = "Maximum iterations";
static const char* PARAM_SHRINK_RATE = "Shrinkage rate";
static const char* PARAM_SUBSAMPLING_MODE = "Subsampling mode";
static const char* PARAM_SUBSAMPLING_PART = "Subsampling part";
static const char* PARAM_HUBER_ALPHA = "Alpha for Huber Regression";
static const char* PARAM_LOSS_FUNCTION = "Loss function for MART";
static const char* PARAM_INFLUENCE_TRIMMING = "Influence trimming parameter";
static const char* PARAM_FOREST_SEL_VARS = "RF selected vars";

static const char* PARAM_VAR_NUM = "Variable number"; // For DNutData

//// Additional params (MART and MCLRT )
static const char* PARAM_MART_RANDOM_SEED = "MART random seed";
static const char* PARAM_MART_HUBER_DELTA = "MART Huber delta";
static const char* PARAM_MART_VAR_IMPS = "MART variable importances";
static const char* PARAM_NUM_ITER = "Current iterations";

//// Pruning data (CART , MART and MCLRT)
static const char* PARAM_PRUNING_STEP = "Pruning step";
static const char* PARAM_TERMINAL_NODES = "Number of terminal nodes";
static const char* PARAM_ALPHA = "Complexity penalty";
static const char* PARAM_REESTIMATION_ERROR = "Reestimation error";
static const char* PARAM_TEST_SAMPLE_ERROR = "Test sample error";
static const char* PARAM_ERROR_DEVIATION = "Error deviation";
static const char* PARAM_OOB_ERROR = "OOB error";
static const char* PARAM_REESTIMATION_LOSS = "Reestimation loss";
static const char* PARAM_TEST_SAMPLE_LOSS = "Test sample loss";
static const char* PARAM_CLASSIFIER_TYPE = "Classifier type";

//// Node and split params ( all tree - based classifiers )
static const char* PARAM_NUM_FALLENS = "Number of observations";
static const char* PARAM_RESPONSE = "Response";
static const char* PARAM_DEPTH = "Depth";
static const char* PARAM_ID = "Node id";
static const char* PARAM_NODE_ERROR = "Node error";
static const char* PARAM_SUM_WEIGHTED_FALLENS = "Sum of fallen strings weighted with priors";
static const char* PARAM_NODE_PRUNING_STEP = "Pruning step";
static const char* PARAM_NODE_DIRECTION = "Majority direction";
static const char* PARAM_NUM_NODES = "Number of nodes";

//const char* PARAM_NUMBER_PRUNED_NODES = "Number of pruned nodes";
//const char* PARAM_PRUNED_NODES = "Pruned nodes";

//// Stop CARTs Calculation Event name (for DNutData)
static const char* STOP_EVENT_NAME = "DNutStopExecThreadEvent";

const float EPS = 1e-5f;
const float INV_EPS = 1e+5f;

enum ESplitType
{
	SPLIT_PRIMARY,
	SPLIT_COMPETITOR,
	SPLIT_SURROGATE,
	SPLIT_MANUAL
};

typedef union CxClassifierVar
{
    int i;
    float fl;
	void *ptr;
} CxClassifierVar;

typedef enum CxMARTLossFunction
{
    CxLFLeastSquares,
    CxLFLeastAbsDeviation,
	CxLFHuber,
	CxLFMultinomial
} CxMARTLossFunction;

typedef enum CxSubsampleMode
{
	CxNoSubsample = 0,
	CxSubsample,
	CxOutOfBag
} CxSubsampleMode;

#define TRACE_MODE 0

typedef int BOOL;
#define TRUE 1
#define FALSE 0
typedef int CxClassifierVarType;
#define FLOAT_ONLY 1
#if (__INTEL_COMPILER)
	#define SSE2_OPTIMIZE 0
#else
	#define SSE2_OPTIMIZE 0
#endif

#ifdef WIN32
#define INLINE_ASM_OPTIMIZE 1
#else
#define INLINE_ASM_OPTIMIZE 0
#endif

#define CART_ALIGN_MEMORY 0
#define MIN_ALIGN 0
#if CART_ALIGN_MEMORY
	#define CART_MEM_ALIGN 16
	#define CART_ELEM_ALIGN (CART_MEM_ALIGN >> 2)
#else
	#define CART_MEM_ALIGN 0
	#define CART_ELEM_ALIGN 0
#endif

/****************************************************************************************/
/*                                Classifier flags
/****************************************************************************************/

#define CX_CLASSIFIER_KIND_BITS 4
#define CX_CLASSIFIER_KIND_MASK ((1 << CX_CLASSIFIER_KIND_BITS) - 1)

enum CxClassifierType
{
	CX_CLASSIFIER_KIND_CART ,
	CX_CLASSIFIER_KIND_NORMAL_BAYES ,
	CX_CLASSIFIER_KIND_K_NEAREST ,
	CX_CLASSIFIER_KIND_SVM ,
	CX_CLASSIFIER_KIND_MART ,
	CX_CLASSIFIER_KIND_MCLRT,
	CX_CLASSIFIER_KIND_FOREST,
	CX_CLASSIFIER_KIND_MART_FOREST,
	CX_CLASSIFIER_KIND_MCLRT_FOREST
};

#define CX_GET_CLASSIFIER_KIND( cfer ) ((cfer)->flags & CX_CLASSIFIER_KIND_MASK)

#define CX_IS_CLASSIFIER_CART( cfer ) \
    (CX_GET_CLASSIFIER_KIND( cfer ) == CX_CLASSIFIER_KIND_CART)
#define CX_IS_CLASSIFIER_NORMAL_BAYES( cfer ) \
    (CX_GET_CLASSIFIER_KIND( cfer ) == CX_CLASSIFIER_KIND_NORMAL_BAYES)
#define CX_IS_CLASSIFIER_K_NEAREST( cfer ) \
    (CX_GET_CLASSIFIER_KIND( cfer ) == CX_CLASSIFIER_KIND_K_NEAREST)
#define CX_IS_CLASSIFIER_SVM( cfer ) \
    (CX_GET_CLASSIFIER_KIND( cfer ) == CX_CLASSIFIER_KIND_SVM)

#define CX_CLASSIFIER_FLAG_SHIFT CX_CLASSIFIER_KIND_BITS

#define CX_CLASSIFIER_FLAG_FEATURES_PURE_32F (1 << CX_CLASSIFIER_FLAG_SHIFT)
#define CX_CLASSIFIER_FLAG_FEATURES_PURE_32S (2 << CX_CLASSIFIER_FLAG_SHIFT)
#define CX_CLASSIFIER_FLAG_FEATURES_PURE_NUMERIC (4 << CX_CLASSIFIER_FLAG_SHIFT)
#define CX_CLASSIFIER_FLAG_FEATURES_MUNDANE (8 << CX_CLASSIFIER_FLAG_SHIFT)

#define CX_CLASSIFIER_TYPE_BITS CX_CLASSIFIER_FLAG_SHIFT
#define CX_CLASSIFIER_TYPE_MASK ((1 << CX_CLASSIFIER_TYPE_BITS) - 1)
#define CX_CLASSIFIER_SUBTYPE_BITS (CX_CLASSIFIER_FLAG_SHIFT + 10)
#define CX_CLASSIFIER_SUBTYPE_MASK ((1 << CX_CLASSIFIER_SUBTYPE_BITS) - 1)

#define CX_GET_CLASSIFIER_TYPE( cfer ) ((cfer)->flags & CX_CLASSIFIER_TYPE_MASK)
#define CX_GET_CLASSIFIER_SUBTYPE( cfer ) ((cfer)->flags & CX_CLASSIFIER_SUBTYPE_MASK)

#define CX_IS_CLASSIFIER_FEATURES_PURE_32F( cfer ) \
    (((cfer)->flags & CX_CLASSIFIER_FLAG_FEATURES_PURE_32F) != 0)
#define CX_IS_CLASSIFIER_FEATURES_PURE_32S( cfer ) \
    (((cfer)->flags & CX_CLASSIFIER_FLAG_FEATURES_PURE_32S) != 0)
#define CX_IS_CLASSIFIER_FEATURES_PURE_NUMERIC( cfer ) \
    (((cfer)->flags & CX_CLASSIFIER_FLAG_FEATURES_PURE_NUMERIC) != 0)
#define CX_IS_CLASSIFIER_FEATURES_PURE_NUMERIC_32F( cfer )     \
    (((cfer)->flags & (CX_CLASSIFIER_FLAG_FEATURES_PURE_32F    \
                       | CX_CLASSIFIER_FLAG_FEATURES_PURE_NUMERIC)) != 0)
#define CX_IS_CLASSIFIER_FEATURES_PURE_NUMERIC_32S( cfer )  \
    (((cfer)->flags & (CX_CLASSIFIER_FLAG_FEATURES_PURE_32S \
                       | CX_CLASSIFIER_FLAG_FEATURES_PURE_NUMERIC)) != 0)
#define CX_IS_CLASSIFIER_FEATURES_MUNDANE( cfer ) \
    (((cfer)->flags & CX_CLASSIFIER_FLAG_FEATURES_MUNDANE) != 0)

#define CX_CLASSIFIER_VAR_TYPE_NUM_STATES_BITS 13
#define CX_CLASSIFIER_VAR_TYPE_NUM_STATES_MASK \
    ((1 << CX_CLASSIFIER_VAR_TYPE_NUM_STATES_BITS) - 1)
#define CX_CLASSIFIER_VAR_TYPE_FLAG_32F (1 << CX_CLASSIFIER_VAR_TYPE_NUM_STATES_BITS)
#define CX_CLASSIFIER_VAR_TYPE_FLAG_NUMERIC (2 << CX_CLASSIFIER_VAR_TYPE_NUM_STATES_BITS)
#define CX_CLASSIFIER_VAR_TYPE_FLAG_TRANSCENDENT (4 << CX_CLASSIFIER_VAR_TYPE_NUM_STATES_BITS)

#define CX_CLASSIFIER_VAR_TYPE_NUMERIC_32F \
    (CX_CLASSIFIER_VAR_TYPE_FLAG_32F | CX_CLASSIFIER_VAR_TYPE_FLAG_NUMERIC)
#define CX_CLASSIFIER_VAR_TYPE_NUMERIC_32S CX_CLASSIFIER_VAR_TYPE_FLAG_NUMERIC
#define CX_CLASSIFIER_VAR_TYPE_CATEGORIC_32F( num_states ) \
    (CX_CLASSIFIER_VAR_TYPE_FLAG_32F | (num_states))
#define CX_CLASSIFIER_VAR_TYPE_CATEGORIC_32S( num_states ) (num_states)
#define CX_CLASSIFIER_VAR_TYPE_CATEGORIC_TRANSCENDENT \
    (CX_CLASSIFIER_VAR_TYPE_FLAG_32F | CX_CLASSIFIER_VAR_TYPE_FLAG_TRANSCENDENT)

#if CART_ALIGN_MEMORY 
#define ALIGN_NUM_RESP(num_resp_aligned) \
	if (num_resp_aligned > MIN_ALIGN)    \
		num_resp_aligned = ((num_resp_aligned-1) & (-CART_ELEM_ALIGN)) + CART_ELEM_ALIGN; 
#else 
#define ALIGN_NUM_RESP(num_resp_aligned) 
#endif
//#define offsetof(CL,EL) (int)(&((CL*)0)->EL)
#define offsetof(s,m)   (size_t)&(((s *)0)->m)

/****************************************************************************************/
/*                          Classifier sample related stuff
/****************************************************************************************/

typedef CxClassifierVar CxClassifierResponse;

#define CX_CLASSIFIER_SAMPLE_CHUNK_FIELDS()                        \
    int is_loaded;                                                 \
    int num_points;                                                \
    int is_locked;                                                 \
    int num_indices_of_interest;                                   \
    int *indices_of_interest;                                      \
    int indices_of_interest_step;                                  \
    CxClassifierVar *features_data;                                \
    int features_step[2];                                          \
    char *missed_data;                                             \
    int missed_step[2];                                            \
    CxClassifierResponse *responses_data;                          \
    int responses_step;                                            \
    float *weights_data;                                           \
    int weights_step;

typedef struct CxClassifierSampleChunk
{
    CX_CLASSIFIER_SAMPLE_CHUNK_FIELDS()
} CxClassifierSampleChunk;

#define CX_CLASSIFIER_SAMPLE_FIELDS()                                    \
    int num_chunks;                                                      \
    CxClassifierSampleChunk **chunk;                                     \
    int is_weighted;                                                     \
    int num_features;                                                    \
    CxClassifierVarType *feature_type;                                   \
    CxClassifierVarType response_type;                                   \
    CxClassifierSampleChunk *(*load_chunk)( CxClassifierSample *, int ); \
    CxClassifierSampleChunk *(*lock_chunk)( CxClassifierSample *, int ); \
    void (*unlock_chunk)( CxClassifierSample *, int );                   \
    void (*release)( CxClassifierSample * );

typedef struct CxClassifierSample
{
    CX_CLASSIFIER_SAMPLE_FIELDS()
} CxClassifierSample;

/****************************************************************************************/
/*                        Generic classifier related stuff
/****************************************************************************************/

#define CX_CLASSIFIER_TRAIN_PARAMS_FIELDS() \
    int num_features_of_interest;           \
    int *features_of_interest;              \
	CxMat* features_of_interest_mat;		\
    int features_of_interest_step;

typedef struct CxClassifierTrainParams
{
    CX_CLASSIFIER_TRAIN_PARAMS_FIELDS()
} CxClassifierTrainParams;

#define CX_PROGRESS_DATA_FIELDS()	\
	int step;						\
    double reestimation_error;		\
	double test_sample_error;		\
	double error_dev;				

typedef struct CxProgressData
{
    CX_PROGRESS_DATA_FIELDS()
} CxProgressData;

#define CX_PROGRESS_INFO_FIELDS()  \
	CxProgressData* progress_data; \
	int progress_data_step;		   \
	int progress_data_size;

typedef struct CxProgressInfo
{
    CX_PROGRESS_INFO_FIELDS()
} CxProgressInfo;

typedef struct CxClassifier* (*create_t)(int num_features, int* feature_type,
										 int response_type, struct CxCARTTrainParams* params,
										 struct CxClassifier* cfer );
typedef void  (*release_t)( struct CxClassifier * );
typedef	void  (*calc_test_errors_pred_t) ( struct CxClassifier* cfer, CxClassifierSample* sample, float* prediction, BOOL calc_err);										
typedef void  (*predict_classification_t)(struct  CxClassifier*, float*, char *, CxClassifierVar*, int , struct CxCARTNode*);
typedef CxClassifierVar (*eval_t)( struct CxClassifier *, void *, int, int , char *, int );
typedef void  (*calc_test_errors_t)( CxClassifier *, CxClassifierSample*);
typedef void  (*dump_t) (FILE* file, struct CxClassifier* cfer, int);
typedef float (*predict_regression_t)( CxClassifier*, char *, CxClassifierVar*, int , struct CxCARTNode*);
typedef float (*predict_regression_t)( CxClassifier*, char *, CxClassifierVar*, int , struct CxCARTNode*);
typedef	void  (*get_var_imps_t) ( struct CxClassifier* , float* , int );
typedef	void  (*write_text_t)(FILE* file, struct CxClassifier* cfer, int indent);		
typedef	BOOL  (*read_text_t)(FILE* file, struct CxClassifier* cfer, char* buf);			

typedef void (*write_node_t)(FILE* file, struct CxClassifier* cart, struct CxForestNode* node,int indent);
typedef BOOL (*read_tree_t)(FILE* file, struct CxCARTBase* cart, int niter, char* buf);
typedef BOOL (*read_iter_t)(FILE* file, struct CxCARTBase* cart, int niter, char* buf);
typedef void (*write_iter_t)(FILE* file, struct CxCARTBase* forest, int iter , int indent);
typedef BOOL (*read_params_t)(FILE* file, struct CxCARTTrainParams*& params, int num_resp, char* buf);
typedef void (*write_params_t)(FILE* file, struct CxCARTBase* cart, int indent);

typedef struct CxClassifierFuncTable
{
	release_t release;
	create_t create;
	eval_t eval;
	predict_regression_t predict_regression;
	predict_classification_t predict_classification;
	calc_test_errors_t calc_test_errors;
	calc_test_errors_pred_t calc_test_errors_pred;
	dump_t dump;
	get_var_imps_t get_var_imps;
	write_text_t write_text;
	read_text_t read_text;
	write_node_t write_node;
	read_params_t read_params;
	write_params_t write_params;
	read_tree_t read_tree;
	read_iter_t read_iter;
	write_iter_t write_iter;
} CxClassifierFuncTable;

#define CX_CLASSIFIER_FIELDS()			\
	CxClassifierFuncTable* vftbl;		\
	int* vbtbl;							\
	CxProgressInfo progress_info;       \
    int flags;                          \
    int num_features;					\
	int  num_response_classes;			\
    CxClassifierVarType *feature_type;  \
    CxClassifierVarType response_type;  
 
typedef struct CxClassifier
{
    CX_CLASSIFIER_FIELDS()
} CxClassifier;


/****************************************************************************************/
/*                       Classification And Regression Tree
/****************************************************************************************/

typedef enum CxCARTSplitCriterion
{
    CxCARTGiniCriterion,
	CxCARTEntropyCriterion,
    CxCARTTwoingCriterion
} CxCARTSplitCriterion;
    
#define CX_CART_TRAIN_PARAMS_FIELDS()     \
    CX_CLASSIFIER_TRAIN_PARAMS_FIELDS()   \
    float* priors;                        \
	float* cost_mat;					  \
    CxCARTSplitCriterion splitting_rule;  \
    int num_competitors;                  \
    float competitor_threshold;           \
    int num_surrogates;                   \
    float surrogate_threshold;            \
    int tree_max_depth;                   \
    int split_min_points;                 \
    int split_max_points;                 \
	int max_clusters;					  \
    CxMat* priors_mat;                    \
	int mem_economy_mode;			  	  \
	float alpha;						  \
	BOOL transform_freq;				  \
	float beta;							  \
	int ref_count;							

typedef struct CxCARTTrainParams
{
    CX_CART_TRAIN_PARAMS_FIELDS()
} CxCARTTrainParams;

#define CX_CART_SPLIT_FIELDS()                 \
    struct CxCARTSplit* next_competitor;       \
	struct CxCARTSplit* next_surrogate;        \
    int feature_idx;                           \
    CxClassifierVar value;                     \
    float weight;							   \
	BOOL revert;                               \


typedef struct CxCARTSplit
{
    CX_CART_SPLIT_FIELDS()
} CxCARTSplit;


typedef struct CxCARTAtom
{
    CxClassifierVar *features;
    CxClassifierResponse response;
	char* missed_mask;
    CxClassifierVar notch;
} CxCARTAtom;

typedef struct CxCARTSubj
{
    CxCARTAtom* body;
    CxClassifierResponse* response;
    int is_valid;
    int size;
    int num_chunks;
    int* markup;
    int features_step;
	int missed_mask_step;
	char* cur_split_directions;
	int ref_count;
//  struct CxCARTNode *master;
} CxCARTSubj;

#define CX_CART_NODE_BASE_FIELDS(NodeClass)          \
	NodeClass* parent;                               \
	NodeClass* child_left;                           \
	NodeClass* child_right;                          \
	CxCARTSplit* split;                              \
	int id;											 \
	int depth;										 \
    int num_fallens;                                 \
    int* fallen_stats;                               \
    int* fallen_idx;                                 \
  	CxClassifierVar response;                        \
	float sum_weighted_fallens;						 \
    BOOL is_copy;									 \
	int direction;									 \
    CxCARTSubj* subj;                                \
	float error;			                         \
  
typedef struct CxForestNode
{
    CX_CART_NODE_BASE_FIELDS(CxForestNode)
} CxForestNode;

#define CX_CART_NODE_FIELDS()                        \
	CX_CART_NODE_BASE_FIELDS(CxCARTNode)			 \
	int cur_cat_prop_pos;							 \
  	int* shrunk_fallen_idx;							 \
	int* num_missed;                                 \
	int** cat_proportions;							 \
  	double branch_error;                             \
	double node_sample_error;                        \
	double branch_sample_error;                      \
	double node_sample_error_dev;                    \
	double branch_sample_error_dev;                  \
	int terminal_nodes;                              \
	int pruning_step;                                \
	BOOL is_idx_expanded;							 \
	int offset;

typedef struct CxCARTNode
{
    CX_CART_NODE_FIELDS()
} CxCARTNode;

////// Single categoric variable category ////////// 
#define CX_VAR_CATEGORY_FIELDS()                     \
	int category;									 \
	float* frequencies;								 \
	float  sum_frequencies;                          \
	CxVarCategory* next_cat;						 \

typedef struct CxVarCategory
{
    CX_VAR_CATEGORY_FIELDS()
} CxVarCategory;

////// Cluster of categoric variable values ///////////
#define CX_VAR_CATEGORY_CLUSTER_FIELDS()             \
	int num_cats;									 \
	float* frequencies;								 \
	float  sum_frequencies;                          \
	CxVarCategory* first_cat;						 \
	CxVarCategory* last_cat;						 \
	struct CxVarCategoryCluster* prev_cluster;		 \
	struct CxVarCategoryCluster* next_cluster;
//	float inv_sum_frequencies;						 


typedef struct CxVarCategoryCluster
{
    CX_VAR_CATEGORY_CLUSTER_FIELDS()
} CxVarCategoryCluster;

enum CxCARTErrorCode
{
	CART_NO_ERROR,
	CART_MEMORY_ERROR,
	CART_CALC_ERROR,
	CART_STOP_CALC,
	CART_INVALID_PARAMS_ERROR,
	CART_UNKNOWN_ERROR
};

enum CxCARTErrorStatus
{
	CART_ERROR_STATUS_NONFATAL,
	CART_ERROR_STATUS_FATAL
};

#ifndef _MAX_PATH
#define _MAX_PATH 512
#endif

#define CX_CART_ERROR_FIELDS()                   \
	int code;				                     \
	char description[_MAX_PATH];				 \
	int status;									 \

typedef struct CxCARTError
{
    CX_CART_ERROR_FIELDS()
} CxCARTError;

#define CX_EXT_CART_INFO_FIELDS()                \
	CxMat* data_mat;		                     \
	CxMat* missed_mat;							 \
	int* sorted_idx;	                         \
    int* num_not_missed;			             \
	int** cat_proportions;						 \
	int* samples_of_interest;                    \

typedef struct CxExtCARTInfo
{
    CX_EXT_CART_INFO_FIELDS()
} CxExtCARTInfo;

#define CX_PRUNING_DATA_FIELDS()	\
	CX_PROGRESS_DATA_FIELDS()		\
	float alpha;					\
    int terminal_nodes;             \
	CxCARTNode** nodes_seq;         \
	int num_nodes;					

typedef struct CxPruningData
{
    CX_PRUNING_DATA_FIELDS()
} CxPruningData;

#define CX_CART_LEVEL_FIELDS()			\
	void* buf_nodes;                    \
	int* buf_shrunk_idx;		     	\
//	int size;							

typedef struct CxCARTLevel
{
    CX_CART_LEVEL_FIELDS()
} CxCARTLevel;

#define MAX_LEVELS 32

#define CX_CART_NODE_STORAGE_FIELDS()	\
	CxCARTLevel levels[MAX_LEVELS];     \
	CxCARTLevel blocks[MAX_LEVELS];	    \
	int levels_per_block[MAX_LEVELS];   \
	int num_blocks;						\
	int max_depth;			            \
	int node_size;				     	\
	int competitors_space;				\
	int surrogates_space;				\
	int manual_split_space;				\
	int split_area_size;				\
	int depth;							\
	BOOL is_compact;
	
typedef struct CxCARTNodeStorage
{
    CX_CART_NODE_STORAGE_FIELDS()
} CxCARTNodeStorage;

#define CX_CART_CALC_STORAGE_FIELDS()			\
	void* buf_fallen_idx;					    \
	int* buf_cat_proportions;					\
	void* buf_splits;							\
	float* buf_resp;							\
	CxClassifierVar* buf_feat_vals;				\
	int cat_prop_top;							\
	int free_blocks[MAX_LEVELS + 2];			\
    int size_cat_proportions;					\
	int size_fallen_idx;						\
	int size_fallen_idx_per_case;				\
	int max_depth;								\
	int num_root_fallens;						\
	int offset_root;							\
	int mem_economy_mode;
//	float* buf_inv;								

typedef struct CxCARTCalcStorage
{
    CX_CART_CALC_STORAGE_FIELDS()
} CxCARTCalcStorage;

#define CX_CART_PRUNING_STORAGEL_FIELDS() \
	CxPruningData* buf;	                  \
	CxCARTNode** pruned_nodes;			  \
	int max_size;						  \
	int size;							  

typedef struct CxCARTPruningStorage
{
    CX_CART_PRUNING_STORAGEL_FIELDS()
} CxCARTPruningStorage;

#define CX_CART_CROSS_VAL_STORAGE_FIELDS() \
	CxPruningData* buf;	                   \
	CxPruningData** seq_ptrs;	    	   \
	int* sizes;  				   	       \
	int num_pruning_seqs;				   \
	int cur_size;						   \
	int max_size;						   \
	int V;									

typedef struct CxCARTCrossValStorage
{
    CX_CART_CROSS_VAL_STORAGE_FIELDS()
} CxCARTCrossValStorage;


#define CX_CART_STORAGE_FIELDS()				\
	CxCARTNodeStorage node_storage;				\
	CxCARTCalcStorage calc_storage;				\
	int num_samples;							\
	int total_num_cats;							\
	int max_num_cats;							\
	int num_categoric_vars;						\

typedef struct CxCARTStorage
{
    CX_CART_STORAGE_FIELDS()
} CxCARTStorage;


#ifdef CART_WITH_GUI
#include <windows.h>
#define WM_DONE_STRING WM_USER + 6
static char strMsg[512];
#endif

#define CX_CART_BASE_FIELDS()                    \
	CX_CLASSIFIER_FIELDS()                       \
	int  eff_num_features;						 \
	int* features_corr;                          \
    int* features_back_corr;                     \
	int* num_classes;                            \
	CxCARTTrainParams* params;                   \
	float* priors_mult;							 \
	int num_nodes;                               \
	CxCARTError error;							 \
	float* cost_error_mult;						 \
	float* cost_gini_mult;						 \
	float* var_importances;			  

typedef struct CxCARTBase
{
    CX_CART_BASE_FIELDS()
} CxCARTBase;

#define CX_CART_EXTRA_FIELDS()                   \
	CxCARTStorage* storage;						 \
	CxCARTStorage inner_storage;                 \
	CxCARTPruningStorage pruning_storage;		 \
	CxCARTCrossValStorage cross_val_storage;	 \
	CxExtCARTInfo* extra_info;					 

#define CX_CART_FIELDS()                         \
	CX_CART_BASE_FIELDS()                        \
	CxCARTNode* root;                            \
	CX_CART_EXTRA_FIELDS()						 

typedef struct CxRootedCARTBase
{
    CX_CART_BASE_FIELDS()                        
	union
	{
		CxCARTNode* croot;                            
		CxForestNode* froot;
	};
} CxRootedCARTBase;

typedef struct CxCART
{
    CX_CART_FIELDS()
} CxCART;

CART_INLINE CxCART* icxGetCART(CxClassifier* cfer)
{
#ifdef _DEBUG
	int* vbtbl = cfer->vbtbl;
	assert(vbtbl && vbtbl[0] > 0 && (vbtbl[1] == 0));
#endif
	return (CxCART*)cfer;
}

#ifndef UNREFERENCED_PARAMETER
	#define UNREFERENCED_PARAMETER(P) (P) = (P); 
#endif

//// Reading parameters
#define READ_PARAM_I(file, param, buf, i) \
	if (strcmp(buf, param) == 0) fscanf(file, "%d", &i);

#define READ_PARAM_F(file, param, buf, f) \
	if (strcmp(buf, param) == 0) fscanf(file, "%g", &f);

#define READ_PARAM_D(file, param, buf, d) \
	if (strcmp(buf, param) == 0)		  \
	{								      \
		float f;						  \
		fscanf(file, "%g", &f);			  \
		d = f;							  \
	}

#define READ_AI(file, ar, n)		\
	int* _ar = (int*)(ar);		 	\
	for (int i = n; i--; _ar++)		\
		fscanf( file, "%d", _ar)

#define READ_AF(file, ar, n)		\
	float* _ar = (float*)(ar);	 	\
	for (int i = n; i--; _ar++)		\
		fscanf( file, "%g", _ar)

#define READ_PARAM_S(file, param, buf, str, len) \
	if (strcmp(buf, param) == 0)		  \
	{								      \
		assert(str && (len > 0));		  \
		eat_white(file);				  \
		fgets(str, len, file);			  \
	}

#define READ_PARAM_AI(file, param, buf, ar, n) \
	if (strcmp(buf, param) == 0)		  \
	{								      \
		assert((ar) && (n > 0));		  \
		READ_AI(file, ar, n);			  \
	}

#define READ_PARAM_AF(file, param, buf, ar, n) \
	if (strcmp(buf, param) == 0)		  \
	{								      \
		assert((ar) && n > 0);			  \
		READ_AI(file, ar, n);			  \
	}

#define FREAD_PARAM_I(param, i) READ_PARAM_I(file, param, buf, i)
#define FREAD_PARAM_F(param, f) READ_PARAM_F(file, param, buf, f)
#define FREAD_PARAM_D(param, f) READ_PARAM_D(file, param, buf, f)
#define FREAD_PARAM_S(param, s, len) READ_PARAM_S(file, param, buf, s, len)

#define FREAD_AI(ar, n) READ_AI(file, ar, n)
#define FREAD_AF(ar, n) READ_AF(file, ar, n)

#define FREAD_PARAM_AI(param, ar, n) READ_PARAM_AI(file, param, buf, ar, n)
#define FREAD_PARAM_AF(param, ar, n) READ_PARAM_AF(file, param, buf, ar, n)
#define FREAD_PARAM_AS(param, ar, n) READ_PARAM_AS(file, param, buf, ar, n)

//// std::vector<int>
#define READ_PARAM_STD_VI(file, param, buf, vec, n) \
	if (strcmp(buf, param) == 0)					\
	{											    \
	    if (n < 0) return FALSE;				  	\
		vec.resize(n);								\
		READ_AI(file, vec.begin(), n);				\
	}

#define FREAD_PARAM_STD_VI(param, vec, n) READ_PARAM_STD_VI(file, param, buf, vec, n)


#define ELIF_READ_PARAM_I(param, i) else READ_PARAM_I(file, param, buf, i)
#define ELIF_READ_PARAM_F(param, f) else READ_PARAM_F(file, param, buf, f)
#define ELIF_READ_PARAM_D(param, f) else READ_PARAM_D(file, param, buf, f)
#define ELIF_READ_PARAM_S(param, s, l) else READ_PARAM_S(file, param, buf, s, l)

#define ELIF_READ_PARAM_AI(param, ar, n) else READ_PARAM_AI(file, param, buf, ar, n)
#define ELIF_READ_PARAM_AF(param, ar, n) else READ_PARAM_AF(file, param, buf, ar, n)
#define ELIF_READ_PARAM_STD_VI(param, vec, n) else FREAD_PARAM_STD_VI(param, vec, n)

//// Wrinting parameters
#define WRITE_PARAM_I(file, param, i, indent) \
		cxWriteTag(file , param, indent);     \
		fprintf(file, "%d\n", i)			  \

#define WRITE_PARAM_F(file, param, f, indent) \
		cxWriteTag(file , param, indent);     \
		fprintf(file, "%.10g\n", f)			  \

#define WRITE_PARAM_S(file, param, s, indent) \
		cxWriteTag(file , param, indent);     \
		fputs(s, file);						  \
		fputc('\n', file)

#define FWRITE_PARAM_I(param, i) WRITE_PARAM_I(file, param, i, indent)
#define FWRITE_PARAM_F(param, f) WRITE_PARAM_F(file, param, f, indent)
#define FWRITE_PARAM_S(param, s) WRITE_PARAM_S(file, param, s, indent)

#define WRITE_S(file, str, indent)				\
	icxWriteIndent(file , indent + 1);			\
	fputs((str), file);							\
	fputc('\n', file)					  

#define FWRITE_S(str) WRITE_S(file, str, indent)

#define WRITE_AI(file, ar, n)			  \
	for (int i = 0; i < n ; i++)		  \
		fprintf( file, "%d ", (ar)[i]);	  \
	fputc('\n', file);					  \


#define WRITE_AF(file, ar, n)			  \
	for (int i = 0; i < n ; i++)		  \
		fprintf( file, "%g ", (ar)[i]);	  \
	fputc('\n', file);					  \

#define WRITE_VI(file, ar, n)				  \
	fprintf(file, "%d ", n);				  \
    if ((ar) && (n > 0))				      \
	{							              \
		for (int i = 0; i < n ; i++)		  \
			fprintf( file, "%d ", (ar)[i]);	  \
	}									      \
	fputc('\n', file)

#define WRITE_VF(file, ar, n)				  \
	fprintf(file, "%d ", n);				  \
    if ((ar) && (n > 0))				      \
	{							              \
		for (int i = 0; i < n ; i++)		  \
			fprintf( file, "%g ", (ar)[i]);	  \
	}										  \
	fputc('\n', file)

#define WRITE_PARAM_AI(file, param, ar, n, indent)    \
	if ((ar) && (n > 0))				      \
	{							              \
		cxWriteTag(file, param, indent);      \
		WRITE_AI( file, ar, n)				  \
	}

#define WRITE_PARAM_AF(file, param, ar, n, indent)    \
	if ((ar) && n > 0)						\
	{										\
		cxWriteTag(file, param, indent);	\
		WRITE_AF( file, ar, n)				\
	}

#define WRITE_PARAM_AS(file, param, ar, n, indent)  \
	if ((ar) && n > 0)								\
	{												\
		cxWriteTag(file, param, indent);			\
		for (int i = 0; i < n ; i++)				\
		{											\
			icxWriteIndent(ostr , indent + 1);		\
			fputs( (ar)[i], file);					\
			fputc('\n', file);						\
		}											\
	}

#define WRITE_PARAM_VI(file, param, ar, n, indent)  \
	cxWriteTag(file, param, indent);				\
	WRITE_VI(file, ar, n)

#define WRITE_PARAM_VF(file, param, ar, n, indent)  \
	cxWriteTag(file, param, indent);				\
	WRITE_VF(file, ar, n)

#define FWRITE_PARAM_AI(param, ar, n) WRITE_PARAM_AI(file, param, ar, n, indent)
#define FWRITE_PARAM_AF(param, ar, n) WRITE_PARAM_AF(file, param, ar, n, indent)
#define FWRITE_PARAM_AS(param, ar, n) WRITE_PARAM_AS(file, param, ar, n, indent)

#define FWRITE_AI(ar, n) WRITE_AI(file, ar, n)
#define FWRITE_AF(ar, n) WRITE_AF(file, ar, n)
#define FWRITE_VI(ar, n) WRITE_VI(file, ar, n)
#define FWRITE_VF(ar, n) WRITE_VF(file, ar, n)

#define FWRITE_PARAM_VI(param, ar, n) WRITE_PARAM_VI(file, param, ar, n, indent)
#define FWRITE_PARAM_VF(param, ar, n) WRITE_PARAM_VF(file, param, ar, n, indent)

//// Section begin / end / skip
#define BEGIN_SECTION(section)				 \
	cxWriteTag(file, section, indent, TRUE); \
	indent++							
				
#define END_SECTION(section)				 \
	cxWriteEndTag(file, indent-1, section)

#define SKIP_START()					 \
	if (strcmp(buf, SECTION_START) == 0) \
		ok = cxSkipSection(file, buf)

#define ELSE_SKIP_START() else SKIP_START()
#define SKIP_BEGIN()		    \
	if (!cxSkipBegin(file,buf)) \
		return FALSE

#define INDENT() icxWriteIndent(file , indent + 1)
	

#define SURE_GOTO __asm jmp

#endif
