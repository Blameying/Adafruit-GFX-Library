#!/bin/bash

# Ugly little Bash script, generates a set of .h files for GFX using
# GNU FreeFont sources.  There are three fonts: 'Mono' (Courier-like),
# 'Sans' (Helvetica-like) and 'Serif' (Times-like); four styles: regular,
# bold, oblique or italic, and bold+oblique or bold+italic; and four
# sizes: 9, 12, 18 and 24 point.  No real error checking or anything,
# this just powers through all the combinations, calling the fontconvert
# utility and redirecting the output to a .h file for each combo.

# Adafruit_GFX repository does not include the source outline fonts
# (huge zipfile, different license) but they're easily acquired:
# http://savannah.gnu.org/projects/freefont/

convert=./fontconvert  #结构体
inpath=~/Desktop/freefont/ #输入位置
outpath=./Fonts/  #输出位置
fonts=(songti)  #字体
styles=("")   #字体样式
sizes=(12)   #字体大小

for f in ${fonts[*]} #遍历字体表
do
	for index in ${!styles[*]} #字体样式
	do
		st=${styles[$index]} #
		for si in ${sizes[*]}
		do
			infile="songti.ttf"
			if [ -f $infile ] # Does source combination exist?
			  then
				outfile=$outpath$f$st$si"pt7b.h"
				printf "%s %s %s > %s\n" $convert $infile $si $outfile
				$convert $infile $si > $outfile
			fi
		done
	done
done
