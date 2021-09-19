import re, os
import sys, json


def readFile(path):
    with open(path, 'r') as f:
        data = f.readlines()
        return data


def fineKeyValue(line, key):
    if key in line:
        m = re.search(f'\"{key}\" \[[^\[]+\]', line)
        if m is not None:
            s = m.group().strip()
            m = re.search(r'\[.+\]', s).group()
            m = m.strip('[').strip(']').strip(" ")
            return m.strip("\"")
    return None


def convertCorrectPath(pbrtFilePath, path):
    ab = os.path.abspath(pbrtFilePath)
    dir = os.path.dirname(ab)
    return dir + "\\" + path


def parseScene(pbrtPath, data):
    startedWorldBegin = False
    materials = {}
    objects = []
    textures = {}
    currMaterials = None
    objcount = 0
    for line in data:
        if "WorldBegin" in line:
            startedWorldBegin = True
        if "WorldEnd" in line:
            startedWorldBegin = False
        name = line.split()[0].strip('"')
        if "Texture" in name:
            name = line.split()[1].strip('"')
            texturePath = convertCorrectPath(pbrtPath, fineKeyValue(line, "string filename").replace('/', '\\'))
            textures[name] = {
                    "texturePath": texturePath
                }

        if "MakeNamedMaterial" in name:
            splited = line.split(" ")
            matName = splited[1]
            matName = re.sub("\"", "", matName)

            useTexture = False
            textureName = ""
            rgb = [0.0, 0.0, 0.0]

            if "texture Kd" in line:
                useTexture = True
                textureName = fineKeyValue(line, "texture Kd")

            if "rgb Kd" in line:
                rgbs = fineKeyValue(line, "rgb Kd")
                rgb[0] = float(rgbs.split()[0])
                rgb[1] = float(rgbs.split()[1])
                rgb[2] = float(rgbs.split()[2])

            materials[matName] = {
                    "useTexture": useTexture,
                    "textureName": textureName,
                    "rgb": rgb
                }

        if "NamedMaterial" in name:
            currMaterials = line.split(" ")[1].strip('"')

        if "Shape" in line:
            meshName = fineKeyValue(line, "string filename")
            if meshName is not None:
                meshPath = convertCorrectPath(pbrtPath, meshName.replace('/', '\\'))
                meshName += str(objcount)
                objcount = objcount + 1
                objects.append({
                    "meshName": meshName,
                    "meshPath": meshPath,
                    "matName": currMaterials,
                })

    return {
        "textures": textures,
        "objects": objects,
        "materials": materials
    }

def convertToKonai3DScreen(doc):
    # make default screen
    Camera = {
        "Far": 1000.0,
        "Fov": 0.7851,
        "Near": 0.1,
        "Position": [0.0, 0.0, -13.0],
        "Ratio": 1.7777777910232544,
        "Target": [0.0, 0.0, 0.0],
        "Up": [0.0, 1.0, 0.0]
    }
    Meshes = []
    Textures = []
    Materials = []
    Options = {
        "EnvMap": "",
        "Height": 1080,
        "VSync": True,
        "Width": 1920,
        "Depth": 13,
    }

    objs = []

    for k in doc["textures"]:
        Textures.append(doc["textures"][k]["texturePath"])

    for k in doc["materials"]:
        mat = doc["materials"][k]
        tex = ''
        if (mat["textureName"] in doc["textures"]):
            tex = doc["textures"][mat["textureName"]]["texturePath"]

        Materials.append({
                        "DiffuseTexturePath": tex,
                        "EmissiveColor": [0.0, 0.0, 0.0],
                        "FuzzValue": 0.0,
                        "MaterialName": k,
                        "MaterialType": 0,
                        "RefractIndex": 1.5,
                        "UseTexture": mat["useTexture"],
                        "BaseColor": mat["rgb"],
                        "Roughness": 0.3,
                        "SpecularPower": 0.0,
                        "Metallic": 1.0,
        })

    for v in doc["objects"]:
        meshPath = v["meshPath"]
        Meshes.append(meshPath)
        objs.append({
            "MaterialName": v["matName"],
            "MeshPath": meshPath,
            "Name": v["meshName"],
            "SubMesh": 0,
            "WorldMatrix": [1.0, 0.0,
                            0.0, 0.0,
                            0.0, 1.0,
                            0.0, 0.0,
                            0.0, 0.0,
                            1.0, 0.0,
                            0.0, 0.0,
                            0.0,
                            1.0]
        })

    j = {
        "Camera": Camera,
        "Meshes": Meshes,
        "Options": Options,
        "RenderObjects": objs,
        "Textures": Textures,
        "Materials": Materials,
    }
    return j

pbrtFile = sys.argv[1]
data = readFile(pbrtFile)
doc = parseScene(pbrtFile, data)
j = convertToKonai3DScreen(doc)

with open('generated.json', 'w') as f:
    json.dump(j, f)