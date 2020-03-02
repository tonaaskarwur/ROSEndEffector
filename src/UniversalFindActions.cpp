/*
 * Copyright (C) 2020 IIT-HHCM
 * Author: Davide Torielli
 * email:  davide.torielli@iit.it
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

#include <ros/ros.h>
#include <ROSEndEffector/UniversalRosEndEffectorExecutor.h>
#include <ROSEndEffector/FindActions.h>
#include <ROSEndEffector/Action.h>
#include <ROSEndEffector/ActionComposed.h>
#include <ROSEndEffector/ActionTimed.h>
#include <ROSEndEffector/ActionGeneric.h>
#include <ROSEndEffector/ParserMoveIt.h>

#include <ROSEndEffector/MapActionHandler.h>

int main ( int argc, char **argv ) {

    ros::init ( argc, argv, "FindActions" );
    
    std::shared_ptr <ROSEE::ParserMoveIt> parserMoveIt = std::make_shared <ROSEE::ParserMoveIt> ();
    parserMoveIt->init ("robot_description") ;
    
    std::string folderForActions = ROSEE::Utils::getPackagePath() + "/configs/actions/" + parserMoveIt->getHandName();
    
    ROSEE::FindActions actionsFinder (parserMoveIt);

    auto maps = actionsFinder.findPinch(folderForActions + "/primitives/");

    std::map <std::string, ROSEE::ActionTrig> trigMap =  actionsFinder.findTrig (ROSEE::ActionPrimitive::Type::Trig, 
                                                                                 folderForActions + "/primitives/") ;

    std::map <std::string, ROSEE::ActionTrig> tipFlexMap = actionsFinder.findTrig (ROSEE::ActionPrimitive::Type::TipFlex, 
                                                                                   folderForActions + "/primitives/");

    std::map <std::string, ROSEE::ActionTrig> fingFlexMap = actionsFinder.findTrig (ROSEE::ActionPrimitive::Type::FingFlex, 
                                                                                    folderForActions + "/primitives/");
    unsigned int nFinger = 3;
    std::map < std::string, ROSEE::ActionMoreTips> moreTipsMap = actionsFinder.findMoreTips (nFinger, folderForActions + "/primitives/") ;
    
    auto mulPinch = actionsFinder.findMultiplePinch(3, folderForActions + "/primitives/" );


    /** ********************* PARSING TEST and print... these things should not be here ****************/
    
    ROSEE::MapActionHandler mapsHandler;
    mapsHandler.parseAllPrimitives(folderForActions + "/primitives/");

        
    /******************************* PRINTS OF PARSED PRIMITIVES *********************************************/
    std::cout << "PARSED MAP OF PINCHESSTRONG FROM YAML FILE:" << std::endl;
    for (auto &i : mapsHandler.getPrimitive("pinchStrong")) {
        i.second->print();
    }    
    std::cout << "PARSED MAP OF PINCHESWEAK FROM YAML FILE:" << std::endl;
    for (auto &i : mapsHandler.getPrimitive("pinchWeak")) {
        i.second->print();
    }
    std::cout << "PARSED MAP OF TRIGS FROM YAML FILE:" << std::endl;
    for (auto &i : mapsHandler.getPrimitive("trig")) {
        i.second->print();
    }
    std::cout << "PARSED MAP OF TIPFLEX FROM YAML FILE:" << std::endl;
    for (auto &i : mapsHandler.getPrimitive("tipFlex")) {
        i.second->print();
    }
    std::cout << "PARSED MAP OF FINGFLEX FROM YAML FILE:" << std::endl;
    for (auto &i : mapsHandler.getPrimitive("fingFlex")) {
        i.second->print();
    }
    std::cout << "PARSED MAP OF MORETIPS FROM YAML FILE:" << std::endl;
    for (auto &i : mapsHandler.getPrimitive("moreTips-3")) {
        i.second->print();
    }
    std::cout << "DEBUG MULTIPINCH PARSED: " << std::endl;
    for (auto &it : mapsHandler.getPrimitive("multiplePinchStrong-3")) {
        it.second->print();
    }
    
    
    ROSEE::YamlWorker yamlWorker;
    /** **************************** COMPOSITE ACTION THINGS ************************************************ */
    
    std::cout << "A composed action with Independent inner action: " << std::endl;
    ROSEE::ActionComposed grasp ("grasp", true);

    if ( mapsHandler.getPrimitive(ROSEE::ActionPrimitive::Type::Trig).size() == parserMoveIt->getFingertipNames().size() ) {
        for (auto trig : mapsHandler.getPrimitive("trig")) {
            grasp.sumAction  (trig.second) ; 
        }
        grasp.print();
       
        yamlWorker.createYamlFile (&grasp, folderForActions + "/generics/");
        
        mapsHandler.parseAllGenerics (folderForActions + "/generics/");
        
        std::cout << "PARSED COMPOSEd" << std::endl;
        mapsHandler.getGeneric("grasp")->print();

        
    }
   

    
    /** **************************** SIMPLE ACTION MANUALLY CREATED ***********************************************    */  

    ROSEE::JointPos jp;

    //for now copy jp of another action
    jp = ROSEE::operator*(maps.first.begin()->second.getJointPos(), 2);
    auto jpc = maps.first.begin()->second.getJointsInvolvedCount();

    ROSEE::ActionGeneric simpleAction("casual", jp, jpc);
    simpleAction.print();
    
    yamlWorker.createYamlFile( &simpleAction,  folderForActions + "/generics/" );

    mapsHandler.parseAllGenerics (folderForActions + "/generics/"); //NOTE already called before

    std::cout << "The parsed casual: " << std::endl;
    mapsHandler.getGeneric("casual")->print();
  

    
    /** **************************** TIMED ACTION THINGS ***********************************************    
    ROSEE::ActionTimed actionTimed("wide_grasp");
    std::set<std::string> one;
    one.insert ("finger_1_joint_1");
    actionTimed.insertAction( mapsHandler.getPrimitive("moreTips-3").at(one), 0, 0.2, 0, 0.5, "GRASP");
    one.clear();
    one.insert("finger_1_joint_1");
    actionTimed.insertAction( mapsHandler.getPrimitive(ROSEE::ActionPrimitive::Type::MoreTips).at(0).at(one), 0, 0.2, 0, 1, "GRASP2");
    
    one.clear();
    one.insert("finger_2_link_3");
    one.insert("finger_middle_link_3");    
    actionTimed.insertAction( mapsHandler.getPrimitive("pinch").at(one), 0, 0.2, 0, 1, "PINCH");
    actionTimed.insertAction( mapsHandler.getPrimitive("pinch").at(one), 0, 0.2, 0, 0.5, "PINCH2");

    actionTimed.print();
    
    yamlWorker.createYamlFile ( &actionTimed, folderForActions + "/timeds/" );
    mapsHandler.parseAllTimeds(folderForActions + "/timeds/");

    std::cout << "The timed action parsed: " << std::endl;
    mapsHandler.getTimed("wide_grasp").print();

*/
    return 0;
    
}
