#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <chrono>
#include <complex>
#include <math.h>
#include <time.h>
#include <thread>
#include <fstream>
//#include<conio.h>
//#include<windows.h>
#include<vector>


#include "SunSpecModbus.h"
#include <sep/der_control.hpp>
#include <sep/event.hpp>
#include <sep/event_status.hpp>
#include <sep/randomizable_event.hpp>
#include <sep/der_control_base.hpp>
#include <sep/device_category_type.hpp>
#include <sep/date_time_interval.hpp>
#include <sep/time_type.hpp>


SunSpecModbus ssmb(1, 1850954613, "192.168.0.64", 502);

    void write_csv(std::string filename, std::vector<std::pair<std::string, std::vector<std::string>>> dataset){
        // Make a CSV file with one or more columns of strings
        // Each column of data is represented by the pair <column name, column data>
        //  as std::pair<std::string, std::vector<std::string>>
        // The dataset is represented as a vector of these columns
        // Note that all columns should be the same size

        // Create an output filestream object
        std::ofstream myFile(filename);

        // Send column names to the stream
        for(int j = 0; j < dataset.size(); ++j)
        {
        myFile << dataset.at(j).first;
        if(j != dataset.size() - 1) myFile << ","; // No comma at end of line
        }       
        myFile << "\n";

        // Send data to the stream
        for(int i = 0; i <dataset.at(0).second.size(); ++i)
        {
        for(int j = 0; j < dataset.size(); ++j)
        {
        myFile << dataset.at(j).second.at(i);
        if(j != dataset.size() -1 ) myFile << ","; // No comma at end of line
        }
        myFile << "\n";
        }
        // Close the file
        myFile.close();
        }


int main()

{


    auto now_p = std::chrono::system_clock::now();
    int time_min {2}; //The inverter is scheduled to absorb power after 2 minutes based on currrent time
    int time1_min {10}; // The inverter is scheduled to output power after 15 minutes based on currrent time
    auto end = now_p + std::chrono::minutes(time_min);
    auto end_1 = now_p + std::chrono::minutes(time1_min);
    time_t nowt =  std::chrono::system_clock::to_time_t ( now_p );
    time_t endt =  std::chrono::system_clock::to_time_t ( end);
    time_t endt_1 = std::chrono::system_clock::to_time_t ( end_1);


// DateTimeInterval
    sep::DateTimeInterval interval(300,endt);   //charging battery time 
   // interval.duration_ = 300;
   // interval.start_ = endt;

    sep::DateTimeInterval interval_1 = sep::DateTimeInterval(300,endt_1);  //discharging
   // interval_1.duration_ = 300;
   // interval_1.start_ = endt_1;

//event status
    sep::CurrentStatus cur_status;
    sep::EventStatus even_sta(cur_status, nowt, false, 0, "none");
    even_sta.current_status_ = cur_status;
    cur_status = sep::CurrentStatus::ACTIVE;

//event -> DateTimeInterval +event status
    sep::Event event(nowt,&even_sta,&interval);      //charging
   // event.creation_time_ = nowt;
   // event.event_status_ -> even_sta; //Note that even_status is a pointer
   // event.interval_ -> interval;

    sep::Event event_1(nowt,&even_sta,&interval_1);   //discharging
   // event_1.creation_time_ = nowt;
   // event_1.event_status_ -> even_sta;
   // event_1.interval_ -> interval_1;

//randomize_event -> event     {Continue working here}             //charging
    sep::RandomizableEvent rand_event(&event,30,30);
   // rand_event.event_ = &event;
   // rand_event.randomize_duration_ = 30;
   // rand_event.randomize_start_ = 30;

    sep::RandomizableEvent rand_event_1(&event_1,40,30);    //discharging
   // rand_event_1.event_ = &event_1;
   // rand_event_1.randomize_duration_ = 40;
   // rand_event_1.randomize_start_ = 30;


//DERControl Base
    sep::DERControlBase derc_base;      //charging
    derc_base.op_mod_connect_ = true;
    derc_base.op_mod_energize_ = true;
    int charg {5000};
    sep::SignedPerCent y(charg);
    derc_base.op_mod_fixed_w_ = &y;  //SunSpec register WMaxLimPct_SF not specify with this attribute

    sep::DERControlBase derc_base1;         //discharging
    derc_base1.op_mod_connect_ = true;
    derc_base1.op_mod_energize_ = true;
    int discharg {-5000};
    sep::SignedPerCent x(discharg);
    derc_base1.op_mod_fixed_w_ = &x;

    sep::DeviceCategoryType a = (sep::DeviceCategoryType) 25;
//DERControl + DERControl BaseAA
sep::DERControl derc(&rand_event, &derc_base, a);//(sep::DeviceCategoryType) 25);       //charging
   // derc.randomize_event_ = &rand_event;
   // derc.der_control_base_ = &derc_base;
   // derc.device_category_ = sep::DeviceCategoryType::OTHER_STORAGE_SYSTEM;

sep::DERControl derc_1(&rand_event_1, &derc_base1, (sep::DeviceCategoryType) 25); //discharging
   // derc_1.randomize_event_ = &rand_event_1;
   // derc_1.der_control_base_ = &derc_base1;
   // derc_1.device_category_ = sep::DeviceCategoryType::OTHER_STORAGE_SYSTEM;



//Charging mode
    if(derc.device_category_ == sep::DeviceCategoryType::OTHER_STORAGE_SYSTEM)
        if (event.creation_time_ == interval.start_)
            if (event.event_status_ -> current_status_ == sep::CurrentStatus::ACTIVE)
                interval.duration_ = 300;
    rand_event.randomize_duration_ = 30;
    rand_event.randomize_start_ = -30;
    derc_base.op_mod_energize_ = true;
    derc_base.op_mod_fixed_w_ == &y;   //(charge)


//Discharging mode
    if(derc_1.device_category_ == sep::DeviceCategoryType::OTHER_STORAGE_SYSTEM)
        if (event_1.creation_time_ == interval_1.start_)
            if (event_1.event_status_ -> current_status_ == sep::CurrentStatus::ACTIVE)
                interval_1.duration_ = 300;
    rand_event_1.randomize_duration_ = 30;
    rand_event_1.randomize_start_ = -30;
    derc_base1.op_mod_energize_ == true;
    derc_base1.op_mod_fixed_w_ == &x;   //(discharge)


/*
    map <string, string> point;
    point["GSconfig_ReFloat_Volts"] = "50";
    ssmb.WritePoint(64116, point);
    point = ssmb.ReadBlock(64116);
*/

    std::map <std::string, std::string> point;
    point["Conn"] = std::to_string (derc_base.op_mod_energize_);
    ssmb.WritePoint(123, point);
    point = ssmb.ReadBlock(123);
    ssmb.PrintBlock(point);


  double result =derc_base.op_mod_fixed_w_ -> value_ /derc_base.op_mod_fixed_w_ -> max_percent_;

    point["WMaxLimPct"] = std::to_string (result); //set maximum input and output power as 50% of the capacity
    ssmb.WritePoint(123, point);                                    // The opmodfixedW and opmodmaxlimW are the same in SunSpec.
    point = ssmb.ReadBlock(123);
    ssmb.PrintBlock(point);


    //}//end void(der_control,ssmb)

//---------------------------------------------------------CSV file-------------------------------------------------------------------
    std::vector <std::string> timedata;
    std::vector <std::string> real_power;
    std::vector <std::string> energize_status;

    for(int i=0; i<=90; i++) //output number of needed time value counting from current time
    {
        time_t current = time(0);
        //std::cout << ctime(&current)<< std::endl;   //verify if the current time value can be print out
        timedata.push_back(ctime(&current));          //put time data into a vector
        energize_status.push_back (point ["Conn"]);
        real_power.push_back(point["WMaxLimPct"]);
        sleep(2000);   //set a time delay to print out next time value (sleep for 2 seconds)
    }



    std::vector<std::pair<std::string, std::vector<std::string>>> vals = {{"Time", timedata}, {"W", real_power},{"State", energize_status}};

    // Write the vector to CSV
    write_csv("example.csv", vals);

    return 0;

}//end main ()
