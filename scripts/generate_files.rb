require 'erb'
require 'fileutils'
require 'pathname'

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

def generate_cpp(packet)
  name = "serde"
  output_dir = Pathname.new("../src/generated")

  header_path = Pathname.new("#{name}.h.erb")
  #source_path = Pathname.new("#{name}.cpp.erb")

  FileUtils.mkdir_p(output_dir)
  header_output_path = output_dir.join("#{name}.h")
  #source_output_path = output_dir.join("#{name}.cpp")
  generate_from_template(packet, header_path, header_output_path)
  #generate_from_template(packet, source_path, source_output_path)
end