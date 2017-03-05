all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lao -lmpg123 -lm -lGL -lglfw -ldl

Debug := CFLAGS= -g

clean:
	rm sample2D
