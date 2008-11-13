#!ruby
# build TwlIPL Root -> build nandfirm -> convert nandfirm -> 
#   build systemlogreader -> build gcdfirm
# ���s��������"dev"��n���ƊJ���r���h(default)�A"prod"��n���ƗʎY�r���h�𐶐�

def putex(str)
	# puts command and execute
	puts "execute : " + str
	system str
end


def buildgcd(buildtype, prodopt)
	puts "buildtype : #{buildtype}"

	# build TwlIPL Root
	Dir.chdir(ENV['TWL_IPL_RED_ROOT']) do
		putex "make -f Makefile.full clean; " +
					 "makesp CYGPATH_NOCMD=TRUE -j 2 -f Makefile.full TWL_FINALROM=TRUE #{prodopt}"
	end

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
			# �ʎY�p�̏ꍇ��srl�Ƀ}�X�^�����O��������
			putex "mastering.TWL.exe SystemLogReader.srl SystemLogReader.srl.master -s -t"
			putex "cp ./SystemLogReader.srl.master $TWL_IPL_RED_ROOT/build/gcdfirm/memory-launcher-writer/SystemLogReader.srl"
		end
	end

	# build memory_launcher
	Dir.chdir(ENV['TWL_IPL_RED_ROOT'] + "/build/nandfirm/memory-launcher") do
		putex "make clean; make TWL_FINALROM=TRUE #{prodopt}"
	end

	# build memorylauncherwriter
	Dir.chdir(ENV['TWL_IPL_RED_ROOT'] + "/build/gcdfirm/memory-launcher-writer") do
		putex "make clean; make TWL_FINALROM=TRUE #{prodopt}"
		putex "cp ./memory_launcher_writer-FINALROM.gcd " + 
				"$TWL_IPL_RED_ROOT/build/systemMenu_tools/SystemLogReader/memory_launcher_writer-FINALROM.#{buildtype}.gcd -f"
	end
end


Rootdir = ENV['TWL_IPL_RED_ROOT'] + '/build/systemMenu_tools/SystemLogReader'
prodopt = ""

# default buildtype = dev
if ARGV.none? || ARGV[0] == 'dev'
	build = ["dev"]
	opt = ["FIRM_USE_PRODUCT_KEYS="]
elsif ARGV[0] == "prod"
	build = ["prod"]
	opt = ["FIRM_USE_PRODUCT_KEYS=TRUE"]
elsif ARGV[0] == "full"
	build = ["dev", "prod"]
	opt = ["FIRM_USE_PRODUCT_KEYS=", "FIRM_USE_PRODUCT_KEYS=TRUE"]
else
	puts "invalid arguments."
	exit
end

p build.zip(opt)
build.zip(opt).each do |b,op|
	buildgcd(b,op)
end

