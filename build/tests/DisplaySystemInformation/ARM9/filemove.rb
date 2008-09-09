#! ruby -Ks
startDir = "$TWL_IPL_RED_ROOT/build/tests/DisplaySystemInformation/ARM9/bin/ARM9-TS.LTD.thumb/Rom"
dstDir = "/build/tests/DisplaySystemInformation/ARM9/bin/ARM9-TS.LTD.thumb/roms"

Dir::chdir "./bin/ARM9-TS.LTD.thumb/Rom"

dirList = Dir::entries "."
`mkdir #{dstDir}`

for dir in dirList

	puts "===== #{dir} ====="
	Dir::chdir "./#{dir}"
	
	Dir::glob("./*.{tad,srl}").each {|f|
		puts "cp #{f} #{dstDir}/"
		`cp #{f} #{dstDir}/`
	}

	

	Dir::chdir startDir
end
