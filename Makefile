#----------- DECLARATION ---------------------------
CC	= g++
RM	= rm -f
Flag_1	= -w -g -o3
Flag_2	= -w -g 
Flag_3	= -w -o
Version	= -std=c++11 -pthread
OBJDIR	= obj
BINDIR	= bin
SRCDIR	= src
SRC	:= $(wildcard $(SRCDIR)/*.cpp)
INC	:= $(wildcard $(SRCDIR)/*.h)
OBJ	:= $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
EXE	:= research
OS      := $(shell uname)

#------------ Prerequisite ------------------------
#------------ COMPILE -------------------------------
all: info Do

Do: $(OBJ)
	@$(CC) $(Version) $(Flag_3) $(EXE) $^
	@echo "Finish Linking"
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INC)
	@$(CC) $(Version) -c $< -o $@
	@echo "Compiled "$<" successfully!"
info:
	@printf "\033[32m-----------------------------------\n"
	@printf "\033[32m--Project Name: HTH               -\n"
  ifeq ($(OS),Darwin)
	@printf "\033[32m--Your OS: Mac OS X (Darwin)      -\n"
	@cp ./MINISATS/minisat_osx ./minisat
  else
	@printf "\033[32m--Your OS: Linux                  -\n"
	@cp ./MINISATS/minisat_linux ./minisat
  endif
	@printf "\033[32m-----------------------------------\n"
#------------- CMD ----------------------------------
.PHONY:clean
clean:
	$(RM) research $(OBJDIR)/*.o
.PHONY:clean
clean_file:
	$(RM) *.rpt *.cp *.vg
