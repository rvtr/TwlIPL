fHead = open("makeHeader.txt")
fFoot = open("makeFooter.txt")
contentHead = fHead.read
contentFoot = fFoot.read
fHead.close
fFoot.close

fCond = open("makeConditions.txt")
numArg = 0

argList = Array.new
fCond.gets.split(/\s+/).each do |arg|
	numArg += 1
	argList.push arg
end

puts "argList"
puts argList


fileIdx = 0

while l = fCond.gets
	next if l[0] == '#'
	argIdx = 0

	puts "file making: Makefile." + fileIdx.to_s
	f = open("Makefile." + fileIdx.to_s, "w")
	f.puts contentHead

	l.split(/\t+/).each do |value|
		strVal = "TRUE" if value == "1"
		strVal = "FALSE" if value == "0"
		strVal = "DONTCARE" if value == "D"

		if argList[argIdx] == "GAMECODE" || argList[argIdx] == "FILE_IDX"
			argSet = argList[argIdx] + "\t=\t"
			argSet += value
		else
			argSet = argList[argIdx].to_s + "\t=\t"
			argSet += strVal
		end

		f.puts argSet

		argIdx += 1
	end

	f.puts contentFoot
	f.close
	fileIdx += 1
end

fCond.close