// xmlreader.h

#ifndef SMILE_XMLREADER_H
#define SMILE_XMLREADER_H

#include <string>

class IXmlReader;

class IXmlHandler
{
public:	
	virtual void OnStartElement(IXmlReader *xmlReader) = 0;
	virtual void OnEndElement(IXmlReader *xmlReader) = 0;
};

class IXmlErrorHandler
{
public:
	virtual void OnParseError(const char *parserMessage, int line, int column) = 0;
	virtual void OnUnknownElement(const char *name, const char *parent, int line, int column) = 0;
	virtual void OnUnknownAttribute(const char *element, const char *attribute, int line, int column) = 0;
};

class IXmlBinding
{
public:	
	virtual void AddChild(const IXmlBinding *child, int occurences) = 0;
	virtual void AddAttribute(const char *attributeName, bool required) = 0;
	virtual void Delete() = 0;

#ifndef NDEBUG
	virtual void Dump(int indent, std::string &output) const = 0;
#endif

	enum
	{
		ChildRequired = 1,
		ChildMultiple = 2,
		ZeroOrMore = ChildMultiple,
		OneOrMore = ChildRequired | ChildMultiple,
		ExactlyOne = ChildRequired,
		ZeroOrOne = 0,
	};

	enum 
	{
		ValidateNone = 0,
		ValidateAttributes = 1,
		ValidateChildren = 2,
		ValidateAll = ValidateAttributes | ValidateChildren,
		ValidateInherit = 4,
	};
};

class IXmlReader
{
public:	
	virtual bool Parse(const char *filename, const IXmlBinding *root, IXmlErrorHandler *errorHandler) = 0;
	virtual void StopParse(const char* errorMessage) = 0;

	virtual void GetCurrentPosition(int &line, int &column) = 0;

	virtual const std::string& GetContent() = 0;
	virtual void GetUnescapedContent(std::string &unescaped) = 0;
	virtual void GetAttribute(const char *name, std::string &value) = 0;
	virtual bool GetAttribute(const char *name, bool &value, bool defValue) = 0;
	virtual bool GetAttribute(const char *name, int &value, int defValue) = 0;
	virtual bool GetAttribute(const char *name, double &value, double defValue) = 0;
		
	virtual IXmlBinding* CreateBinding(IXmlHandler *handler, const char *name, int validation) = 0;
	
	virtual void Delete() = 0;
};


template <class T>
class XmlDelegator : public IXmlHandler
{
public:
	XmlDelegator(
		T *subject, 
		void (T::*startFunc)(IXmlReader *),
		void (T::*endFunc)(IXmlReader *),
		IXmlReader *xmlReader,
		const char *element, 
		int validation = IXmlBinding::ValidateInherit)
	{
		binding = xmlReader->CreateBinding(this, element, validation);
		this->subject = subject;
		this->startFunc = startFunc;
		this->endFunc = endFunc;
	}

	~XmlDelegator() { binding->Delete(); }

	virtual void OnStartElement(IXmlReader *xmlReader)
	{
		if (startFunc) (subject->*startFunc)(xmlReader);
	}

	virtual void OnEndElement(IXmlReader *xmlReader)
	{
		if (endFunc) (subject->*endFunc)(xmlReader);
	}

	void AddChild(const XmlDelegator<T> &child, int occurences)
	{
		binding->AddChild(child.GetBindingInterface(), occurences);
	}

	void AddChild(IXmlBinding *child, int occurences)
	{
		binding->AddChild(child, occurences);
	}

	void AddAttribute(const char *attributeName, bool required)
	{
		binding->AddAttribute(attributeName, required);
	}
	
	IXmlBinding* GetBindingInterface() const { return binding; }

protected:
	T* GetSubject() { return subject; }
	
private:
	IXmlBinding *binding;
	T* subject;
	void (T::*startFunc)(IXmlReader *);
	void (T::*endFunc)(IXmlReader *);
};

#endif // SMILE_XMLREADER_H
