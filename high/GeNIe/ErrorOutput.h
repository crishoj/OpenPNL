// ErrorOutput.h

#ifndef GENIE_INTERFACES_ERROROUTPUT_H
#define GENIE_INTERFACES_ERROROUTPUT_H

class IErrorOutput
{
public:
	virtual void Write(bool throwRuntimeError, const char *format, ...) = 0;
};

#endif // !GENIE_INTERFACES_ERROROUTPUT_H
