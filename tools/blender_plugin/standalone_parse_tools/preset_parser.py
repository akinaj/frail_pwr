##################### Uzycie ####################
#	python level_parser.py filename		        #
#	   filename: pelna nazwa pliku z presetami	#
#			np. ModelObject.json				#
#				                 				#
#################################################
import os
import sys
import re

def parse(ugly_file):

	os.chdir("../../../build/data/presets")
	jin = open(ugly_file)
	out = open("../mod_presets/" + ugly_file,'w')
	lines = jin.readlines()
	curly_pattern = re.compile('\{ *(-?\d+\.\d+) *,? *(-?\d+\.\d+) *,? *(-?\d+\.\d+) *,? *\}') #wtf.


	for i in range(0,len(lines)):
		line = lines[i]
		match = curly_pattern.search(line)
		
		if match:
			repl = '[' + match.group(1) + ', ' + match.group(2) + ', ' + match.group(3) + ']'
			line = curly_pattern.sub(repl,line)
			out.write(line)
		# komentarze
		elif (line.strip()).startswith("//"):
			out.write(' ')
		else:
			out.write(line)


if __name__ == '__main__':
 		parse(sys.argv[1])
 		#todo: mozna dodac jakie except, czekam na pomysly.
