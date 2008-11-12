#!ruby
# build nandfirm -> convert nandfirm -> build systemlogreader -> build gcdfirm
# 実行時引数に"dev"を渡すと開発ビルド、"prod"を渡すと量産ビルドを生成

def putex(str)
	# puts command and execute
	puts "execute : " + str
	system str
end

Rootdir = ENV['TWL_IPL_RED_ROOT'] + '/build/systemMenu_tools/SystemLogReader'
prodopt = ""

# default buildtype = dev
if ARGV.none? || ARGV[0] == 'dev'
	buildtype = "dev"
elsif ARGV[0] = "prod"
	buildtype = "prod"
	prodopt = "FIRM_USE_PRODUCT_KEYS=TRUE"
else
	puts "invalid arguments."
	exit
end

puts "buildtype : #{buildtype}"

# build nandfirm
Dir.chdir(ENV['TWL_IPL_RED_ROOT'] + "/build/nandfirm/menu-launcher") do
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
	putex "cp nandfirm.#{buildtype}.o #{Rootdir}/ARM9.TWL/nandfirm/"
end

# build systemlogreader
putex "make clean; make TWL_FINALROM=TRUE #{prodopt}"
Dir.chdir(Rootdir + "/ARM9.TWL/bin/ARM9-TS.LTD.thumb/Rom") do
	if buildtype == "dev"
		putex 'cp ./SystemLogReader.srl $TWL_IPL_RED_ROOT/build/gcdfirm/memory-launcher-writer/'
	else
		# 量産用の場合はsrlにマスタリングをかける
		putex "mastering.TWL.exe SystemLogReader.srl SystemLogReader.srl.master -s -t"
		putex "cp ./SystemLogReader.srl.master $TWL_IPL_RED_ROOT/build/gcdfirm/memory-launcher-writer/SystemLogReader.srl"
	end
end

Dir.chdir(ENV['TWL_IPL_RED_ROOT'] + "/build/gcdfirm/memory-launcher-writer") do

	# build memorylauncherwriter
	putex "make clean; make TWL_FINALROM=TRUE #{prodopt}"
	putex "cp ./memory_launcher_writer-FINALROM.gcd " + 
			"$TWL_IPL_RED_ROOT/build/systemMenu_tools/SystemLogReader/memory_launcher_writer-FINALROM.#{buildtype}.gcd -f"
end