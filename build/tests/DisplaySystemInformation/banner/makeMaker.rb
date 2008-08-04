fFoot = open "MakeFooter.txt"
fHead = open "MakeHeader.txt"

contentFoot = fFoot.read
contentHead = fHead.read

fFoot.close
fHead.close

gamecodeList = ["020A","021A","022A","023A","024A","025A"]
fileNameList = []

for gamecode in gamecodeList

	fileName = "Makefile.#{gamecode}"
	fileNameList << fileName
	f = open( fileName, "w")
	f.puts contentHead
	f.puts "GAMECODE\t=\t#{gamecode}"
	f.puts contentFoot
	f.close
end

print fileNameList
