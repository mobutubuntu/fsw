// system_clock::to_time_t
// Goal of this program: read the current time in DD/MM/YY HH:MM:SS:milliseconds to a file.
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
// #include <boost/thread.hpp>
#include <thread>

// int main ()
// {
//   using namespace std::chrono;

//   system_clock::time_point today = system_clock::now();

//   time_t tt;

//   tt = system_clock::to_time_t ( today );
//   std::cout << ctime(&tt);

//   return 0;
// }


int f()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
 
int main()
{
    auto t1 = std::chrono::high_resolution_clock::now();
    f();
    auto t2 = std::chrono::high_resolution_clock::now();
 
    // floating-point duration: no duration_cast needed
    std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
 
    // integral duration: requires duration_cast
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
 
    // converting integral duration to integral duration of shorter divisible time unit:
    // no duration_cast needed
    std::chrono::duration<long, std::micro> int_usec = int_ms;
 
    std::cout << "f() took " << fp_ms.count() << " ms, "
              << "or " << int_ms.count() << " whole milliseconds "
              << "(which is " << int_usec.count() << " whole microseconds)" << std::endl;
    return 0;
}



/*

Here are all the things the code needs to do:

*   Start at 0 microseconds (but don't print 0 microseconds)
*   Print the current microseconds (relative to 0 microseconds) after each line capture.
*   Print one linebreak between the start time and end time of each datacube.

*/