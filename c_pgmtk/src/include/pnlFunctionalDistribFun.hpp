/**
 * @file pnlFunctionalDistribFun.hpp
 * @brief Declaration of the CFunctionalDistribFun class.
 * 
 * This header file declares the CFunctionalDistribFun class.
 * 
 * Derived from pnlGaussianDistribFun.hpp.
 * 
 * See also: pnlFunctionalDistribFun.cpp
 * 
 * TODO: Full conversion
 */

#ifndef __PNLFUNCTIONALDISTRIBFUN_HPP__
#define __PNLFUNCTIONALDISTRIBFUN_HPP__

#include "pnlParConfig.hpp"
#include "pnlDistribFun.hpp"
#include "pnlFunction.hpp"

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

PNL_BEGIN

template <class Type> class C2DNumericDenseMatrix;

/**
 * @brief CFunctionalDistribFun class
 * 
 */
class PNL_API CFunctionalDistribFun : public CDistribFun
{
public:

    static CFunctionalDistribFun* Create(int isPotential, int NumberOfNodes,
            const CNodeType *const* NodeTypes, const CFunction* function);
    static CFunctionalDistribFun
    * CreateUnitFunctionDistribution(int NumberOfNodes,
            const CNodeType *const*nodeTypes, int isPotential = 1);

    static CFunctionalDistribFun* Copy(const CFunctionalDistribFun* pInpDistr);

    CDistribFun& operator=(const CDistribFun& pInputDistr);

    virtual CDistribFun* Clone() const;
    virtual CDistribFun* CloneWithSharedMatrices();

    void CreateDefaultMatrices(int typeOfMatrices);
    void AllocMatrix(const float *data, EMatrixType mType,
            int numberOfWeightMatrix = -1, const int *parentIndices = NULL);
    void AttachMatrix(CMatrix<float>* pMatrix, EMatrixType mType,
            int numberOfWeightMatrix = -1, const int *parentIndices = NULL,
            bool isMultipliedByDelta = 0);
    // void ReleaseAllMatrices();

    virtual bool IsValid(std::string* descriptionOut = NULL) const;

    void Update();

    CDistribFun *ConvertCPDDistribFunToPot() const;

    /* Might still need these...
     CDistribFun *CPD_to_pi(CDistribFun *const* allPiMessages,
     int *multParentIndices, int numMultNodes, int posOfExceptParent = -1,
     int maximizeFlag = 0) const;
     CDistribFun *CPD_to_lambda(const CDistribFun *lambda,
     CDistribFun *const* allPiMessages, int *multParentIndices,
     int numMultNodes, int posOfExceptNode = -1, int maximizeFlag = 0) const;
     */

    inline int GetFactorFlag() const;
    inline float GetCoefficient() const;
    void SetCoefficient(float coeff);
    
    virtual void MarginalizeData(const CDistribFun* pOldData,
            const int *DimOfKeep, int NumDimsOfKeep, int maximize);
    void ShrinkObservedNodes(const CDistribFun* pOldData,
            const int *pVarsObserved, const Value* const* pObsValues,
            int numObsVars, const CNodeType* pObsTabNT, const CNodeType* pObsGauNT);
    void ExpandData(const int* pDimsToExtend, int numDimsToExpand,
            const Value* const* valuesArray,
            const CNodeType* const*allFullNodeTypes);

    virtual void MultiplyInSelfData(const int *pBigDomain,
            const int *pSmallDomain, const CDistribFun *pOtherData);
    virtual void SumInSelfData(const int *pBigDomain, const int *pSmallDomain,
            const CDistribFun *pOtherData);
    virtual void DivideInSelfData(const int *pBigDomain,
            const int *pSmallDomain, const CDistribFun *pOtherData);
    /*	virtual void DivideData( const int *pDomain1, const int *pDomain2,
     const CDistribFun *pData1,
     const CDistribFun *pData2);*/
    virtual CMatrix<float> *GetMatrix(EMatrixType mType, int numWeightMat = -1,
            const int *parentIndices = NULL) const;
    virtual CMatrix<float> *GetStatisticalMatrix(EStatisticalMatrix mType,
            int *parentIndices = NULL) const;
    virtual void SetStatistics(const CMatrix<float> *pMat,
            EStatisticalMatrix matrix, const int* parentsComb = NULL);
    virtual void UpdateStatisticsEM(const CDistribFun* infData,
            const CEvidence *pEvidence = NULL, float weightingCoeff = 1.0f,
            const int* domain = NULL);
    virtual void UpdateStatisticsML(const CEvidence* const* pEvidences,
            int EvidenceNumber, const int *domain, float weightingCoeff = 1.0f);
    virtual void ClearStatisticalData();
    float ProcessingStatisticalData(float numEvidences);

    void BayesUpdateFactor(const CEvidence* const* pEvidences,
            int EvidenceNumber, const int *domain);
    void PriorToCPD(floatVecVector &parentPriors); //convert pseudocounts to probability
    void SetFreedomDegrees(int, int);

    virtual int IsEqual(const CDistribFun *dataToCompare, float epsilon,
            int withCoeff = 1, float* maxDifferenceOut = NULL) const;
    virtual CDistribFun* GetNormalized() const;
    virtual void Normalize();
    virtual CNodeValues* GetMPE();
    //methods to convert distribution with dense matrices to distribution with sparse
    //if its already sparse - return copy
    virtual CDistribFun* ConvertToSparse() const;

    //methods to convert distribution with sparse matrices to distribution with dense
    //if its already dense - return copy
    virtual CDistribFun* ConvertToDense() const;
    // virtual void GenerateSample(pValueVector &obsVls ) const;

    virtual int IsSparse() const;
    virtual int IsDense() const;

    virtual void Dump() const;

    ~CFunctionalDistribFun();

    //compute probability for given variable vector
    double ComputeProbability(const C2DNumericDenseMatrix<float>* pMatVariable,
            int asLog = 1, int numObsParents = 0,
            const int* obsParentsIndices = NULL,
            C2DNumericDenseMatrix<float>*const * pObsParentsMats = NULL) const;
    //asLog = 1 - it means we return only exponential order
    //asLog = 0 - we return the value of exponent with coefficient
    //asLog = 2 - return exponential order without processing of coefficient

    virtual int GetNumberOfFreeParameters() const;

#ifdef PAR_PNL
    void UpdateStatisticsML(CDistribFun *pDF);
#endif
    
#ifdef PNL_RTTI
    virtual const CPNLType &GetTypeInfo() const
    {
        return GetStaticTypeInfo();
    }
    static const CPNLType &GetStaticTypeInfo()
    {
        return CFunctionalDistribFun::m_TypeInfo;
    }
#endif
    
protected:
    void ReleaseAllMatrices();
    void ShrinkToMatrices(const int* keepPositions, int numKeepPositions,
            C2DNumericDenseMatrix<float>* obsMatrix,
            C2DNumericDenseMatrix<float>** resVecMatOut,
            C2DNumericDenseMatrix<float>** resSquareMatOut, float *coeffOut) const;
    //the method provide marginalization in conditions of no positions of Delta multiply
    void MarginalizeOnMatrices(const int* keepVariables, int numKeepVariables,
            const int* allVariableSizes, int numVariables,
            C2DNumericDenseMatrix<float>* vecMat,
            C2DNumericDenseMatrix<float>* squareMat, float coeff, int isInMoment,
            C2DNumericDenseMatrix<float>** resVecMatOut,
            C2DNumericDenseMatrix<float>** resSquareMatOut, float *resCoeffOut,
            int maximize) const;
    CFunctionalDistribFun(int isPotential, int NodeNumber,
            const CNodeType *const* nodeTypes, const float *dataMean,
            const float *dataCov, const float **dataWeight = NULL);
    CFunctionalDistribFun(int NumberOfNodes, const CNodeType *const* nodeTypes,
            const float *dataH, const float *dataK, float g = 0.0f);
    CFunctionalDistribFun(int NumberOfNodes, const CNodeType *const* nodeTypes,
            const float *dataMean, int isMoment, int isPotential);
    CFunctionalDistribFun(int NumberOfNodes, const CNodeType *const*nodeTypes,
            int isPotential, int isCanonical);
    CFunctionalDistribFun(const CFunctionalDistribFun & inpDistr);
    C2DNumericDenseMatrix<float> * GetBlock(intVector &ind1, intVector &ind2,
            intVector &ns, const C2DNumericDenseMatrix<float> *pMat,
            const C2DNumericDenseMatrix<float> *pMat1 = NULL);
    C2DNumericDenseMatrix<float> * FormXX(intVector& unclumpedDims,
            intVector& nsVec, bool bMeanCl, float nEv);
    C2DNumericDenseMatrix<float> * FormMuX(intVector& unclumpedDims,
            intVector& nsVec);
    C2DNumericDenseMatrix<float> * FormCov(intVector& nsVec, float nEv);

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif
private:

    CFunction* m_function;

    int m_numberOfDims;

    int m_bPotential;

    C2DNumericDenseMatrix<float> *m_pMatrixMean;
    C2DNumericDenseMatrix<float> *m_pMatrixCov;

    // C2DNumericDenseMatrix<float> **m_pMatricesWeight;

    float m_normCoeff;

    /* Canonical shit? Might need it...
     float m_g;
     C2DNumericDenseMatrix<float> *m_pMatrixH;
     C2DNumericDenseMatrix<float> *m_pMatrixK;
     */

    C2DNumericDenseMatrix<float> *m_pLearnMatrixMean;
    C2DNumericDenseMatrix<float> *m_pLearnMatrixCov;

    //Wishart priors params - only for moment form
    int m_freedomDegreeMean;
    int m_freedomDegreeCov;
    C2DNumericDenseMatrix<float> *m_pPseudoCountsMean;
    C2DNumericDenseMatrix<float> *m_pPseudoCountsCov;

};

/*
 * INLINE FUNCTIONS
 */

inline int CFunctionalDistribFun::GetFactorFlag() const
{
    return m_bPotential;
}

inline float CFunctionalDistribFun::GetCoefficient() const
{
    return m_normCoeff;
}

PNL_END

#endif //__PNLFUNCTIONALDISTRIBFUN_HPP__
