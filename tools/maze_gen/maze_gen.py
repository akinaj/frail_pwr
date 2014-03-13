# dead simple 'level' generator
# generates maze using dfs and then clears random rectangular
# chambers until specified part of map is empty (walkable)
# todo: add data needed by game (player/enemies possible spawn points etc)
import random, sys

kInvalid          = '@'
kEmpty            = ' '
kWall             = '#'
kObstacle         = 'o'
kBigObstacle      = 'O'

# not implemented yet
kPlayerSpawnPoint = 'P'
kEnemySpawnPoint  = 'E'
kTrap             = 'T'

kWidth  = 9
kHeight = 9

kSpawnPtsDensity = 0.0
kObstacleDensity = 0.3

kPrimitiveObstaclePercentage = 0.5
kComplexObstaclePercentage = 1 - kPrimitiveObstaclePercentage

def get_crd_idx(x, y):
    return x + kWidth * y

def get_idx_crd(idx):
    return (idx % kWidth, idx / kWidth)

class Cell:
      def __init__(self, idx):
          self.idx = idx
          x, y = get_idx_crd(idx)
          self.visited = False
          self.type = kWall
          self.crd = (x, y)

          ne = [-1, -1, -1, -1]
          if (x + 1) < kWidth: ne[0] = get_crd_idx(x + 1, y)   # west
          if (y + 1) < kHeight: ne[1] = get_crd_idx(x, y + 1)  # south
          if (x - 1) >= 0: ne[2] = get_crd_idx(x - 1, y)       # east
          if (y - 1) >= 0: ne[3] = get_crd_idx(x, y - 1)       # north
          self.neighbors = tuple(ne)

          self.walls = [kObstacle, kBigObstacle] # west-north walls only

      def get_neighbors(self):
          return [board[idx] for idx in self.neighbors if idx != -1]

      def rm_wall(self, from_ne):
          rx, ry = ((from_ne.crd[0] - self.crd[0]), (from_ne.crd[1] - self.crd[1]))
          if   rx == 1:  self.walls[0] = kEmpty
          elif ry == 1:  self.walls[1] = kEmpty

      def clear_walls(self):
          self.walls = [kEmpty, kEmpty]

      def is_empty(self):
          return self.walls == [kEmpty, kEmpty]


board = [Cell(i) for i in range(kWidth * kHeight)]

# draw cells and their walls
def print_board():
    # northern level wall
    print kWall * (kWidth * 2 + 2)

    for y in range(kHeight):
        sys.stdout.write(kWall) # first western level wall segment in this line
        for x in range(kWidth):
            c = board[get_crd_idx(x, y)]
            sys.stdout.write(str(c.type) + str(c.walls[0]))
        sys.stdout.write(kWall)
        sys.stdout.write("\n")

        sys.stdout.write(kWall) # second western level wall segment in this line
        for x in range(kWidth):
            c = board[get_crd_idx(x, y)]
            if c.is_empty():
               sys.stdout.write("  ")
            else: # print also corner walls of not empty cells
                 sys.stdout.write(str(c.walls[1]) + kEmpty)
        sys.stdout.write(kWall)
        sys.stdout.write("\n")
    print kWall * (kWidth * 2 + 2)

# maze generation using dfs
stack = [random.choice(board)]
while len(stack) > 0:
    cur_cell = stack[-1]
    cur_cell.type = kEmpty
    cur_cell.visited = True

    nes = filter(lambda cell: not cell.visited, cur_cell.get_neighbors())
    if len(nes) > 0:
        ne = random.choice(nes)
        ne.rm_wall(cur_cell)
        cur_cell.rm_wall(ne)
        stack.append(ne)
    else:
        stack.pop()

# put random empty chambers on top of generated maze for required density
def maze_density():
    empty = len(filter(Cell.is_empty, board))
    return empty / float(len(board))

i = 0
kChamberMaxWidth = kWidth / 4
kChamberMaxHeight = kHeight / 8
while maze_density() < 0.6 and i < 50:
      i += 1
      ch_w = random.randrange(kChamberMaxWidth) + 1
      ch_h = random.randrange(kChamberMaxHeight) + 1
      ch_x = random.randrange(kWidth - ch_w - 2) + 1
      ch_y = random.randrange(kHeight - ch_h - 2) + 1

      for y in range(ch_y, ch_y + ch_h):
          for x in range(ch_x, ch_x + ch_w):
              cell = board[get_crd_idx(x, y)]
              cell.clear_walls()

# put enemy spawn points on top of generated level
num_enemy_spawn_pts = int(kWidth * kHeight * kSpawnPtsDensity)

for i in range(num_enemy_spawn_pts):
    cell = random.choice(board)
    cell.type = kEnemySpawnPoint

# choose random empty cell as player spawn point
player_spawn_pt = random.choice(filter(lambda x: x.type == kEmpty, board))
player_spawn_pt.type = kPlayerSpawnPoint

# insert obstacles
num_obstacle_blocks = int(kWidth * kHeight * kObstacleDensity)

# TODO: complex obstacles
for i in range(num_obstacle_blocks):
    cell = random.choice(board)
    cell.type = kObstacle


print_board()
