/*                C A M E R A M O D E M G E D . C X X
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

/** @file CameraModeMGED.cxx
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * @brief
 *	Implementation of the Camera mode "MGED" of 3D Geometry Editor
 *	(g3d).
 */

#include <cmath>

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreRay.h>

#include "CameraModeMGED.h"

#include "Logger.h"

/*******************************************************************************
 * CameraModeMGED
 ******************************************************************************/
const float CameraModeMGED::ROTATION_STEP = M_PI/12.0f; // 15 degrees, in radians
const float CameraModeMGED::PAN_FRACTION = 20.0f; // m
const float CameraModeMGED::ZOOM_STEP = 1.25f; // ratio
const float CameraModeMGED::ZOOM_SCALE = 4.0f; // ratio

CameraModeMGED::CameraModeMGED() :
  CameraMode("MGED"),
  _keyControlPressed(false),
  _keyAltPressed(false),
  _keyShiftPressed(false),
  _translateModeEnabled(false),
  _rotateModeEnabled(false),
  _scaleModeEnabled(false),
  _constrainedSubmodeEnabled(false),
  _mouseButtonsPressed(0),
  _dragOriginX(0), _dragOriginY(0),
  _dragOriginalRadius(0.0f),
  _dragOriginalHorizontalRotation(0.0f), _dragOriginalVerticalRotation(0.0f)
{
  VSETALL(_dragOriginalCenter, 0);
}

bool CameraModeMGED::injectKeyPressed(QKeyEvent *e)
{
  if(e->modifiers() & Qt::KeypadModifier) {
    switch (e->key()) {
    case Qt::Key_5:
      // reset to center
      setResetToCenter(true);
      return true;
      
    default:
      break;
    }
  } else {
    switch(e->key()) {
    case Qt::Key_Control:
      _keyControlPressed = true;
      setMode();
      return true;
      
    case Qt::Key_Alt:
      _keyAltPressed = true;
      setMode();
      return true;
      
    case Qt::Key_Shift:
      _keyShiftPressed = true;
      setMode();
      return true;
      
    default:
      break;
    }
  }

  return false;
}

bool CameraModeMGED::injectKeyReleased(QKeyEvent *e)
{
  switch (e->key()) {
  case Qt::Key_Control:
    _keyControlPressed = false;
    setMode();
    return true;
    
  case Qt::Key_Alt:
    _keyAltPressed = false;
    setMode();
    return true;
    
  case Qt::Key_Shift:
    _keyShiftPressed = false;
    setMode();
    return true;
    
  default:
    break;
  }

  return false;
}

bool CameraModeMGED::injectMouseMotion(QMouseEvent *e)
{
  if (_scaleModeEnabled && _mouseButtonsPressed > 0) {
    // calculate the x position normalized between -1.0 and 1.0
    // w.r.t. screen coordinates (inside windows, if mouse goes
    // outside it depends on the windowing system)
    float horizDiffNorm = (e->x() - _dragOriginX)/static_cast<float>(_windowWidth/2.0f);
    float scale = pow(ZOOM_SCALE, horizDiffNorm);
    //Logger::logDEBUG("x diff: %g; scale: %g", horizDiffNorm, scale);

    // zoom freely, left zooms out and right zooms in
    doZoomScale(scale);

    return true;
  } else if (_translateModeEnabled && _mouseButtonsPressed > 0) {
    // pan given amount of screen units
    pan(e->x() - _dragOriginX, e->y() - _dragOriginY);
    _dragOriginX = e->x();
    _dragOriginY = e->y();
    return true;
  } else if (_rotateModeEnabled && _mouseButtonsPressed > 0) {
    // mafm: originally copied from blender mode

    // calculate the difference since last update, normalized between
    // -1.0 and 1.0 w.r.t. screen coordinates
    float horizDiffNorm = -(e->x() - _dragOriginX)/(_windowWidth/2.0f);
    float vertDiffNorm = -(e->y() - _dragOriginY)/(_windowHeight/2.0f);
    // Logger::logDEBUG("%.03f %.03f", horizDiffNorm, vertDiffNorm);

    // orbit freely, setting absolute position
    _horizontalRot = _dragOriginalHorizontalRotation + horizDiffNorm*M_PI;
    _verticalRot = _dragOriginalVerticalRotation + vertDiffNorm*M_PI;

    // Protect against overflows without causing viewjumping.
    circularIncrement(_horizontalRot, 0);
    circularIncrement(_verticalRot, 0);

    return true;
  } else {
    return false;
  }
}

bool CameraModeMGED::injectMousePressed(QMouseEvent *e)
{
  // increase the count of mouse buttons pressed, for all purposes
  ++(_mouseButtonsPressed);

  if (_translateModeEnabled || _rotateModeEnabled || _scaleModeEnabled) {
    switch (e->button()) {
    case Qt::LeftButton:
      _constrainedToAxis = X;
      _dragOriginX = e->x();
      _dragOriginY = e->y();
      _dragOriginalRadius = _radius;
      _dragOriginalHorizontalRotation = _horizontalRot;
      _dragOriginalVerticalRotation = _verticalRot;
      VMOVE(_dragOriginalCenter, _center);
      break;
      
    case Qt::MidButton:
      _constrainedToAxis = Y;
      _dragOriginX = e->x();
      _dragOriginY = e->y();
      _dragOriginalRadius = _radius;
      _dragOriginalHorizontalRotation = _horizontalRot;
      _dragOriginalVerticalRotation = _verticalRot;
      VMOVE(_dragOriginalCenter, _center);
      break;
      
    case Qt::RightButton:
      _constrainedToAxis = Z;
      _dragOriginX = e->x();
      _dragOriginY = e->y();
      _dragOriginalRadius = _radius;
      _dragOriginalHorizontalRotation = _horizontalRot;
      _dragOriginalVerticalRotation = _verticalRot;
      VMOVE(_dragOriginalCenter, _center);
      break;
      
    default:
      // nothing
      break;
    }
  } else {
    switch (e->button()) {
    case Qt::LeftButton:
      doZoomOut();
      break;
      
    case Qt::MidButton:
      /// \todo mafm: pan same distance as from point clicked to
      /// center, but in opposite direction
      break;
      
    case Qt::RightButton:
      doZoomIn();
      break;
      
    default:
      // nothing
      break;
    }
  }

  return true;
}

bool CameraModeMGED::injectMouseReleased(QMouseEvent *e)
{
  // decrease the count of mouse buttons pressed, for all purposes
  --(_mouseButtonsPressed);

  if (_translateModeEnabled || _rotateModeEnabled) {
    _constrainedSubmodeEnabled = false;
  }

  return true;
}

void CameraModeMGED::setMode()
{
  if (_keyControlPressed 
      && _keyAltPressed 
      && _keyShiftPressed) {
    // all keys active at once: scale mode
    _scaleModeEnabled = true;
    // ...and disable the rest
    _translateModeEnabled = false;
    _rotateModeEnabled = false;
    _constrainedSubmodeEnabled = false;
    //Logger::logDEBUG("set scale mode");
  } else {
    // submode "constrained"?
    if (_keyAltPressed) {
      _constrainedSubmodeEnabled = true;
    } else {
      _constrainedSubmodeEnabled = false;
    }

    // control key: rotation mode
    if (_keyControlPressed) {
      _rotateModeEnabled = true;
      // ...and disable the rest
      _scaleModeEnabled = false;
      _translateModeEnabled = false;

      //Logger::logDEBUG("set rotate mode, constrained=%d", _constrainedSubmodeEnabled);
    } else {
      if (_rotateModeEnabled) {
	//Logger::logDEBUG("unset rotate mode");
	_rotateModeEnabled = false;
      }
    }

    // shift key: translation mode
    if (_keyShiftPressed) {
      _translateModeEnabled = true;
      // ...and disable the rest
      _scaleModeEnabled = false;
      _rotateModeEnabled = false;

      //Logger::logDEBUG("set translate mode, constrained=%d", _constrainedSubmodeEnabled);
    } else {
      if (_translateModeEnabled) {
	//Logger::logDEBUG("unset translate mode");
	_translateModeEnabled = false;
      }
    }
  }
}

void CameraModeMGED::doZoomScale(float scale)
{
  _radius = _dragOriginalRadius / scale; // divide, because it means zoom in
  if (_radius > RADIUS_MAX_DISTANCE) {
    _radius = RADIUS_MAX_DISTANCE;
  } else if (_radius < RADIUS_MIN_DISTANCE) {
    _radius = RADIUS_MIN_DISTANCE;
  } 
  //Logger::logDEBUG("CameraModeMGED scale: radius=%g, scale=%g", _radius, scale);
}

void CameraModeMGED::cameraControlUpPressed()
{
  // nothing
}

void CameraModeMGED::cameraControlDownPressed()
{
  // nothing
}

void CameraModeMGED::cameraControlLeftPressed()
{
  // nothing
}

void CameraModeMGED::cameraControlRightPressed()
{
  // nothing
}

void CameraModeMGED::cameraControlZoomInPressed()
{
  // nothing
}

void CameraModeMGED::cameraControlZoomOutPressed()
{
  // nothing
}

void CameraModeMGED::cameraControlCenterPressed()
{
  // nothing
}

void CameraModeMGED::cameraControlUpReleased()
{
  panUp();
}

void CameraModeMGED::cameraControlDownReleased()
{
  panDown();
}

void CameraModeMGED::cameraControlLeftReleased()
{
  panLeft();
}

void CameraModeMGED::cameraControlRightReleased()
{
  panRight();
}

void CameraModeMGED::cameraControlZoomInReleased()
{
  doZoomIn();
}

void CameraModeMGED::cameraControlZoomOutReleased()
{
  doZoomOut();
}

void CameraModeMGED::cameraControlCenterReleased()
{
  setResetToCenter(true);
}

void CameraModeMGED::doZoomIn()
{
  divideVarWithLimit(_radius, ZOOM_STEP, RADIUS_MIN_DISTANCE);
}

void CameraModeMGED::doZoomOut()
{
  multiplyVarWithLimit(_radius, ZOOM_STEP, RADIUS_MAX_DISTANCE);
}

void CameraModeMGED::panUp()
{
  pan(0, (_camera->getOrthoWindowHeight()/PAN_FRACTION));
}

void CameraModeMGED::panDown()
{
  pan(0, -(_camera->getOrthoWindowHeight()/PAN_FRACTION));
}

void CameraModeMGED::panLeft()
{
  pan((_camera->getOrthoWindowWidth()/PAN_FRACTION), 0);
}

void CameraModeMGED::panRight()
{
  pan(-(_camera->getOrthoWindowWidth()/PAN_FRACTION), 0);
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
