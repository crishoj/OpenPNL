#ifndef __WCSV_HPP__
#define __WCSV_HPP__

#ifndef __PNLHIGHCONF_HPP__
#include "pnlHighConf.hpp"
#endif

// FORWARDS

PNLW_BEGIN

class WLex
{
ALLOW_TESTING
public:
    WLex(const char *fname, bool bRead, char delim = ',');
    WLex();
    ~WLex();
    bool Open(const char *fname, bool bRead, char delim = ',');
    void SetDelimiter(char delim) { m_Delimiter = delim; }

    bool GetValue(String *pStr);
    void PutValue(String &str);

    bool IsEof() const { return m_bEOF; }
    bool IsEol() const { return m_bEOL; }
    void Eol() { m_bEOL = true; putc('\n', m_File); }

protected:
    bool IsSpecial(int ch) const;
    bool IsDelimiter(int ch) const { return m_Delimiter == ch; }
    bool IsEscape(int ch) const { return ch == '\\'; }
    int Delimiter() const { return m_Delimiter; }

private:
    int m_Delimiter;
    FILE *m_File;
    bool m_bEOF;
    bool m_bEOL;
    bool m_bRead;
};

PNLW_END

#endif //__WCSV_HPP__
