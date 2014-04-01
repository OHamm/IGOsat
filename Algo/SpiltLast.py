def splitter():
	f = open('EventsScintillator.txt','r')
	t = open('splitLast','w')
	#Sauter les premieres lignes
	for line in range (0,50):
		f.readline()
	for line in f:
		tmp = line.split()
		for i in tmp[13:]:
			t.write(i)
			t.write(" ")
		t.write("\n")
	f.close()
	t.close()
