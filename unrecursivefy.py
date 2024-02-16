file = open("testRecursive.txt","w+") # opening the text file
reflect = "reflect" #predefining variables that are commonly used
refract = "refract"
base = "base"
prefixTree = {} #tree to hold prefixes

def writeCode (h, idx):
    file.write("\n")
    file.write(f"///////////////////////////////////////////// Node {idx} /////////////////////////////////////////////\n")
    file.write("\n")
    currPrefix = prefixTree[idx] #set current prefix
    if idx > 2**(h+1)-2: #check if it is a leaf node
        file.write(f"{currPrefix}Color = background;\n") #set value to background since it is leaf level
        return
    pt = (f"{currPrefix}Point") #setting values for this node
    N = (f"{currPrefix}N")
    mat = (f"{currPrefix}Material")
    file.write(f"Vec3f {pt}, {N};\n") #initializing values for this node
    file.write(f"Material {mat};\n")
    if idx == 0: #check if root node
        file.write(f"if (!scene_intersect(orig, dir, spheres, {pt}, {N}, {mat})) {{\n")
        file.write("    return background;\n")
    else: #for nodes other than root
        file.write(f"if (!scene_intersect({currPrefix}orig, {currPrefix}dir, spheres, {pt}, {N}, {mat})) {{\n")
        file.write(f"    {currPrefix}Color = background;\n")
    file.write("}\n")
    leftChild = idx*2 + 1 #get indexes for the current node's children
    rightChild = idx*2 + 2
    if idx == 0: #check if root node
        file.write(f"Vec3f {prefixTree[leftChild]}dir = reflect(dir, {N}).normalize();\n")
        file.write(f"Vec3f {prefixTree[leftChild]}orig = {prefixTree[leftChild]}dir * {N} < 0 ? {pt} - {N}*1e-3 : {pt} + {N}*1e-3;\n")
    elif leftChild <= (2**(h+1)) - 2: #only use if it is not a leaf node ( -2 because 0 indexed and there are 2^h - 1 nodes)
        file.write(f"Vec3f {prefixTree[leftChild]}dir = reflect({currPrefix}dir, {N}).normalize();\n")
        file.write(f"Vec3f {prefixTree[leftChild]}orig = {prefixTree[leftChild]}dir * {N} < 0 ? {pt} - {N}*1e-3 : {pt} + {N}*1e-3;\n")
    file.write(f"Vec3f {prefixTree[leftChild]}Color;\n") 
    writeCode(h, leftChild) #call the left child after the color is initialized
    file.write("\n")
    file.write(f"///////////////////////////////////////////// Node {idx} /////////////////////////////////////////////\n")
    file.write("\n")    
    if idx == 0: #come back from reflect and set up the refract
        file.write(f"Vec3f {prefixTree[rightChild]}dir = refract(dir, {N}, {mat}.refractive_index).normalize();\n")
        file.write(f"Vec3f {prefixTree[rightChild]}orig = {prefixTree[rightChild]}dir * {N} < 0 ? {pt} - {N}*1e-3 : {pt} + {N}*1e-3;\n")
    elif rightChild <= (2**(h+1))-2: #only use if it is not a leaf node ( -2 because 0 indexed and there are 2^h - 1 nodes)
        file.write(f"Vec3f {prefixTree[rightChild]}dir = refract({currPrefix}dir, {N}, {mat}.refractive_index).normalize();\n")
        file.write(f"Vec3f {prefixTree[rightChild]}orig = {prefixTree[rightChild]}dir * {N} < 0 ? {pt} - {N}*1e-3 : {pt} + {N}*1e-3;\n")
    file.write(f"Vec3f {prefixTree[rightChild]}Color;\n")
    writeCode(h, rightChild) #call the right child after the color is initialized
    file.write("\n")
    file.write(f"///////////////////////////////////////////// Node {idx} /////////////////////////////////////////////\n")
    file.write("\n")  
    if idx == 2**(h)-1: #this corresponds to the bottom leftmost node in preorder traversal and can only initialize variable once
        file.write("float diffuse_light_intensity = 0, specular_light_intensity = 0;\n")
    else:
        file.write("diffuse_light_intensity = 0, specular_light_intensity = 0;\n")
    file.write("for (size_t i = 0; i < lights.size(); i++) {\n") #perform lighting calculations
    file.write(f"   Vec3f light_dir = (lights[i].position - {pt}).normalize();\n")
    file.write(f"   float light_distance = (lights[i].position - {pt}).norm();\n")
    file.write("\n")
    file.write(f"   Vec3f shadow_orig = light_dir * {N} < 0 ? {pt} - {N} * 1e-3 : {pt} + {N} * 1e-3;\n")
    file.write("    Vec3f shadow_pt, shadow_N;\n")
    file.write("    Material tmpmaterial;\n")
    file.write("    if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)\n")
    file.write("        continue;\n")
    file.write("\n")
    file.write(f"   diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * {N});\n")
    if idx == 0: #check if root node or not
        file.write(f"   specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, {N}) * dir), {mat}.specular_exponent) * lights[i].intensity;\n")
    else:
        file.write(f"   specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, {N}) * {currPrefix}dir), {mat}.specular_exponent) * lights[i].intensity;\n")
    file.write("}\n")
    if idx == 0: #check if root node (this value returns from the overall function)
        file.write(f"return {mat}.diffuse_color * diffuse_light_intensity * {mat}.albedo[0] + \n")
    else:
        file.write(f"{currPrefix}Color = {mat}.diffuse_color * diffuse_light_intensity * {mat}.albedo[0] + \n")
    file.write(f"   Vec3f(1., 1., 1.) * specular_light_intensity * {mat}.albedo[1] + \n") #last lighting calculation
    file.write(f"   {prefixTree[leftChild]}Color * {mat}.albedo[2] + \n")
    file.write(f"   {prefixTree[rightChild]}Color * {mat}.albedo[3];\n")
    return

def writeTree(d):
    prefixTree[0] = base #set three values that are always present (never have a tree with depth < 0)
    prefixTree[1] = reflect
    prefixTree[2] = refract
    if (d == 0): #check to see if tree is depth 0 or if the function should continue
        return
    for i in range(3,2**(d+2)-1): #iterate from 3rd node to last node
        # go to 2^(h+2)-1 because there are 2^h - 1 nodes but since there is a base layer and 0th layer so need to add 2 to power
        prefixTree[i] = prefixTree[(i-1)//2] + refract if i % 2 == 0 else prefixTree[(i-1)//2] + reflect #append reflect or refract to prefix depending on evenness of index
    return

desiredDepth = 2 #set depth
writeTree(desiredDepth)
file.write("Vec3f background = Vec3f (.2,.7,.8);\n")
writeCode(desiredDepth, 0) 
file.close() #close file