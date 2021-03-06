/*
 * Copyright (C) 2019 IIT-HHCM
 * Author: Luca Muratore
 * email:  luca.muratore@iit.it
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef __ROSEE_UNIVERSAL_ROS_END_EFFECTOR_EXECUTOR_
#define __ROSEE_UNIVERSAL_ROS_END_EFFECTOR_EXECUTOR_

#include <memory>
#include <string>

#include <ros/console.h>

#include <sensor_msgs/JointState.h>

#include <ROSEndEffector/Parser.h>
#include <ROSEndEffector/EEInterface.h>
#include <ROSEndEffector/EEHal.h>
#include <ROSEndEffector/DummyHal.h>
#include <ROSEndEffector/Utils.h>

#include <ros_end_effector/EEGraspControl.h>
#include <ros_end_effector/EEPinchControl.h>

//to find relative path for the config files
#include <boost/filesystem/path.hpp>


namespace ROSEE
{

/**
 * @brief Class representing the ROS executor implementing the unviversal ROS EE concept
 *
 */
class UniversalRosEndEffectorExecutor
{

public:

    typedef std::shared_ptr<UniversalRosEndEffectorExecutor> Ptr;
    typedef std::shared_ptr<const UniversalRosEndEffectorExecutor> ConstPtr;

    UniversalRosEndEffectorExecutor ( std::string ns = "" );
    //EEInterface ( const EEInterface& other );
    //EEInterface& operator= ( const EEInterface& p );
    virtual ~UniversalRosEndEffectorExecutor();

    void spin();

    void timer_callback ( const ros::TimerEvent& timer_ev );

    void graspCallback ( const ros_end_effector::EEGraspControlConstPtr& msg );
    
    void pinchCallback ( const ros_end_effector::EEPinchControlConstPtr& msg );

private:

    void fill_publish_joint_states();
    
    void set_references();

    bool init_primitive_subscribers();
    
    void move_joint_in_finger(double upper_limit, double lower_limit, int id);
    
    ros::NodeHandle _nh;
    ros::Timer _loop_timer;

    double _time, _period, _rate;

    ROSEE::EEInterface::Ptr _ee;

    ros::Publisher _joint_state_pub;
    sensor_msgs::JointState _js_msg;

    int _joint_num = 0;
    int _seq_id = 0;

    ROSEE::EEHal::Ptr _hal;

    std::vector<std::string> _all_joints;
    std::vector<std::string> _joints;

    ros_end_effector::EEGraspControl _ctrl_msg;
    ros::Subscriber _sub_grasp, _sub_pinch;

    Eigen::VectorXd _qref, _qref_filtered;
    
    ROSEE::Utils::SecondOrderFilter<Eigen::VectorXd> _filt_q;

};

}


#endif //__ROSEE_UNIVERSAL_ROS_END_EFFECTOR_EXECUTOR_
