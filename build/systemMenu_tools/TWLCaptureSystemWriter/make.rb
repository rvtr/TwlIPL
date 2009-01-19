#!ruby
# build TwlIPL Root -> build nandfirm -> convert nandfirm -> 
#   build systemlogreader -> build gcdfirm
# 実行時引数に"dev"を渡すと開発ビルド(default)、"prod"を渡すと量産ビルドを生成

# mastering.TWL.exeにパスが通っている環境
#MASTERING = "mastering"
# mastering.TWL.exeにパスが通っていない環境
MASTERING = ENV['TWL_IPL_RED_PRIVATE_ROOT'] + "/tools/bin/mastering.TWL.exe -i " +
						ENV['TWL_IPL_RED_PRIVATE_ROOT'] + "/tools/bin/mastering.TWL.ini -s -t"

BIN2OBJ = ENV['TWLSDK_ROOT'] + '/tools/bin/bin2obj.TWL.exe'
TARGET_BIN = "TWLCaptureSystemWriter"
INCLUDE_FILE = ['HNAA', 'HNBA', 'HNCA', 'HNHA', 'HNLA', 'nandfirm']
ROOT_DIR = ENV['TWL_IPL_RED_ROOT'] + "/build/systemMenu_tools/#{TARGET_BIN}"
def putex(str)
	# puts command and execute
	puts 'execute : ' + str
	system str
end

def namesafix(basename)
	if basename == 'nandfirm'
		return 'nand'
	else
		return 'tad'
	end
end	

# .tad/.nandを探す
def checkOriginalFile(basename)
	return Dir.entries(".").include?("#{basename}.#{namesafix(basename)}")
end

def checkObjectFile(basename)
	return Dir.entries(".").include?("#{basename}.o")
end

def convertToObj(basename)
	putex("#{BIN2OBJ} -b #{basename.downcase}_begin -e #{basename.downcase}_end #{basename}.#{namesafix(basename)} #{basename}.o")
end

def build_TWLIPLTree(prodopt)
	Dir.chdir(ENV['TWL_IPL_RED_ROOT']) do
		putex "make -f Makefile.full clean; " +
					 "makesp CYGPATH_NOCMD=TRUE -j 2 -f Makefile.full TWL_FINALROM=TRUE #{prodopt}"
	end
end

def build_target(buildtype, prodopt)
	# build target
	putex "makesp CYGPATH_NOCMD=TRUE -j 2 clean; makesp CYGPATH_NOCMD=TRUE -j 2 TWL_FINALROM=TRUE #{prodopt}"
	Dir.chdir(ROOT_DIR + "/ARM9.TWL/bin/ARM9-TS.LTD.thumb/Rom") do
		if buildtype == "dev"
			putex "cp ./#{TARGET_BIN}.srl $TWL_IPL_RED_ROOT/build/gcdfirm/memory-launcher-writer/"
		else
			# 量産用の場合はsrlにマスタリングをかける
			putex "#{MASTERING} #{TARGET_BIN}.srl #{TARGET_BIN}.srl.master -s -t"
			putex "cp ./#{TARGET_BIN}.srl.master $TWL_IPL_RED_ROOT/build/gcdfirm/memory-launcher-writer/#{TARGET_BIN}.srl"
		end
	end
end

def build_nandfirm(prodopt)

	kDir.chdir(ENV['TWL_IPL_RED_ROOT'] + "/build/nandfirm/menu-launcher") do
		putex "make clean; make TWL_FINALROM=TRUE #{prodopt}"
		nandfirm = Dir.glob("menu_launcher*#{buildtype}.nand")
		if nandfirm.none?
			puts "build nandfirm failed."
			exit
		else
			nandfirm = nandfirm[0]
		end
		putex "$TWLSDK_ROOT/tools/bin/bin2obj.TWL.exe #{nandfirm} nandfirm.#{buildtype}.o " +
						"-b nandfirm_begin -e nandfirm_end"
		putex "cp nandfirm.#{buildtype}.o #{ROOT_DIR}/ARM9.TWL/nandfirm/"
	end
end

def build_includeFile(buildtype)
	Dir.chdir(ROOT_DIR + "/ARM9.TWL/data/#{buildtype}") do
		INCLUDE_FILE.each do |f|
			if checkOriginalFile f
				convertToObj f
			elsif checkObjectFile f
				puts "WARNING(#{f}): original file notfound. using object file in existence."
			else
				puts "ERROR(#{f}): both tad/nand file and object file is not found."
				exit
			end
		end
	end
end

def mainproc(buildtype, shortbuild)
	prodopt = buildtype == "dev"? "FIRM_USE_PRODUCT_KEYS=" : "FIRM_USE_PRODUCT_KEYS=TRUE"
	puts "buildtype : #{buildtype}"
	puts "**shortbuild**" if shortbuild

	# build TwlIPL Root
	unless shortbuild
		build_TWLIPLTree(prodopt)
	end

	build_includeFile buildtype
	build_target buildtype, prodopt
	build_gcdfirm(buildtype, prodopt)

end


def build_gcdfirm(buildtype, prodopt)	

	# build memory_launcher
	Dir.chdir(ENV['TWL_IPL_RED_ROOT'] + "/build/nandfirm/memory-launcher") do
		putex "make clean; make TWL_FINALROM=TRUE #{prodopt}"
	end

	# build memorylauncherwriter
	Dir.chdir(ENV['TWL_IPL_RED_ROOT'] + "/build/gcdfirm/memory-launcher-writer") do
		putex "make clean; make TWL_FINALROM=TRUE APPEND_SRL=#{TARGET_BIN}.srl #{prodopt} "
		putex "cp ./memory_launcher_writer-FINALROM.gcd " + 
				"$TWL_IPL_RED_ROOT/build/systemMenu_tools/#{TARGET_BIN}/memory_launcher_writer-FINALROM.#{buildtype}.gcd -f"
	end
end

#=== main ===


shortbuild = false

# default buildtype = dev
if ARGV.none? || ARGV.include?('dev')
	build = ["dev"]
elsif ARGV.include?("prod")
	build = ["prod"]
elsif ARGV.include? "full"
	build = ["dev", "prod"]
else
	puts "invalid arguments."
	exit
end

if ARGV.include? "short"
	# build=fullでshortbuildは無効
	if ARGV.include? "full"
		puts 'build "full" needs to re-build TWLIPL tree.'
		exit
	else
		shortbuild = true
	end
end


build.each do |b|
	mainproc(b,shortbuild)
end

