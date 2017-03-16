#!ruby

#FILENAME = 'HappyLee_SMB_TAS.txt'

if ARGV.size < 1
  STDERR.print "Usage: ruby #{$0} [INPUT FILE(.txt)] [OUTPUT FILE(.c)]\n"
  exit
end

INPUTFILE   = ARGV[0]
OUTPUTFILE  = ARGV[1]

f = File.open( INPUTFILE, 'rb' )
w = File.open( OUTPUTFILE, "w" )

i = 0
n = f.read

n.each_byte { |c|
	w.printf "0x%02x,", c
    w.printf " // %d\n", i if ( i % 16 == 15 )
    i += 1
}
