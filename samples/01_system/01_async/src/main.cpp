/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : main.cpp
	Description		 : This sample shows how to write async tasks
	Comment          : Read more information about this sample on http://wolfsource.io/gpunotes/wolfengine/
*/

#include "pch.h"

//namespaces
using namespace std;
using namespace wolf::system;

WOLF_MAIN()
{
    //initialize logger, and log in to the output debug window of visual studio(just for windows) and Log folder inside running directory
    WOLF_INIT(L"01_async");

    //log to output file
    logger.write(L"Wolf started");

    //execute async task in standard c++
    w_task::execute_async([]()-> void
    {
        logger.write("async task 01 done");
    }, []()
    {
        logger.write("async task 02 done");
    });

    //define deferred task
    w_task::execute_deferred([]()-> void
    {
        logger.write("deferred task 01 started");
        //wait for 5 sec
        std::this_thread::sleep_for(5s);
        logger.write("deferred task 01 done");
    });
    //main thread will wait for only 1 sec and then continue
    auto _status = w_task::wait_for(std::chrono::seconds(1).count());
    //check status of deferred task
    switch (_status)
    {
    case std::future_status::ready:
        logger.write("deferred task is ready");
        break;
    case std::future_status::timeout:
        logger.write("deferred task timeout");
        break;
    case std::future_status::deferred:
        logger.write("deferred task deferred");
        //call it
        wolf::system::w_task::get();
        break;
    }

    //create another deferred task
    w_task::execute_deferred([]()-> void
    {
        logger.write("deferred task 02 started");
        //wait for 1 sec
        std::this_thread::sleep_for(1s);
        logger.write("deferred task 02 done");
    });
    //main thread will block until deferred task done
    w_task::wait();

    logger.write("all done");

    //output a message to the log file
    logger.write(L"shutting down Wolf");

    //release logger
    logger.release();

    return EXIT_SUCCESS;
}


