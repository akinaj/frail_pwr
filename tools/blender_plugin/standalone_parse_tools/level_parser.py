##################### Uzycie ####################
#	python level_parser.py -f | -b levelname	#
#		-f (forwards): parses ugly .json to     #
#			pretty (vanilla) .json				#
#		-b (backwards): the opposite way		#
#		levelname: full level name, i ex     	#
#				arena_a.json					#
#################################################
import os
import re
import sys


def parse(ugly_file):
	os.chdir("../../../build/data/levels")
	jin = open(ugly_file)
	out = open("../mod_levels/" + ugly_file,'w')

	curly_pattern = re.compile('\{ *(-?\d+\.\d+) *,? *(-?\d+\.\d+) *,? *(-?\d+\.\d+) *,? *\}') #wtf.
	curly_pattern_ext = re.compile('\{ *(-?\d+\.\d+) *,? *(-?\d+\.\d+) *,? *(-?\d+\.\d+) *,? *(-?\d+\.\d+) *\}')
	lines = jin.readlines()

	for i in range(0,len(lines)):
		line = lines[i]
		# vectors (pf.) -> arrays
		match = curly_pattern.search(line)
		match2 = curly_pattern_ext.search(line)
		if match:
			print match.group(1)
			repl = '[' + match.group(1) + ', ' + match.group(2) + ', ' + match.group(3) + ']'
			line = curly_pattern.sub(repl,line)
			out.write(line)
		elif match2:
			repl = '[' + match2.group(1) + ', ' + match2.group(2) + ', ' + match2.group(3) +  ', ' + match2.group(4) + ']'
			line = curly_pattern_ext.sub(repl,line)
			out.write(line)
		# comments
		elif (line.strip()).startswith("//"):
			out.write(' ')
		else:
			out.write(line)


def parse_back(pretty_file):
	os.chdir("../../../build/data/mod_levels")
	jin = open(pretty_file)
	(name, ext) = os.path.splitext(pretty_file)
	out = open('../levels/' + name + '_mod' + ext,'w') 

	lines = jin.readlines()

	for i in range(0,len(lines)):
		line = lines[i]
		# arrays -> vectors
		line = re.sub('\[','{',line)
		line = re.sub('\]','}',line)
		out.write(line)


		
if __name__ == '__main__':
 	if sys.argv[1] == '-f':
 		parse(sys.argv[2])
 	elif sys.argv[1] == '-b':
 		parse_back(sys.argv[2])
 	else:
 		raise AttributeError('Invalid command line parameters! See the script header for more info')