require './packet'
require './generate_files'

packet = Packet.build do
  long  :id_prefix
  long  :id_suffix
  float :lat_degrees
  float :lon_degrees
  bool  :fix
end

generate_files(packet)