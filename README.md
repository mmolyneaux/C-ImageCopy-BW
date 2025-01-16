C-ImageCopy_rgb_to_gray
This program can read a color .bmp file and create a copy that is gray scale version.


Compile: gcc -o rgb_to_gray main.c

Usage:
rgb_to_gray filename1.bmp filename2.bmp
or 
rgb_to_gray filename1.bmp

Filenames must end with .bmp
filename1 is the source, filename2 is the dest and will be overridden if it exists.
If only filename1 is specified, output will be filename1_copy.bmp