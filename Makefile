##
# @file This file is part of snakesGL.
#
# @section LICENSE
# MIT License
# 
# Copyright (c) 2018 Rajdeep Konwar, Luke Rohrer
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# @section DESCRIPTION
# Makefile.
##

CXX?=g++
CXXFLAGS=-std=c++11 -O3 -Wall
LDFLAGS=-lGL -lGLU -lGLEW -lglfw3 -L/usr/local/lib -lXi -lm -lpthread -pthread -ldl -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lglfw -lrt -lXrandr -lXinerama -lXxf86vm -lXext -lXcursor -lXrender -lXfixes -lX11 -lxcb -lXau -lXdmcp

OUT_DIR = ./obj
MKDIR_P = mkdir -p
${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}
SRC_DIR = ./src

OBJECTS=snakesGL.o Bezier.o SceneGraph.o Shader.o Window.o

snakesGL: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o snakesGL $(LDFLAGS)

snakesGL.o: snakesGL.cpp

Bezier.o: Bezier.cpp

SceneGraph.o: SceneGraph.cpp

Shader.o: Shader.cpp

Window.o: Window.cpp

.PHONY: clean
clean:
	rm -f *.o snakesGL
