#!ruby
# build TwlIPL Root -> convert nandfirm -> 
#   build systemlogreader -> build gcdfirm
# 実行時引数に"dev"を渡すと開発ビルド(default)、"prod"なら量産ビルド、"full"なら両方を生成。
# また一緒に"short"を指定するとTwlIPLRootのビルドをやり直さない短縮ビルドを行う。

def putex(str)
	# puts command and execute
	puts "execute : " + str
	system str
end


def buildgcd(buildtype, prodopt, shortbuild)
	puts "buildtype : #{buildtype}"
	puts "**shortbuild**" if shortbuild

	unless shortbuild
		# build TwlIPL Root
		Dir.chdir(ENV['TWL_IPL_RED_ROOT']) do
			putex "make -f Makefile.full clean; " +
						 "makesp CYGPATH_NOCMD=TRUE -j 2 -f Makefile.full TWL_FINALROM=TRUE #{prodopt}"
		end
	end
	
	
	# build nandfirm
	nandfirmDir = "#{ENV['TWL_IPL_RED_PRIVATE_ROOT']}/resources/nandfirm_bin/"
	nandfirm = 
		if buildtype == "dev"
			"#{nandfirmDir}/menu_launcher.nand"
		else
			"#{nandfirmDir}/menu_launcher-FINALROM-2435-8325.prod.nand"
			end
	
	unless FileTest.exist? nandfirm
		puts "[ERROR] prebuilt nandfirm is not found. : #{nandfirm}"
		exit
	end
	
	putex "mkdir -p #{Rootdir}/ARM9.TWL/nandfirm"
	putex "$TWLSDK_ROOT/tools/bin/bin2obj.TWL.exe #{nandfirm} #{Rootdir}/ARM9.TWL/nandfirm/nandfirm.#{buildtype}.o " +
		"-b nandfirm_begin -e nandfirm_end"

	# build cardboard eraser
	putex "makesp CYGPATH_NOCMD=TRUE -j 2 clean; makesp CYGPATH_NOCMD=TRUE -j 2 TWL_FINALROM=TRUE #{prodopt}"
	putex "rm $TWL_IPL_RED_ROOT/build/gcdfirm/memory-launcher-writer/CardboardEraser.srl"
	
	Dir.chdir(Rootdir + "/ARM9.TWL/bin/ARM9-TS.LTD.thumb/Rom") do
		if buildtype == "dev"
			putex 'cp ./CardboardEraser.srl $TWL_IPL_RED_ROOT/build/gcdfirm/memory-launcher-writer/'
		else
			# 量産用の場合はsrlにマスタリングをかける
			masterbin = ENV['TWL_IPL_RED_PRIVATE_ROOT'] + "/tools/bin/mastering.TWL.exe"
			masterini = ENV['TWL_IPL_RED_PRIVATE_ROOT'] + "/tools/bin/mastering.TWL.ini"
			
			putex "#{masterbin} CardboardEraser.srl CardboardEraser.srl.master -s -t -i #{masterini}"
			putex "cp ./CardboardEraser.srl.master $TWL_IPL_RED_ROOT/build/gcdfirm/memory-launcher-writer/CardboardEraser.srl"
		end
	end
	
	unless FileTest.exist? "#{ENV['TWL_IPL_RED_ROOT']}/build/gcdfirm/memory-launcher-writer/CardboardEraser.srl"
		puts "[ERROR] building CardboardEraser failed."
		exit
	end
	
	# build memory_launcher
	Dir.chdir(ENV['TWL_IPL_RED_ROOT'] + "/build/nandfirm/memory-launcher") do
		putex "make clean; make TWL_FINALROM=TRUE #{prodopt}"
	end

	# build memorylauncherwriter
	Dir.chdir(ENV['TWL_IPL_RED_ROOT'] + "/build/gcdfirm/memory-launcher-writer") do
		putex "make clean; make TWL_FINALROM=TRUE #{prodopt} APPEND_SRL=./CardboardEraser.srl"
		putex "cp ./memory_launcher_writer-FINALROM.gcd " + 
				"$TWL_IPL_RED_ROOT/build/systemMenu_tools/CardboardEraser/memory_launcher_writer-FINALROM.#{buildtype}.gcd -f"
	end
end



# ===== main script =====

# nandfirmが必要なのでPRIVATE_ROOT必須
unless ENV['TWL_IPL_RED_PRIVATE_ROOT']
	puts "[ERROR] env valiable: TWL_IPL_RED_PRIVATE_ROOT is needed."
	exit
end

Rootdir = ENV['TWL_IPL_RED_ROOT'] + '/build/systemMenu_tools/CardboardEraser'
prodopt = ""
shortbuild = false

# default buildtype = dev
if ARGV.none?
	puts "usage:"
	puts "ruby make.rb [dev|prod|full] [short]"
	exit
elsif ARGV.include?('dev')
	build = ["dev"]
	opt = ["FIRM_USE_PRODUCT_KEYS="]
elsif ARGV.include?("prod")
	build = ["prod"]
	opt = ["FIRM_USE_PRODUCT_KEYS=TRUE"]
elsif ARGV.include? "full"
	build = ["dev", "prod"]
	opt = ["FIRM_USE_PRODUCT_KEYS=", "FIRM_USE_PRODUCT_KEYS=TRUE"]
else
	puts "[ERROR] invalid arguments."
	exit
end

if ARGV.include? "short"
	shortbuild = true
end

p build.zip(opt)
build.zip(opt).each do |b,op|
	buildgcd(b,op,shortbuild)
end

