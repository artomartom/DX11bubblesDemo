#pragma once
#include "Hello/Hello.hpp"

 

int s_main()
{
  ::Log<Console>::Write("hello");
     
 
    return 0;
};

int main() { return Invoke(s_main); };
int wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) { return Invoke(s_main); };



