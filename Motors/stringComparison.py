testVar = None
run = True
while run == 1:
	testVar = raw_input("Ask user for something.\n")
	if testVar == "exit":
		run = False
		print "System Exiting..."
	else:
		print testVar