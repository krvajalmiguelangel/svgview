//////////////////////////////////////////////////////////////////////////////
// Name:        SVGURIReference.h
// Author:      Alex Thuering
// Copyright:   (c) 2005 Alex Thuering
// Licence:     wxWindows licence
// Notes:       generated by generate.py
//////////////////////////////////////////////////////////////////////////////

#ifndef WX_SVG_URI_REFERENCE_H
#define WX_SVG_URI_REFERENCE_H

#include "SVGAnimatedString.h"
#include "String_wxsvg.h"
#include "Element.h"
#include "SVGAnimatedType.h"

class wxSVGURIReference
{
  protected:
    wxSVGAnimatedString m_href;

  public:
    inline const wxSVGAnimatedString& GetHref() const { return m_href; }
    inline void SetHref(const wxSVGAnimatedString& n) { m_href = n; }
    inline void SetHref(const wxString& n) { m_href.SetBaseVal(n); }

  public:
    virtual ~wxSVGURIReference() {}
    bool HasAttribute(const wxString& name) const;
    wxString GetAttribute(const wxString& name) const;
    bool SetAttribute(const wxString& name, const wxString& value);
    wxSvgXmlAttrHash GetAttributes() const;
    bool SetAnimatedValue(const wxString& name, const wxSVGAnim