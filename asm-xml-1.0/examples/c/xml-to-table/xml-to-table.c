///////////////////////////////////////////////////////////////////////////////
// xml-to-table.c - Convert an xml file to a tabular file
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

static const int  chunkSize = 16*1024*1024; // 16M
static const char schemaFilename[] = "data" SEP "employees-schema.xml";
static const char xmlFilename[]    = "data" SEP "employees.xml";
static const char outFilename[]    = "table.csv";

char buffer[65536];

///////////////////////////////////////////////////////////////////////////////
// Print an attribute / text value
///////////////////////////////////////////////////////////////////////////////
char* appendAttribute(char* p, AXAttribute* attr)
{
  const char* start = attr->begin;
  const char* limit = attr->limit;
  size_t size = limit - start;
  memcpy(p, start, size);
  return p + size;
}

///////////////////////////////////////////////////////////////////////////////
// Print an error code from the parser
///////////////////////////////////////////////////////////////////////////////
void printAsmXmlError(AXParseContext* context)
{
  fprintf(stderr, "Error (%d,%d): %d\n", context->line, context->column, context->errorCode);
}
  
///////////////////////////////////////////////////////////////////////////////
// Read Schema Definition
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// Read Document
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// main
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  int             res;
  AXClassContext  classContext;
  AXParseContext  parseContext;
  AXElementClass* employeeClass;
  AXElement*      employees;
  AXElement*      employee;
  char*           p;
  FILE*           out;

  // Initialize the AsmXml library
  //
  // Pass the malloc() and free() functions
  //
  ax_initialize(malloc, free);

  // Initialize the class context
  //
  // It can store one or more classes. Classes read with this
  // context are kept in memory as long as it is not released.
  //
  res = ax_initializeClassParser(&classContext);
  // An error while initialization means that allocation failed. 
  // It should never happen since it allocates only 4K.
  if( res != 0 )
    return 1;

  // Read the schema and compile it
  //
  employeeClass = readClass(schemaFilename, &classContext);
  if( employeeClass == NULL )
    return 1;

  // Initialize the parser
  //
  // Documents read with this parser will stay in memory as long as
  // the parser is not released.
  //
  // The choice of the chunk size is very important since the
  // performance can be affected by this value. The parser allocates
  // memory by chunks to reduce calls to malloc that can be very slow.
  // The ideal value is around 50% of the source XML to process.
  //
  res = ax_initializeParser(&parseContext, chunkSize);
  // An error while initialization means that initial allocation failed.
  if( res != 0 )
    return 1;

  // Read the file and parse it
  //
  employees = readDocument(xmlFilename, &parseContext, employeeClass);
  if( employees == NULL )
  {
    printAsmXmlError(&parseContext);
    return 1;
  }

  out = fopen(outFilename, "w");
  if( out == NULL )
  {
    fprintf(stderr, "Can't open output file");
    return 1;
  }
  
  // Write title of columns
  p = "Employee id;Manager id;First Name;Last Name;Email;Position\n";
  fwrite(p, strlen(p), 1, out);
  
  // Enumerate child elements
  employee = employees->firstChild;
  while( employee )
  {
    p = buffer;
    p = appendAttribute(p, &employee->attributes[0]);
    *p++ = ';';
    p = appendAttribute(p, &employee->attributes[1]);
    *p++ = ';';
    p = appendAttribute(p, &employee->attributes[2]);
    *p++ = ';';
    p = appendAttribute(p, &employee->attributes[3]);
    *p++ = ';';
    p = appendAttribute(p, &employee->attributes[4]);
    *p++ = ';';
    p = appendAttribute(p, &employee->attributes[5]);
    *p++ = '\n';

    fwrite(buffer, p - buffer, 1, out);
    
    employee = employee->nextSibling;
  }

  fclose(out);

  // Release the document and its class
  ax_releaseParser(&parseContext);
  ax_releaseClassParser(&classContext);
  return 0;
}
