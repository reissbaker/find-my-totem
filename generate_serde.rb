require 'erb'
require 'fileutils'
require 'pathname'

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

template = ERB.new(File.read("./serde.h.erb"))
output = template.result(packet.generate_binding)
output_dir = Pathname.new("./generated")
FileUtils.mkdir_p(output_dir)
output_path = output_dir.join("serde.h")
File.write(output_path, output)
puts "Generated #{output_path}"
