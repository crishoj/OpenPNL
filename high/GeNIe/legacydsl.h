// LegacyDsl.h

#ifndef GENIE_INTERFACES_LEGACYDSL_H
#define GENIE_INTERFACES_LEGACYDSL_H

struct LegacyDslFileInfo
{
	typedef std::vector<std::pair<std::string, std::string> > Links;
	
	struct Icon
	{
		std::string name;
		std::string description;
		COLORREF clrInterior;
		COLORREF clrText;
		COLORREF clrOutline;
		COLORREF outlineWidth;
		RECT position;
		void *submodel;
	};

	struct Outcome
	{
		std::string name;
		std::string label;
		std::string fix;
		std::string description;
		Links links;
	};

	struct Node : public Icon
	{
		int handle;
		int specNameFmt;
		std::string question;
		Links links;
		std::vector<Outcome> outcomes;
	};
	
	struct Submodel : public Icon
	{
		std::string id;
		void *handle;
		RECT windowPosition;
	};

	struct TextBox
	{
		std::string caption;
		RECT position;
		void *submodel;
	};

	std::string name;
	std::string description;
	int specNameFmt;
	
	std::vector<Node> nodes;
	std::vector<Submodel> submodels;
	std::vector<TextBox> textboxes;
};

#endif // !GENIE_INTERFACES_LEGACYDSL_H
