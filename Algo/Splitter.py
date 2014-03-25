def splitter():
	f = open('EventsScintillator.txt','r')
	t2 = open('tmpconv/t2','w')
	t3 = open('tmpconv/t3','w')
	t4 = open('tmpconv/t4','w')
	t5_11 = open('tmpconv/t5_11','w')
	t11 = open('tmpconv/t11','w')
	t12 = open('tmpconv/t12','w')
	t13 = open('tmpconv/t13','w')
	stat = open('tmpconv/stat','w')
	val = open('tmpconv/val','w')
	#Sauter la description
	for line in range (0,50):
		f.readline()
	for line in f:
		tmp = line.split()
		#list(map(float,tmp))
		#temps
		t2.write(tmp[2])
		t2.write("\n")
		#type d'evenement
		t3.write(tmp[3])
		t3.write("\n")
		#energie
		t4.write(tmp[4])
		t4.write("\n")
		#coordonnee
		for i in tmp[5:11]:
			t5_11.write(i)
			t5_11.write(" ")
		t5_11.write("\n")
		#Multiplicite
		t11.write(tmp[11])
		t11.write("\n")
		#energie
		t12.write(tmp[12])
		t12.write("\n")
		#temps
		t13.write(tmp[13])
		t13.write("\n")
		#Statut
		for i in range(14,46):
			if i%2 == 0:
				stat.write(tmp[i])
				stat.write(" ")
		stat.write("\n")
		#Valeur
		for i in range(14,47):
			if i%2 == 1:
				val.write(tmp[i])
				val.write(" ")
		val.write("\n")
	#closeall()

#Unary compression
#Alphabet:
#0, 10, 110, 1110
def statConv():
	stat = open('tmpconv/stat','r')
	#-1, 0, 1, 2
	freq = {"-1" : 0, "0" : 0, "1" : 0, "2" : 0}
	for tmp in stat:
		tmpp = tmp.split()
		for val in tmpp:
			freq[val] = freq[val] + 1
	stat.close()
	print(freq)
	#-1, 0, 1, 2
	tmpF = [(freq["-1"],"-1"), (freq["0"],"0"), (freq["1"],"1"), (freq["2"],"2")]
	#Sort from smallest to biggest
	tmpF.sort()
	#link alphabet to values, highest frequency =  smallest value
	alphaB = {tmpF[3][1] : "\x00".encode(), tmpF[2][1] : "\x10".encode(), tmpF[1][1] : "\x11\x00".encode(), tmpF[0][1] : "\x11\x10".encode()}
	statC = open('tmpconv/statCHuff','wb')
	stat = open('tmpconv/stat','r')
	print(alphaB)
	for tmp in stat:
		tmpp = tmp.split()
		for val in tmpp:
			statC.write(alphaB[val])
	stat.close()
	statC.close()

#Normalized size, each value is coded on 2 bits
#-1 = 00, 0 = 01, 1 = 10, 2 = 11
def statConv():
	stat = open('tmpconv/stat','r')
	freq = {"-1" : 0, "0" : 0, "1" : 0, "2" : 0}
	for tmp in stat:
		tmpp = tmp.split()
		for val in tmpp:
			freq[val] = freq[val] + 1
	stat.close()
	print(freq)

	stat = open('tmpconv/stat','r')
	stat.close()
	statC = open('tmpconv/statCNorm','wb')
	statC.close()

#Adaptive Huffman
#It looks like the Unary compression except that the binary values for the
#alphabet will be shorter
#The alphabet is self defined

#This is a tree to stock values and create the alphabet
class Node(object):
	def __init__(self, a):
		self.tree = [None, None]
		self.val = None
		self.freq = a[0]
		self.freqSum = a[1]
	def __init__(self):
		self.tree = [None, None]
		self.val = None
		self.freq = 0
		self.freqSum = 0
	def setLeftLeaf(self, n):
		self.tree[0] = n
		self.setFreq()
	def setRightLeaf(self, n):
		self.tree[1] = n
		self.setFreq()
	def setFreq(self):
		self.freqSum = 0
		if self.tree[0] != None:
			self.freqSum += self.tree[0].freq
		if self.tree[1] != None:
			self.freqSum += self.tree[1].freq

def statConv():
	stat = open('tmpconv/stat','r')
	#-1, 0, 1, 2
	freq = {"-1" : 0, "0" : 0, "1" : 0, "2" : 0}
	for tmp in stat:
		tmpp = tmp.split()
		for val in tmpp:
			freq[val] = freq[val] + 1
	stat.close()
	print(freq)
	#-1, 0, 1, 2
	tmpF = [(freq["-1"],"-1"), (freq["0"],"0"), (freq["1"],"1"), (freq["2"],"2")]
	tmpF.sort()

	root = Node()
	root.setLeftLeaf(Node(tmpF[0]))
	root.setRightLeaf(Node(tmpF[1]))

	for i in range(2, len(tmpF)):
		n = Node(tmpF[i])
		
	
	alphaB = {tmpF[3][1] : "\x00".encode(), tmpF[2][1] : "\x10".encode(), tmpF[1][1] : "\x11\x00".encode(), tmpF[0][1] : "\x11\x10".encode()}
	statC = open('tmpconv/statCHuff','wb')
	stat = open('tmpconv/stat','r')
	print(alphaB)
	for tmp in stat:
		tmpp = tmp.split()
		for val in tmpp:
			statC.write(alphaB[val])
	stat.close()
	statC.close()