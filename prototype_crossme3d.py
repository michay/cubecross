# rubiks algorithm prototype, implemented in c 

import time

COLOR_BLUE = 0
COLOR_WHITE = 1
COLOR_GREEN = 2
COLOR_ORANGE = 3
COLOR_RED = 4
COLOR_YELLOW = 5
COLOR_COUNT = 6

def color_to_text(color):

    colors_array = ['B', 'W', 'G', 'O', 'R', 'Y']
    if color < 10:
        return colors_array[color-1]
    elif not CSide.is_debug_mode:
        return colors_array[color/10 - 1]
    else:
        #return '%s%d' % (colors_array[color/10 - 1], color%10)
        return str(color)
        
class CLink(object):
    
    def __init__(self, connected_side, sticker_index):
        self.connected_side = connected_side
        self.sticker_index = sticker_index
        
    def __repr__(self):
        #return '%s%d' % (color_to_text(self.connected_side.color), self.sticker_index)
        return str(self.connected_side.color*10 + self.sticker_index)
        
    def get_unique(self):
        return self.connected_side.get_sticker_unique(self.sticker_index)
        

class CSide(object):

    is_debug_mode = False
    
    '''
      U U U
    L 0 1 2 R 0 1 2
    L 3 4 5 R 3 4 5
    L 6 7 8 R 6 7 8
      D D D
    '''
    
    '''
    0 1 2     6 3 0
    3 4 5  => 7 4 1 
    6 7 8     8 5 2

    2 5 8     0 1 2
    1 4 7  <= 3 4 5 
    0 3 6     6 7 8
    '''
    
    cube_size = 3
    
    def __init__(self, color):
        
        color += 1
        self.color = color
        array_size = CSide.cube_size * CSide.cube_size
        self.stickers = [color * 10 + i for i in xrange(array_size)]
        self.linked_stickers = [[] for x in xrange(array_size)]
        
    def __repr__(self):
        result = []
        
        for i in xrange(3):
            result.append(' '.join([color_to_text(self.stickers[i*CSide.cube_size + x])  for x in xrange(CSide.cube_size)]))
        return '\n'.join([rrr for rrr in result])
        
    def get_sticker_unique(self, sticker_index):
        return self.color * 10 + sticker_index
        
        
    def rotate(self, is_clockwise):
        if is_clockwise:
            new_array_pos = [6, 3, 0, 7, 4, 1, 8, 5, 2]
        else:
            new_array_pos = [2, 5, 8, 1, 4, 7, 0, 3, 6]

        prev_stickers = self.stickers
        inverse_new_pos = CSide.get_inverse_array(new_array_pos)
        self.stickers = []
        for pos in new_array_pos:
            self.stickers.append(prev_stickers[pos])
            
        old_stickers = {}
        for i in xrange(len(self.stickers)):
            prev_link = self.linked_stickers[i]
            new_link = self.linked_stickers[inverse_new_pos[i]]
            assert len(prev_link) == len(new_link)
            
            for j in xrange(len(prev_link)):
                plink = prev_link[len(prev_link)-1-j]
                nlink = new_link[j]
                
                old_sticker_value = nlink.connected_side.stickers[nlink.sticker_index]
                
                new_sticker_value = plink.connected_side.stickers[plink.sticker_index]
                if old_stickers.has_key(plink.get_unique()):
                    new_sticker_value = old_stickers[plink.get_unique()]
                
                old_stickers[nlink.get_unique()] = old_sticker_value
                
                nlink.connected_side.stickers[nlink.sticker_index] = new_sticker_value
            
    @staticmethod
    def get_inverse_array(original_array):
        result = [0] * len(original_array)
        for i in xrange(len(original_array)):
            result[original_array[i]] = i
        return result

class CCube(object):
    
    def __init__(self):
        self.sides = []
        for i in xrange(COLOR_COUNT):
            self.sides.append(CSide(i))

        self.init_sides_hash()
        #self.sides[self.sides_hash['B']].update_back_side()
        
        self.init_linked_stickers()
        
    def init_sides_hash(self):
        self.sides_hash = {}
        self.sides_hash['B'] = COLOR_BLUE
        self.sides_hash['F'] = COLOR_GREEN
        self.sides_hash['L'] = COLOR_ORANGE
        self.sides_hash['U'] = COLOR_WHITE
        self.sides_hash['R'] = COLOR_RED
        self.sides_hash['D'] = COLOR_YELLOW
        
    def init_linked_stickers(self):
    
        back_side = self.sides[self.sides_hash['B']]
        left_side = self.sides[self.sides_hash['L']]
        up_side = self.sides[self.sides_hash['U']]
        right_side = self.sides[self.sides_hash['R']]
        down_side = self.sides[self.sides_hash['D']]

        #'''
        self.link_up_down('F', 'D')
        self.link_up_down('U', 'F')
        for i in xrange(3):
            up_side.linked_stickers[0 + i].append(CLink(back_side, 2 - i))
            back_side.linked_stickers[2 - i].append(CLink(up_side, 0 + i))

            back_side.linked_stickers[8 - i].append(CLink(down_side, 6 + i))
            down_side.linked_stickers[6 + i].append(CLink(back_side, 8 - i))
        #'''
        
        #'''
        self.link_left_right('L', 'F')
        self.link_left_right('F', 'R')
        self.link_left_right('R', 'B')
        self.link_left_right('B', 'L')
        #'''

        #'''
        for i in xrange(3):
            left_side.linked_stickers[0 + i].append(CLink(up_side, 0 + i*3))
            up_side.linked_stickers[0 + i*3].append(CLink(left_side, 0 + i))

            up_side.linked_stickers[2 + i*3].append(CLink(right_side, 2 -  i))
            right_side.linked_stickers[2 - i].append(CLink(up_side, 2 + i*3))

            right_side.linked_stickers[6 + i].append(CLink(down_side, 2 + i*3))
            down_side.linked_stickers[2 + i*3].append(CLink(right_side, 6 + i))

            down_side.linked_stickers[0 + i*3].append(CLink(left_side, 8 - i))
            left_side.linked_stickers[8 - i].append(CLink(down_side, 0 + i*3))
        #'''
        
        self.verify_links()
        
    def verify_links(self):
        '''
        veirfy_dict = {}
        for j in xrange(6):
            cside = self.sides[j]
            for i in xrange(len(cside.linked_stickers)):
                linked = cside.linked_stickers[i]
                veirfy_dict[cside.get_sticker_unique(i)] = []
                for ll in linked:
                    veirfy_dict[cside.get_sticker_unique(i)].append(ll.get_unique())
        
        for k in veirfy_dict.keys():
            for l in veirfy_dict[k]:
                if not k in veirfy_dict[l]:
                    print '%s:%s' % (k, veirfy_dict[k])
                    print '%s:%s' % (l, veirfy_dict[l])
                    print ''
        '''
        
    def link_up_down(self, up, down):
        
        up_side = self.sides[self.sides_hash[up]]
        down_side = self.sides[self.sides_hash[down]]

        for i in xrange(CSide.cube_size):
            down_side.linked_stickers[0 + i].append(CLink(up_side, 6 + i))
            up_side.linked_stickers[6 + i].append(CLink(down_side, 0 + i))


    def link_left_right(self, left, right):

        left_side = self.sides[self.sides_hash[left]]
        right_side = self.sides[self.sides_hash[right]]

        for i in xrange(CSide.cube_size):
            left_side.linked_stickers[2 + i*3].append(CLink(right_side, 0 + i*3))
            right_side.linked_stickers[0 + i*3].append(CLink(left_side, 2 + i*3))

    def rotate(self, action, do_print = False):
            vals = action.strip().split(' ')
            for action in vals:
                repeat_me = 1
                if action[-1] == '2':
                    action = action[:-1]
                    repeat_me = 2

                for i in xrange(repeat_me):
                    self.rotate_single(action)

            if do_print:
                print 'Rotation: %s' % ' '.join(vals)
                print ''

    def rotate_single(self, action):
        
        is_clockwise = len(action) == 1
        action = action[0]
        
        self.sides[self.sides_hash[action]].rotate(is_clockwise)
        
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
    
    def is_cross_solved(self):
        
        are_all_same = True
        good_cross = {'U': [1, 3, 4, 5, 7], 'L' : [1, 4], 'R' : [1, 4], 'F': [1, 4], 'B': [1, 4]}
        for k in good_cross:
            cross_indices = good_cross[k]
            cross_side = self.sides[self.sides_hash[k]]
            for i in cross_indices:
                if cross_side.stickers[i] / 10 != cross_side.color:
                    are_all_same = False
        return are_all_same
        
        
    def solve_cross(self, max_depth = 6, do_show_time = False):
        
        all_solutions = []
        search_nodes = 0
        
        all_actions = []
        for a in ['R', 'L', 'U', 'D', 'F', 'B']:
            all_actions.extend([[a], [a + "'"], [a + "2"]])
            
        queue = []
        queue.extend(all_actions)
        
        start = time.time()
        prev_rotations = []
        
        max_depth_reached = 0
        while len(queue) > 0:
            
            action = queue.pop(0)
            self.rotate_between_rotations(prev_rotations, action)
            prev_rotations = action
            search_nodes += 1
            
            max_depth_reached = max(max_depth_reached, len(action))

            if self.is_cross_solved():
                max_depth = min(len(action) + 1, max_depth)
                all_solutions.append(' '.join(action))
                continue
            
            if len(action) < max_depth:
                for a in all_actions:
                    
                    # Skip same operation
                    if action[-1][0] == a[0][0]:
                        continue
                    
                    z = action[:]
                    z.extend(a)
                    queue.append(z)

        end = time.time()
        
        if do_show_time:
            print max_depth_reached
            print 'Searched %s variations for %.1f seconds' % (search_nodes, end - start)

        return all_solutions
        
    def undo_rotation(self, rotation):
        anti_rotation = (rotation + "'").replace("''", "").replace("2'", "2")
        #print '    %s/%s' % (rotation, anti_rotation)
        self.rotate(anti_rotation)
    
    def rotate_between_rotations(self, rotation1, rotation2):
        
        # Gets current ""rotation1"" and peforms necessary rotations to get to ""rotation2"
        anti_count, for_count = self.diff_between_rotations(rotation1, rotation2)
        #print '%s -> %s | %d, %d, %s, %s' % (rotation1, rotation2, anti_count, for_count, rotation1[anti_count:], rotation2[-for_count:])
        
        for i in xrange(anti_count):
            #print '   rotation1[-%d] = %s' % (i, rotation1[-i-1])
            self.undo_rotation(rotation1[-i - 1])
            
        self.rotate(' '.join(rotation2[-for_count:]))
    
    def diff_between_rotations(self, rotation1, rotation2):
        
        # rotation1 is current rotation
        # will return how much anti-rotation is required for rotation2
        
        len_rotation1 = len(rotation1)
        len_rotation2 = len(rotation2)
        lll = min(len_rotation1, len_rotation2)
        
        result = 0
        for i in xrange(lll):
            if rotation1[i] != rotation2[i]:
                break

            result += 1
        
        # How much does "rotation1" has to go back to allow second rotation
        # How much "rotations2" needs to perform
        return len_rotation1 - result, len_rotation2 - result
        
    def print_cross(self):
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

        print '\n'.join(result)
        print ''
        
    
    def print_all_links(self, side_id = -1):
        for j in xrange(6):
            if side_id != -1 and side_id != j:
                continue
            print j
            cside = cube.sides[j]
            for i in xrange(len(cside.linked_stickers)):
                linked = cside.linked_stickers[i]
                print '%s: %s' % (color_to_text(cside.stickers[i]), linked)
            print ''


cube = CCube()
cube.rotate("F2 B' L2 U2 B' D2 B2 R2 U2 B2 D2 R' U F2 R F' L' U' R F' B2", do_print = True)
print cube
print ''

print 'cross solution: %s' % cube.solve_cross(do_show_time = True)

'''
Example:

cube = CCube()
cube.rotate("F2 B' L2 U2 B' D2 B2 R2 U2 B2 D2 R' U F2 R F' L' U' R F' B2", do_print = True)
print cube
print 'cross solution: %s' % cube.solve_cross(do_show_time = True)


Result:
Rotation: F2 B' L2 U2 B' D2 B2 R2 U2 B2 D2 R' U F2 R F' L' U' R F' B2

        O W R
        W W Y
        B R G

B O R   Y Y R   W G B   W B W
B O B   O G W   R R G   W B R
Y O G   Y Y W   G G R   Y Y O

        O O O
        G Y R
        B B G

Searched 65073 variations for 8.1 seconds
cross solution: ["R D' F2", "R U' R2 U", "R F D' F2", "R F' D' F2", "R F2 D' F2", "U' R' U R2", "D R D' F2", "D' R D' F2", "D2 R D' F2"]
'''
