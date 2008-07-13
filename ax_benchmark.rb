require "benchmarkx"
require "asm_xml"
require "rexml/document"
require "xml/libxml"

data = File.read("asm-xml-1.0/examples/data/employees-big.xml")
scheme = File.read("asm-xml-1.0/examples/data/employees-schema.xml")

BenchmarkX.bm do |x|
  x.filename = "result.png"
  x.gruff.title = "Parsing speed of 'employees-big.xml'"
  x.report("AsmXml") { AsmXml.parse(data, scheme) }
  x.report("libxml") { XML::Reader.new(data) }
  x.report("REXML") { REXML::Document.new(data) }
end
