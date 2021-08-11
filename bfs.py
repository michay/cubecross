
actions = [['R'], ['L'], ['U'], ['F']]

queue = []
queue.extend(actions)
max_depth = 6
print queue

while len(queue) > 0:
    
    p = queue.pop(0)
    print p
    
    if len(p) < max_depth:

        for a in actions:
            z = p[:]
            z.extend(a)
            queue.append(z)
       