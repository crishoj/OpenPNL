// xmlwriter.h

#ifndef SMILE_XMLWRITER_H
#define SMILE_XMLWRITER_H

#include <string>

class IXmlWriter
{
public:
	virtual bool OpenFile(const char * filename) = 0;
	virtual bool CloseFile() = 0;

	virtual void OpenElement(const char * name) = 0;
	virtual void CloseElement(const char * name) = 0;

	virtual void WriteElement(
		const char * name, 
		const char * content = NULL, 
		bool escapeWhitespace = false) = 0;

	virtual void PushAttribute(const char *name, const char * value) = 0;
	virtual void PushAttribute(const char *name, int value) = 0;
	virtual void PushAttribute(const char *name, bool value) = 0;
	virtual void PushAttribute(const char *name, double value) = 0;

	virtual void Delete() = 0;

	class ScopedElement
	{
	public:	
		ScopedElement(IXmlWriter *writer, const char *name)
		{
			this->writer = writer;
			this->name = name;
			writer->OpenElement(name);
		}

		~ScopedElement()
		{
			writer->CloseElement(name.c_str());
		}
	
	private:
		std::string name;
		IXmlWriter *writer;
	};
};

class IXmlWriterExtension
{
public:	
	virtual void WriteExtension(IXmlWriter *) = 0;
	virtual void Delete() = 0;
};

#endif // SMILE_XMLWRITER_H
