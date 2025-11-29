using Plots
using PortAudio, SampledSignals
using FFTW

HPP_HEADER_STRING = """
#pragma once

#include <cstdint>

struct WaveTableBand
{
    const int16_t * data;
    uint32_t length;
    const float f;

    //array acces operator to get the data
    inline int16_t operator[](uint32_t index) const 
    {
        return data[index]; 
    }
    
};

struct WaveTable
{
    const WaveTableBand *b_ptr;
    uint32_t num;

    //returns a struct containing {data: pointer to start of wt data for given band, length: length of this data, f: recommended frequency to switch to this band}
    inline WaveTableBand operator[](uint32_t index) const 
    {
        return b_ptr[index];
    }
    
};

extern const WaveTable wt_library[];


"""

CPP_HEADER_STRING = """
#include "wave_tables.hpp" 

"""
TERM_STRING = "};\n"
TABLE_TYPE_STRING = "const int16_t"
TABLE_BAND_NAME = "table_band"
TABLE_BAND_TYPE = "const WaveTableBand"
FULL_TABLE_NAME = "wave_table"
FULL_TABLE_TYPE = "const WaveTable"
WTS_WRITTEN = 0
FULL_WTS_WRITTEN = 0

HPP = "wavetable_data.hpp"
CPP = "wavetable_data.cpp"

maxint16 = 32767 
SPS = 45045
bsize = 1024

function sine()
	x = range(0,1,bsize)
	y = sin.(x.*2*pi)
end

# ╔═╡ ed798827-d5c6-4b16-b668-5c046bcf5ceb
function sawtooth(bsize)
	x = range(0,1,bsize)
	y = (x).*2 .- 1
	return y
end

# ╔═╡ 4a21688c-9680-4600-8788-a509ab90ad3f
function tri(bsize)
    # Generate x from 0 to 1
    x = range(0, 1, length=bsize)
    
    # 2/pi * asin(sin) creates a linear mapping of the phase
    return (2 / π) .* asin.(sin.(2π .* x))
end

# ╔═╡ 60205ec8-e52f-454f-934b-3e1f57457574
function sqr(bsize)
    x = range(0, 1, length=bsize)
    
    # sign() returns -1, 0, or 1 based on the input
    return sign.(sin.(2π .* x))
end

# ╔═╡ c6f1c7bf-f226-46ae-8c6a-3b0c196463d2
function chirp(bsize)
    x = range(0, 1, length=bsize)
    
    # We multiply x by itself (x^2) inside the sine.
    # The '10' determines how many cycles occur (the max frequency).
    return sin.(2π .* (x .^ 2) .* 10)
end

# ╔═╡ ff58a66a-8d92-465b-bab1-8590ea196670
function rtoint(range)
	return Int.(floor.(collect(range)))
end

function ftg(p,w)
    x = range(1,1024,1024)
    return exp.(-(((x.-p)./w).^2))
end
# ╔═╡ 3d1b4c08-b7be-434e-ad61-9ff5dff5bd22

function gau(p,w)
    x = range(0,10,1024)
    return exp.(-(((x .- p) ./ w) .^ 2))
end

# ╔═╡ 7d404746-c7cc-4245-ad02-bc6ee48afda9
Plots.plot(range(0,1,1024),maxint16*sawtooth(1024))

function getx(bsize)
    return collect(range(0,1,1024))
end

function fmsine(a, b, k1, k2)
    p = 2*pi
    x = getx(1024)
    M_2 = sin.(k2*p*x)
    M_1 = sin.(k1*p*x .+ b*M_2)
    return sin.(p*x + a*M_1)
end

function  wp5(table)
    return table.^5
end

function T_2(x)
    return 2*x.^2 .- 1
end

function T_3(x)
    return 4*x.^3 .- 3*x
end

function T_4(x)
    return 8*x.^4 .- 8*x.^2 .+ 1
end

# ╔═╡ 12bf0cc4-92c8-489f-85c8-1f32254e3999
function writeTable(table)
	io = open("table.dat","w")
	for i in 1:length(table)
		print(io, table[i])
		print(io, ",")
	end
	close(io)
end

# ╔═╡ 264967bc-6704-46cc-87c2-312a437054bb
#add multiplier function
function playtable(table, time)
	stream = PortAudioStream(0,1, warn_xruns = false)
	S = stream.sample_rate
	current = 1
	while current < time*S
		write(stream, 0.7*table)
		current += length(table)
	end
	nothing
end

# ╔═╡ 7e08f8a5-d035-406e-99b5-66b782556653
function playsynth(table::Vector{Float64}, f, time)
	stream = PortAudioStream(0,1, warn_xruns = false)
	buff = zeros(1024)
	S = stream.sample_rate
	current = 1
    samplecounter = 0
	while current < time*S
		s_inc = f*(1024/S)
        for i in 1:1024
            samplecounter += s_inc
            if samplecounter > 1024
                samplecounter -= 1024
            end
            buff[i] = 0.7 * table[1+Int(floor(samplecounter))]
        end
		write(stream, buff)
        current += 1024
	end
end



function runfft(arr)
    return FFTW.fft(arr)
end

function coff(arr, harm)
	#cutoff = (bsize / SPS) * f_cutoff
	for i in 1:length(arr)
		if(i > (harm) )
			arr[i] = 0
		end
	end
    return arr
end

function runifft(arr)
    return real(FFTW.ifft(arr))
end

function ftplot(arr)
    plot(abs.(arr),xlims=(0.,100.))
end

function ftplot(arr1, arr2)
    plot(abs.(arr1),xlims=(0.,100.))
    plot!(abs.(arr2),xlims=(0.,100.))
end

function ipc(arr1, arr2, fac)
    r1 = real(arr1)
    i1 = imag(arr1)
    r2 = real(arr2)
    i2 = imag(arr2)
    r3 = ((1 - fac)*r1) .+ (fac*r2)
    i3 = ((1 - fac)*i1) .+ (fac*i2)
    return r3 .+ (im*i3)
end

function ipp(arr1, arr2, fac)
    m1 = abs.(arr1)
    a1 = angle.(arr1)
    m2 = abs.(arr2)
    a2 = angle.(arr2)
    m3 = ((1-fac)*m1) .+ (fac*m2)
    a3 = ((1-fac)*a1) .+ (fac*a2)
    return m3.*exp.(im*a3)
end

function ipm(arr1, arr2, fac)
    m1 = abs.(arr1)
    a1 = angle.(arr1)
    m2 = abs.(arr2)
    a2 = angle.(arr2)
    m3 = ((1 - fac) * m1) .+ (fac * m2)
    a3 = ((1 - fac) * a1) .+ (fac * a2)
    return m3 .* exp.(im*a1)
end

function noph(arr)
    return abs.(arr)
end

function ip(arr1, arr2, fac)
    return ((1-fac)*arr1) .+ (fac*arr2)
end

function blimit(t, n)
    return runifft(coff(runfft(t),n))
end

function fttablewrite(t)
    tw = Int.(floor.(maxint16*t))
    writeTable(tw)
end

function writeHeaders()
    global WTS_WRITTEN
    global FULL_WTS_WRITTEN
    WTS_WRITTEN = 0
    FULL_WTS_WRITTEN = 0
    iohpp = open(HPP,"w")
    iocpp = open(CPP,"w")

    print(iohpp, HPP_HEADER_STRING)
    print(iocpp, CPP_HEADER_STRING)

    close(iohpp)
    close(iocpp)
end

function writeTableData(io, table, size)
    lbreack = 0
    for i in 1:size 
        print(io,table[i])
        print(io,",")
        if lbreack >= 20
            print(io,"\n")
            lbreack = 0
        end
        lbreack += 1
    end
end

function writeWT(table)
    #conversion to int happens in this function
    global WTS_WRITTEN
    global FULL_WTS_WRITTEN
    iohpp = open(HPP, "a")
    iocpp = open(CPP, "a")
    
    #write with band limit 90,45,22,11,5
    #frequencies 0, C4, C5, C6, C7
    #in hz 0, 261.63, 523.25, 1046.5, 2093.005
    farr = [0, 261.63, 523.25, 1046.5, 2093.002]
    barr = [90, 45, 22, 11, 5]
    for i in 1:5
        bt = blimit(table, barr[i])
        #write into hpp
        #something like extern tabletype tablename[1024]
        index = string(Int(WTS_WRITTEN + i))
        print(iohpp, "extern $(TABLE_TYPE_STRING) $(TABLE_BAND_NAME)$(index)[1024];\n")
        #write into cpp file 
        print(iocpp, "$(TABLE_TYPE_STRING) $(TABLE_BAND_NAME)$(index)[1024] = {\n")
        writeTableData(iocpp, Int.(floor.(maxint16*bt)), 1024)
        print(iocpp, TERM_STRING)
        print(iocpp, "\n")
    end

    print(iohpp, "\n")
    print(iohpp, "extern $(TABLE_BAND_TYPE) $(FULL_TABLE_NAME)$(FULL_WTS_WRITTEN)[];\n")
    print(iohpp, "\n")

    print(iocpp, "\n")
    print(iocpp, "$(TABLE_BAND_TYPE) $(FULL_TABLE_NAME)$(FULL_WTS_WRITTEN)[] = {\n")
    for i in 1:5
        index = string(Int(WTS_WRITTEN + i))
        print(iocpp, "  {$(TABLE_BAND_NAME)$(index), ")
        print(iocpp, 1024)
        print(iocpp, ", ")
        print(iocpp, farr[i])
        print(iocpp, "},\n")
    end
    print(iocpp, TERM_STRING)
    print(iocpp, "\n\n")


    WTS_WRITTEN += 5
    FULL_WTS_WRITTEN += 1

    close(iohpp)
    close(iocpp)
end

function writeLibrary()
    iohpp = open(HPP, "a")
    iocpp = open(CPP, "a")

    print(iocpp, "$(FULL_TABLE_TYPE) wt_library[] = {\n")
    for i in 0:FULL_WTS_WRITTEN-1
        index = string(Int(i))
        print(iocpp, "  {$(FULL_TABLE_NAME)$(index), ")
        print(iocpp, 5)
        print(iocpp, "},\n")
    end
    print(iocpp, TERM_STRING)

    close(iohpp)
    close(iocpp)
end