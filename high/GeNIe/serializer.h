// serializer.h

#ifndef GENIE_INTERFACES_SERIALIZER_H
#define GENIE_INTERFACES_SERIALIZER_H

class ISerializer
{
public:
	virtual void WriteInt(int value) = 0;
	virtual void WriteDouble(double value) = 0;
	virtual void WriteString(const char * value) = 0;
	virtual void WriteBinary(const void *buffer, int length) = 0;

	virtual int ReadInt() = 0;
	virtual double ReadDouble() = 0;
	virtual std::string ReadString() = 0;
	virtual void ReadBinary(void *buffer, int length) = 0;
};

#endif // !GENIE_INTERFACES_SERIALIZER_H
