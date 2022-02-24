require 'erb'
require 'fileutils'
require 'pathname'

MAGIC_BYTE = 123

class FieldDef
  attr_reader :name, :type
  def initialize(name, type)
    @name = name
    @type = type
  end
end

types = [ :byte, :long ]

class Packet
  def initialize
    @fields = []
  end

  def generate_binding
    klass = Class.new
    fields = @fields
    klass.define_method(:fields) { fields }
    klass.define_method(:magic_byte) { MAGIC_BYTE }
    klass.new.instance_eval { binding }
  end

  class << self
    def build(&block)
      packet = Packet.new
      packet.instance_exec(&block)
      packet
    end
  end
end

types.each do |type|
  Packet.define_method(type) do |name|
    @fields << FieldDef.new(name, type)
  end
end

packet = Packet.build do
  long :id_prefix
  long :id_suffix
end

def strip_whitespace(str)
  str.gsub(/\s+$/, '')
end

def render_template(template, packet)
  template.result(packet.generate_binding)
end

def read_template(filename)
  ERB.new(File.read(filename))
end

def generate_from_template(packet, template_path, output_path)
  template = read_template(template_path)
  mod_warning = <<-EOF
/*
WARNING: GENERATED CODE
DO NOT MODIFY
YOUR MODIFICATIONS WILL BE OVERWRITTEN

Modify .#{template_path} rather than this file.
*/
EOF
  output = strip_whitespace(mod_warning + render_template(template, packet))
  File.write(output_path, output)
  puts "Generated #{output_path}"
end

output_dir = Pathname.new("./generated")
FileUtils.mkdir_p(output_dir)
header_output_path = output_dir.join("serde.h")
source_output_path = output_dir.join("serde.cpp")
generate_from_template(packet, "./serde.h.erb", header_output_path)
generate_from_template(packet, "./serde.cpp.erb", source_output_path)