#
#   FILE      	  : makefile
#   PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #3
#   PROGRAMMER    : Alex Braverman
#   FIRST VERSION : March 25, 2020 
#   DESCRIPTION   : 
# 		This make file will create all the relevant objects that encodeInput
#		is dependent on.
#


# FINAL BINARY Target
# Instead of writing out all these object names when linking them in encodeInput
# we use a convenient variable instead!
TARGET_DEPENDENCIES := ./obj/chatClient.o ./obj/ncursesFunctions.o
TARGET_DEPENDENCIES += ./obj/cursorSemFunctions.o ./obj/listeningThread.o
TARGET_DEPENDENCIES += ./obj/socketFunctions.o ./obj/sendingThread.o


# Link everything, optionally including the math library (add -lm after -o ./bin/DC)
./bin/chatClient : $(TARGET_DEPENDENCIES)
	cc -g $(TARGET_DEPENDENCIES) -lncurses -o ./bin/chatClient -lpthread
#
# =======================================================
#                     Dependencies
# =======================================================   
# There are no debugging symbols being compiled, to add them add "-g" after "cc -c" for 
# each of the following object file directives                  
./obj/chatClient.o : ./src/chatClient.c ./inc/chatClient.h
	cc -c -g ./src/chatClient.c -o ./obj/chatClient.o

# ../Common/obj/genRandSleep.o : ../Common/src/genRandSleep.c ../Common/inc/genRandSleep.h
# 	cc -c ../Common/src/genRandSleep.c -o ../Common/obj/genRandSleep.o

./obj/ncursesFunctions.o : ./src/ncursesFunctions.c ./inc/chatClient.h
	cc -c -g ./src/ncursesFunctions.c -o ./obj/ncursesFunctions.o 

./obj/cursorSemFunctions.o : ./src/cursorSemFunctions.c ./inc/chatClient.h
	cc -c -g ./src/cursorSemFunctions.c -o ./obj/cursorSemFunctions.o 

./obj/listeningThread.o : ./src/listeningThread.c ./inc/chatClient.h
	cc -c -g ./src/listeningThread.c -o ./obj/listeningThread.o 

./obj/socketFunctions.o : ./src/socketFunctions.c ./inc/chatClient.h
	cc -c -g ./src/socketFunctions.c -o ./obj/socketFunctions.o 

./obj/sendingThread.o : ./src/sendingThread.c ./inc/chatClient.h
	cc -c -g ./src/sendingThread.c -o ./obj/sendingThread.o 


#
# =======================================================
# Other targets
# =======================================================                     
all : ./bin/chatClient

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
	

