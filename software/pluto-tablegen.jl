### A Pluto.jl notebook ###
# v0.20.21

using Markdown
using InteractiveUtils

# ╔═╡ fc1fe640-cb95-11f0-8f18-bfb500d04050
begin
	using Plots
	using PortAudio, SampledSignals
	using FFTW
end

# ╔═╡ 22d67da8-d273-4fb7-9501-c8ac6ddb39c2
maxint16 = 32767 

# ╔═╡ 19a57fcf-b180-4d80-80ce-9b3482e8247d
SPS = 45045

# ╔═╡ db122401-cafd-4632-ae68-1b3b1ad0c2cd
bsize = 1024

# ╔═╡ a3d163f3-a247-419a-94b9-18a6b9480951
x = range(1,10,100)

# ╔═╡ e9bc5ad9-6486-4ecd-9212-856bf1fdd7c4
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

# ╔═╡ 3d1b4c08-b7be-434e-ad61-9ff5dff5bd22


# ╔═╡ 7d404746-c7cc-4245-ad02-bc6ee48afda9
Plots.plot(range(0,1,1024),maxint16*sawtooth())

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
function playsynth(table, f, time)
	stream = PortAudioStream(0,1, warn_xruns = false)
	buff = zeros(1024)
	S = stream.sample_rate
	current = 1
	while current < time*S
		samplecounter = 1
		s_inc = f*(1024/S)
		for i in 1:1024
			samplecounter += s_inc
			if samplecounter > 1025
				samplecounter -= 1024
			end
			buff[i] = table[Int(floor(samplecounter))]
		end
		write(stream, buff)
	end
end

# ╔═╡ 0dc8c5ff-e645-439c-aed2-8dcd55bd6fe2
#writeTable(rtoint(maxint16*sawtooth()))

# ╔═╡ 5922ce2d-24db-46de-8f91-3af3749767d5
playtable(collect(sawtooth()),2)

# ╔═╡ 05003c8e-58d5-4c53-b338-d52dcd7f7bd1
arr = collect(sawtooth(bsize))

# ╔═╡ 45c0cfbd-e543-441e-aa4e-69a303ab5ce5
begin
	arrmul = repeat(arr, 1)
	fft = FFTW.fft(arrmul)
end

# ╔═╡ 5cf3caf0-0617-4e67-938f-2caf5b3d89be
Plots.plot(abs.(fft),xlims = (0.,800.))

# ╔═╡ 1b55e3ac-c453-49a1-833d-cc465ffee9aa
f_cutoff = 200

# ╔═╡ 2837a70b-c7af-465d-849c-59a624d5726d
begin
	cutoff = (bsize / SPS) * f_cutoff
	for i in 1:length(fft)
		if(i > (25) )
			fft[i] = 0
		end
	end
end

# ╔═╡ 0b716992-6140-44ac-8031-58bf786dc834
Plots.plot(abs.(fft),xlims = (0.,50.))

# ╔═╡ a2557210-7488-4e5c-a2df-91fb164192ba
begin
	ifft = FFTW.ifft(fft)
	recon = real(ifft)
end

# ╔═╡ 025d4098-80aa-421c-92fb-ae4698f73789
begin
	plot(repeat(recon,3),xlims=(0,2048))
	plot!(0.5*repeat(arr,3),xlims=(0,2048))
end

# ╔═╡ d58cc1eb-8aed-4f32-8fed-7bf9f0f05101
toexport = recon[1:1024]

# ╔═╡ 3bfabbfc-8269-4808-92ff-3acbddaef715
writeTable(rtoint(maxint16*toexport))

# ╔═╡ 52c1b6b3-c094-4e9e-bb9f-c0dde160533d
#playsynth(toexport, 10, 5)

# ╔═╡ Cell order:
# ╠═fc1fe640-cb95-11f0-8f18-bfb500d04050
# ╠═22d67da8-d273-4fb7-9501-c8ac6ddb39c2
# ╠═19a57fcf-b180-4d80-80ce-9b3482e8247d
# ╠═db122401-cafd-4632-ae68-1b3b1ad0c2cd
# ╠═a3d163f3-a247-419a-94b9-18a6b9480951
# ╠═e9bc5ad9-6486-4ecd-9212-856bf1fdd7c4
# ╠═ed798827-d5c6-4b16-b668-5c046bcf5ceb
# ╠═4a21688c-9680-4600-8788-a509ab90ad3f
# ╠═60205ec8-e52f-454f-934b-3e1f57457574
# ╠═c6f1c7bf-f226-46ae-8c6a-3b0c196463d2
# ╠═ff58a66a-8d92-465b-bab1-8590ea196670
# ╠═3d1b4c08-b7be-434e-ad61-9ff5dff5bd22
# ╠═7d404746-c7cc-4245-ad02-bc6ee48afda9
# ╠═12bf0cc4-92c8-489f-85c8-1f32254e3999
# ╠═264967bc-6704-46cc-87c2-312a437054bb
# ╠═7e08f8a5-d035-406e-99b5-66b782556653
# ╠═0dc8c5ff-e645-439c-aed2-8dcd55bd6fe2
# ╠═5922ce2d-24db-46de-8f91-3af3749767d5
# ╠═05003c8e-58d5-4c53-b338-d52dcd7f7bd1
# ╠═45c0cfbd-e543-441e-aa4e-69a303ab5ce5
# ╠═5cf3caf0-0617-4e67-938f-2caf5b3d89be
# ╠═1b55e3ac-c453-49a1-833d-cc465ffee9aa
# ╠═2837a70b-c7af-465d-849c-59a624d5726d
# ╠═0b716992-6140-44ac-8031-58bf786dc834
# ╠═a2557210-7488-4e5c-a2df-91fb164192ba
# ╠═025d4098-80aa-421c-92fb-ae4698f73789
# ╠═d58cc1eb-8aed-4f32-8fed-7bf9f0f05101
# ╠═3bfabbfc-8269-4808-92ff-3acbddaef715
# ╠═52c1b6b3-c094-4e9e-bb9f-c0dde160533d
