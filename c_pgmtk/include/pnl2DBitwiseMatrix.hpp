// pnl2DBitwiseMatrix.h: interface for the C2DBitwiseMatrix class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __PNL2DBITWISEMATRIX_H__
#define __PNL2DBITWISEMATRIX_H__

#include "pnlObject.hpp"

PNL_BEGIN

#define BITWISEMATRIX_BYTESIZE (8 * sizeof(unsigned char))

class PNL_API C2DBitwiseMatrix  : public CPNLBase
{
private:
	unsigned char** m_pMatrix;
	int		m_nHeight;
	int		m_nWidth;
	bool	CreateMatrix(int nWidth, int nHeight);
protected:
	C2DBitwiseMatrix();
public:
	void	ZeroOneRow(int nRow);
	bool	AddOneRowWithAnother(int nSourceRow, int nDestinationRow);
	int		GetWidth() const;
	int		GetHeight() const;
	void	SetValue(int nRow, int nCol, bool bValue);
	bool	GetValue(int nRow, int nCol);
	static	C2DBitwiseMatrix* Create(int nWidth, int nHeight);
	void	operator =(const C2DBitwiseMatrix &SrcMatrix);
	virtual ~C2DBitwiseMatrix();

};

PNL_END

#endif // !defined(AFX_PNL2DBITWISEMATRIX_H__15730FA5_58DB_460C_BBDB_1D4197025D64__INCLUDED_)
