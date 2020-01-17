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

#ifndef YAMLWORKER_H
#define YAMLWORKER_H

#define COLLIDER_REL_PATH "/configs/moveItCollider/"

#include <memory>

//yaml
#include <yaml-cpp/yaml.h>

//to create directories
#include <ROSEndEffector/Utils.h>

#include <ROSEndEffector/ActionPrimitive.h>
#include <ROSEndEffector/ActionPinch.h>
#include <ROSEndEffector/ActionTrig.h>

//TODO put this in the generic class
#include <moveit/robot_model_loader/robot_model_loader.h>
#include <moveit/planning_scene/planning_scene.h>


namespace ROSEE
{

/**
 * @todo write docs
 */
class YamlWorker
{
    
public:
    
    /**
     * Default constructor
     */
    YamlWorker(std::string handName);
    
    std::string createYamlFile ( std::map < std::set <std::string> , ActionPrimitive* >  ) ;
    
    std::map < std::set < std::string>, std::shared_ptr<ROSEE::ActionPrimitive> > parseYaml ( std::string filename, int act );


    std::string dirPath;
    
private:
    std::string emitYaml ( std::map < std::set <std::string> , ActionPrimitive* > ) ;

    
    

};

} //namespace ROSEE

#endif // YAMLWORKER_H
