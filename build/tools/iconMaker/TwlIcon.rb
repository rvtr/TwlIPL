require 'bmp.rb'

# フォントテーブルを用いて、文字列から32*32のアイコンbmpを出力するスクリプト
# 入力は文字列か、それを格納したテキストファイルから行う
# BLACK//WHITE//0A01//test//tools とかそんな感じになるかなー

class TwlIcon < BitMap

	# ファイルサイズ
	SIZE_X = 32
	SIZE_Y = 32

	# フォント幅の定義
	FONT_HEIGHT = 9

	FONT_KIND = 3

	FONTWIDTH = [
	#	FONTWIDTH_LOWERCASE
	#		a	b	c	d	e	f	g	h	i	j	k	l	m	n	
		[	4,	4,	4,	4,	4,	3,	4,	4,	1,	3,	3,	3,	5,	4,
	#		o	p	q	r	s	t	u	v	w	x	y	z
			4,	4,	4,	3,	4,	3,	4,	5,	5,	5,	5,	4 ],

	#	FONTWIDTH_UPPERCASE
	#		a	b	c	d	e	f	g	h	i	j	k	l	m	n	
		[	5,	4,	5,	5,	4,	4,	5,	4,	3,	5,	4,	4,	5,	5,
	#		o	p	q	r	s	t	u	v	w	x	y	z
			5,	4,	6,	5,	5,	5,	5,	5,	5,	5,	5,	5 ],

	#	FONTWIDTH_NUMERIC
	#		0	1	2	3	4	5	6	7	8	9
		[	5,	3,	4,	4,	5,	4,	5,	5,	5,	5 ]
	]


	def initialize( fg = 0x0, bg = 0xf )
		super( SIZE_X, SIZE_Y )
		clear( 0xF )
		@str = [ "", "", "" ]
		@fgColor = fg
		@bgColor = bg
	end

	def setColor( fgColor, bgColor )
		@fgColor = fgColor
		@bgColor = bgColor
	end

	def setFg( clr )
		@fgColor = clr
	end

	def setBg( clr )
		@bgColor = clr
	end

	def setFilename( name )
		@filename = name
	end

	attr_accessor :filename

	def setString( line, str )
		@str[line] = str
	end

	def write( filename )

		retFlag = true

		for i in 0..2
			putStr( self, @@fontbmp, @str[i], 1, ( FONT_HEIGHT + 1 ) * i + 2 ) 
			retFlag = false if getWidthS(@str[i]) >= SIZE_X
		end

		self.changeColor( @bgColor, @fgColor )
		super( filename )

		return retFlag
	end
		

	# 初期化作業
	# 基本的に呼ばれないと死ぬ
	# フォントデータのインデクスを作ったりする
	def TwlIcon.init
		@@fontbmp = BitMap.read( "./font.bmp" )

		sFontinfo = Struct.new(:width, :index)
		sFont = Struct.new(:lowercase, :uppercase, :numeric)

		@@fonts = sFont.new
		for fontidx in 0...FONT_KIND
			# フォント幅を元に、ファイル内でのインデクスデータを作っていく
			indexlist = [0]
			sum = 0

			for charidx in 0...FONTWIDTH[fontidx].size do
				sum += FONTWIDTH[fontidx][charidx] + 1
				indexlist.push sum
			end

			obj = sFontinfo.new( FONTWIDTH[fontidx], indexlist )
			@@fonts[fontidx] = obj
		end

	end

	# 受け取った文字列strのピクセル幅を計算する
	def getWidthS( str )
		width = 0

		str.each_byte do |c|
			# 文字間に1ピクセルスペースが入る
			width += 1 + getWidthC(c)
		end

		# 最後の一文字のあとのスペースは不要
		width -= 1
	end

	# 受け取った文字のピクセル幅をテーブルから呼んで返す
	def getWidthC(char)

		case char
		when ?a..?z
			return @@fonts.lowercase.width[ char - ?a ]
		when ?A..?Z
			# 今は小文字で統一
			return @@fonts.uppercase.width[ char - ?A ]
		when ?0..?9
			return @@fonts.numeric.width[ char - ?0 ]
		end
	end


	# 受け取った文字のフォントbmpにおけるX座標インデクスデータを返す
	def getIndexX(char)
		case char
		when ?a..?z
			return @@fonts.lowercase.index[ char - ?a ]
		when ?A..?Z
			return @@fonts.uppercase.index[ char - ?A ]
		when ?0..?9
			return @@fonts.numeric.index[ char - ?0 ]
		end		
	end

	# 受け取った文字のフォントbmpにおけるX座標インデクスデータを返す
	def getIndexY(char)
		case char
		when ?a..?z
			return 0
		when ?A..?Z
			return (FONT_HEIGHT + 1) * 2
		when ?0..?9
			return FONT_HEIGHT + 1
		end
		
	end

	# フォントbmpから抽出した文字列を今のバッファに埋め込む
	# image:埋め込み対象 fontimage:フォント画像 str:文字列 x,y:座標
	def putStr( image, fontimage, str, x, y )

		#p "inserting #{str}"
		str.each_byte do |c|
			putChar( image, fontimage, c, x, y )
			x += 1 + getWidthC(c)
		end

	end

	# フォントbmpの対象文字を今のバッファに埋め込む
	def putChar( image, fontimage, char, x, y )

		#p "index of '#{char.chr}' : #{getIndexX(char)} #{getIndexY(char)} , x:#{x} y:#{y}"

		charimg = fontimage.clip( getIndexX(char), getIndexY(char), getIndexX(char) + getWidthC(char)-1, getIndexY(char) + FONT_HEIGHT-1 )
		image.paste( charimg, x, y )
		
	end

	# バッファの中の白い部分を背景色、黒い部分を文字色で塗りつぶす
	# 4bitColorBMP仕様になったので未使用
	def changeColor( bgColor, fgColor )

		(0...@buf_size).each do |idx|
			head = @buf[idx] >> 4
			foot = @buf[idx] & 0x0f
			
			if head == 0xf then
				head = bgColor
			elsif head == 0x0
				head = fgColor
			end

			if foot == 0xf then
				foot = bgColor
			elsif foot == 0x0
				foot = fgColor
			end

			@buf[idx] = (head << 4) | foot
		end
	end
end


TwlIcon.init
