/*****************************************************************
|
|   Platinum - Xml Helper
|
|   Copyright (c) 2004-2008 Sylvain Rebaud
|   Author: Sylvain Rebaud (sylvain@rebaud.com)
|
 ****************************************************************/

#ifndef _PLT_XML_HELPER_H_
#define _PLT_XML_HELPER_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
/*----------------------------------------------------------------------
|   PLT_XmlAttributeFinder
+---------------------------------------------------------------------*/
class PLT_XmlAttributeFinder
{
public:
    // if 'namespc' is NULL, we're looking for ANY namespace
    // if 'namespc' is '\0', we're looking for NO namespace
    // if 'namespc' is non-empty, look for that SPECIFIC namespace
    PLT_XmlAttributeFinder(const NPT_XmlElementNode& element, 
                           const char*               name, 
                           const char*               namespc) : 
      m_Element(element), m_Name(name), m_Namespace(namespc) {}

    bool operator()(const NPT_XmlAttribute* const & attribute) const {
        if (attribute->GetName() == m_Name) {
            if (m_Namespace) {
                const NPT_String& prefix = attribute->GetPrefix();
                if (m_Namespace[0] == '\0') {
                    // match if the attribute has NO namespace
                    return prefix.IsEmpty();
                } else {
                    // match if the attribute has the SPECIFIC namespace
                    // we're looking for
                    const NPT_String* namespc = m_Element.GetNamespaceUri(prefix);
                    return namespc && *namespc == m_Namespace;
                }
            } else {
                // ANY namespace will match
                return true;
            }
        } else {
            return false;
        }
    }

private:
    const NPT_XmlElementNode& m_Element;
    const char*               m_Name;
    const char*               m_Namespace;
};

/*----------------------------------------------------------------------
|   PLT_XmlHelper
+---------------------------------------------------------------------*/
class PLT_XmlHelper
{
public:

    // static methods
    static NPT_Result GetChildText(NPT_XmlElementNode* node, 
                                   const char*         tag, 
                                   NPT_String&         value,
                                   const char*         namespc = "") {
        value = "";

        if (!node) return NPT_FAILURE;

        // special case "" means we look for the same namespace as the parent
        if (namespc && namespc[0] == '\0') namespc = node->GetNamespace()?node->GetNamespace()->GetChars():NPT_XML_NO_NAMESPACE;

        NPT_XmlElementNode* child = node->GetChild(tag, namespc);
        if (!child) return NPT_FAILURE;

        const NPT_String* text = child->GetText();
        value = text?*text:"";
        return NPT_SUCCESS;
    }

                                   
    static NPT_Result GetAttribute(NPT_XmlElementNode* node, 
                                   const char*         name,
                                   NPT_XmlAttribute*&  attr,
                                   const char*         namespc = "") {
        attr = NULL;

        if (!node) return NPT_FAILURE;

        // special case "" means we look for the same namespace as the parent
        if (namespc && namespc[0] == '\0') namespc = node->GetNamespace()?node->GetNamespace()->GetChars():NPT_XML_NO_NAMESPACE;

        NPT_List<NPT_XmlAttribute*>::Iterator attribute;
        attribute = node->GetAttributes().Find(PLT_XmlAttributeFinder(*node, name, namespc));
        if (!attribute) return NPT_FAILURE;

        attr = (*attribute);
        return NPT_SUCCESS;
    }


    static NPT_Result GetAttribute(NPT_XmlElementNode* node, 
                                   const char*         name,
                                   NPT_String&         value,
                                   const char*         namespc = "") {
        value = "";
        
        NPT_XmlAttribute* attribute = NULL;
        NPT_CHECK(GetAttribute(node, name, attribute, namespc));
        if (!attribute) return NPT_FAILURE;

        value = attribute->GetValue();
        return NPT_SUCCESS;
    }

    static NPT_Result AddChildText(NPT_XmlElementNode* node,
                                   const char*         tag,
                                   const char*         text,
                                   const char*         prefix = NULL) {
        if (!node) return NPT_FAILURE;
        NPT_XmlElementNode* child = new NPT_XmlElementNode(prefix, tag);
        child->AddText(text);
        return node->AddChild(child);
    }

    static bool IsMatch(const NPT_XmlNode* const & node, const char* tag, const char* namespc_mapped) {
        // if m_Namespace is NULL, we're looking for ANY namespace
        // if m_Namespace is '\0', we're looking for NO namespace
        // if m_Namespace is non-empty, look for that SPECIFIC namespace
        
        const NPT_XmlElementNode* element = node->AsElementNode();
        // is tag the same (case sensitive)?
        if (element && element->GetTag() == tag) {
            if (namespc_mapped) {
                // look for a SPECIFIC namespace or NO namespace
                const NPT_String* namespc = element->GetNamespace();
                if (namespc) {
                    // the element has a namespace, match if it is equal to
                    // what we're looking for
                    return *namespc == namespc_mapped;
                } else {
                    // the element does not have a namespace, match if we're
                    // looking for NO namespace
                    return namespc_mapped[0] == '\0';
                }
            } else {
                // ANY namespace will match
                return true;
            }
        }
        return false;
    } 

    static NPT_Result GetChildren(NPT_XmlElementNode*              node,
                                  NPT_Array<NPT_XmlElementNode*>&  children,
                                  const char*                      tag,
                                  const char*                      namespc = "") {
        if (!node) return NPT_FAILURE;

        // special case "" means we look for the same namespace as the parent
        if (namespc && namespc[0] == '\0') namespc = node->GetNamespace()?node->GetNamespace()->GetChars():NPT_XML_NO_NAMESPACE;

        const char* namespc_mapped = (namespc==NULL)?"":(namespc[0]=='*' && namespc[1]=='\0')?NULL:namespc;

        // get all children first
        NPT_List<NPT_XmlNode*>& allchildren = node->GetChildren();

        // iterate through children and add only elements with matching tag
        NPT_List<NPT_XmlNode*>::Iterator child = allchildren.GetFirstItem();
        while  (child) {
            if (IsMatch(*child, tag, namespc_mapped)) {
                children.Add((*child)->AsElementNode());
            }
            ++child;
        }
        return NPT_SUCCESS;
    }

    static NPT_XmlElementNode* GetChild(NPT_XmlElementNode* node,
                                        const char*         tag,
                                        const char*         namespc = "") {
        if (!node) return NULL;

        // special case "" means we look for the same namespace as the parent
        if (namespc && namespc[0] == '\0') namespc = node->GetNamespace()?node->GetNamespace()->GetChars():NPT_XML_NO_NAMESPACE;

        return node->GetChild(tag, namespc);
    }

    static NPT_Result GetChild(NPT_XmlElementNode*  parent,
                               NPT_XmlElementNode*& child,
                               NPT_Ordinal          n = 0) {
        if (!parent) return NPT_FAILURE;

        // reset child
        child = NULL;

        // get all children first
        NPT_List<NPT_XmlNode*>::Iterator children = parent->GetChildren().GetFirstItem();
        while  (children) {
            if ((*children)->AsElementNode() && n-- == 0) {
                child = (*children)->AsElementNode();
                return NPT_SUCCESS;
            }
            children++;
        }

        return NPT_FAILURE;
    }

    static NPT_Result Serialize(NPT_XmlNode& node, NPT_String& xml) {
        NPT_XmlWriter writer(2);
        NPT_MemoryStreamReference stream(new NPT_MemoryStream());
        NPT_CHECK(writer.Serialize(node, *stream));

        NPT_Size size;
        stream->GetAvailable(size);
        xml.Reserve(size);
        stream->Read(xml.UseChars(), size);
        xml.SetLength(size);
        return NPT_SUCCESS;
    }
private:
    // members
};

#endif // _PLT_XML_HELPER_H_









