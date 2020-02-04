/*
 * Copyright 2020 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */






#include <ROSEndEffector/ParserMoveIt.h>

ROSEE::ParserMoveIt::ParserMoveIt() {

}

ROSEE::ParserMoveIt::~ParserMoveIt() {

}

bool ROSEE::ParserMoveIt::init ( std::string robot_description ) {
    
    if (robot_model != nullptr ) {
        std::cerr << "[PARSER]: init() already called by someone " << std::endl;;
        return false;
    }
    // it is a ros param in the launch, take care that also sdrf is read 
    // (param for srd is robot_description_semantic)
    this->robot_description = robot_description;
    
    //false: we dont need kinematic solvers now
    robot_model_loader::RobotModelLoader robot_model_loader(robot_description, false) ; 
    robot_model = robot_model_loader.getModel() ;
    if (robot_model == nullptr) {
        std::cerr << " [PARSER]: Fail To load robot model " << robot_description << std::endl;;
        return false;
    }
    std::cout << "[PARSER] Parsed Model: " << robot_model->getName() << std::endl; ;
    
    handName = robot_model->getName();
    
    lookForFingertips();
    lookJointsTipsCorrelation();
    
    return true;
    
}

std::string ROSEE::ParserMoveIt::getHandName() const {
    return handName;
}

std::vector<std::string> ROSEE::ParserMoveIt::getFingertipNames() const {
    return fingertipNames;
}

unsigned int ROSEE::ParserMoveIt::getNumberOfTips () const {
    return fingertipNames.size();
}

const robot_model::RobotModelPtr ROSEE::ParserMoveIt::getRobotModel () const {
    return robot_model;
}

std::map<std::string, std::vector<std::string> > ROSEE::ParserMoveIt::getFingertipsOfJointMap() const {
    return fingertipsOfJointMap;
}

std::map<std::string, std::vector<std::string> > ROSEE::ParserMoveIt::getJointsOfFingertipMap() const {
    return jointsOfFingertipMap;
}

robot_model::RobotModelPtr ROSEE::ParserMoveIt::getCopyModel() const {
    robot_model_loader::RobotModelLoader robot_model_loader(robot_description); 
    return robot_model_loader.getModel();
}

std::vector < std::string > ROSEE::ParserMoveIt::getGroupOfLink ( std::string linkName ) { 
    
    std::vector < std::string > groupsReturn;

    if (robot_model == nullptr) {
        std::cerr << " [PARSER::" << __func__ << 
            "]: robot_model is null. Have you called init() before?"  << std::endl;
        return groupsReturn;
    }
    
    for (auto group : robot_model->getJointModelGroups() ) {
        
        if ( group->hasLinkModel(linkName) ) {
                
            groupsReturn.push_back ( group->getName() ) ;
        }
    }
    return groupsReturn;
}


bool ROSEE::ParserMoveIt::checkIfContinuosJoint ( std::string jointName) const {
    if (robot_model == nullptr) {
        std::cerr << " [PARSER::" << __func__ << 
            "]: robot_model is null. Have you called init() before?"  << std::endl;
        return false;
    }
    return (ROSEE::ParserMoveIt::checkIfContinuosJoint(robot_model->getJointModel(jointName)));     
}

bool ROSEE::ParserMoveIt::checkIfContinuosJoint ( const moveit::core::JointModel* joint ) const {
    if (robot_model == nullptr) {
        std::cerr <<  " [PARSER::" << __func__ << 
            "]: robot_model is null. Have you called init() before?" << std::endl;
        return false;
    }
    
    // for moveit, a continuos joint is a revolute joint
    if (joint->getType() != moveit::core::JointModel::REVOLUTE ) {
        return false;
    }
    
    //if revolute, only one limit (at.(0))
    moveit::core::JointModel::Bounds limits = joint->getVariableBounds();
    if ( limits.at(0).max_position_ - limits.at(0).min_position_ >= (2*EIGEN_PI) ) {
        return true;
    }
    
    return false;
}

std::vector <double> ROSEE::ParserMoveIt::getBiggerBoundFromZero ( std::string jointName ) const {
    if (robot_model == nullptr) {
        std::cerr << " [PARSER::" << __func__ << 
            "]: robot_model is null. Have you called init() before?" << std::endl;
        return std::vector<double>();
    }
    return ( ROSEE::ParserMoveIt::getBiggerBoundFromZero (robot_model->getJointModel(jointName) ) );

}

std::vector <double> ROSEE::ParserMoveIt::getBiggerBoundFromZero ( const moveit::core::JointModel* joint ) const {
    if (robot_model == nullptr) {
        std::cerr << " [PARSER::" << __func__ << 
            "]: robot_model is null. Have you called init() before?" << std::endl;
        return std::vector<double>();
    }
    
    moveit::core::JointModel::Bounds limits = joint->getVariableBounds();

    std::vector <double> trigMax;
    for ( auto limit : limits ) {
        if ( std::abs(limit.max_position_) > std::abs(limit.min_position_)) {
            trigMax.push_back ( limit.max_position_ ) ;
        } else {
            trigMax.push_back ( limit.min_position_ ) ;
        }
    }
    return trigMax;
}

unsigned int ROSEE::ParserMoveIt::getNExclusiveJointsOfTip ( std::string tipName, bool continuosIncluded ) const {
    
    if (jointsOfFingertipMap.size() == 0) {
        std::cerr << " [PARSER::" << __func__ << 
            "]: jointsOfFingertipMap empty. Have you called init() before? Also check your URDF and SRDF files" 
            << std::endl;
        return 0;
    }
    
    if (fingertipsOfJointMap.size() == 0) {
        std::cerr << " [PARSER::" << __func__ << 
            "]: jointsOfFingertipMap empty. Have you called init() before? Also check your URDF and SRDF files" 
            << std::endl;
        return 0;
    }
    
    
    unsigned int nExclusiveJoints = 0;

    for (auto jointOfTip : jointsOfFingertipMap.find(tipName)->second ) { 
        
        if ( !continuosIncluded && checkIfContinuosJoint(jointOfTip) == true ) {
            continue; //we dont want to count a continuos joint
        }
        
        
        
        //check if the joints of the tip move only that tip
        if ( fingertipsOfJointMap.find(jointOfTip)->second.size() == 1 ) {
            
            if (fingertipsOfJointMap.find(jointOfTip)->second.at(0).compare (tipName) != 0) {
                //  this condition is false if jointsOfFingertipMap and fingertipsOfJointMap are built well
                std::cerr << " [PARSER::" << __func__ << 
                "]: Strange error in fingertipsOfJointMap and jointsOfFingertipMap: they are not consistent: " 
                << "The unique tip present in the map for the key " << jointOfTip 
                << " is " << fingertipsOfJointMap.find(jointOfTip)->second.at(0) 
                << " and not " << tipName << " as it should be" 
                << std::endl;
                return 0;  
            }
            
            nExclusiveJoints++;
        }
    }
    
    return nExclusiveJoints;  
}


std::string ROSEE::ParserMoveIt::getFirstActuatedParentJoint ( std::string linkName, bool includeContinuos ) const {

    const moveit::core::LinkModel* linkModel = robot_model->getLinkModel ( linkName ) ;

    while ( linkModel->getParentJointModel()->getMimic() != NULL || 
            linkModel->parentJointIsFixed() ||
            linkModel->getParentJointModel()->isPassive() || 
            (!includeContinuos && checkIfContinuosJoint(linkModel->getParentJointModel())) ) {
        
        //an active joint is not any of these condition.
        //passive is an attribute of the joint in the srdf, so it may be not setted 
        // (default is not passive), so we need also the getMimic == NULL 
        // (ie: an actuated joint dont mimic anything)
        //WARNING these 4 conditions should be enough I think
        
        linkModel = linkModel->getParentLinkModel();
    }
    
    if (linkModel == NULL ) {
        
        std::cerr << " [PARSER::" << __func__ << 
            "]: Strange error: fingertipsOfJointMap, jointsOfFingertipMap, and/or other structures " <<
            "may have been built badly"  << std::endl ;
        return "";
    }
    
    return (linkModel->getParentJointModel()->getName());
}


std::string ROSEE::ParserMoveIt::getFirstActuatedJointInFinger (std::string linkName) const {
    const moveit::core::LinkModel* linkModel = robot_model->getLinkModel(linkName);
    const moveit::core::JointModel* joint;

    // we stop when the link has more than 1 joint: so linkModel will be the parent of the first 
    // link of the finger group and in joint we have stored the actuated (not continuos) 
    // child joint most near to linkModel
    while ( (linkModel != NULL) && (linkModel->getChildJointModels().size() < 2) ) {
        
        //if the parent joint is an actuated (not cont) joint, store it (or overwrite the previous stored)
        if ( linkModel->getParentJointModel()->getMimic() == NULL && 
            (!linkModel->parentJointIsFixed()) &&
            (!linkModel->getParentJointModel()->isPassive()) &&
            (!checkIfContinuosJoint(linkModel->getParentJointModel() ))  ) {
            
            joint = linkModel->getParentJointModel();
        }
        
        linkModel = linkModel->getParentLinkModel();
    }
    
    return joint->getName();
}



/*********************************** PRIVATE FUNCTIONS **********************************************************/
void ROSEE::ParserMoveIt::lookForFingertips() {
     for (auto it: robot_model->getJointModelGroups()) {
        
        std::string logGroupInfo;
        logGroupInfo = "[PARSER] Found Group '" + it->getName() + "', " ;
        
        if (it->getSubgroupNames().size() != 0 ) {
            logGroupInfo.append("but it has some subgroups \n");
            
        } else {
            if (! it->isChain()) {
                logGroupInfo.append("but it is not a chain \n");
                
            } else {
                logGroupInfo.append("with links: \n");
                for (auto itt : it->getLinkModels()) {
                   
                    logGroupInfo.append("\t'" + itt->getName() + "' ");
                    if (itt->getChildJointModels().size() != 0) {
                       
                       logGroupInfo.append("not a leaf link (not a fingertip)\n");
                       
                    } else {
                       logGroupInfo.append("a leaf link (a fingertip)\n");
                       fingertipNames.push_back(itt->getName());
                    }
                }
            }
        }
        std::cout << logGroupInfo << std::endl;
    }
}

void ROSEE::ParserMoveIt::lookJointsTipsCorrelation() {
    
    //initialize the map with all tips and with empty vectors of its joints
    for (const auto &it: fingertipNames) { 
        jointsOfFingertipMap.insert ( std::make_pair (it, std::vector<std::string>() ) );
    }
    
    //initialize the map with all the actuated joints and an empty vector for the tips that the joint move
    for (const auto &it: robot_model->getActiveJointModels()) { 
        fingertipsOfJointMap.insert ( std::make_pair (it->getName(), std::vector<std::string>() ) );
    }
    
    for ( const auto &joint: robot_model->getActiveJointModels()){ //for each actuated joint        
        for (const auto &link : joint->getDescendantLinkModels()) { //for each descendand link
            
            if (std::find(fingertipNames.begin(), fingertipNames.end(), link->getName()) != fingertipNames.end()){
                jointsOfFingertipMap.at ( link->getName() ).push_back( joint->getName() );
                fingertipsOfJointMap.at ( joint->getName() ).push_back( link->getName() );
            }
        }
    }
}
