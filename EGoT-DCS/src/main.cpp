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
#include<vector>


#include <SunSpecModbus.h>
#include <SunSpecModel.h>
#include <sep/der_control.hpp>
#include <sep/event.hpp>
#include <sep/event_status.hpp>
#include <sep/randomizable_event.hpp>
#include <sep/der_control_base.hpp>
#include <sep/device_category_type.hpp>
#include <sep/date_time_interval.hpp>
#include <sep/time_type.hpp>
#include <sep/der_curve.hpp>
#include <sep/der_curve_type.hpp>
#include <sep/der_curve_data.hpp>



int main()

{

SunSpecModbus ssmb(1, 1850954613, "192.168.0.64", 502);


    auto now_p = std::chrono::system_clock::now(); //get the current time
    int time_min {2}; //The inverter is scheduled to absorb power after 2 minutes based on currrent time
    int time1_min {10}; // The inverter is scheduled to output power after 15 minutes based on currrent time
    auto end = now_p + std::chrono::minutes(time_min);              //get the scheduled two minutes for charging battery based on current time
    auto end_1 = now_p + std::chrono::minutes(time1_min);           // get the scheduled five minutes for discharging battery
    time_t nowt =  std::chrono::system_clock::to_time_t ( now_p );  //get the readable current time for human
    time_t endt =  std::chrono::system_clock::to_time_t ( end);     //get the readable scheduled time for human
    time_t endt_1 = std::chrono::system_clock::to_time_t ( end_1);  //get the readable scheduled time for human


//event status (creat event status object)
sep::EventStatus even_sta;
even_sta.current_status = sep::CurrentStatus::kActive;
even_sta.date_time = nowt;
even_sta.potentially_superseded = false;
even_sta.potentially_superseded_time = 0;
even_sta.reason = "none";

//event -> DateTimeInterval + event status
//create event object and link to other relevant IEEE 2030.5 attributes include operation interval of the inverter for both charging and discharging

sep::Event event;   //Absorb power 
event.creation_time = nowt;     
event.event_status = even_sta;

event.interval.duration = 300;      
event.interval.start = endt;

sep::Event event_1; //Eject power
event_1.creation_time = nowt;
event_1.event_status = even_sta;

event_1.interval.duration = 300;
event_1.interval.start = end_1;

//randomize_event -> event
sep::RandomizableEvent rand_event; // charging
rand_event.event = event;
rand_event.randomizable_duration = 30;      //The duration of charging event will randomly start within 30 seconds. int16_t range (-3600 to 3600)
rand_event.randomizable_start = 30;         // The start time of charging event will randomly start within 30 seconds.

sep::RandomizableEvent rand_event_1; // discharging
rand_event_1.event = event_1;
rand_event_1.randomizable_duration = 30;    //The duration of discharging event will randomly start within 30 seconds.
rand_event_1.randomizable_start = 30;       // The start time of discharging event will randomly start within 30 seconds.


//DERControl Base
//SingedPercent = uint16_t; -10000 to 10000 (10000=100%)
sep::DERControlBase derc_base; //charging
sep::DERControlBase derc_base1; //discharging



//DERControl + DERControl Base
sep::DERControl derc;
derc.randomize_event = rand_event;
derc.der_control_base = derc_base;
derc.device_category = sep::DeviceCategoryType::kOtherStorageSystem;

sep::DERControl derc_1;
derc_1.randomize_event = rand_event_1;
derc_1.der_control_base = derc_base1;
derc_1.device_category = sep::DeviceCategoryType::kOtherStorageSystem;


//Using map function to link IEEE 2030.5 to SunSpec (multiple points are created since each SunSpec register need to be write individually)
std::map <std::string, std::string> point, point_1, point_2, point_3, point_4, point1, point2, point3, point4, point5, point6, point7, point8, point9, point10, point11, point12, point13, 
point14, point15, point16, point17, point18, point19, point20,point21,point22;

//Charging mode (The following are trying to determine when the inverter need to be energized and absorb specific power)
    if(derc.device_category == sep::DeviceCategoryType::kOtherStorageSystem)
        if(event.creation_time == event.interval.start)
            if(event.event_status.current_status == sep::CurrentStatus::kActive)

            derc_base.op_mod_connect = 1;       //Enumrated value: 0-> DISCONNECT 1-> CONNECT
            derc_base.op_mod_fixed_w =80;       //(W_SF is 1 -> 80 is 8000 means 80% of maximum power)
            derc_base.op_mod_volt_var;          //(goes into op_mod_volt_var attribute)
            derc_base.op_mod_freq_watt;          //(goes into op_mod_freq_watt attribute)

            point21["IMMED_INVERTER_CONTROLS_Conn"] = std::to_string (derc_base.op_mod_connect); //Energize inverter (write value into this SunSpec register will cause a segmentation fault)
            ssmb.WritePoint(123, point21);
            point18["IMMED_INVERTER_CONTROLS_WMaxLimPct"] = std::to_string (derc_base.op_mod_fixed_w); 
            ssmb.WritePoint(123, point18);      //block DID:123 Immediate Control Block
            point = ssmb.ReadBlock(123);        //Read the immediate block 123
            ssmb.PrintBlock(point);             // Print out all values in block 123

//Charging mode (The following are trying to determine when the inverter need to be energized and absorb specific power)
if(derc_1.device_category == sep::DeviceCategoryType::kOtherStorageSystem)
        if(event_1.creation_time == event.interval.start)
            if(event_1.event_status.current_status == sep::CurrentStatus::kActive)

            derc_base1.op_mod_connect = 1;       
            derc_base1.op_mod_fixed_w =-50;     // ("-" sign means discharge as 50% of maximum power)       
            derc_base1.op_mod_volt_var;         
            derc_base1.op_mod_freq_watt;          

            point22["IMMED_INVERTER_CONTROLS_Conn"] = std::to_string (derc_base.op_mod_connect); //Energize inverter (write value into this SunSpec register will cause a segmentation fault)
            ssmb.WritePoint(123, point22);
            point20["IMMED_INVERTER_CONTROLS_WMaxLimPct"] = std::to_string (derc_base.op_mod_fixed_w); 
            ssmb.WritePoint(123, point20);      
            point = ssmb.ReadBlock(123);       
            ssmb.PrintBlock(point);


    //-------------------------Set Volt-Var and Freq-Watt Curve Control------------------------

    //-------Static Volt-VAr Curve control -----

    sep::CurveData vv[20];
    //---First point of Volt_VAr Curve
    vv[0].x = 41;  // 41 means 41%*WMax (set the minimum voltage that inverter will export maximum capacitive power)
    vv[0].y = 46;  // The scale factor is 1 --> 460 Var                
    //--Dead-band of Volt-VAr curve
    vv[1].x = 44;  // lower limit of dead-band on volt-var curve
    vv[1].y = 0;   // Ideally, the export var in the dead-band should be 0
    vv[2].x = 52;  // Upper limit of dead-band on volt-var curve
    vv[2].y = 0;   
    //--last point of Volt-Var Curve
    vv[3].x = 55;   // Set the maximum voltage that inverter will export maximum induction var
    vv[3].y = 46;   // The sign of this point is default to negative on the SunSpec register


//--Line IEEE2030.5 Volt-VAr points to SunSpec registers
    point1["V1"] =std::to_string(vv[0].x);
    ssmb.WritePoint(126,point1);
    point2["VAr1"] =std::to_string(vv[0].y);
    ssmb.WritePoint(126,point2);
    point3["V2"] =std::to_string(vv[1].x);
    ssmb.WritePoint(126,point3);
    point4["VAr2"] =std::to_string(vv[1].y);
    ssmb.WritePoint(126,point4);
    point5["V3"] =std::to_string(vv[2].x);
    ssmb.WritePoint(126,point5);
    point6["VAr3"] =std::to_string(vv[2].y);
    ssmb.WritePoint(126,point6);
    point7["V4"] =std::to_string(vv[3].x);
    ssmb.WritePoint(126,point7);
    point8["VAr4"] =std::to_string(vv[3].y);
    ssmb.WritePoint(126,point8);

    point = ssmb.ReadBlock(126);
    ssmb.PrintBlock(point);



//Freq-Watt Curve Control -- Set points directly using IEEE 2030.5 attributes


    sep::CurveData fw[20];
    //---First point of Freq-Watt Curve
    fw[0].x = 57;  // The SunSoec standard specifies the scale factor of frequnecy-point is -2, so the frequency should be 5700. However, the sacle factor of frequnecy in the AXS port is +2, so the frequnecy point cannot be set in the SunSpec register in AXS port
    fw[0].y = 800;  // Set 80% of the maximum power that inverter will output when freq-wattt function is enabled                
    //--Dead-band of Freq-Watt curve
    fw[1].x = 58;  // lower limit of dead-band on freq-watt curve
                   // Inverter will increase export power as the grid frequency lower than this value
    fw[1].y = 0;   
    fw[2].x = 60;  // Upper limit of dead-band on freq-watt curve
                   // Inverter will reduce export power or consume additional power on the grid as the frequency higher than this value
    fw[2].y = 0;   
    //--last point of Freq-watt Curve
    fw[3].x = 62;   // Maximum frequency that enable inverter to consume maximum power
    fw[3].y = -800;   


//--Line IEEE2030.5 Volt-VAr points to SunSpec registers
    point9["V1"] =std::to_string(fw[0].x);
    ssmb.WritePoint(134,point9);
    point10["VAr1"] =std::to_string(fw[0].y);
    ssmb.WritePoint(134,point10);
    point11["V2"] =std::to_string(fw[1].x);
    ssmb.WritePoint(134,point11);
    point12["VAr2"] =std::to_string(fw[1].y);
    ssmb.WritePoint(134,point12);
    point13["V3"] =std::to_string(fw[2].x);
    ssmb.WritePoint(134,point13);
    point14["VAr3"] =std::to_string(fw[2].y);
    ssmb.WritePoint(134,point14);
    point15["V4"] =std::to_string(fw[3].x);
    ssmb.WritePoint(134,point15);
    point16["VAr4"] =std::to_string(fw[3].y);
    ssmb.WritePoint(134,point16);

    point = ssmb.ReadBlock(134);
    ssmb.PrintBlock(point);



    return 0;

}//end main ()
