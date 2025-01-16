C-ImageCopy_bw
This program will now move towards using flags to enhance the functionality.

Compile: gcc -o imagecopy main.c

Usage:
imagecopy filename1.bmp filename2.bmp
or 
imagecopy filename1.bmp

Filenames must end with .bmp
filename1 is the source, filename2 is the dest and will be overridden if it exists.
If only filename1 is specified, output will be filename1_copy.bmp
