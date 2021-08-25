output: ECEditorTest.o ECTextViewImp.o 
	g++ ECEditorTest.o ECTextViewImp.o -o editor
ECEditorTest.o: ECEditorTest.cpp
	g++ -c ECEditorTest.cpp
ECTextViewImp.o: ECTextViewImp.cpp ECTextViewImp.h
	g++ -c ECTextViewImp.cpp
clean: 
	rm *.o editor