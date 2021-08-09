
COLOR_BLUE = 0
COLOR_WHITE = 1
COLOR_GREEN = 2
COLOR_ORANGE = 3
COLOR_RED = 4
COLOR_YELLOW = 5
COLOR_COUNT = 6

def color_to_text(color):

    colors_array = ['B', 'W', 'G', 'O', 'R', 'Y']
    if not CSide.is_debug_mode:
        return colors_array[color]
    else:
        #if color % 10 == 0:
            #return '%s%d' % (colors_array[color/10 - 1], color/10)
        #return str(color)
        return '%s%d' % (colors_array[color/10 - 1], color%10)

class CSide(object):

    '''
    c0 e0 c1
    e3    e1
    c3 e2 c2
    '''
    
    is_debug_mode = False
    
    def __init__(self, color):
        
        if not CSide.is_debug_mode:
            self.center = color
            self.edges = [color] * 4
            self.corners = [color] * 4
        else:
            color += 1
            self.center = color * 10
            self.edges = [color*10 + i*2 + 2 for i in xrange(4)]
            self.corners = [color*10 + i*2 + 1 for i in xrange(4)]
        self.is_back_side = False
        
    def update_back_side(self):
        self.is_back_side = True
        
    def __repr__(self):
        result = []
        if True: #not self.is_back_side:
            result.append([self.corners[0], self.edges[0], self.corners[1]])
            result.append([self.edges[3], self.center, self.edges[1]])
            result.append([self.corners[3], self.edges[2], self.corners[2]])
        #'''
        else:
            result.append([self.corners[1], self.edges[0], self.corners[0]])
            result.append([self.edges[1], self.center, self.edges[3]])
            result.append([self.corners[2], self.edges[2], self.corners[3]])
        #'''
            
        return '\n'.join([' '.join(color_to_text(x) for x in rrr) for rrr in result])
        
    def get_side(self, side):
        
        #'''
        if self.is_back_side:
            if side == 'L':
                side = 'R'
            elif side == 'R':
                side = 'L'
        #'''
        
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
        #'''
        if self.is_back_side:
            if side == 'L':
                side = 'R'
            elif side == 'R':
                side = 'L'
        #'''
        if side == 'U':
            [self.corners[0], self.edges[0], self.corners[1]] = new_vals
        elif side == 'D':
            [self.corners[3], self.edges[2], self.corners[2]] = new_vals
        elif side == 'L':
            [self.corners[0], self.edges[3], self.corners[3]] = new_vals
        elif side == 'R':
            [self.corners[1], self.edges[1], self.corners[2]] = new_vals
        else:
            assert False
        
        
    def rotate(self, is_clockwise):
        #if self.is_back_side:
            #is_clockwise = not is_clockwise
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

class CCube(object):
    
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
        
        self.sides[self.sides_hash['B']].update_back_side()
        
    def init_sides_hash(self):
        self.sides_hash['B'] = COLOR_BLUE
        self.sides_hash['F'] = COLOR_GREEN
        self.sides_hash['L'] = COLOR_ORANGE
        self.sides_hash['U'] = COLOR_WHITE
        self.sides_hash['R'] = COLOR_RED
        self.sides_hash['D'] = COLOR_YELLOW
        
    def rotate(self, action):
        vals = action.split(' ')
        print ' '.join(vals)
        for action in vals:
            repeat_me = 1
            if action[-1] == '2':
                action = action[:-1]
                repeat_me = 2

            for i in xrange(repeat_me):
                self.rotate_single(action)
        print ''
    
    def translate_rotate_action(self, action, flow):
        if action =='F':
            reverse_hash = {'U': 'D', 'D': 'U', 'L': 'R', 'R': 'L'}
            #reverse_hash = {'U': 'U', 'D': 'D', 'L': 'L', 'R': 'R'}
            return reverse_hash[flow]
        elif action == 'B':
            reverse_hash = {'U': 'U', 'D': 'D', 'L': 'L', 'R': 'R'}
            return reverse_hash[flow]
        else:
            return action
    
    def rotate_single(self, action):
    
        is_clockwise = len(action) == 1
        action = action[0]

        side = self.sides_hash[action]
        self.sides[side].rotate(is_clockwise)
        is_opposite = False
        
        if action == 'R' or action == 'L':
            flow = 'UBDF'
            is_opposite = (action == 'R' and not is_clockwise) or (action == 'L' and is_clockwise)
        elif action == 'U' or action == 'D':
            flow = 'FLBR'
            is_opposite = (action == 'U' and not is_clockwise) or (action == 'D' and is_clockwise)
        elif action == 'F' or action == 'B':
            flow = 'URDL'
            is_opposite = (action == 'F' and not is_clockwise) or (action == 'B' and is_clockwise)
        else:
            assert False
        
        if is_opposite:
            flow = flow[::-1]

        flow = flow[-1] + flow

        new_vals = self.sides[self.sides_hash[flow[0]]].get_side(self.translate_rotate_action(action, flow[0]))
        for i in xrange(len(flow) - 1):
            old_vals = self.sides[self.sides_hash[flow[i+1]]].get_side(self.translate_rotate_action(action, flow[i+1]))
            if action == 'B' and flow[i+1] in ['U', 'D']:
                new_vals = new_vals[::-1]

            #print '%s: %s %s' % (flow[i+1], action, ' '.join([color_to_text(x) for x in new_vals]))
            if action in ['R', 'L'] and flow[i+1] in ['B']:
                new_vals = new_vals[::-1]
                #print 1
            if action in ['R', 'L'] and flow[i] in ['B']:
                #print 2
                new_vals = new_vals[::-1]
            '''
            if action in ['D', 'U', 'R', 'L'] and flow[i+1] in ['B']:
                print 1
                print ' '.join([color_to_text(x) for x in new_vals])
                new_vals = new_vals[::-1]
            #'''
            self.sides[self.sides_hash[flow[i+1]]].update_side(self.translate_rotate_action(action, flow[i+1]), new_vals)
            new_vals = old_vals
            
    def __repr__(self):
    
        result = []

        top_side = str(cube.sides[1])
        vals = top_side.split('\n')
        for v in vals:
            result.append(' '* (len(v) + 3) + v)
        result.append('')

        left_side = str(cube.sides[3])
        lvals = left_side.split('\n')

        front_side = str(cube.sides[2])
        fvals = front_side.split('\n')

        right_side = str(cube.sides[4])
        rvals = right_side.split('\n')

        back_side = str(cube.sides[0])
        bvals = back_side.split('\n')

        for i in xrange(len(lvals)):
            sss = '%s   %s   %s   %s' % (lvals[i], fvals[i], rvals[i], bvals[i])
            result.append(sss)
        result.append('')

        down_side = str(cube.sides[5])
        vals = down_side.split('\n')
        for v in vals:
            result.append(' '* (len(v) + 3) + v)

        return '\n'.join(result)


cube = CCube()

            #L2 U' L2 U L R' F' L2 F L R

#print cube
cube.rotate("R2 U R' B' R B2 U' B D2 F L2 F' D2 B2 R2")
print cube

#cube.rotate("F'")
#print cube



