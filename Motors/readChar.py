import pygame

pygame.init()

display_width = 100
display_height = 600

gameDisplay = pygame.display.set_mode((display_width, display_height))
pygame.display.set_caption('Keyboard Example')

while not crashed:
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			crashed = True

			##########################
		if event.type == pygame.KEYDOWN:
			if event.key == pygame.K_LEFT:
				print "Left Key"
			if event.type == pygame.K_RIGHT:
				print "Right Key"