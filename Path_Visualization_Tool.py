import pygame
import math 
from queue import PriorityQueue

WHITE = (255,255,255)
ORANGE = (255, 165 ,0)
PURPLE = (128, 0, 128)
RED = (255,0,0)
TURQUOISE = (64, 224, 208)
GREEN = (0,255,0)
BLUE = (0,0,255)
GREY = (220,220,220)
BLACK = (0,0,0)

dim = 500
rows = 50
box_dim = dim//rows
screen = pygame.display.set_mode((dim,dim))
pygame.display.set_caption("Path Visualization Tool.")

class Grid:
    def __init__(self,screen):
        self.grid = [[Spot(i,j) for i in range(rows)] for j in range(rows)]
        self.START = None
        self.END = None
        screen.fill(WHITE)
        for i in range(rows):
            pygame.draw.line(screen,GREY,(0,i*box_dim),(dim,i*box_dim))
        for i in range(rows):
            pygame.draw.line(screen,GREY,(i*box_dim,0),(i*box_dim,dim))
        pygame.display.update()

    def add_neighbor(self):
        for i in self.grid:
            for spot in i:
                if spot.x < rows-1 and self.grid[spot.y][spot.x + 1].color != BLACK:
                    spot.neighbor.append(self.grid[spot.y][spot.x + 1])
                if spot.x > 0 and self.grid[spot.y][spot.x - 1].color != BLACK:
                    spot.neighbor.append(self.grid[spot.y][spot.x - 1])
                if spot.y < rows-1 and self.grid[spot.y + 1][spot.x].color != BLACK:
                    spot.neighbor.append(self.grid[spot.y + 1][spot.x])
                if spot.y > 0 and self.grid[spot.y - 1][spot.x].color != BLACK:
                    spot.neighbor.append(self.grid[spot.y - 1][spot.x])
    
    def start(self):
        while True:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    break
                if pygame.mouse.get_pressed()[0]:
                    pos = pygame.mouse.get_pos()
                    spot = self.grid[pos[1]//box_dim][pos[0]//box_dim]
            
                    if self.START == None and spot != self.END:
                        self.START = spot
                        self.START.color = BLUE
                        self.START.draw()
                    elif self.END == None and spot != self.START:
                        self.END = spot
                        self.END.color = ORANGE
                        self.END.draw()                
                    elif spot != self.START and spot != self.END:
                        spot.color = BLACK
                        spot.draw()
                if pygame.mouse.get_pressed()[2]:
                    pos = pygame.mouse.get_pos()
                    spot = self.grid[pos[1]//box_dim][pos[0]//box_dim]
                    spot.color = WHITE
                    if spot == self.START:
                        self.START = None
                    elif spot == self.END:
                        self.END = None
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_c:
                        self = Grid(screen)
                    if event.key == pygame.K_n:
                        for i in self.START.neighbor:
                            i.color = TURQUOISE
                            i.draw()
                    if event.key == pygame.K_SPACE:
                        self.add_neighbor()
                        if self.algo() == True:
                            print("Done")
                            self.path_construct()
                        else:
                            print("No path possible.")      
            pygame.display.update()
    
    def algo(self):
        self.START.f = self.START.h_func(self.END)
        self.START.g = 0
        open_set = PriorityQueue()
        open_set.put((self.START.f, self.START))
        open_set_hash = {self.START}
        while open_set.empty() == False:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    return False

            current = open_set.get()[1]
            open_set_hash.remove(current)
            if current == self.END:
                current.color = ORANGE
                current.draw()
                return True
            for neighbor in current.neighbor:
                if current.g + 1 < neighbor.g:
                    neighbor.g = current.g + 1
                    neighbor.f = neighbor.g + neighbor.h_func(self.END)
                    neighbor.prev = current
                    if neighbor not in open_set_hash:
                        open_set.put((neighbor.f,neighbor))
                        open_set_hash.add(neighbor)
                        neighbor.color = PURPLE
                        neighbor.draw()
            if current != self.START:
                current.color = RED
                current.draw()
            pygame.display.update()
        return False
    
    def path_construct(self, current = None):
        if current == None:
            current = self.END.prev
        if current == self.START:
            return
        else:
            current.color = GREEN
            current.draw()
            pygame.display.update()
            return self.path_construct(current.prev)



class Spot:
    def __init__(self,x,y):
        self.x = x
        self.y = y
        self.f = float('inf')
        self.g = float('inf')
        self.h = float('inf')
        self.color = WHITE
        self.neighbor = []
        self.prev = None
    
    def draw(self):
        pygame.draw.rect(screen,self.color,(self.x*box_dim,self.y*box_dim,box_dim,box_dim))
    
    def h_func(self,other):
        return abs(self.x - other.x) + abs(self.y - other.y)
    
    def __lt__(self,other):
        return False

x = Grid(screen)
x.start()