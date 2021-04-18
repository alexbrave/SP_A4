#
#   FILE      	 : makefile
#   PROJECT       : SENG2030-21W-Sec1-System Programming - Assignment #4
#   PROGRAMMER    : Andrey Takhtamirov, Alex Braverman
#   FIRST VERSION : March 25, 2020 
#   DESCRIPTION   : 
# 		This make file will create all the relevant objects that tcip-server
#		is dependent on.


# FINAL BINARY Target
TARGET_DEPENDENCIES := ./obj/chatServer.o ./obj/clientHandlerThread.o
TARGET_DEPENDENCIES += ./obj/clientManagement.o ./obj/messageParsing.o

# Link everything
./bin/chatServer : $(TARGET_DEPENDENCIES)
	cc -g $(TARGET_DEPENDENCIES) -o ./bin/chatServer -lpthread

#
# =======================================================
#                     Dependencies
# =======================================================                     
./obj/chatServer.o : ./src/chatServer.c ./inc/chatServer.h
	cc -c -g ./src/chatServer.c -o ./obj/chatServer.o

./obj/clientHandlerThread.o : ./src/clientHandlerThread.c ./inc/chatServer.h
	cc -c -g ./src/clientHandlerThread.c -o ./obj/clientHandlerThread.o 

./obj/clientManagement.o : ./src/clientManagement.c ./inc/chatServer.h
	cc -c -g ./src/clientManagement.c -o ./obj/clientManagement.o 

./obj/messageParsing.o : ./src/messageParsing.c ./inc/chatServer.h
	cc -c -g ./src/messageParsing.c -o ./obj/messageParsing.o 


#
# =======================================================
# Other targets
# =======================================================                     
clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
