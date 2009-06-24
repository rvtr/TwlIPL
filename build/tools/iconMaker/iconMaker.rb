require './TwlIcon.rb'

$fgColor = 0x0
$bgColor = 0xf

COLOR_TABLE = {
	"black" => 0, "maroon" => 1, "green" => 2, "olive" => 3,
	"navy" => 4, "purple" => 5, "teal" => 6, "gray" => 7,
	"lightgray" => 8, "red" => 9, "lime" => 10, "yellow" => 11,
	"blue" => 12, "fuchsia" => 13, "aqua" => 14, "white" => 15
}
def readArg( str )

	lineNum = 0
	argList = str.split( "/" )
	return nil if argList.size == 0

	newicon = TwlIcon.new
	
	while( arg = argList.shift ) do
		next if arg.size == 0

		head = arg[0]
		data = arg[1..arg.size]

		if head == ?O then
			newicon.filename = data
		elsif head == ?I then
			newicon.setString( lineNum, data )
			lineNum += 1
		elsif head == ?F
			if data =~ /\d{1,2}?/ && (0..15).include?( data.to_i ) then
				$fgColor = data.to_i
			elsif COLOR_TABLE[data]
				$fgColor = COLOR_TABLE[data]
			end
		elsif head == ?B
			if data =~ /\d{1,2}?/ && (0..15).include?( data.to_i ) then
				$bgColor = data.to_i
			elsif COLOR_TABLE[data]
				$bgColor = COLOR_TABLE[data]
			end
		end
	end

	newicon.setColor( $fgColor, $bgColor )
	
	return newicon
end

=begin
# 各項目を//で区切られた引数を文字列リストに分割して
# TwlIconオブジェクトを生成する
def readArg( str )
	argList = str.split( "/" )
	p argList
	
	return nil unless argList.size == 3 || argList.size == 5

	newicon = TwlIcon.new

	if argList.size == 5 then
		# fgcolorとbgcolorを割り当てる
		argList.delete 0..1
	end
	
	# こっから表示文字列
	for i in 0..2 do
		newicon.setString( i, argList[i] )
		newicon.setColor(0x3, 0xc)
	end

	return newicon
end
=end

TwlIcon::init


iconlist = Array.new 0
argList = []

ARGV.each do |str|
	if str =~ /(.+\.txt)/ then
		fileArgList = open(str).read.split(/\s/)
		fileArgList.each do |fileArg|
			newicon = readArg fileArg
			if newicon then
				iconlist.push newicon
				argList.push fileArg
			end
		end
	else
		newicon = readArg str
		if newicon then
			iconlist.push newicon
			argList.push str
		end
	end
end

idx = 0
iconlist.each do |icon|
	filename = icon.filename ? icon.filename : "test#{idx}.bmp"
	puts "*** warning ***\n width overflow: #{argList[idx]}" unless icon.write( filename )
	#p filename
	idx += 1
end
