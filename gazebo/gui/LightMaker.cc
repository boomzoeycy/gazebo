/*
 * Copyright (C) 2012-2016 Open Source Robotics Foundation
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
 *
*/

#ifdef _WIN32
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
#endif

#include <iostream>
#include <sstream>

#include "gazebo/transport/Request.hh"

#include "gazebo/rendering/UserCamera.hh"
#include "gazebo/rendering/Light.hh"
#include "gazebo/rendering/Scene.hh"

#include "gazebo/gui/GuiIface.hh"
#include "gazebo/gui/LightMakerPrivate.hh"
#include "gazebo/gui/LightMaker.hh"

using namespace gazebo;
using namespace gui;

/////////////////////////////////////////////////
LightMaker::LightMaker() : dataPtr(new LightMakerPrivate)
{
  msgs::Set(this->dataPtr->msg.mutable_diffuse(),
      common::Color(0.5, 0.5, 0.5, 1));
  msgs::Set(this->dataPtr->msg.mutable_specular(),
      common::Color(0.1, 0.1, 0.1, 1));

  this->dataPtr->msg.set_attenuation_constant(0.5);
  this->dataPtr->msg.set_attenuation_linear(0.01);
  this->dataPtr->msg.set_attenuation_quadratic(0.001);
  this->dataPtr->msg.set_range(20);
}

/////////////////////////////////////////////////
LightMaker::~LightMaker()
{
}

/////////////////////////////////////////////////
bool LightMaker::InitFromLight(const std::string &_lightName)
{
  rendering::ScenePtr scene = gui::get_active_camera()->GetScene();
  if (!scene)
    return false;

  if (this->dataPtr->light)
  {
    scene->RemoveLight(this->dataPtr->light);
    this->dataPtr->light.reset();
  }

  rendering::LightPtr sceneLight = scene->GetLight(_lightName);
  if (!sceneLight)
  {
    gzerr << "Light: '" << _lightName << "' does not exist." << std::endl;
    return false;
  }

  this->dataPtr->light = sceneLight->Clone(_lightName + "_clone_tmp", scene);

  if (!this->dataPtr->light)
  {
    gzerr << "Unable to clone\n";
    return false;
  }

  this->dataPtr->lightTypename =  this->dataPtr->light->Type();
  this->dataPtr->light->FillMsg(this->dataPtr->msg);

  std::string newName = _lightName + "_clone";
  int i = 0;
  while (scene->GetLight(newName))
  {
    newName = _lightName + "_clone_" +
      boost::lexical_cast<std::string>(i);
    i++;
  }

  this->dataPtr->msg.set_name(newName);

  return true;
}

/////////////////////////////////////////////////
bool LightMaker::Init()
{
  rendering::ScenePtr scene = gui::get_active_camera()->GetScene();
  if (!scene)
    return false;

  this->dataPtr->light.reset(new rendering::Light(scene));
  this->dataPtr->light->Load();
  scene->AddLight(this->dataPtr->light);

  this->dataPtr->light->SetLightType(this->dataPtr->lightTypename);
  this->dataPtr->light->SetPosition(ignition::math::Vector3d(0, 0, 1));
  if (this->dataPtr->lightTypename == "directional")
    this->dataPtr->light->SetDirection(ignition::math::Vector3d(.1, .1, -0.9));

  this->dataPtr->msg.set_name(
      "user_" + this->dataPtr->lightTypename + "_light");

  return true;
}

/////////////////////////////////////////////////
void LightMaker::Start()
{
  EntityMaker::Start();

  if (!this->dataPtr->light)
    this->Init();
}

/////////////////////////////////////////////////
void LightMaker::Stop()
{
  if (this->dataPtr->light)
  {
    rendering::ScenePtr scene = gui::get_active_camera()->GetScene();
    if (scene)
      scene->RemoveLight(this->dataPtr->light);
    this->dataPtr->light.reset();
  }
  EntityMaker::Stop();
}

/////////////////////////////////////////////////
void LightMaker::CreateTheEntity()
{
  msgs::Set(this->dataPtr->msg.mutable_pose()->mutable_position(),
            this->dataPtr->light->Position());
  msgs::Set(this->dataPtr->msg.mutable_pose()->mutable_orientation(),
            ignition::math::Quaterniond());

  transport::RequestInsert(this->dataPtr->msg);
}

/////////////////////////////////////////////////
ignition::math::Vector3d LightMaker::EntityPosition() const
{
  return this->dataPtr->light->Position();
}

/////////////////////////////////////////////////
void LightMaker::SetEntityPosition(const ignition::math::Vector3d &_pos)
{
  this->dataPtr->light->SetPosition(_pos);
}

/////////////////////////////////////////////////
PointLightMaker::PointLightMaker() : LightMaker()
{
  this->dataPtr->msg.set_type(msgs::Light::POINT);
  this->dataPtr->msg.set_cast_shadows(false);
  this->dataPtr->lightTypename = "point";
}

/////////////////////////////////////////////////
SpotLightMaker::SpotLightMaker() : LightMaker()
{
  this->dataPtr->msg.set_type(msgs::Light::SPOT);
  msgs::Set(this->dataPtr->msg.mutable_direction(),
            ignition::math::Vector3d(0, 0, -1));
  this->dataPtr->msg.set_cast_shadows(false);

  this->dataPtr->msg.set_spot_inner_angle(0.6);
  this->dataPtr->msg.set_spot_outer_angle(1.0);
  this->dataPtr->msg.set_spot_falloff(1.0);
  this->dataPtr->lightTypename  = "spot";
}

/////////////////////////////////////////////////
DirectionalLightMaker::DirectionalLightMaker() : LightMaker()
{
  this->dataPtr->msg.set_type(msgs::Light::DIRECTIONAL);
  msgs::Set(this->dataPtr->msg.mutable_direction(),
            ignition::math::Vector3d(.1, .1, -0.9));
  this->dataPtr->msg.set_cast_shadows(true);

  this->dataPtr->lightTypename  = "directional";
}
