/*                C A M E R A M O D E M G E D . H
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

/** @file CameraModeMGED.h
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * @brief
 *	Declaration of the Camera mode "MGED" of 3D Geometry Editor
 *	(g3d).
 */

#ifndef __G3D_CAMERAMODEMGED_H__
#define __G3D_CAMERAMODEMGED_H__

#include "CameraMode.h"

/** @brief MGED camera mode
 *
 * @author Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
 *
 * The behavior of this camera tries to mimic the behaviour of
 * traditional BRL-CAD program MGED.  The shift-grips bindings are
 * described in:
 * http://brlcad.org/w/images/8/8c/Shift_Grips_Quick_Reference_Guide.pdf
 */
class CameraModeMGED : public CameraMode
{
public:
  /** Default constructor */
  CameraModeMGED();

  /** @see CameraMode::injectKeyPressed */
  virtual bool injectKeyPressed(QKeyEvent *e);
  /** @see CameraMode::injectKeyReleased */
  virtual bool injectKeyReleased(QKeyEvent *e);
  /** @see CameraMode::injectMouseMotion */
  virtual bool injectMouseMotion(QMouseEvent *e);
  /** @see CameraMode::injectMousePressed */
  virtual bool injectMousePressed(QMouseEvent *e);
  /** @see CameraMode::injectMouseReleased */
  virtual bool injectMouseReleased(QMouseEvent *e);

  /** @see CameraMode::cameraControlUpPressed */
  virtual void cameraControlUpPressed();
  /** @see CameraMode::cameraControlDownPressed */
  virtual void cameraControlDownPressed();
  /** @see CameraMode::cameraControlLeftPressed */
  virtual void cameraControlLeftPressed();
  /** @see CameraMode::cameraControlRightPressed */
  virtual void cameraControlRightPressed();
  /** @see CameraMode::cameraControlZoomInPressed */
  virtual void cameraControlZoomInPressed();
  /** @see CameraMode::cameraControlZoomOutPressed */
  virtual void cameraControlZoomOutPressed();
  /** @see CameraMode::cameraControlCenterPressed */
  virtual void cameraControlCenterPressed();
  /** @see CameraMode::cameraControlUpReleased */
  virtual void cameraControlUpReleased();
  /** @see CameraMode::cameraControlDownReleased */
  virtual void cameraControlDownReleased();
  /** @see CameraMode::cameraControlLeftReleased */
  virtual void cameraControlLeftReleased();
  /** @see CameraMode::cameraControlRightReleased */
  virtual void cameraControlRightReleased();
  /** @see CameraMode::cameraControlZoomInReleased */
  virtual void cameraControlZoomInReleased();
  /** @see CameraMode::cameraControlZoomOutReleased */
  virtual void cameraControlZoomOutReleased();
  /** @see CameraMode::cameraControlCenterReleased */
  virtual void cameraControlCenterReleased();

private:
  /** Default rotation step */
  static const float ROTATION_STEP; // radians
  /** Default pan fraction */
  static const float PAN_FRACTION; // ratio
  /** Default zoom step ratio */
  static const float ZOOM_STEP; // ratio
  /** Default zoom scale ratio */
  static const float ZOOM_SCALE; // ratio

  /** Key state */
  bool _keyControlPressed;
  /** Key state */
  bool _keyAltPressed;
  /** Key state */
  bool _keyShiftPressed;

  /** Mode */
  bool _translateModeEnabled;
  /** Mode */
  bool _rotateModeEnabled;
  /** Mode */
  bool _scaleModeEnabled;
  /** Submode */
  bool _constrainedSubmodeEnabled;
  /** Mode helper */
  bn_vector_component _constrainedToAxis;
  /** Mode helper */
  int _mouseButtonsPressed;
  /** Mode helper */
  int _dragOriginX;
  /** Mode helper */
  int _dragOriginY;
  /** Mode helper */
  float _dragOriginalRadius;
  /** Mode helper */
  float _dragOriginalHorizontalRotation;
  /** Mode helper */
  float _dragOriginalVerticalRotation;
  /** Mode helper */
  vect_t _dragOriginalCenter;
  

  /** Common functionality for all places needing it */
  void setMode();
  /** Common functionality for all places needing it */
  void doZoomScale(float scale);
  /** Common functionality for all places needing it */
  void doZoomIn();
  /** Common functionality for all places needing it */
  void doZoomOut();
  /** Common functionality for all places needing it */
  void panUp();
  /** Common functionality for all places needing it */
  void panDown();
  /** Common functionality for all places needing it */
  void panLeft();
  /** Common functionality for all places needing it */
  void panRight();
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
