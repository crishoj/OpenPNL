#ifndef __PNLTIMER_HPP__
#define __PNLTIMER_HPP__

#include "pnlLog.hpp"
#include <sstream>
#include <time.h>

#ifdef PNL_RTTI
#include "pnlpnlType.hpp"
#endif 

#define PNL_TIMERLOG_LEVEL_AND_SERVICE  pnl::eLOG_ALL, pnl::eLOGSRV_ALL

#define DEFAULT_HEADER                  " empty header "
#define DEFAULT_MESSAGE                 " time elapsed - "
#define DEFAULT_FOOTER                  " empty footer "

PNL_BEGIN

class PNL_API CTimerAndLogWriter : public CPNLBase
{
public:

    // ------------------ construction/desctruction ------------------- //

    static CTimerAndLogWriter* Create();
    
    static void                Release(CTimerAndLogWriter** pTimer);

    // ----------------- write available info to log ------------------ //

    inline void                WriteProblemHeader(const std::string& 
                                                  headerStr = DEFAULT_HEADER);

    inline void                WriteTimingResultsToLog( const string& message
                                                        = DEFAULT_MESSAGE,
                                                        bool TimeInSeconds
                                                        = true );

    inline void                WriteSumOfNTimerResults(bool TimeInSeconds
                                                       = true);

    inline void                WriteAverageOfNTimerResults(bool TimeInSeconds
                                                           = true);

    inline void                WriteProblemFooter(const std::string&
                                                  footerStr = DEFAULT_FOOTER);

    // -------------------------- utility ----------------------------- //

    inline void                SetDivider(const std::string& divider);    
    
    // --------------------------- timing ----------------------------- //
    
    inline void                StartTimer() const;
    
    inline void                StopTimer() const;
    
    inline void                SaveNextNTimerResults(int numOfResToSave) const;
    
#ifdef PNL_RTTI
  virtual const CPNLType &GetTypeInfo() const
  {
    return GetStaticTypeInfo();
  }
  static const CPNLType &GetStaticTypeInfo()
  {
    return CTimerAndLogWriter::m_TypeInfo;
  }
#endif

protected:
    
    CTimerAndLogWriter();
    
    ~CTimerAndLogWriter();

#ifdef PNL_RTTI
    static const CPNLType m_TypeInfo;
#endif 

protected:

    // ------------------------ write utility ------------------------- //

    inline void                WriteMsgToLog(const std::string& msg);

    inline void                WriteDivider();

    inline void                SkipLine();

    // ------------------------ timing utility ------------------------ //

    inline bool                DoNeedToSaveThisResult() const;
    
    inline void                SaveCurrentTimerResult() const;

    inline bool                AreAllNResultsSaved() const;

    inline clock_t             GetSumOfNTimerResults() const;
    
    inline double              GetAverageOfNTimerResults() const;

private:

    Log                        m_timeLog;

    std::string                m_divider;

    mutable clock_t            m_startTime;
    
    mutable clock_t            m_finishTime;

    mutable int                m_numOfSavedTimerResults;

    mutable pnlVector<clock_t> m_savedNTimerResults;

};
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::
WriteProblemHeader(const std::string& headerStr)
{
    WriteDivider();
    
    WriteMsgToLog(headerStr);
    
    SkipLine();
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::
WriteTimingResultsToLog( const string& message, bool TimeInSeconds )
{
    std::stringstream msgstream;
    
    msgstream << message.c_str()
        << (double)( m_finishTime - m_startTime )
        / ( TimeInSeconds ? CLOCKS_PER_SEC : 1.0 )
        << ( TimeInSeconds ? " seconds " : " clocks " )
        << "\n";
    
    WriteMsgToLog(msgstream.str());
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::
WriteSumOfNTimerResults(bool TimeInSeconds)
{
    std::stringstream msgstream;
    
    msgstream << " a sum of last " 
        << m_numOfSavedTimerResults
        << " timer results is "
        << (double)GetSumOfNTimerResults()
        / ( TimeInSeconds ? CLOCKS_PER_SEC : 1.0 )
        << ( TimeInSeconds ? " seconds " : " clocks " )
        << "\n";
    
    WriteMsgToLog(msgstream.str());
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::
WriteAverageOfNTimerResults(bool TimeInSeconds)
{
    std::stringstream msgstream;
    
    msgstream << " an average of last " 
        << m_numOfSavedTimerResults
        << " timer results is "
        << (double)GetAverageOfNTimerResults()
        / ( TimeInSeconds ? CLOCKS_PER_SEC : 1.0 )
        << ( TimeInSeconds ? " seconds " : " clocks " )
        << "\n";
    
    WriteMsgToLog(msgstream.str());
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::
WriteProblemFooter(const std::string& footerStr)
{
    WriteMsgToLog(footerStr);

    WriteDivider();
    
    SkipLine();
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::
SetDivider(const std::string& divider)
{
    m_divider = divider;
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::StartTimer() const
{
    m_startTime = clock();
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::StopTimer() const
{
    m_finishTime = clock();

    if( DoNeedToSaveThisResult() )
    {
        SaveCurrentTimerResult();
    }
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::
SaveNextNTimerResults(int numOfResToSave) const
{
    PNL_CHECK_LEFT_BORDER( numOfResToSave, 2 );

    m_savedNTimerResults.resize(numOfResToSave);

    m_numOfSavedTimerResults = 0;
}
//////////////////////////////////////////////////////////////////////////

inline clock_t CTimerAndLogWriter::
GetSumOfNTimerResults() const
{
    if( !AreAllNResultsSaved() )
    {
        PNL_THROW( CAlgorithmicException,
            " not all N timer results accumulater " );
    }

    clock_t totalElapsedTime = 0;

    pnlVector<clock_t>::iterator timesIt   = m_savedNTimerResults.begin(),
                                 times_end = m_savedNTimerResults.end();

    for( ; timesIt != times_end; totalElapsedTime += *(timesIt++) );

    return totalElapsedTime;
}
//////////////////////////////////////////////////////////////////////////

inline double CTimerAndLogWriter::
GetAverageOfNTimerResults() const
{
    return (double)GetSumOfNTimerResults() / m_numOfSavedTimerResults;
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::
WriteMsgToLog(const std::string& msg)
{
    m_timeLog << msg.c_str() << "\n";
    
    m_timeLog.flush();
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::WriteDivider()
{
    WriteMsgToLog(m_divider);
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::SkipLine()
{
    WriteMsgToLog("\n");
}
//////////////////////////////////////////////////////////////////////////

inline bool CTimerAndLogWriter::DoNeedToSaveThisResult() const
{
    return !m_savedNTimerResults.empty()
        && ( m_numOfSavedTimerResults < m_savedNTimerResults.size() );
}
//////////////////////////////////////////////////////////////////////////

inline void CTimerAndLogWriter::SaveCurrentTimerResult() const
{
    PNL_CHECK_RANGES(m_numOfSavedTimerResults, 0,
        m_savedNTimerResults.size() - 1);
    m_savedNTimerResults[m_numOfSavedTimerResults++]
        = m_finishTime - m_startTime;
}
//////////////////////////////////////////////////////////////////////////

inline bool CTimerAndLogWriter::AreAllNResultsSaved() const
{
    return int(m_savedNTimerResults.size()) == m_numOfSavedTimerResults;
}
//////////////////////////////////////////////////////////////////////////

#ifdef PNL_DO_TIMING

#define PNL_CREATE_TIMER                       pnl::CTimerAndLogWriter* pTimer = pnl::CTimerAndLogWriter::Create()
#define PNL_RELEASE_TIMER                      pnl::CTimerAndLogWriter::Release(&pTimer)
#define PNL_WRITE_HEADER( headerStr )          pTimer->WriteProblemHeader(headerStr)
#define PNL_WRITE_FOOTER( footerStr )          pTimer->WriteProblemFooter(footerStr)
#define PNL_SAVE_NEXT_N_TIMER_RESULTS( N )     pTimer->SaveNextNTimerResults(N)
#define PNL_START_TIMER                        pTimer->StartTimer()
#define PNL_STOP_TIMER                         pTimer->StopTimer()
#define PNL_WRITE_TIME( str, insecs )          pTimer->WriteTimingResultsToLog( (str), (insecs) )
#define PNL_WRITE_SUM_OF_N_RESULTS(insecs)     pTimer->WriteSumOfNTimerResults(insecs)
#define PNL_WRITE_AVERAGE_OF_N_RESULTS(insecs) pTimer->WriteAverageOfNTimerResults(insecs)
#define PNL_EXEC_AND_TIME( fun, insecs ) \
    PNL_START_TIMER;                     \
    fun;                                 \
    PNL_STOP_TIMER;                      \
    {                                    \
        std::string msg = " ";           \
        msg += #fun;                     \
        msg += " was running for ";      \
        PNL_WRITE_TIME( msg, insecs );   \
    } assert(true)

#else

#define PNL_CREATE_TIMER 
#define PNL_RELEASE_TIMER 
#define PNL_WRITE_HEADER( headerStr ) 
#define PNL_WRITE_FOOTER( footerStr ) 
#define PNL_SAVE_NEXT_N_TIMER_RESULTS( N ) 
#define PNL_START_TIMER 
#define PNL_STOP_TIMER 
#define PNL_WRITE_TIME( str, insecs ) 
#define PNL_WRITE_SUM_OF_N_RESULTS(insecs) 
#define PNL_WRITE_AVERAGE_OF_N_RESULTS(insecs) 
#define PNL_EXEC_AND_TIME( fun, insecs ) fun;

#endif // DO_TIMING
//////////////////////////////////////////////////////////////////////////

// For inner use only - it is not exported
class CTimer
{
public:
    CTimer();
    void Start();
    void Stop();
    void Reset() { m_Duration = 0; }
    double Duration() const { return m_Duration; }
    double DurationLast() const { return m_DurationLast; }

private:
    double m_Duration;
    double m_DurationLast;
    char m_InnerUnionBuf[32];
};

PNL_END

#endif // __PNLTIMER_HPP__
