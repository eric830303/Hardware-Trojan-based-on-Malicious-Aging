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
Chk_Sv	:= $(shell test -e "Vth_pv_Sv.txt" && echo "Y" || echo "N" )
Chk_mi	:= $(shell test -d "MINISATS" && echo "Y" || echo "N" )
Chk_bk	:= $(shell test -d "benchmark" && echo "Y" || echo "N" )
Chk_qy	:= $(shell test -d "quality" && echo "Y" || echo "N" )

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
	@printf "\033[32m Project Name: HTH               \n"

##----------- Check first file ----------------------------
  ifeq ($(Chk_Sv),N)
	@printf "\033[31m Check: Vth_pv_Sv.txt --> Not exist !\n"
  else
	@printf "\033[32m Check: Vth_pv_Sv.txt --> Exist !\n"
  endif
##----------- Check first file ----------------------------
  ifeq ($(Chk_mi),N)
	@printf "\033[31m Check: MINISATS(dir) --> Not exist !\n"
  else
	@printf "\033[32m Check: MINISATS(dir) --> Exist !\n"
  endif
##----------- Check first file ----------------------------
  ifeq ($(Chk_bk),N)
	@printf "\033[31m Check: benchmark(dir) --> Not exist !\n"
  else
	@printf "\033[32m Check: benchmark(dir) --> Exist !\n"
  endif
##----------- Check first file ----------------------------
  ifeq ($(Chk_qy),N)
	@printf "\033[31m Check: quality(dir) --> Not exist !\n"
  else
	@printf "\033[32m Check: quality(dir) --> Exist !\n"
  endif
##----------- Check OS ------------------------------------
  ifeq ($(OS),Darwin)
	@printf "\033[32m Your OS: Mac OS X (Darwin)      \n"
	@cp ./MINISATS/minisat_osx ./minisat
  else
	@printf "\033[32m Your OS: Linux                  \n"
	@cp ./MINISATS/minisat_linux ./minisat
  endif
	@printf "\033[32m-----------------------------------\n"
#------------- CMD ----------------------------------
.PHONY:clean
clean:
	$(RM) research $(OBJDIR)/*.o
	$(RM) CNF/*.cnf
.PHONY:clean
clean_file:
	$(RM) *.rpt *.cp *.vg
