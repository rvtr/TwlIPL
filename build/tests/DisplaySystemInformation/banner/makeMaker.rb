fFoot = open "MakeFooter.txt"
fHead = open "MakeHeader.txt"

contentFoot = fFoot.read
contentHead = fHead.read

fFoot.close
fHead.close

gamecodeList = ["020A","021A","022A","023A","024A","025A", 
				"0B0A","0B1A","0B2A","0B3A","0B4A","0B5A",
				"0B6A","0B7A","0B8A","0B9A","0BAA","0BBA",
				]
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
