from SimpleCV import Image 
pennies = Image("pennies.png") 
binPen = pennies.binarize() 
blobs = binPen.findBlobs() 
blobs.show(width=5)