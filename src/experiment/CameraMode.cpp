/*                C A M E R A M O D E . C X X
 * BRL-CAD
 *
 * Copyright (c) 2008-2011 United States Government as represented by the
 * U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */

/** @file CameraMode.cxx
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * @brief
 *	Implementation of the base class for Camera modes of 3D
 *	Geometry Editor (g3d).
 */

#include "CameraMode.h"

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreViewport.h>

#include "Logger.h"

/*******************************************************************************
 * CameraMode
 ******************************************************************************/
const float CameraMode::ROTATION_DEFAULT_SPEED = (2.0f*M_PI)/4.0f; // 4s for full revolution
const float CameraMode::ZOOM_DEFAULT_SPEED_RATIO = 4.0f; // 4 times per second
const float CameraMode::RADIUS_MAX_DISTANCE = 10000.0f; // m
const float CameraMode::RADIUS_MIN_DISTANCE = 0.1f; // m
const float CameraMode::RADIUS_DEFAULT_DISTANCE = 500.0f; // m

CameraMode::CameraMode(const char* name) :
  _name(name),
  _camera(0),
  _windowWidth(0), _windowHeight(0),
  _orthoWindowDefaultWidth(0.0f), _orthoWindowDefaultHeight(0.0f),
  _actionRotateX(NEUTRAL), _actionRotateY(NEUTRAL), _actionRotateZ(NEUTRAL),
  _actionZoom(NEUTRAL),
  _actionResetToCenter(false),
  _rotationSpeed(ROTATION_DEFAULT_SPEED),
  _zoomSpeedRatio(ZOOM_DEFAULT_SPEED_RATIO),
  _radius(RADIUS_DEFAULT_DISTANCE), _previousRadius(_radius),
  _horizontalRot(0.0f), _verticalRot(0.0f)
{
  VSETALL(_actionPan, 0);
  VSETALL(_center, 0);
}

void CameraMode::updateCamera(Ogre::Camera* camera, double elapsedSeconds)
{
  _camera = camera;
  _windowWidth = camera->getViewport()->getActualWidth();
  _windowHeight = camera->getViewport()->getActualHeight();
  if (_orthoWindowDefaultWidth == 0.0f) {
    _orthoWindowDefaultWidth = _camera->getOrthoWindowWidth();
    _orthoWindowDefaultHeight = _camera->getOrthoWindowHeight(); 
  }

  // apply rotations
  if (_actionResetToCenter) {
    // center (reset rotation) when requested
    _horizontalRot = 0.0f;
    _verticalRot = 0.0f;
    VSETALL(_center, 0);
    _radius = RADIUS_DEFAULT_DISTANCE;
    _previousRadius = _radius;
    _camera->setOrthoWindowWidth(_orthoWindowDefaultWidth);
    _actionResetToCenter = false;
  } else {
    // vertical rotation
    if (_actionRotateX == POSITIVE) {
      circularIncrement(_verticalRot, _rotationSpeed * elapsedSeconds);
    } else if (_actionRotateX == NEGATIVE) {
      circularIncrement(_verticalRot, -_rotationSpeed * elapsedSeconds);
    }

    // horizontal rotation
    if (_actionRotateY == POSITIVE) {
      circularIncrement(_horizontalRot, _rotationSpeed * elapsedSeconds);
    } else if (_actionRotateY == NEGATIVE) {
      circularIncrement(_horizontalRot, -_rotationSpeed * elapsedSeconds);
    }
  }

  // radius
  if (_actionZoom == POSITIVE) {
    divideVarWithLimit(_radius,
		       1.0f + (ZOOM_DEFAULT_SPEED_RATIO*elapsedSeconds),
		       RADIUS_MIN_DISTANCE);
  } else if (_actionZoom == NEGATIVE) {
    multiplyVarWithLimit(_radius,
			 1.0f + (ZOOM_DEFAULT_SPEED_RATIO*elapsedSeconds),
			 RADIUS_MAX_DISTANCE);
  }

  // set the resulting position to the camera
  if (!(_actionPan[X] == 0.0f &&
	_actionPan[Y] == 0.0f &&
	_actionPan[Z] == 0.0f)) {
    //Logger::logDEBUG("panning: %g %g %g", _actionPan.x, _actionPan.y, _actionPan.z);

    // get center relative to camera
    Ogre::Vector3 cameraPos = camera->getPosition();
    Ogre::Vector3 difference(_center[X], _center[Y], _center[Z]);
    difference -= cameraPos;
    // pan camera (relative to its position)
    camera->moveRelative(Ogre::Vector3(-_actionPan[X], _actionPan[Y], 0));
    cameraPos = camera->getPosition();
    //Logger::logDEBUG(" - pos: %g %g %g", cameraPos.x, cameraPos.y, cameraPos.z);
    // restore center
    VSET(_center,
	 cameraPos.x + difference.x,
	 cameraPos.y + difference.y,
	 cameraPos.z + difference.z);

    //Logger::logDEBUG(" - center: %g %g %g", _center.x, _center.y, _center.z);
    camera->lookAt(_center[X], _center[Y], _center[Z]);

    // stop panning
    VSETALL(_actionPan, 0);
  } else {
    Ogre::SceneNode tmpNode(0);

    // set initial center
    Ogre::Vector3 centerTranslation(_center[X], _center[Y], _center[Z]);
    tmpNode.translate(centerTranslation, Ogre::SceneNode::TS_LOCAL);

    // rotations
    tmpNode.yaw(Ogre::Radian(_horizontalRot));
    tmpNode.pitch(Ogre::Radian(_verticalRot));

    // position -- push back given radius
    Ogre::Vector3 radiusDistance(0, 0, _radius);
    tmpNode.translate(radiusDistance, Ogre::SceneNode::TS_LOCAL);

    if (camera->getPosition() != tmpNode.getPosition()) {
      //Logger::logDEBUG("Camera position (%0.1f, %0.1f, %0.1f)", pos.x, pos.y, pos.z);

      camera->setPosition(tmpNode.getPosition());
      camera->lookAt(_center[X], _center[Y], _center[Z]);
    }

    // zoom for orthogonal mode
    if (_camera->getProjectionType() == Ogre::PT_ORTHOGRAPHIC
	&& _previousRadius != _radius) {
      float orthoRatio = _radius/_previousRadius;
      _camera->setOrthoWindowWidth(orthoRatio * _camera->getOrthoWindowWidth());
      //Logger::logDEBUG("Orthogonal ratio: %g", orthoRatio);
      //Logger::logDEBUG("window width: %g", _camera->getOrthoWindowWidth());
    } else if (_camera->getProjectionType() == Ogre::PT_PERSPECTIVE) {
      _camera->setOrthoWindowWidth(_orthoWindowDefaultWidth);
    }

    // update _previousRadius variable, for next time
    _previousRadius = _radius;
  }
}

const char* CameraMode::getName() const
{
  return _name;
}

float CameraMode::getXRotation() const
{
  return _camera->getRealOrientation().getPitch().valueRadians();
}

float CameraMode::getYRotation() const
{
  return _camera->getRealOrientation().getYaw().valueRadians();
}

float CameraMode::getZRotation() const
{
  return _camera->getRealOrientation().getRoll().valueRadians();
}

void CameraMode::setResetToCenter(bool b)
{
  _actionResetToCenter = b;
}

void CameraMode::setZoom(Direction direction)
{
  _actionZoom = direction;
}

void CameraMode::setRotateX(Direction direction)
{
  _actionRotateX = direction;
}

void CameraMode::setRotateY(Direction direction)
{
  _actionRotateY = direction;
}

void CameraMode::setRotateZ(Direction direction)
{
  _actionRotateZ = direction;
}

void CameraMode::setRotationSpeed(float speed)
{
  _rotationSpeed = speed;
}

float CameraMode::getRotationSpeed() const
{
  return _rotationSpeed;
}

void CameraMode::setZoomSpeedRatio(float ratio)
{
  _zoomSpeedRatio = ratio;
}

float CameraMode::getZoomSpeedRatio() const
{
  return _zoomSpeedRatio;
}

void CameraMode::stop()
{
  _actionRotateX = NEUTRAL;
  _actionRotateY = NEUTRAL;
  _actionRotateZ = NEUTRAL;
  _actionZoom = NEUTRAL;
}

void CameraMode::pan(float x, float y, vect_t originalCenter)
{
  VSET(_actionPan, x, y, 0);
  VMOVE(_center, originalCenter);
}

void CameraMode::pan(float x, float y)
{
  VSET(_actionPan, x, y, 0);
  //Logger::logDEBUG("panning: %g %g", _actionPan.x, _actionPan.y);
}

float CameraMode::degreesToRadians(float degrees)
{
  return (degrees*M_PI)/180.0f;
}

void CameraMode::circularIncrement(float& var, float value, float limit) 
{
  var += value;
  if(var > limit) {
    var -= 2*limit;
  } else if(var < -limit) {
    var += 2*limit;
  }
}

void CameraMode::multiplyVarWithLimit(float& var, float value, float limit)
{
  var *= value;
  if (var > limit) {
    var = limit;
  }
}

void CameraMode::divideVarWithLimit(float& var, float value, float limit)
{
  var /= value;
  if (var < limit) {
    var = limit;
  }
}

float CameraMode::getVarWithinLimits(float var, float min, float max)
{
  if (var < min) {
    return min;
  } else if (var > max) {
    return max;
  } else {
    return var;
  }
}

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
