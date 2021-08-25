// Test code for editor
#include "ECTextViewImp.h"
#include "ECKeywatcher.h"
#include "ECDocument.h"
#include <iostream>
#include <string>
#include <unistd.h>

using namespace  std;

int main(int argc, char *argv[])
{
    //
    ECTextViewImp wndTest;
    ECDocument docTest(argv[1]);
    ECKeywatcher keyTest(&wndTest, &docTest);
    wndTest.Show();
    
    return 0;
}
