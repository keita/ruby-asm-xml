///////////////////////////////////////////////////////////////////////////////
// docbook-to-html.cpp - rudimentary docbook to HTML converter
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asm-xml.h"

#ifdef WIN32
  #define SEP "\\"
#else
  #define SEP "/"
#endif

static const int  chunkSize = 1*1024*1024; // 1M
static const char schemaFilename[] = "data" SEP "docbook-schema.xml";

// Very poor file management: files are loaded in this buffer,
// a file bigger than this buffer will make the app crashes.
static char buffer[65536];

//[of]:HTMLCharMap
//[c]Quick detection of character to be escaped
//[c]
const char hcm_ok   = '\0';
const char hcm_amp  = '\1';
const char hcm_lt   = '\2';
const char hcm_gt   = '\3';

class HTMLCharMap
{
public:
  char map[256];

  HTMLCharMap()
  {
    char* p = map;
    char* limit = p+256;
    while( p<limit ) *p++ = hcm_ok;
    map['&']  = hcm_amp;
    map['<']  = hcm_lt;
    map['>']  = hcm_gt;
  }
};

static HTMLCharMap charMap;

//[cf]
//[of]:HTMLBuffer
//[of]:Definition
class HTMLBuffer
{
private:
  char*   base    ; // 
  char*   limit   ; //
  int     tally   ; // allocated buffer
  int     size    ; // used buffer
  char*   current ;

  void reserve(int n);
  inline void ensure(int n) { if( current + n > limit ) reserve(n); }

public:
  HTMLBuffer();
  ~HTMLBuffer();

  char*   getBuffer() const;
  int     getSize() const;

  void appendText(const char* text, int n);
  void append(const char* s, int n);
  void append(const char* s);

  const char* toString() { append("", 1); return base; }
};
//[cf]
//[c]
//[of]:Constructor
#define INITIAL_SPACE   65536

HTMLBuffer::HTMLBuffer()
{
  base    = new char[INITIAL_SPACE];
  limit   = base + INITIAL_SPACE;
  current = base;
  tally   = INITIAL_SPACE;
  size    = 0;
}
//[cf]
//[of]:Destructor
HTMLBuffer::~HTMLBuffer()
{
  delete []base;
}
//[cf]
//[c]
//[of]:getBuffer
char* HTMLBuffer::getBuffer() const
{
  return base;
}
//[cf]
//[of]:getSize
int HTMLBuffer::getSize() const
{
  return size;
}
//[cf]
//[c]
//[of]:reserve
void HTMLBuffer::reserve(int n)
{
  int len   = current - base;
  int newTally = len + n + tally / 2 + 1;

  char* old = base;
  base  = new char[newTally];
  limit = base + newTally;
  tally = newTally;
  memcpy(base, old, len);
  delete []old;
  current = base + len;
}
//[cf]
//[of]:appendText
void HTMLBuffer::appendText(const char* text, int n)
{
  const char* limit = text + n;
  const char* const map = charMap.map;
  const char* s = text;
  const char* start = s;
  while(1)
  {
    do { } while( s < limit && map[*s++] == hcm_ok );
    if( s < limit ) --s;
    int n = s - start;
    ensure(n);
    memcpy(current, start, n);
    current += n;
    if( s == limit ) return;

    switch( map[*s] )
    {
      case hcm_amp:
        append("&amp;", 5);
        break;
      case hcm_lt:
        append("&lt;", 4);
        break;
      case hcm_gt:
        append("&gt;", 4);
        break;
      default:
        ;
    }
    ++s;
    start = s;
  }
}
//[cf]
//[of]:append
void HTMLBuffer::append(const char* text, int n)
{
  ensure(n);
  memcpy(current, text, n);
  current += n;
}
//[cf]
//[of]:append
void HTMLBuffer::append(const char* text)
{
  append(text, strlen(text));
}
//[cf]
//[cf]
//[of]:DocbookToHTML
//[of]:Definition
class DocbookToHTML : public HTMLBuffer
{
protected:
  void appendAttribute(AXAttribute* attr);

  void doElement(AXElement* e);
  void doChapter(AXElement* chapter);

public:
  void processBook(AXElement* book);
};
//[cf]
//[of]:Element Ids
//[c]Element Ids - these ids must match the ones defined in docbook-schema
//[c]
enum ids
{
  id_pcdata,

  id_article = 20,
  id_preface,
  id_chapter,
  id_section,
  id_simplesect,

  id_itemizedlist = 40,
  id_listitem,
  
  id_emphasis = 100,
  id_filename,
  id_varname,
  id_guimenuitem,
  id_guimenu,
  id_code,

  id_para = 200,
  id_note,
  id_programlisting,

};
//[cf]
//[c]
//[of]:appendAttribute
void DocbookToHTML::appendAttribute(AXAttribute* attr)
{
  const char* start = attr->begin;
  const char* limit = attr->limit;
  size_t size = limit - start;
  appendText(start, size);
}
//[cf]
//[of]:doElement
void DocbookToHTML::doElement(AXElement* e)
{
  int  level = 2;
  bool start = true;

  e = e->firstChild;
  while( e )
  {
    switch( e->id )
    {
      case id_pcdata:
        appendAttribute(ax_getAttribute(e, 0));
        break;

      case id_article:
        doElement(e);
        break;

      case id_chapter:
      case id_preface:
      case id_section:
      case id_simplesect:
      {
        char buf[1];
        buf[0] = '0'+level;
        AXElement* title = ax_getElement(e, 0);
        append("\n\n<h");
        append(buf, 1);
        append(">");
        appendAttribute(ax_getAttribute(title, 0));
        append("</h");
        append(buf, 1);
        append(">\n\n");
        doElement(e);
        break;
      }
      
      case id_programlisting:
        append("<pre>");
        appendAttribute(ax_getAttribute(e, 0));
        append("</pre>");
        break;

      case id_itemizedlist:
        append("<ul>");
        doElement(e);
        append("</ul>");
        break;

      case id_listitem:
        append("<li>");
        doElement(e);
        append("</li>");
        break;

      case id_para:
      case id_note:
        if( !start ) append("\n\n<p>");
        doElement(e);
        if( !start ) append("</p>\n");
        start = false;
        break;

      case id_emphasis:
        append("<b>");
        doElement(e);
        append("</b>");
        break;

      case id_varname:
      case id_guimenuitem:
      case id_guimenu:
      case id_filename:
      case id_code:
        append("<tt>");
        doElement(e);
        append("</tt>");
        break;

    }
    e = e->nextSibling;
  }
}
//[cf]
//[of]:doChapter
void DocbookToHTML::doChapter(AXElement* chapter)
{
  AXElement* title = ax_getElement(chapter, 0);
  append("<h1>");
  appendAttribute(ax_getAttribute(title, 0));
  append("</h1>\n");

  doElement(chapter);

}
//[cf]
//[c]
//[of]:processBook
void DocbookToHTML::processBook(AXElement* book)
{
  AXElement* chapter = book->firstChild;
  while( chapter )
  {
    doChapter(chapter);
    chapter = chapter->nextSibling;
  }
}
//[cf]
//[cf]
//[c]
//[of]:printAsmXmlError
//[c]Print an error code from the parser
//[c]
void printAsmXmlError(AXParseContext* context)
{
  fprintf(stderr, "Error (%d,%d): %d\n", 
    context->line, 
    context->column, 
    context->errorCode);
}
//[cf]
//[of]:readClass
//[c]Read Schema Definition
//[c]
AXElementClass* readClass(const char* filename, AXClassContext* classContext)
{
  FILE*    f;
  size_t  size;
  
  f = fopen(filename, "rb");
  if( f == NULL )
  {
    fprintf(stderr, "can't open schema '%s'\n", filename);
    return NULL;
  }
  size = fread(buffer, 1, 65535, f);
  buffer[size] = 0;
  fclose(f);

  // Parse the string and build the class
  return ax_classFromString(buffer, classContext);
}
//[cf]
//[of]:readDocument
//[c]Read Document
//[c]
AXElement* readDocument(const char*     filename, 
                        AXParseContext* parseContext,
                        AXElementClass* clazz)
{
  FILE*    f;
  size_t  size;
  
  f = fopen(filename, "rb");
  if( f == NULL )
  {
    fprintf(stderr, "can't open file '%s'\n", filename);
    return NULL;
  }
  size = fread(buffer, 1, 65535, f);
  buffer[size] = 0;
  fclose(f);

  // Parse the string and build the class
  return ax_parse(parseContext, buffer, clazz, 1);
}
//[cf]
//[of]:main
int main(int argc, char *argv[])
{
  if( argc != 2 )
  {
    puts("usage: db2html <docbook_file>");
    return 1;
  }

  ax_initialize((void*)malloc, (void*)free);

  AXClassContext classContext;
  int res = ax_initializeClassParser(&classContext);
  if( res != 0 )
    return 1;

  AXElementClass* docbookClass = readClass(schemaFilename, &classContext);
  if( docbookClass == NULL )
    return 1;

  AXParseContext parseContext;
  res = ax_initializeParser(&parseContext, chunkSize);
  if( res != 0 )
    return 1;

  AXElement* book = readDocument(argv[1], &parseContext, docbookClass);
  if( book == NULL )
  {
    printAsmXmlError(&parseContext);
    return 1;
  }

  DocbookToHTML db;
  db.processBook(book);
  puts(db.toString());

  // Release the document and its class
  ax_releaseParser(&parseContext);
  ax_releaseClassParser(&classContext);
  return 0;
}
//[cf]
