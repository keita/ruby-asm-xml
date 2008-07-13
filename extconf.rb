require "mkmf"
require "rbconfig"

# set asm-xml header path
$CFLAGS << " -Iasm-xml-1.0/include -Wall"

# object
case Config::CONFIG['target_os']
when "linux"
  $LDFLAGS << " asm-xml-1.0/obj/elf/asm-xml.o"
end

# Makefile
create_makefile("asm_xml")
