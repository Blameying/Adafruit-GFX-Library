all: fontconvert

CC     = gcc
# 这个是加入头文件，让在打包时能找到头文件，
# 找到自己的freetype2库文件位置，按这个格式改掉下面的内容
CFLAGS = -Wall -I/opt/homebrew/Cellar/freetype/2.13.3/include/freetype2 -I/opt/homebrew/Cellar/freetype/2.13.3/include 
LIBS   = -lfreetype -L/opt/homebrew/Cellar/freetype/2.13.3/lib

fontconvert: fontconvert.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
	strip $@

clean:
	rm -f fontconvert
