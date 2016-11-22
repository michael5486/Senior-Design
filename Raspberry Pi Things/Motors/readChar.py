from khbit import KBHit

kb = KBHit()

print ('Enter a key')

while True:
	if kb.kbhit():
		c = kg.getch()
		if ord(c) == 27:
			break
		print(c)
		print ord(c)
	kb.set_normal_term()	