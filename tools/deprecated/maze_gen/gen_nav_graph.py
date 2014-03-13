# generates navigation graph
# dead simple because of simple form of level (2d rectangular grid)

import sys

kEmpty = " "

def firstMatchingPredicateIdx(lst, pred):
    """ Returns index of first element of list lst matching
    predicate pred. If no such element is found, returns (last valid idx + 1)"""
    idx = 0
    end = len(lst)
    while idx < end and not pred(lst[idx]):
          idx += 1

    return idx

def isWalkable(cell):
    return cell == kEmpty

def firstWalkableIdx(line):
    return firstMatchingPredicateIdx(line, isWalkable)

def firstNotWalkableIdx(line):
    return firstMatchingPredicateIdx(line, lambda t: not isWalkable(t))

def isRangeWalkable(line, start_x, end_x):
    return all([isWalkable(cell) for cell in line[start_x:end_x]])

lvl = []
kWidth = 1
for line in sys.stdin:
      if line == None or len(line) == 0:
         break
      else:
           lvl = lvl + [list(line.strip())]
           kWidth = max(kWidth, len(line.strip()))

out_lvl = [list(line) for line in lvl]

# find largest rectangular free spaces
spaces_chars = "asdfghjklzxcvbnm,.?<>/;':\"[]{}\|!@$%^&()-+="
spaces = []
last_space_idx = 9
for y in range(len(lvl)):
    cur_line = lvl[y]
    start_x = firstWalkableIdx(cur_line)
    while start_x < len(cur_line):
       end_x = start_x + firstNotWalkableIdx(cur_line[start_x:])
       last_x = end_x - 1

       # found walkable surface from (start_x, y) to (last_x, y),
       # try to extend it vertically

       # end_y will be the first line after y not fully walkable in range [start_x, end_x)
       end_y = y + 1
       while end_y < len(lvl) and isRangeWalkable(lvl[end_y], start_x, end_x):
             end_y += 1

       # "alloc" new space and fill output lvl with indices
       last_space_idx += 1
       for out_y in range(y, end_y):
           for out_x in range(start_x, end_x):
               out_lvl[out_y][out_x] = last_space_idx#spaces_chars[last_space_idx]

       # find next walkable line in this row
       #print start_x,
       start_x = end_x + firstWalkableIdx(cur_line[end_x:])
       #print start_x

# print out level
for line in lvl:
    print " ".join(map(lambda t: str(t) * 2, line))
for line in out_lvl:
    ln = ""
    for c in line:
        if type(c) is int: ln = ln + str(c) + " "
        else: ln = ln + str(c) * 2 + " "
    print ln

