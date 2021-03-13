#ifndef Html_Element
#define Html_Element

#include "HtmlNode.hpp"

#include <string>

class HtmlElement : HtmlNode
{
public:
    HtmlElement(GumboNode* node);
    bool isElement() override;
    bool isTagA() const;
    std::string getAttribute(const std::string& name);
};

#endif