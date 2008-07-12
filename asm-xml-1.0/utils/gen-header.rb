# Simple C++ header generator
#
#	This utility generates a C++ header that includes constant and
#	structure definitions for processing a document parsed with
#	AsmXml.
#
# Usage:
#	ruby gen-header input-file > output-file
#
# Where
#	input-file is the AsmXml schema
#	output-file is the C++ header file
#

require "rexml/document"

InsertFoldMarkers = false
ClassNamePrefix = "AX"

#[of]:class Generator
class Generator

#[of]:initialize
def initialize(root)
	@decls = []
	@structs = []
	@defines = []
	@globals = Hash.new
	@globalElements = Hash.new
	@structNames = Hash.new

	root.each_element do |child|
		case child.name
		when "group"
			@globals[child.attributes['name']] = child
		when "element"
			@globalElements[child.attributes['name']] = child
		end
	end

	root.each_element do |child|
		case child.name
		when "collection"
			generateElementClass(child)
		when "element"
			generateElementClass(child)
		end
	end

end
#[cf]
#[of]:className
def className(node)
	name = node.attributes['name']
	name = name.gsub( /[-:]/, '_')
	s = ""
	i = 0
	n = name.size
	while i<n
		s << name[i,1].upcase
		i = i + 1
		while i<n and name[i] != ?_
			s << name[i]
			i = i+1
		end
		i = i + 1
	end
	return ClassNamePrefix + s
end
#[cf]
#[of]:fieldName
def fieldName(node)
	name = node.attributes['name']
	name = name.gsub( /[-:]/, '_')
	s = ""
	i = 0
	n = name.size
	while i<n
		if i > 0
			s << name[i,1].upcase
		else
			s << name[i,1]
		end
		i = i + 1
		while i<n and name[i] != ?_
			s << name[i]
			i = i+1
		end
		i = i + 1
	end
	return s
end
#[cf]
#[of]:align
def align(s)
	t = s
	n = 32 - t.size
	while n > 0
		t << " "
		n = n - 1
	end
	return t
end
#[cf]
#[of]:processChildren
def processChildren(node, s, padCount)

	node.each_element do |child|
		if child.attributes['ignore'] != "yes"
			case child.name
			when "attribute"
				s << "  #{align('AXAttribute')} #{fieldName(child)};\n" 
			when "text"
				s << "  #{align('AXAttribute')} #{fieldName(child)};\n" 
			when "element"
				childName = generateElementClass(child)
				s << "  #{align(childName+'*')} #{align(fieldName(child))};  void* _pad#{padCount};\n"
				padCount = padCount + 1
			when "collection"
				generateElementClass(child)
			when "include"
				groupName = child.attributes['name']
				group = @globals[groupName]
				raise "Error: group '#{groupName}' unknown" if !group
				padCount = processChildren(group, s, padCount)
			when "reference"
				child= @globalElements[child.attributes['name']]
				if child
					childName= className(child)
					s << "  #{align(childName+'*')} #{align(fieldName(child))};  void* _pad#{padCount};\n"
					padCount = padCount + 1
				end
			else
				raise "Error: #{child.name}"
			end
		end
	end

	return padCount
end
#[cf]
#[of]:generateElementClass
def generateElementClass(node)
	
	className = className(node)
	if @structNames.has_key? className
		return className
	end
	@structNames[className] = 1

	if node.attributes['id']
		@defines << "#define #{align(className+'Id')} #{node.attributes['id']}"
	end

	padCount = 1
	s = ""
	s << "//[of]:#{className}\n" if InsertFoldMarkers
	s << "struct #{className}\n"
	s << "{\n"
	s << "  int                              elementId;\n"
	s << "  AXElement*                       nextSibling;\n"
	s << "  AXElement*                       firstChild;\n"
	s << "  AXElement*                       lastChild;\n"
	s << "  AXAttribute                      reserved;\n"
	s << "\n"

	type = node.attributes['type']
	if type == "text"
		s << "  #{align('AXAttribute')} text;\n" 
	end
	processChildren(node, s, padCount)	

	s << "};\n\n"
	s << "//[cf]\n"  if InsertFoldMarkers
	@decls << "struct #{className};"
	@structs << s

	return className
end
#[cf]
#[of]:decls
def decls
	return @decls
end
#[cf]
#[of]:structs
def structs
	return @structs
end
#[cf]
#[of]:defines
def defines
	return @defines
end
#[cf]

end
#[cf]
#[of]:main
if ARGV.size != 1
	puts "Usage:"
	puts "	ruby gen-header input-file > output-file"
	exit	1
end

file = File.new(ARGV[0])
doc = REXML::Document.new file
root = doc.root

gen = Generator.new(root)
document = root.elements['document']
gen.generateElementClass(document)

puts "// This file is auto generated, do not edit."
puts ""
# dump defines
gen.defines.each { |s| puts s }
puts ""
gen.decls.each { |s| puts s }
puts ""
# dump structs
gen.structs.each { |s| puts s }

#[cf]
