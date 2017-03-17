#!ruby

FORMAT_VERSION = "0.10"

PAD_RIGHT  = (1<<7)
PAD_LEFT   = (1<<6)
PAD_DOWN   = (1<<5)
PAD_UP     = (1<<4)
PAD_START  = (1<<3)
PAD_SELECT = (1<<2)
PAD_B      = (1<<1)
PAD_A      = (1<<0)

PAD_SELECT2 = (PAD_UP|PAD_DOWN)
PAD_RLEFLAG = (1<<2)

def usage()
	STDERR.print "Usage: ruby #{$0} [INPUT FILE(.txt)] [OUTPUT FILE(.h)]\n"
end

if ARGV.size < 1
	usage
	exit
end

INPUTFILE   = ARGV[0]
OUTPUTFILE  = ARGV[1]

MAX_RLE_LENGTH = 255	# 8bit


def rle( input )

    hash = Hash.new(0)

    dat = input.unpack("C*")
	buf = ""

	i = 0

	while( dat.size > i ) do
		pad = dat[i];	i += 1

		# セレクトボタンのビットをRLE用に
		if ( pad & PAD_SELECT ) == PAD_SELECT
			pad &= ~PAD_SELECT
			pad |= PAD_SELECT2
		end

		count = 0
		while( count < MAX_RLE_LENGTH ) do
			next_pad = dat[i];	i += 1
			count += 1

			break if next_pad != pad 
		end

		i -= 1

		if count == 1
			buf << [pad].pack("C")
		else
			buf << [pad | PAD_RLEFLAG].pack("C")
			buf << [count].pack("C")
		end
	end

	
#    hash.sort { |(k1,v1),(k2,v2)| v2<=>v1 }.each { |n|
#    	ratio = n[1].to_f / input.size.to_f
#    	printf( "0x%02x:%6d (%3.2f%%)\n", n[0], n[1], ratio * 100.0 )
#    }

	return buf
end

def rld( input )
	dat = input.unpack("C*")
	buf = ""

	i = 0
	while ( dat.length > i ) do
		pad   = dat[i];		i += 1
		count = 1

		if ( pad & PAD_RLEFLAG ) == PAD_RLEFLAG
			pad  &= ~PAD_RLEFLAG
			count = dat[i];		i += 1
		end

		count.times { buf += [pad].pack("C") }
	end

	return buf
end

def convert( inputfile, outputfile ) 
	data = File.open( inputfile, 'rb' ).read

	n     = rle( data )
    ratio = n.size.to_f / data.size.to_f
	printf( "\nOriginal size   :%6d\nCompressed size:%6d (%3.2f%%)", data.size, n.size, ratio * 100.0 )

	# decode
    dec = rld( n )
    if dec == data 
    else
    	puts "Verify error!"
    	exit -1
    end
#	File.open("verify.bin","wb").write dec

	w = File.open( outputfile, "w" )
	w.puts "// format version #{FORMAT_VERSION}"
	i     = 0
	n.each_byte { |c|
	    i += 1
		w.printf "0x%02x,", c

	    w.printf " // %d\n", i if ( i % 16 == 0 )
	}
end


convert( INPUTFILE, OUTPUTFILE )
