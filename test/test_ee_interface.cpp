#include <gtest/gtest.h>
#include "testUtils.h"

#include <ros/ros.h>
#include <ros/console.h>

#include <sensor_msgs/JointState.h>

#include <ROSEndEffector/Parser.h>
#include <ROSEndEffector/EEInterface.h>
#include <ROSEndEffector/Utils.h>

namespace {

class testEEInterface: public ::testing::Test {


protected:

    testEEInterface() {
    }

    virtual ~testEEInterface() {
    }

    virtual void SetUp() {

        ros::NodeHandle nh;

        ROSEE::Parser p ( nh );
        p.init (  ROSEE::Utils::getPackagePath() + "/configs/test_ee.yaml" );
        p.printEndEffectorFingerJointsMap();

        ee = std::make_shared<ROSEE::EEInterface>(p);
    }

    virtual void TearDown() {
    }

    ROSEE::EEInterface::Ptr ee;
};


TEST_F ( testEEInterface, checkFingers ) {

    std::vector<std::string> fingers;
    fingers = ee->getFingers();

    EXPECT_FALSE (fingers.empty());

    ROS_INFO_STREAM ( "Fingers in EEInterface: " );
    for ( auto& f : fingers ) {
        ROS_INFO_STREAM ( f );
    }

    EXPECT_TRUE ( ee->isFinger ( "finger_1" ) );

    EXPECT_FALSE ( ee->isFinger ( "finger_4" ) );



}

TEST_F ( testEEInterface, checkActuatedJointsNum ) {

    int joint_num = ee->getActuatedJointsNum();
    EXPECT_EQ ( 6, joint_num );

    EXPECT_FALSE ( joint_num < 0 );

}

TEST_F ( testEEInterface, checkEEFingerJoints ) {

    int joint_num_finger1 = ee->getActuatedJointsNumInFinger("finger_1");

    int joint_num = ee->getActuatedJointsNum();

    EXPECT_TRUE ( joint_num >= joint_num_finger1 );

    int joint_num_counter = 0;
    std::vector<std::string> fingers = ee->getFingers();



    for ( auto& f : fingers ) {
        joint_num_counter += ee->getActuatedJointsNumInFinger(f);
    }

    EXPECT_TRUE ( joint_num == joint_num_counter );

}



} //namespace

int main ( int argc, char **argv ) {

    if (argc < 2 ) {

        std::cout << "[TEST ERROR] Insert hand name as argument" << std::endl;
        return -1;
    }

    /* Run tests on an isolated roscore */
    if(setenv("ROS_MASTER_URI", "http://localhost:11322", 1) == -1)
    {
        perror("setenv");
        return 1;
    }

    //run roscore
    std::unique_ptr<ROSEE::TestUtils::Process> roscore;
    roscore.reset(new ROSEE::TestUtils::Process({"roscore", "-p", "11322"}));

    if ( ROSEE::TestUtils::prepareROSForTests ( argc, argv, "testEEInterface" ) != 0 ) {

        std::cout << "[TEST ERROR] Prepare Funcion failed" << std::endl;
        return -1;
    }

    ::testing::InitGoogleTest ( &argc, argv );
    return RUN_ALL_TESTS();
}
