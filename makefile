#************************************************************************************************************
#
# Repast HPC Tutorial Makefile
#
#************************************************************************************************************

include ./env

.PHONY: all
all : 


.PHONY: clean_output_files
clean_output_files:
	rm -f *.csv
	rm -f *.txt
	rm -f ./output/*.csv
	rm -f ./output/*.txt
	rm -f ./logs/*.*

.PHONY: clean_compiled_files
clean_compiled_files:
	rm -f *.exe
	rm -f ./bin/*.exe
	rm -f *.o
	rm -f ./object/*.o
	
.PHONY: clean
clean: clean_compiled_files clean_output_files remove_subdirectories
	rm -f *.cpp
	rm -f ./src/*.cpp
	rm -f *.props
	rm -f ./props/*.props
	rm -f ./include/*.h

.PHONY: CommuterModel
CommuterModel: clean_compiled_files
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Main.cpp -o ./objects/Main.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/CommuterModel.cpp -o ./objects/CommuterModel.o
	$(MPICXX) $(REPAST_HPC_DEFINES) $(BOOST_INCLUDE) $(REPAST_HPC_INCLUDE) -I./include -c ./src/Commuter.cpp -o ./objects/Commuter.o
	$(MPICXX) $(BOOST_LIB_DIR) $(REPAST_HPC_LIB_DIR) -o ./bin/CommuterModel.exe  ./objects/Main.o ./objects/CommuterModel.o ./objects/Commuter.o $(REPAST_HPC_LIB) $(BOOST_LIBS)



