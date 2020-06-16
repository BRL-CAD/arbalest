/*                C A M E R A M O D E . H
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

/** @file CameraMode.h
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * @brief
 *	Declaration of the base class for Camera modes of 3D Geometry
 *	Editor (g3d).
 */

#ifndef __G3D_CAMERAMODE_H__
#define __G3D_CAMERAMODE_H__

#include <cmath>

#include <brlcad/vmath.h>

#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>

namespace Ogre {
  class Camera;
}

/** @brief Base class for camera mode
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * Implements the actions asked to the camera, each derived class
 * behaves different according to its own nature (assigns different
 * key/mouse bindings for the actions of translation, rotation, etc).
 *
 * We keep common functionality implemented here.
 */
class CameraMode
{
public:
  /** Direction value: Negative/Neutral/Positive */
  enum Direction {
    NEGATIVE = -1,
    NEUTRAL = 0,
    POSITIVE = 1
  };

  /** Default constructor */
  CameraMode(const char* name);
  /** Destructor */
  virtual ~CameraMode() { }

  /** Called every frame via camera manager, with the time elapsed
   * since last update, so we move the camera of the engine, and thus
   * control how do we view the scene. */
  void updateCamera(Ogre::Camera* camera, double elapsedSeconds);

  /** Get the name */
  const char* getName() const;
  /** Get axis rotation (in radians) */
  float getXRotation() const;
  /** Get axis rotation (in radians) */
  float getYRotation() const;
  /** Get axis rotation (in radians) */
  float getZRotation() const;

  /** Set flag for this camera action */
  void setZoom(Direction direction);
  /** Set flag for this camera action */
  void setRotateX(Direction direction);
  /** Set flag for this camera action */
  void setRotateY(Direction direction);
  /** Set flag for this camera action */
  void setRotateZ(Direction direction);

  /** Set/get value for this camera value */
  void setRotationSpeed(float speed);
  /** Set/get value for this camera value */
  float getRotationSpeed() const;
  /** Set/get value for this camera value */
  void setZoomSpeedRatio(float ratio);
  /** Set/get value for this camera value */
  float getZoomSpeedRatio() const;

  /** Set flag for this camera action */
  void setResetToCenter(bool b);

  /** Stop all movements and rotations */
  void stop();

  /** Translate camera position and center position by given amount
   * (screen coordinates) relative to original center */
  void pan(float screenX, float screenY, vect_t originalCenter);
  /** Translate camera position and center position by given amount
   * (screen coordinates) relative to current center */
  void pan(float screenX, float screenY);

  /** Convert from degrees to radians */
  static float degreesToRadians(float degrees);
  
  /** Add delta to var such that var is always within +/-limit,
   * wrapping overflows. */
  static void circularIncrement(float& var, float delta, float limit = M_PI);

  /** Multiply the variable by given value, but result not more than
   * given limit */
  static void multiplyVarWithLimit(float& var, float value, float limit);
  /** Divide the variable by given value, but result not less than
   * given limit */
  static void divideVarWithLimit(float& var, float value, float limit);
  /** Correct var with limits */
  static float getVarWithinLimits(float var, float min, float max);

  /** Inject input */
  virtual bool injectKeyPressed(QKeyEvent * /* keyEvent */)
    { return false; }
  /** Inject input */
  virtual bool injectKeyReleased(QKeyEvent * /* keyEvent */)
    { return false; }
  /** Inject input */
  virtual bool injectMouseMotion(QMouseEvent *)
    { return false; }
  /** Inject input */
  virtual bool injectMousePressed(QMouseEvent * /* buttonId */)
    { return false; }
  /** Inject input */
  virtual bool injectMouseReleased(QMouseEvent * /* buttonId */)
    { return false; }
  /** Inject input */
  virtual bool injectMouseScrolled(Direction /* direction */)
    { return false; }

  /** Bindings for buttons in control window */
  virtual void cameraControlUpPressed() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlDownPressed() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlLeftPressed() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlRightPressed() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlZoomInPressed() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlZoomOutPressed() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlCenterPressed() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlUpReleased() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlDownReleased() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlLeftReleased() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlRightReleased() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlZoomInReleased() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlZoomOutReleased() = 0;
  /** Bindings for buttons in control window */
  virtual void cameraControlCenterReleased() = 0;

protected:
  /** Speed of the rotation */
  static const float ROTATION_DEFAULT_SPEED; // cycles/second
  /** Speed ratio of the zoom */
  static const float ZOOM_DEFAULT_SPEED_RATIO; // times per second
  /** Maximum radius distance */
  static const float RADIUS_MAX_DISTANCE; // m
  /** Minimum radius distance */
  static const float RADIUS_MIN_DISTANCE; // m
  /** Default radius distance */
  static const float RADIUS_DEFAULT_DISTANCE; // m

  /** Name of the mode */
  const char* _name;

  /** Pointer to the camera
   *
   * \note mafm: it would be nice if we could leave only by using the
   * camera in the updateCamera() method, but for flexibility with
   * some camera modes it seems better to have this pointer, at the moment.
   */
  Ogre::Camera* _camera;
  /** Window parameters */
  int _windowWidth;
  /** Window parameters */
  int _windowHeight;
  /** Default orthographic window width */
  float _orthoWindowDefaultWidth;
  /** Default orthographic window height */
  float _orthoWindowDefaultHeight;

  /** Flag for camera action */
  Direction _actionRotateX;
  /** Flag for camera action */
  Direction _actionRotateY;
  /** Flag for camera action */
  Direction _actionRotateZ;
  /** Flag for camera action */
  Direction _actionZoom;
  /** Flag for camera action */
  vect_t _actionPan;

  /** Flag for camera action */
  bool _actionResetToCenter;

  /** Basic value for calculation */
  float _rotationSpeed; /// cycles/second
  /** Basic value for calculation */
  float _zoomSpeedRatio; /// times per second
  /** Current radius */
  float _radius;
  /** Previous radius */
  float _previousRadius;
  /** Current horizontal rotation */
  float _horizontalRot;
  /** Current vertical rotation */
  float _verticalRot;
  /** Coordinates to take as center */
  vect_t _center;
};

#endif

/*
 * Local Variables:
 * tab-width: 8
 * mode: C
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
