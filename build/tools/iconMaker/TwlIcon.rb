require 'bmp.rb'

# �t�H���g�e�[�u����p���āA�����񂩂�32*32�̃A�C�R��bmp���o�͂���X�N���v�g
# ���͕͂����񂩁A������i�[�����e�L�X�g�t�@�C������s��
# BLACK//WHITE//0A01//test//tools �Ƃ�����Ȋ����ɂȂ邩�ȁ[

class TwlIcon < BitMap

	# �t�@�C���T�C�Y
	SIZE_X = 32
	SIZE_Y = 32

	# �t�H���g���̒�`
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
		

	# ���������
	# ��{�I�ɌĂ΂�Ȃ��Ǝ���
	# �t�H���g�f�[�^�̃C���f�N�X��������肷��
	def TwlIcon.init
		@@fontbmp = BitMap.read( "./font.bmp" )

		sFontinfo = Struct.new(:width, :index)
		sFont = Struct.new(:lowercase, :uppercase, :numeric)

		@@fonts = sFont.new
		for fontidx in 0...FONT_KIND
			# �t�H���g�������ɁA�t�@�C�����ł̃C���f�N�X�f�[�^������Ă���
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

	# �󂯎����������str�̃s�N�Z�������v�Z����
	def getWidthS( str )
		width = 0

		str.each_byte do |c|
			# �����Ԃ�1�s�N�Z���X�y�[�X������
			width += 1 + getWidthC(c)
		end

		# �Ō�̈ꕶ���̂��Ƃ̃X�y�[�X�͕s�v
		width -= 1
	end

	# �󂯎���������̃s�N�Z�������e�[�u������Ă�ŕԂ�
	def getWidthC(char)

		case char
		when ?a..?z
			return @@fonts.lowercase.width[ char - ?a ]
		when ?A..?Z
			# ���͏������œ���
			return @@fonts.uppercase.width[ char - ?A ]
		when ?0..?9
			return @@fonts.numeric.width[ char - ?0 ]
		end
	end


	# �󂯎���������̃t�H���gbmp�ɂ�����X���W�C���f�N�X�f�[�^��Ԃ�
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

	# �󂯎���������̃t�H���gbmp�ɂ�����X���W�C���f�N�X�f�[�^��Ԃ�
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

	# �t�H���gbmp���璊�o��������������̃o�b�t�@�ɖ��ߍ���
	# image:���ߍ��ݑΏ� fontimage:�t�H���g�摜 str:������ x,y:���W
	def putStr( image, fontimage, str, x, y )

		#p "inserting #{str}"
		str.each_byte do |c|
			putChar( image, fontimage, c, x, y )
			x += 1 + getWidthC(c)
		end

	end

	# �t�H���gbmp�̑Ώە��������̃o�b�t�@�ɖ��ߍ���
	def putChar( image, fontimage, char, x, y )

		#p "index of '#{char.chr}' : #{getIndexX(char)} #{getIndexY(char)} , x:#{x} y:#{y}"

		charimg = fontimage.clip( getIndexX(char), getIndexY(char), getIndexX(char) + getWidthC(char)-1, getIndexY(char) + FONT_HEIGHT-1 )
		image.paste( charimg, x, y )
		
	end

	# �o�b�t�@�̒��̔���������w�i�F�A���������𕶎��F�œh��Ԃ�
	# 4bitColorBMP�d�l�ɂȂ����̂Ŗ��g�p
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
