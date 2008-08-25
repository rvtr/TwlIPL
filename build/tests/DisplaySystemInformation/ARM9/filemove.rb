#! ruby -Ks
startDir = "/cygdrive/c/home/TWL_IPL/build/tests/DisplaySystemInformation/ARM9/bin/ARM9-TS.LTD.thumb/Rom"
dstDir = "/cygdrive/c/home/TWL_IPL/build/tests/DisplaySystemInformation/ARM9/bin/ARM9-TS.LTD.thumb/roms"

Dir::chdir startDir

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
