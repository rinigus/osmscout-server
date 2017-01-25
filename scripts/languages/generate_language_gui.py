#!/usr/bin/env python3

# To run this code:
#
# 1. make a symbolic link from this directory to libpostal source
#  code. For example, ln -s ../../../libpostal
#
# 2. run this script from its directory,
#  i.e. ./generate_language_gui.py


import string
import langcodes
import os

# get list of all libpostal supported languages
LangsPostal = []
for lng in os.listdir("libpostal/resources/dictionaries"):
    if lng != "all" and lng.find("README") < 0:
        LangsPostal.append(lng)

LangsPostal.sort()

CodeGuiSwitches = ""
CodeInit = ""
CodeApply = ""

def swid(lang): return "input_" + lang

for lng in LangsPostal:
    lcode = langcodes.get(lng)
    autonym = lcode.autonym()
    name = lcode.language_name()

    if autonym == lng:
        print("Skipping since we don't know much about it: " + lng)
        continue
    
    print(lcode.language,  autonym, name)

    CodeGuiSwitches += """
            TextSwitch {
                id: %s
                width: parent.width
                text: "%s"
            }
""" % (swid(lng), autonym + " (" + lng + ", " + name + ")")

    CodeInit += """            if (l === "%s") %s.checked = true
""" % (lng, swid(lng))

    CodeApply += """        if (%s.checked) add("%s")
""" % (swid(lng), lng)

base_qml = open("LanguageSelectorBase.qml", "r").read()
base_qml = base_qml.replace('// language selection switches', "\n" + CodeGuiSwitches)
base_qml = base_qml.replace("// language apply selection", "\n" + CodeApply)
base_qml = base_qml.replace("// language initialize", "\n" + CodeInit)

fcode = open("LanguageSelector.qml", "w")
fcode.write(base_qml)
