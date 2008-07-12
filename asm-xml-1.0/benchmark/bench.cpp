///////////////////////////////////////////////////////////////////////////////
// bench.cpp - Compare speed of different parsers
//
///////////////////////////////////////////////////////////////////////////////

#define EXPAT 1

#ifdef WIN32 // Because I couldn't compile xerces on Linux
#define XERCES 1
#endif

#ifdef WIN32
  #include <windows.h>
  #include <io.h>
#else
  #include <stdlib.h>
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "asm-xml.h"

#ifdef EXPAT
#include "expat.h"
#endif

#ifdef XERCES
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#endif

//[of]:getTicks
long getTicks()
{
#ifdef WIN32
  return GetTickCount();
#else
  struct timeval  tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}
//[cf]
//[of]:readFile
static char* readFile(const char* filename) 
{
  FILE* f = fopen(filename, "rb");
  if( !f )
  {
    fprintf(stderr, "Can't open file '%s'\n", filename);
    return NULL;
  }

#ifdef WIN32
  HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if( hFile == INVALID_HANDLE_VALUE )
    return NULL;
  unsigned int size = GetFileSize(hFile, NULL);
  CloseHandle(hFile);
#else
  struct stat st;
  #ifdef MACOS
    fstat(f->_file, &st);
  #else
    fstat(f->_fileno, &st);
  #endif
  unsigned int size = st.st_size;
#endif

  char* buf = (char*)malloc(size+1);
  size = fread(buf,1, size, f);
  fclose(f);
  buf[size] = 0;
  return buf;
}
//[cf]
//[of]:AsmXml
//[c]
static const int  chunkSize = 16*1024*1024; // 16M
//[c]
class BenchApplication
{
public:
  AXClassContext  classContext;
  AXElementClass* docClass;

//[c]
//[of]:  initializeAsmXml
int initializeAsmXml(const char* schemaFilename)
{
  // Initialize the AsmXml library
  ax_initialize((void*)malloc, (void*)free);

  // Initialize the class context
  int res = ax_initializeClassParser(&classContext);
  if( res != 0 )
    return 1;

  // Read the schema and compile it
  char* buf = readFile(schemaFilename);
  if( !buf )
    return 1;
  docClass = ax_classFromString(buf, &classContext);
  free(buf);
  if( docClass == NULL )
    return 1;

  return 0;
}
//[cf]
//[of]:  releaseAsmXml
void releaseAsmXml()
{
  ax_releaseClassParser(&classContext);
}
//[cf]
//[of]:  parseAsmXml
//[c]Parses an XML string
//[c]
//[of]:  printAsmXmlError
void printAsmXmlError(AXParseContext* context)
{
  fprintf(stderr, "Error (%d,%d): %d\n", 
    context->line, 
    context->column, 
    context->errorCode);
}
//[cf]
//[c]
int parseAsmXml(const char* buf)
{
  AXParseContext parseContext;

  // Initialize the parser
  int res = ax_initializeParser(&parseContext, chunkSize);
  if( res != 0 )
    return 1;

  AXElement* root = ax_parse(&parseContext, buf, docClass, 1);
  if( root == NULL )
  {
    printAsmXmlError(&parseContext);
    return 1;
  }

  ax_releaseParser(&parseContext);
  return 0;
}
//[cf]
//[c]
};
//[c]
//[cf]
//[of]:Expat
#ifdef EXPAT

void starttag(void *UserData, const XML_Char *name, const XML_Char **attrs) 
{
}

void endtag(void *UserData, const XML_Char *name) 
{
}

void texttag(void *UserData, const XML_Char *data, int len) 
{
}

XML_Parser doc;
static int initializeExpat() 
{
    doc = XML_ParserCreate(NULL);
    XML_SetStartElementHandler(doc,&starttag);
    XML_SetEndElementHandler(doc,&endtag);
    XML_SetCharacterDataHandler(doc,&texttag);
    return 0;
}

static void releaseExpat() 
{
    XML_ParserFree(doc);
}

static int parseExpat(const char* buf, unsigned int len) 
{
  XML_Status status = XML_Parse(doc, buf, len, 1);
  if( status != XML_STATUS_OK )
    return 1;
  if( XML_ParserReset(doc, NULL) == XML_FALSE )
    return 1;
  return 0;
}

#endif
//[cf]
//[of]:Xerces
#ifdef XERCES

#if defined(XERCES_HAS_CPP_NAMESPACE)
    XERCES_CPP_NAMESPACE_USE
#endif

class DocHandlers : public HandlerBase 
{
public:
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name);
  void characters(const XMLCh* const text, const unsigned int len);
  void resetDocument();
};

void DocHandlers::startElement(const XMLCh* const name, AttributeList& attributes)
{
}

void DocHandlers::endElement(const XMLCh* const name)
{
}

void DocHandlers::characters(const XMLCh* const text, const unsigned int len) 
{
}

void DocHandlers::resetDocument() 
{
}

SAXParser* parser;
DocHandlers* handler;

void initializeXerces()
{
  XMLPlatformUtils::Initialize();
  parser = new SAXParser;
  handler = new DocHandlers;
  parser->setDocumentHandler(handler);
}

void releaseXerces() 
{
  delete handler;
  delete parser;
  XMLPlatformUtils::Terminate();
}

void parseXerces(const char* buf, unsigned long len)
{
  const char id[5]="ID1\0";
  MemBufInputSource* buffer;

  buffer = new MemBufInputSource((XMLByte*)buf, len, (const char*)&id);
  buffer->setCopyBufToStream(false);
  parser->parse(*buffer);
  delete buffer;
}

#endif
//[cf]
//[of]:main
int main(int argc, char *argv[])
{
  int repeat = atoi(argv[1]);
  printf("Iterations = %d \n", repeat);

  // Read the file
  char* buf = readFile(argv[3]);
  if( !buf )
    return 1;
  unsigned int size = strlen(buf);

  BenchApplication bench;
  if( bench.initializeAsmXml(argv[2]) )
    return 1;

  int i = repeat;
  long t1 = getTicks();
  do
  {
    if( bench.parseAsmXml(buf) )
      return 1;
  } while( --i > 0 );
  long t2 = getTicks();
  printf("AsmXml....: %ld ms\n", t2 - t1);
  bench.releaseAsmXml();

#ifdef EXPAT
  initializeExpat();
  i = repeat;
  t1 = getTicks();
  do
  {
    if( parseExpat(buf, size) )
      return 1;
  } while( --i > 0 );
  t2 = getTicks();
  printf("Expat.....: %ld ms\n", t2 - t1);
  releaseExpat();
#endif

#ifdef XERCES
  initializeXerces();
  i = repeat;
  t1 = getTicks();
  do
  {
    parseXerces(buf, size);
  } while( --i > 0 );
  t2 = getTicks();
  printf("Xerces.....: %ld ms\n", t2 - t1);
  releaseXerces();
#endif

  return 0;
}
//[cf]
