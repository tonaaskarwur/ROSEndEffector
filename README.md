# ROS End-Effector 
ROS End-Effector package: provides a ROS-based set of standard interfaces to command robotics end-effectors in an agnostic fashion.

Wiki is here: https://github.com/ADVRHumanoids/ROSEndEffector/wiki

Documentation about the API of ROS End-Effector can be found here https://advrhumanoids.github.io/ROSEndEffector/index.html

CI powered by Travis CI (.org) [![Build Status](https://travis-ci.org/ADVRHumanoids/ROSEndEffector.svg?branch=master)](https://travis-ci.org/ADVRHumanoids/ROSEndEffector)

https://travis-ci.org/ADVRHumanoids/ROSEndEffector

***
<!-- 
    ROSIN acknowledgement from the ROSIN press kit
    @ https://github.com/rosin-project/press_kit
-->

<a href="http://rosin-project.eu">
  <img src="http://rosin-project.eu/wp-content/uploads/rosin_ack_logo_wide.png" 
       alt="rosin_logo" height="60" >
</a>

Supported by ROSIN - ROS-Industrial Quality-Assured Robot Software Components.  
More information: <a href="http://rosin-project.eu">rosin-project.eu</a>

<img src="http://rosin-project.eu/wp-content/uploads/rosin_eu_flag.jpg" 
     alt="eu_flag" height="45" align="left" >  

This project has received funding from the European Union’s Horizon 2020  
research and innovation programme under grant agreement no. 732287. 




# Actions Finder Node
* This node will explore your robot model (i.e. *urdf* and *srdf* files) in order to find some primitive actions (e.g. __pinch__, __trig__ ) that your hand can do. 
* MoveIt library is used to parse the models and to find collisions (to find the **pinches**)
* Information about each action are stored in some *yaml* files, that can be parsed afterwards. The main information in the *yaml* files is the necessary position of the joints to perform that action (see some examples [here](configs/actionExamples)).
* Additional primitive actions can be added, deriving the c++ class or creating directly by hand the *yaml* file.

For more complicated action, there is the possibility to define them as **composed** action, that is, an action *composed* by different others. In this case the joint positions of the composed action will consider all the action included in it.

## How to Run
#### Install Dependencies
```bash
sudo apt-get install ros-kinetic-moveit #moveit
```
#### Install Rosee package

#### Run with your model
Run the node **UniversalFindActions** after filling the two ROS parameters *robot_description* and *robot_description_semantic* with your *urdf* and *srdf* model respectively. An easy way is to use launch files and then launch with **roslaunch**:
```xml
<launch>
  <!-- send urdf to parameter server -->
  <param name="robot_description" 
         textfile="<your URDF file>"/>
  <param name="robot_description_semantic" 
         textfile="<your SRDF file>" />

    <node type="UniversalFindActions" 
          name="UniversalFindActions" 
          pkg="ros_end_effector" output="screen"/>
</launch>
```
This will generate yaml files in the *config/action* folder. The same yaml files are also parsed by the same node to test (and print) the correctness. 
**WARNING** old action *yaml* files will be ovewritten every time you run again the node.
Also, take care to generate correctly the srdf (see later).
## Examples 
#### TestEE Example (a simple 3 fingers model without pinch)
```bash
source ~/ROSEE/devel/setup.bash # or wherever it is
roslaunch ros_end_effector findActionsTestEE.launch

#To play with the hand and manually set the poses
roslaunch ros_end_effector two_finger_ee_startup.launch inSlider:=true
```

#### 2Finger (simple 2 finger model with pinch)
```bash
source ~/ROSEE/devel/setup.bash # or wherever it is
roslaunch ros_end_effector findActionsTwoFinger.launch

#To play with the hand and manually set the poses
roslaunch ros_end_effector test_ee_startup.launch inSlider:=true
```

#### Schunk Hand (very complex hand with lot of dofs)
First, install schunk package: (taken from [here](http://wiki.ros.org/schunk_svh_driver), section "2.4 From Source")
```bash
mkdir ~/schunk_ws
mkdir ~/schunk_ws/src
cd ~/schunk_ws/src
git clone https://github.com/fzi-forschungszentrum-informatik/fzi_icl_core.git #schunk library, I am not sure if needed for only the simulation
git clone https://github.com/fzi-forschungszentrum-informatik/fzi_icl_comm.git #schunk library, I am not sure if needed for only the simulation
git clone https://github.com/fzi-forschungszentrum-informatik/schunk_svh_driver.git #the main schunk repo
cd ..
catkin_make_isolated
source devel_isolated/setup.bash 
```
Similar to the other examples, launch the node:

```bash
source ~/ROSEE/devel/setup.bash # or wherever it is
roslaunch ros_end_effector findActionsSchunk.launch

#to play with the hand
source devel_isolated/setup.bash
roslaunch schunk_svh_driver svh_controller.launch standalone:=true gui:=true simulation:=true
```
Be sure to put in rviz as __fixed frame__ the  __base_link__

#### Robotic-3f (3 finger hand with two motors (I think) )
```bash
mkdir ~/robotiq_ws
cd robotiq_ws
mkdir src
cd src
git clone https://github.com/ros-industrial/robotiq.git
git checkout kinetic-devel
cd ..
rosdep update
rosdep install robotiq_modbus_tcp
sudo apt-get install ros-kinetic-soem
rosdep install --from-paths src/ --ignore-src --rosdistro kinetic
catkin_make
source devel/setup.bash
```

launch the find actions node:
```bash
source ~/ROSEE/devel/setup.bash # or wherever it is
roslaunch ros_end_effector findActionsRobotiq_3f.launch
```
Still trying to manage to run rviz and joint publisher to move the hand (the model does not load properly in rviz for now). For now, you can move the hand with [moveit assistant](http://docs.ros.org/kinetic/api/moveit_tutorials/html/doc/setup_assistant/setup_assistant_tutorial.html).
**Note** : I modified a bit the original *urdf* from robotiq. In their file, all joints are actuated. In truth, watching video of how the hand moves, there should be a unique joint that close all the fingers and another one that spread the two fingers on on side of the palm. I do not add "mimicing" of distal phalanges, they are not so linearly coupled.

# To run the ROS EE on the Schunk

```
roslaunch ros_end_effector schunk_startup.launch gui:=true simulation:=true
```

## Creating srdf files
Both *moveit* and this *ROSEE node* refers to srdf file to explore your hand. So it is important to provide a right srdf file. The convention used is that each finger is a *group* (from the base of the finger to the tip).
Even for very complicated hand like schunk hand, this file is easy to create (see the one for the schunk [here](configs/srdf/svh-ROSEE.srdf)). 
You must simply add a chain for every group. Take care to include in the chain all the joint that move that group (i.e. that finger). The chain can also have a common root.
If you don't want to create this by hand, you can use the [moveit assistant](http://docs.ros.org/kinetic/api/moveit_tutorials/html/doc/setup_assistant/setup_assistant_tutorial.html), which will help to create srdf files (among the other things) through a GUI. 
In the *srdf* file is also important to set the passive joints: these will be considered not actuated. This is necessary if you do not want to 
modify the *urdf* setting these joints as mimic or fixed.

## How to check if things are good with google tests
- Compile:
    ```bash
    cd <ROSEE_pkg_path>/build
    make tests
    ```
- Run Test OPTION 1 (Colorful cout but all tests togheter)
    ```bash
    roslaunch ros_end_effector googleTest_run_all.launch 
    ```
    Check the **googleTest_run_all.launch** file to change the hand for the tests
    
- Run Test OPTION 2 (each test sequentially)
    With  ``` make test ARGS="-V" ``` we first need to load the urdf and srdf file "manually" with ``` rosparam set ```, for example:
    ```bash
    # test_ee hand
    rosparam set -t $(rospack find ros_end_effector)/configs/urdf/test_ee.urdf robot_description
    rosparam set -t $(rospack find ros_end_effector)/configs/srdf/test_ee.srdf robot_description_semantic
    make test ARGS="-V"
    ```
    ```bash
    # 2_finger hand
    rosparam set -t $(rospack find ros_end_effector)/configs/urdf/two_finger.urdf robot_description
    rosparam set -t $(rospack find ros_end_effector)/configs/srdf/two_finger.srdf robot_description_semantic
    make test ARGS="-V"
    ```
    Similar for other hands.
## Possible Issues
* From 28-01-2020 the use_gui param gives an error because it is deprecated. This causes the sliders of joint state publisher not shown. To solve : 
    ```bash
    sudo apt install ros-kinetic-joint-state-publisher-gui
    ```


 * With the schunk hand, if we move only the middle finger (base phalange)
toward the hand, a collision between index tip, middle tip and ring tip is detected. 
Easy reproducible with the [moveit assistant](http://docs.ros.org/kinetic/api/moveit_tutorials/html/doc/setup_assistant/setup_assistant_tutorial.html), in the set pose section (when we move the middle finger it finds a collision when visually is not present)
I dont know if it is a problem of schunk model, moveit, or both.
