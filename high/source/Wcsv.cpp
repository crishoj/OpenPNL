#include "Wcsv.hpp"

PNLW_BEGIN

WLex::WLex(const char *fname, bool bRead, char delim): m_Delimiter(delim),
m_bEOF(false), m_bEOL(true), m_bRead(bRead)
{
    m_File = fopen(fname, bRead ? "rb":"wb");

    if(!m_File)
    {
	ThrowUsingError("can't open file", bRead ? "ReadCSV":"WriteCSV");
    }
}

WLex::WLex(): m_Delimiter(','), m_File(0), m_bEOF(false), m_bEOL(true),
	    m_bRead(true)
{
}

bool WLex::Open(const char *fname, bool bRead, char delim)
{
    m_File = fopen(fname, bRead ? "rb":"wb");
    m_Delimiter = delim;
    m_bEOF  = false;
    m_bEOL  = true;
    m_bRead = bRead;

    return m_File != 0;
}

WLex::~WLex()
{
    if(m_File)
    {
	fclose(m_File);
    }
}

bool WLex::GetValue(String *pStr)
{
    if(m_bEOF)
    {
	return false;
    }
    m_bEOL = false;
    pStr->resize(0);

    int ch;

    for(;;)
    {
	ch = getc(m_File);
	if(ch == EOF)
	{
	    m_bEOL = m_bEOF = true;
	    break;
	}
	if(ch == '\n')
	{
	    m_bEOL = true;
	    break;
	}

	if(IsEscape(ch))
	{
	    int chx = getc(m_File);

	    if(chx == EOF)
	    {
		m_bEOF = true;
		break;
	    }
	    pStr->append(1, char(ch));
	    continue;
	}
	if(IsDelimiter(ch))
	{
	    break;
	}
	pStr->append(1, char(ch));
    }

    return pStr->length() > 0;
}

void WLex::PutValue(String &str)
{
    if(!IsEol())
    {
	putc(Delimiter(), m_File);
    }
    else
    {
	m_bEOL = false;
    }
    for(int i = 0; i < int(str.length()); ++i)
    {
	if(IsSpecial(str[i]))
	{
	    putc('\\', m_File);
	}
	putc(str[i], m_File);
    }
}

bool WLex::IsSpecial(int ch) const
{
    return IsDelimiter(ch) || IsEscape(ch) || ch == '\n';
}

PNLW_END
