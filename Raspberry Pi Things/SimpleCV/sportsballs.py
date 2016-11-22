from SimpleCV import Color, Image
import time

img = Image("sportsballs.jpg")
circles = img.findCircle(canny=200, thresh=250, distance=15)
circles.sortArea()
circles.draw(width=4)
circles[0].draw(color=Color.RED, width = 4)
img_with_circles = img.applyLayers()
edges_in_image = img.edges(t2=200)
final = img.sideBySide(edges_in_image.sideBySide(img_with_circles)).scale(0.5)
final.show()
time.sleep(15)