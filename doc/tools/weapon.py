#!/bin/env python

import xml.dom.minidom as dom
import sys
import struct

WEAP_NUM = 780

struct_fmt = "<H BBHBBBB 8B8B8b8b8b8b8H bbBBBB"

def pack_weapon(dict):
	l = []

	l.append(dict['drain'])
	l.append(dict['shotRepeat'])
	l.append(dict['multi'])
	l.append(dict['weapAni'])
	l.append(dict['max'])
	l.append(dict['tx'])
	l.append(dict['ty'])
	l.append(dict['aim'])

	tmp = dict['patterns']
	for j in range(8):
		l.append(tmp[j]['attack'])
	for j in range(8):
		l.append(tmp[j]['del'])
	for j in range(8):
		l.append(tmp[j]['sx'])
	for j in range(8):
		l.append(tmp[j]['sy'])
	for j in range(8):
		l.append(tmp[j]['bx'])
	for j in range(8):
		l.append(tmp[j]['by'])
	for j in range(8):
		l.append(tmp[j]['sg'])

	l.append(dict['acceleration'])
	l.append(dict['accelerationx'])
	l.append(dict['circleSize'])
	l.append(dict['sound'])
	l.append(dict['trail'])
	l.append(dict['shipBlastFilter'])

	return struct.pack(struct_fmt, *l)

def unpack_weapon(str):
	tup = struct.unpack(struct_fmt, str)
	dict = {}

	dict['drain'] = tup[0]
	dict['shotRepeat'] = tup[1]
	dict['multi'] = tup[2]
	dict['weapAni'] = tup[3]
	dict['max'] = tup[4]
	dict['tx'] = tup[5]
	dict['ty'] = tup[6]
	dict['aim'] = tup[7]

	i = 8

	tmp = [{} for j in range(8)]
	for j in range(8):
		tmp[j]['attack'] = tup[i]
		i += 1
	for j in range(8):
		tmp[j]['del'] = tup[i]
		i += 1
	for j in range(8):
		tmp[j]['sx'] = tup[i]
		i += 1
	for j in range(8):
		tmp[j]['sy'] = tup[i]
		i += 1
	for j in range(8):
		tmp[j]['bx'] = tup[i]
		i += 1
	for j in range(8):
		tmp[j]['by'] = tup[i]
		i += 1
	for j in range(8):
		tmp[j]['sg'] = tup[i]
		i += 1
	dict['patterns'] = tmp

	dict['acceleration'] = tup[i]
	dict['accelerationx'] = tup[i+1]
	dict['circleSize'] = tup[i+2]
	dict['sound'] = tup[i+3]
	dict['trail'] = tup[i+4]
	dict['shipBlastFilter'] = tup[i+5]
	
	return dict

def DOMToDict(doc, weap_node):
	dict = {}

	for i in weap_node.childNodes:
		if i.nodeType != i.ELEMENT_NODE:
			continue

		if i.hasAttribute("value"):
			dict[i.tagName] = int(i.getAttribute("value"))
		elif i.tagName == "patterns":
			dict['patterns'] = [{} for el in range(8)]
			index = 0
			for j in i.childNodes:
				if j.nodeType != i.ELEMENT_NODE:
					continue

				attrs = [j.attributes.item(i) for i in range(j.attributes.length)]
				for i in attrs:
					dict['patterns'][index][i.name] = int(i.nodeValue)
				index += 1

	return dict

def dictToDOM(doc, root, dict, index=None):
	entry = doc.createElement("weapon")
	if index != None:
		entry.setAttribute("index", "%04X" % (index,))
	
	keys = list(dict)
	keys.sort()
	for i in keys:
		node = doc.createElement(i)
		if isinstance(dict[i], list):
			for j in dict[i]:
				keys = list(j)
				keys.sort()

				n = doc.createElement("entry")
				for i in keys:
					n.setAttribute(i, str(j[i]))
				node.appendChild(n)
		else:
			node.setAttribute("value", str(dict[i]))
		entry.appendChild(node)
	
	root.appendChild(entry)

def toXML(hdt, output):
	doc = dom.getDOMImplementation().createDocument(None, "TyrianHDT", None)

	try:
		f = open(hdt, "rb")
	except IOError:
		print("%s couldn't be opened for reading." % (hdt,))
		sys.exit(1)

	try:
		outf = open(output, "w")
	except IOError:
		print("%s couldn't be opened for writing." % (output,))
		sys.exit(1)

	f.seek(struct.unpack("<i", f.read(4))[0])
	f.read(7*2)


	sys.stdout.write("Converting weapons")
	index = 0

	for i in range(WEAP_NUM+1):
		tmp = f.read(struct.calcsize(struct_fmt))
		shot = unpack_weapon(tmp)
		dictToDOM(doc, doc.documentElement, shot, index)
		index += 1

		sys.stdout.write(".")
		sys.stdout.flush()

	sys.stdout.write("Done!\n")
	sys.stdout.write("Writing XML...")
	sys.stdout.flush()
	doc.writexml(outf, addindent="\t", newl="\n")
	sys.stdout.write("Done!\n")

def toHDT(input, hdt):
	try:
		f = open(input, "r")
	except IOError:
		print("%s couldn't be opened for reading." % (input,))
		sys.exit(1)

	try:
		outf = open(hdt, "r+b")
	except IOError:
		print("%s couldn't be opened for writing." % (hdt,))
		sys.exit(1)

	outf.seek(struct.unpack("<i", outf.read(4))[0])
	outf.read(7*2)

	sys.stdout.write("Reading XML...")
	sys.stdout.flush()
	doc = dom.parse(f)
	sys.stdout.write("Done!\n")

	sys.stdout.write("Writing weapons")

	for i in doc.documentElement.childNodes:
		if i.nodeType != i.ELEMENT_NODE:
			continue

		shot = DOMToDict(doc, i)
		str = pack_weapon(shot)

		outf.write(str)

		sys.stdout.write(".")
		sys.stdout.flush()

	sys.stdout.write("Done!\n")

def printHelp():
	print("Usage: weapon.py toxml path/to/tyrian.hdt output.xml")
	print("       weapon.py tohdt input.xml path/to/tyrian.hdt")
	sys.exit(1)

##############################

if __name__ == "__main__":
	if len(sys.argv) != 4:
		printHelp()

	if sys.argv[1] == "toxml":
		toXML(sys.argv[2], sys.argv[3])
	elif sys.argv[1] == "tohdt":
		toHDT(sys.argv[2], sys.argv[3])
	else:
		printHelp()
