// pnl2DBitwiseMatrix.cpp: implementation of the C2DBitwiseMatrix class.
//
//////////////////////////////////////////////////////////////////////
#include "pnlConfig.hpp"
#include "pnl2DBitwiseMatrix.hpp"
#include "pnlException.hpp"
#include <stdio.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
PNL_USING

C2DBitwiseMatrix::C2DBitwiseMatrix()
{
    m_nWidth =0;
    m_nHeight =0;
    m_pMatrix = NULL;
}

C2DBitwiseMatrix::~C2DBitwiseMatrix()
{
    for(int i=0; i<m_nHeight; i++)
	delete []m_pMatrix[i];
    delete []m_pMatrix;
}

bool C2DBitwiseMatrix::CreateMatrix(int nWidth, int nHeight)
{
    int i;

    if(m_pMatrix)
    {
	for(i=0; i<m_nHeight; i++)
	    delete []m_pMatrix[i];
	delete []m_pMatrix;
    }

    m_nHeight = nHeight;
    m_nWidth = nWidth;
    m_pMatrix = new unsigned char*[nHeight];

    int nLength = m_nWidth /BITWISEMATRIX_BYTESIZE;

    if(nLength*BITWISEMATRIX_BYTESIZE < m_nWidth)
	nLength ++;
    for(i=0; i<m_nHeight; i++)
    {
	m_pMatrix[i] = new unsigned char[nLength];
	for(int j=0; j<nLength; j++)
	    m_pMatrix[i][j] =0;
    }

    return true;
}
C2DBitwiseMatrix* C2DBitwiseMatrix::Create(int nWidth, int nHeight)
{
    C2DBitwiseMatrix* pRet = new C2DBitwiseMatrix();
    if(pRet->CreateMatrix(nWidth, nHeight))
	return pRet;
    delete pRet;
    return NULL;
}

bool C2DBitwiseMatrix::GetValue(int nRow, int nCol)
{
    if(nRow <0 || nRow>= m_nHeight)
    {
	PNL_THROW(COutOfRange, "Row out of range!");
	return false;
    }
    if(nCol <0 || nCol >=m_nWidth)
    {
	PNL_THROW(COutOfRange, "Column out of range!");
	return false;
    }

    unsigned char bValueByte = m_pMatrix[nRow][nCol/BITWISEMATRIX_BYTESIZE];
    int nMode = nCol % BITWISEMATRIX_BYTESIZE;
    unsigned char bValueMask = (unsigned char)(1 << nMode);

    return (bValueByte & bValueMask) != 0;
}

void C2DBitwiseMatrix::SetValue(int nRow, int nCol, bool bValue)
{
    if(nRow <0 || nRow>= m_nHeight)
    {
	PNL_THROW(COutOfRange, "Row out of range!");
	return;
    }
    if(nCol <0 || nCol >=m_nWidth)
    {
	PNL_THROW(COutOfRange, "Column out of range!");
	return;
    }

    unsigned char bValueByte = m_pMatrix[nRow][nCol/BITWISEMATRIX_BYTESIZE];
    int nMode = nCol % BITWISEMATRIX_BYTESIZE;
    unsigned char bValueMask = (unsigned char)(1 << nMode);

    if(bValue)
    {
	m_pMatrix[nRow][nCol/BITWISEMATRIX_BYTESIZE] = (unsigned char)(bValueByte | bValueMask);
    }
    else
    {
	bValueMask = (unsigned char)(~bValueMask);
	m_pMatrix[nRow][nCol/BITWISEMATRIX_BYTESIZE] = (unsigned char)(bValueByte & bValueMask);
    }
}

int C2DBitwiseMatrix::GetHeight() const
{
    return m_nHeight;
}

int C2DBitwiseMatrix::GetWidth() const
{
    return m_nWidth;
}

bool C2DBitwiseMatrix::AddOneRowWithAnother(int nSourceRow, int nDestinationRow)
{
    if(nSourceRow <0 || nSourceRow>= m_nHeight)
    {
	PNL_THROW(COutOfRange, "Row out of range!");
	return false;
    }
    if(nDestinationRow <0 || nDestinationRow>= m_nHeight)
    {
	PNL_THROW(COutOfRange, "Row out of range!");
	return false;
    }
    int nLength = m_nWidth / BITWISEMATRIX_BYTESIZE;
    if(nLength*BITWISEMATRIX_BYTESIZE < m_nWidth)
	nLength ++;
    for(int i=0; i<nLength; i++)
    {
	m_pMatrix[nDestinationRow][i] = (unsigned char)(m_pMatrix[nDestinationRow][i] | m_pMatrix[nSourceRow][i]);
    }
    return true;

}

void C2DBitwiseMatrix::operator =(const C2DBitwiseMatrix& SrcMatrix)
{
    CreateMatrix(SrcMatrix.GetWidth(), SrcMatrix.GetHeight());
    int nLength = m_nWidth / BITWISEMATRIX_BYTESIZE;
    if(nLength*BITWISEMATRIX_BYTESIZE < m_nWidth)
	nLength ++;
    for(int i=0; i<m_nHeight; i++)
	for(int j=0; j<nLength; j++)
	    m_pMatrix[i][j] = SrcMatrix.m_pMatrix[i][j];
}

void C2DBitwiseMatrix::ZeroOneRow(int nRow)
{
    int nLength = m_nWidth / BITWISEMATRIX_BYTESIZE;
    if(nLength*BITWISEMATRIX_BYTESIZE < m_nWidth)
	nLength ++;
    for(int i=0; i<nLength; i++)
	m_pMatrix[nRow][i] =0;
}

#ifdef PNL_RTTI
const CPNLType C2DBitwiseMatrix::m_TypeInfo = CPNLType("C2DBitwiseMatrix", &(CPNLBase::m_TypeInfo));

#endif