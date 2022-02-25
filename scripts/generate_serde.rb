require 'pathname'
require './packet'
require './generate_files'

packet = Packet.build do
  long :id_prefix
  long :id_suffix
end

generate_cpp(packet)