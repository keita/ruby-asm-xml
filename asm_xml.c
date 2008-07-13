#include "ruby.h"
#include "asm-xml.h"

static VALUE mAsmXml;
static VALUE Element;
static VALUE ClassContext;
static VALUE ElementClass;
static VALUE ParseContext;

static VALUE
rbax_s_parse(argc, argv, mod)
     int argc;
     VALUE *argv;
     VALUE mod;
{
  VALUE doc, scheme, opts, obj;
  AXClassContext ccontext;
  AXElementClass *eclass;
  AXParseContext pcontext;
  AXElement *elt;

  rb_scan_args(argc, argv, "21", &doc, &scheme, &opts);

  /* scheme -> element class */
  ax_initializeClassParser(&ccontext);
  eclass = ax_classFromString(StringValuePtr(scheme), &ccontext);
  if (eclass == NULL) return Qnil;

  /* parse */
  ax_initializeParser(&pcontext, 5*1024*1024);
  elt = ax_parse(&pcontext, StringValuePtr(doc), eclass, 1);

  obj = Data_Wrap_Struct(Element, 0, 0, elt);
  rb_ivar_set(obj,
	      rb_intern("@element_class"),
	      Data_Wrap_Struct(ElementClass, 0, 0, eclass));
  return obj;
}

VALUE
rbax_element_element_class(self)
     VALUE self;
{
  return rb_ivar_get(self, rb_intern("@element_class"));
}

VALUE
rbax_element_next(self)
     VALUE self;
{
  AXElement *elt, *next;
  Data_Get_Struct(self, AXElement, elt);
  next = elt->nextSibling;
  if (next == NULL) return Qnil;

  return Data_Wrap_Struct(Element, 0, 0, next);
}

VALUE
rbax_element_first_child(self)
     VALUE self;
{
  AXElement *elt, *child;

  Data_Get_Struct(self, AXElement, elt);
  child = elt->firstChild;
  if (child == NULL) return Qnil;

  return Data_Wrap_Struct(Element, 0, 0, child);
}

VALUE
rbax_element_last_child(self)
     VALUE self;
{
  AXElement *elt, *child;

  Data_Get_Struct(self, AXElement, elt);
  child = elt->lastChild;
  if (child == NULL) return Qnil;

  return Data_Wrap_Struct(Element, 0, 0, child);
}

VALUE
rbax_element_each(self)
     VALUE self;
{
  AXElement *elt, *child;
  Data_Get_Struct(self, AXElement, elt);
  child = elt->firstChild;
  while (child) {
    rb_yield(Data_Wrap_Struct(Element, 0, 0, child));
    child = child->nextSibling;
  }
  return Qnil;
}

VALUE
rbax_element_attributes(self, idx)
     VALUE self, idx;
{
  AXElement *elt;
  AXAttribute *attr;
  const char *ptr = NULL;
  size_t len;
  VALUE str;

  Data_Get_Struct(self, AXElement, elt);
  attr = &elt->attributes[NUM2INT(idx)];
  len = attr->limit - attr->begin;
  str = rb_str_new(ptr, len);
  memcpy(StringValuePtr(str), attr->begin, len);
  return str;
}

VALUE
rbax_ec_property_count(self)
     VALUE self;
{
  AXElementClass *eclass;
  Data_Get_Struct(self, AXElementClass, eclass);
  return UINT2NUM(eclass->propertyCount);
}

VALUE
rbax_ec_child_count(self)
     VALUE self;
{
  AXElementClass *eclass;
  Data_Get_Struct(self, AXElementClass, eclass);
  return UINT2NUM(eclass->childCount);
}

void
Init_asm_xml()
{
  ax_initialize(malloc, free);
  mAsmXml = rb_define_module("AsmXml");
  rb_define_singleton_method(mAsmXml, "parse", rbax_s_parse, -1);
  Element = rb_define_class_under(mAsmXml, "Element", rb_cData);
  rb_define_method(Element, "element_class", rbax_element_element_class, 0);
  rb_define_method(Element, "next", rbax_element_next, 0);
  rb_define_method(Element, "first_child", rbax_element_first_child, 0);
  rb_define_method(Element, "last_child", rbax_element_last_child, 0);
  rb_define_method(Element, "each_child", rbax_element_each, 0);
  rb_define_method(Element, "attributes", rbax_element_attributes, 1);
  ClassContext = rb_define_class_under(mAsmXml, "ClassContext", rb_cData);
  ElementClass = rb_define_class_under(mAsmXml, "ElementClass", rb_cData);
  rb_define_method(ElementClass, "property_count", rbax_ec_property_count, 0);
  rb_define_method(ElementClass, "child_count", rbax_ec_child_count, 0);
  ParseContext = rb_define_class_under(mAsmXml, "ParseContext", rb_cData);
}
