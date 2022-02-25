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