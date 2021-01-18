# Stitch Functions

stitch2.py contains `stitch2()`, the function which does the stitching. Stitcher.py provides the CLI interface, and is used to compile the compressed `--onefile` pyinstaller exe, which is then moved to the root folder. Stitcher.exe needs to be in the root folder to work, because I'm rad.

If you want more control over the way maps are stitched, currently you have to call `stitch2()` from a python interpreter directly. This will allow you to set a different destination file, change the amount of image feathering for each image, and also control how much the images are cropped.
