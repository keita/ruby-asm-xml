require "asm_xml"

doc = File.read("asm-xml-1.0/examples/data/employees.xml")
scheme = File.read("asm-xml-1.0/examples/data/employees-schema.xml")
puts doc
puts scheme
elt = AsmXml.parse(doc, scheme)
child = elt.first_child

while child do
  p child
  child = child.next
end

elt.each_child do |child|
  puts "================================"
  puts "Employee id.: #{child.attributes(0)}"
  puts "Manager id..: #{child.attributes(1)}"
  puts "First Name..: #{child.attributes(2)}"
  puts "Last Name...: #{child.attributes(3)}"
  puts "Email.......: #{child.attributes(4)}"
  puts "Position....: #{child.attributes(5)}"
  child.attributes(6)
end

p elt.element_class
p elt.element_class.property_count
p elt.element_class.child_count
