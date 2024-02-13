file = open("testRecursive.txt","w+") # opening the text file
reflect = "reflect" #predefining variables that are commonly used
refract = "refract"
base = "base"
layerCount = 0 #used for stopping recursion
prefixTree = {}

def writeCode (h, idx):
    file.write("\n")
    file.write(f"///////////////////////////////////////////// Node {idx} /////////////////////////////////////////////\n")
    file.write("\n")
    currPrefix = prefixTree[idx]
    if idx > 2**(h+1)-2:

        file.write(f"{currPrefix}Color = background;\n")
        return
    pt = (f"{currPrefix}Point")
    N = (f"{currPrefix}N")
    mat = (f"{currPrefix}Material")
    file.write(f"Vec3f {pt}, {N};\n")
    file.write(f"Material {mat};\n")
    if idx == 0:
        file.write(f"if (!scene_intersect(orig, dir, spheres, {pt}, {N}, {mat})) {{\n")
        file.write("    return background;\n")
    else:
        file.write(f"if (!scene_intersect({currPrefix}orig, {currPrefix}dir, spheres, {pt}, {N}, {mat})) {{\n")
        file.write(f"    {currPrefix}Color = background;\n")
    file.write("}\n")
    leftChild = idx*2 + 1
    rightChild = idx*2 + 2
    if idx == 0:
        file.write(f"Vec3f {prefixTree[leftChild]}dir = reflect(dir, {N}).normalize();\n")
        file.write(f"Vec3f {prefixTree[leftChild]}orig = {prefixTree[leftChild]}dir * {N} < 0 ? {pt} - {N}*1e-3 : {pt} + {N}*1e-3;\n")
    elif leftChild < 2**(h+1) - 2:
        file.write(f"Vec3f {prefixTree[leftChild]}dir = reflect({currPrefix}dir, {N}).normalize();\n")
        file.write(f"Vec3f {prefixTree[leftChild]}orig = {prefixTree[leftChild]}dir * {N} < 0 ? {pt} - {N}*1e-3 : {pt} + {N}*1e-3;\n")
    file.write(f"Vec3f {prefixTree[leftChild]}Color;\n")
    writeCode(h, leftChild)
    file.write("\n")
    file.write(f"///////////////////////////////////////////// Node {idx} /////////////////////////////////////////////\n")
    file.write("\n")    
    if idx == 0:
        file.write(f"Vec3f {prefixTree[rightChild]}dir = refract(dir, {N}, {mat}.refractive_index).normalize();\n")
        file.write(f"Vec3f {prefixTree[rightChild]}orig = {prefixTree[rightChild]}dir * {N} < 0 ? {pt} - {N}*1e-3 : {pt} + {N}*1e-3;\n")
    elif rightChild < 2**(h+1)-2:
        file.write(f"Vec3f {prefixTree[rightChild]}dir = refract({currPrefix}dir, {N}, {mat}.refractive_index).normalize();\n")
        file.write(f"Vec3f {prefixTree[rightChild]}orig = {prefixTree[rightChild]}dir * {N} < 0 ? {pt} - {N}*1e-3 : {pt} + {N}*1e-3;\n")
    file.write(f"Vec3f {prefixTree[rightChild]}Color;\n")
    writeCode(h, rightChild)
    file.write("\n")
    file.write(f"///////////////////////////////////////////// Node {idx} /////////////////////////////////////////////\n")
    file.write("\n")  
    if idx == 2**(h)-1:
        file.write("float diffuse_light_intensity = 0, specular_light_intensity = 0;\n")
    else:
        file.write("diffuse_light_intensity = 0, specular_light_intensity = 0;\n")
    file.write("for (size_t i = 0; i < lights.size(); i++) {\n")
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
    if idx == 0:
        file.write(f"   specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, {N}) * dir), {mat}.specular_exponent) * lights[i].intensity;\n") #check the direction
    else:
        file.write(f"   specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, {N}) * {currPrefix}dir), {mat}.specular_exponent) * lights[i].intensity;\n") #check the direction
    file.write("}\n")
    if idx == 0:
        file.write(f"return {mat}.diffuse_color * diffuse_light_intensity * {mat}.albedo[0] + \n")
    else:
        file.write(f"{currPrefix}Color = {mat}.diffuse_color * diffuse_light_intensity * {mat}.albedo[0] + \n")
    file.write(f"   Vec3f(1., 1., 1.) * specular_light_intensity * {mat}.albedo[1] + \n")
    file.write(f"   {prefixTree[leftChild]}Color * {mat}.albedo[2] + \n")
    file.write(f"   {prefixTree[rightChild]}Color * {mat}.albedo[3];\n")
    return

def writeTree(d):
    prefixTree[0] = base #set three values that are always present (never have a tree with depth < 0)
    prefixTree[1] = reflect
    prefixTree[2] = refract
    if (d == 0): #check to see if tree is depth 0 or if the function should continue
        return
    for i in range(3,2**(d+2)-1):
        prefixTree[i] = prefixTree[(i-1)//2] + refract if i % 2 == 0 else prefixTree[(i-1)//2] + reflect
    return

desiredDepth = 1
writeTree(desiredDepth)
file.write("Vec3f background = Vec3f (.2,.7,.8);\n")
writeCode(desiredDepth, 0)
file.close()