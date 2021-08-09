
COLOR_BLUE = 0
COLOR_WHITE = 1
COLOR_GREEN = 2
COLOR_ORANGE = 3
COLOR_RED = 4
COLOR_YELLOW = 5
COLOR_COUNT = 6

def color_to_text(color):
    colors_array = ['B', 'W', 'G', 'O', 'R', 'Y']
    return colors_array[color]

class CSide(object):

    '''
    c0 e0 c1
    e3    e1
    c3 e2 c2
    '''
    
    def __init__(self, color):
        
        self.center = color
        self.edges = [color] * 4
        self.corners = [color] * 4
        
    def __repr__(self):
        result = []
        result.append([self.corners[0], self.edges[0], self.corners[1]])
        result.append([self.edges[3], self.center, self.edges[1]])
        result.append([self.corners[3], self.edges[2], self.corners[2]])
        
        return '\n'.join([' '.join(color_to_text(x) for x in rrr) for rrr in result])
        
    def get_side(self, side):
        
        if side == 'U':
            return [self.corners[0], self.edges[0], self.corners[1]]
        if side == 'D':
            return [self.corners[3], self.edges[2], self.corners[2]]
        if side == 'L':
            return [self.corners[0], self.edges[3], self.corners[3]]
        if side == 'R':
            return [self.corners[1], self.edges[1], self.corners[2]]
            
        assert False
        
    def update_side(self, side, new_vals):
        if side == 'U':
            [self.corners[0], self.edges[0], self.corners[1]] = new_vals
        if side == 'D':
            [self.corners[3], self.edges[2], self.corners[2]] = new_vals
        if side == 'L':
            [self.corners[0], self.edges[3], self.corners[3]] = new_vals
        if side == 'R':
            [self.corners[1], self.edges[1], self.corners[2]] = new_vals
        
        
    def rotate(self, is_clockwise):
        if is_clockwise:
            x = self.edges.pop(-1)
            self.edges.insert(0, x)
            
            x = self.corners.pop(-1)
            self.corners.insert(0, x)
        else:
            x = self.edges.pop(0)
            self.edges.append(x)

            x = self.corners.pop(0)
            self.corners.append(x)

class CRubiks(object):
    
    '''
      B
    O W R
      G
      Y
    '''
    
    def __init__(self):
        self.sides = []
        self.sides_hash = {}
        for i in xrange(COLOR_COUNT):
            self.sides.append(CSide(i))
        self.init_sides_hash()
        
    def init_sides_hash(self):
        self.sides_hash['B'] = COLOR_BLUE
        self.sides_hash['F'] = COLOR_GREEN
        self.sides_hash['L'] = COLOR_ORANGE
        self.sides_hash['U'] = COLOR_WHITE
        self.sides_hash['R'] = COLOR_RED
        self.sides_hash['D'] = COLOR_YELLOW
        
    
    def rotate_axis(self, axis):
        
        flow = ''
        rotate_me = ''
        is_clockwise = True
        is_anti = False
        if axis[0] == 'Y':
            flow = 'FLBR'
            rotate_me = ["U'", "D"]

        if axis[0] == 'Z':
            flow = 'LURD'
            rotate_me = ["F'", "B"]
        
        if axis[-1] == "'":
            flow = flow[::-1]
            is_anti = True

        for rr in rotate_me:
            side = rr[0]
            is_clockwise  = rr[-1] == "'"
            if is_anti:
                is_clockwise = not is_clockwise
            self.sides[self.sides_hash[side]].rotate(is_clockwise)
            
        assert len(flow)
        flow = flow[-1] + flow
        new_val = self.sides[self.sides_hash[flow[0]]]
        for i in xrange(len(flow) - 1):
            old_val = self.sides[self.sides_hash[flow[i+1]]]
            self.sides[self.sides_hash[flow[i+1]]] = new_val
            new_val = old_val
            
    def rotate(self, action):
        vals = action.split(' ')
        print vals
        for action in vals:
            repeat_me = 1
            if action[-1] == '2':
                action = action[:-1]
                repeat_me = 2
            
            print ('%s%d' % (action, repeat_me)),
            for i in xrange(repeat_me):
                self.rotate_single(action)
        print ''
    
    def rotate_single(self, action):
    
        is_clockwise = len(action) == 1
        action = action[0]
        rotated = ''
        
        if action == 'F':
            rotated = 'Y'
            action = 'L'
            
        elif action == 'B':
            rotated = 'Y'
            action = 'R'

        elif action == 'U':
            rotated = 'Z'
            action = 'R'

        elif action == 'D':
            rotated = 'Z'
            action = 'L'

        if rotated:
            self.rotate_axis(rotated)

        side = self.sides_hash[action]
        
        self.sides[side].rotate(is_clockwise)
        
        is_opposite = False
        if action == 'R' or action == 'L':
            flow = 'UBDF'
            is_opposite = (action == 'R' and not is_clockwise) or (action == 'L' and is_clockwise)
        else:
            assert False
        
        if is_opposite:
            flow = flow[::-1]

        flow = flow[-1] + flow
        new_vals = self.sides[self.sides_hash[flow[0]]].get_side(action)
        for i in xrange(len(flow) - 1):
            old_vals = self.sides[self.sides_hash[flow[i+1]]].get_side(action)
            self.sides[self.sides_hash[flow[i+1]]].update_side(action, new_vals)
            new_vals = old_vals

        if rotated:
            self.rotate_axis(rotated + "'")
            
    def __repr__(self):
        result = []
        back_side = str(cube.sides[0])
        vals = back_side.split('\n')
        for v in vals:
            result.append(' '* (len(v) + 3) + v)
        result.append('')
        
        left_side = str(cube.sides[3])
        lvals = left_side.split('\n')

        top_side = str(cube.sides[1])
        tvals = top_side.split('\n')

        right_side = str(cube.sides[4])
        rvals = right_side.split('\n')
        
        for i in xrange(len(lvals)):
            sss = '%s   %s   %s' % (lvals[i], tvals[i], rvals[i])
            result.append(sss)
        result.append('')
        
        front_side = str(cube.sides[2])
        vals = front_side.split('\n')
        for v in vals:
            result.append(' '* (len(v) + 3) + v)
        result.append('')

        down_side = str(cube.sides[5])
        vals = down_side.split('\n')
        for v in vals:
            result.append(' '* (len(v) + 3) + v)

        return '\n'.join(result)


cube = CRubiks()
'''
for i in xrange(6):
    cube.rotate_single("R")
    cube.rotate_single("U")
    cube.rotate_single("R'")
    cube.rotate_single("U'")
#'''

print cube
cube.rotate("R2 B2 U B2 U2 D' F2 D B2 R2 D' F' B' D F U2 R' U F' L' F2")
print cube
