/*
   Copyright (C) Italian Institute of Technology

   Developer:
       Luca Muratore (2020-, luca.muratore@iit.it)
      

*/

#ifndef __HERI_2_EE_HAL_H__
#define __HERI_2_EE_HAL_H__

#include <iit/ecat/advr/ec_boards_iface.h>

#include <XBotCore-interfaces/XDomainCommunication.h>
//#include <XBotCore/HALBase.h>

#include <ros_end_effector/EEHal.h>

#include <yaml-cpp/yaml.h>

#include <queue>
#include <iostream>
#define ECAT_PTHREAD_STACK_SIZE (16*1024*1024) // 16MB

namespace ROSEE{
    
struct JointActuationInfo {
    std::string joint_name;
    unsigned short int board_id;
    unsigned short int finger_in_board_id;
    double motor_lower_limit;
    double motor_upper_limit;
    double joint_lower_limit;
    double joint_upper_limit;
    double joint_to_moto_slope; // motor_upper - motor_lower / joint_upper - joint_lower
    double moto_to_joint_slope; // joint_upper - joint_lower / motor_upper - motor_lower
};
    
class Heri2EEHal : public ROSEE::EEHal,
                   public iit::ecat::advr::Ec_Boards_ctrl 
{
public:
    
    Heri2EEHal ( const char * config_yaml, 
                 ROSEE::EEInterface::Ptr ee_interface );
    virtual ~Heri2EEHal();
     
    virtual bool init() override;
    virtual bool sense() override;
    virtual bool move() override;
    
    virtual bool getMotorPosition( std::string joint_name, double& motor_position ) override;
    virtual bool getMotorVelocity( std::string joint_name, double& motor_velocity ) override;
    virtual bool getMotorEffort( std::string joint_name, double& motor_effort ) override;
    
    //TODO override this from IMotor, I dont know why but it gives error if I put this as pure virtual in Imotor. For now it is in EEHAL
    virtual bool getMotorCurrent ( std::string motor_name, double& motor_current ) override;
    
    
    bool getJointPosition(std::string joint_name, double& joint_position) override;
    
    bool getPressure(std::string sensor_name, double& sensor_value) override;

    virtual bool setPositionReference( std::string joint_name, double joint_position_reference );
    
    // TODO override from a virtual of hal?
    bool jointToActuatorPosition(std::string joint_name, double joint_pos, double& actuator_pos);
    bool actuatorToJointPosition(std::string joint_name,  double actuator_pos, double& joint_pos);


protected :

    iit::ecat::stat_t  s_loop;::
    uint64_t start_time, tNow, tPre;
    

    
    std::map<std::string, std::tuple<unsigned short int, unsigned short int, unsigned short int>> sensorName_to_motorId;
   
private:
    
    void post_init();
    
    void base_init();
    int base_start();
    int base_stop();
    
    bool move_finger(int finger_id, 
                     int motor_in_finger_id, 
                     double pos_ref);

    iit::ecat::ec_timing_t timing;
    int _wkc = -1;
    
    /** map with key the joint name and with value joint and motor information*/
    std::map<std::string, JointActuationInfo> _joint_actuation_info;
    
    //TODO obsolete remove
    std::map<int, iit::ecat::advr::HeriHandESC*>  fingers;
    std::map<int, float> min_pos, max_pos, min_pos_2, max_pos_2;
    
};

}

#endif //__HERI_2_EE_HAL_H__
