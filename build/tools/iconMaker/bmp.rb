# ruby用bmp操作ライブラリ
# パレットはwindows標準の固定16色に決め打ち
# 出力するときはTwlIconクラスのfgColorとbgColorで該当パレットを置換する

class BitMap
	PALETTE_SIZE = 16*4

  def initialize(width, height, dpi = 96)
    @width = width
    @height = height
    @line_size = (width + (8 - width % 8 ) % 8) / 2
    @buf_size = @line_size * height
    @buf = [ 0x00 ] * @buf_size
    @bit_count = 4
    @compression = 0  # 圧縮無し
    @size_image = 0
    @x_pix_per_meter = 0
    @y_pix_per_meter = 0
    @clr_used = 16
    @cir_important = 0
	@palette_size = (2**@bit_count) * 4
	@palette = "\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x00\x00\x00\x80\x80\x00" +
				"\x80\x00\x00\x00\x80\x00\x80\x00\x80\x80\x00\x00\x80\x80\x80\x00" +
				"\xC0\xC0\xC0\x00\x00\x00\xFF\x00\x00\xFF\x00\x00\x00\xFF\xFF\x00" +
				"\xFF\x00\x00\x00\xFF\x00\xFF\x00\xFF\xFF\x00\x00\xFF\xFF\xFF\x00"

	#puts "Initialize..."
	#p "width :#{@width} line_size :#{@line_size}"
  end

  def clear( clrIdx = 15 )
    @buf = [ (clrIdx << 4) + clrIdx ] * @height * @line_size
  end

  attr_writer :buf
  attr_reader :width, :height

  # BMPファイルを出力する
  def write(filename)
    file_size = 14 + 40 + @palette_size + @buf_size
    palette_offset = 14 + 40
	data_offset = palette_offset + @palette_size

    open(filename, "wb") do |f|
      f.print 'BM'
      f.print [file_size, 0, data_offset].pack("l*")
      f.print [40, @width, @height].pack("l*")
      f.print [1, @bit_count].pack("S*")
      f.print [@compression, @size_image,
               @x_pix_per_meter, @y_pix_per_meter,
               @clr_used, @cir_important].pack("l*")
      f.print @palette
      f.print @buf.pack("c*")

    end
  end


  # BMPファイルを読み込む
  def BitMap.read(filename)
    buf = nil
    open(filename, "rb") do |f|
      buf = f.read
    end
	
    if buf[0] != ?B or buf[1] != ?M
      raise('[Error] read: Invalid Header')
    end
    real_buf_size = buf.size
    buf_size = (buf[2, 4].unpack("l*"))[0]
    if buf_size > real_buf_size
      raise('[Error] read: Invalid Buffer Size')
    end
    data_offset = (buf[10, 4].unpack("l*"))[0]
    if data_offset != 54 + PALETTE_SIZE
      raise('[Error] read: Invalid Data Offset')
    end

    width = (buf[18, 4].unpack("l*"))[0]
    height = (buf[22, 4].unpack("l*"))[0]

    bit_count = (buf[28, 2].unpack("s*"))[0]
    if bit_count != 4
      raise('[Error] read: Unsupported Color Depth')
    end

    compression = (buf[30, 4].unpack("l*"))[0]
    if compression != 0
      raise('[Error] read: Compression Not Supported')
    end

    pix_per_meter = (buf[38, 4].unpack("l*"))[0]
    dpi = pix_per_meter / 39.375

    image_buf = buf[54 + PALETTE_SIZE, buf_size].unpack("C*")
    image = BitMap.new(width, height, dpi)
    image.buf = image_buf
    return image
  end


  # (x1, y1) - (x2, y2)の部分画像を取り出す
  def clip(x1, y1, x2, y2)
    return if x1 > x2
    return if y1 > y2
    return if x2 < 0
    return if y2 < 0
    return if x1 >= @width
    return if y1 >= @height
    x1 = 0 if x1 < 0
    y1 = 0 if y1 < 0
    x2 = @width - 1  if x2 >= @width
    y2 = @height - 1 if y2 >= @height

    clip_width  = x2 - x1 + 1
    clip_height = y2 - y1 + 1

    clip_image = BitMap.new(clip_width, clip_height, self.get_dpi)

    for y in 0 .. (clip_height - 1)
      for x in 0 .. (clip_width - 1)
        color = self.pget(x1 + x, y1 + y)
        clip_image.pset(x, y, color)
      end
    end

    return clip_image
  end


  # x, y, r, g, b は整数であることを期待している
  def pset(x, y, clr)
    return if x < 0 or @width <= x
    return if y < 0 or @height <= y
    clr = 0 if clr < 0
    clr = 15 if clr > 15

    nowClr = @buf[(@height - 1 - y) * @line_size + x / 2]

	if x%2 == 0 then
		nowClr = (nowClr & 0x0F) + (clr << 4)
	else
		nowClr = (nowClr & 0xF0) + clr
	end

	@buf[(@height - 1 - y) * @line_size + x / 2] = nowClr

  end

  # x, yは整数であることを期待している
  # 戻り値はパレットのインデクス整数値
  def pget(x, y)
    x = 0 if x < 0
    x = @width - 1 if x >= @width
    y = 0 if y < 0
    y = @height - 1 if y >= @height

    addr = (@height - 1 - y) * @line_size + x / 2

	if x%2 == 0 then
		retValue = @buf[addr] >> 4
	else
		retValue = @buf[addr] & 0x0F
	end

	return retValue

  end

  def get_dpi()
    return (@x_pix_per_meter / 39.375).round
  end

  def set_dpi(dpi)
    @x_pix_per_meter = (39.375 * dpi).round
    @y_pix_per_meter = @x_pix_per_meter
  end

  # x0, y0 は、貼り付ける始点(左上)の座標
  def paste(image, x0 = 0, y0 = 0)
    return if image == nil

    image.height.times do |from_y|
      y = y0 + from_y
      next if y < 0 or @height <= y

      image.width.times do |from_x|
        x = x0 + from_x
        next if x < 0 or @width <= x
        color = image.pget(from_x, from_y)
        self.pset(x, y, color)
      end
    end

  end
end
