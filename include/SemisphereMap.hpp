#pragma once


#ifndef _USE_MATH_DEFINES 
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <glm/glm.hpp>

const float OCTANT_8_OFFSET = (7 * M_PI) / 4;
const float OCTANT_7_OFFSET = (6 * M_PI) / 4;
const float OCTANT_6_OFFSET = (5 * M_PI) / 4;
const float OCTANT_5_OFFSET = (4 * M_PI) / 4;
const float OCTANT_4_OFFSET = (3 * M_PI) / 4;
const float OCTANT_3_OFFSET = (2 * M_PI) / 4;
const float OCTANT_2_OFFSET = (1 * M_PI) / 4;
const float OCTANT_1_OFFSET = 0;

// Forward declaration

glm::vec3 PointToSemisphere(glm::vec2 pos);
glm::vec2 SemisphereToPoint(const glm::vec3& direction);


float ClampAngle(float angle) {

	// To avoid correcting wrong calculation,
	// let's assert that the angle is relately near
	// the clamp level
	assert(angle < 1.1f);
	assert(angle > -1.1f);

	// To correct floating point errors
	if (angle > 1.0f) angle = 1.0f;
	if (angle < -1.0f) angle = -1.0f;
	return angle;
}

/*
* This function takes a point in the unit square,
* and maps it to a point on the unit hemisphere.
*
* Copyright 1994 Kenneth Chiu
*
* This code may be freely distributed and used
* for any purpose, commercial or non-commercial,
* as long as attribution is maintained.
*
* From: Notes on Adaptive Quadrature on the Hemisphere
* at https://www.semanticscholar.org/paper/Notes-on-Adaptive-Quadrature-on-the-Hemisphere-Shirley/c20d497c31cae7fb3c972bf8d205c232a4dd9257
*/
glm::vec3 PointToSemisphere(glm::vec2 pos) {
	glm::vec2 original = pos;
	float xx, yy, offset, theta, phi;

	/* This function should only be called with values in range in [0; 1] */
	assert(pos.x >= 0.0f && pos.x <= 1.0f);
	assert(pos.y >= 0.0f && pos.y <= 1.0f);

	pos = (pos * 2.0f) - 1.0f;

	assert(pos.x >= -1.0f && pos.x <= 1.0f);
	assert(pos.y >= -1.0f && pos.y <= 1.0f);

	if (pos.y > -pos.x) { // Above pos.y = -pos.x
		if (pos.y < pos.x) { // Below pos.y = pos.x
			xx = pos.x;
			if (pos.y > 0) { // Above pos.x-axis
			/*
			* Octant 1
			*/
				offset = OCTANT_1_OFFSET;
				yy = pos.y;
			}
			else { // Below and including pos.x-axis
				/*
* Octant 8
*/
				offset = OCTANT_8_OFFSET;
				yy = pos.x + pos.y;
			}
		}
		else { // Above and including pos.y = pos.x
			xx = pos.y;
			if (pos.x > 0) { // Right of pos.y-axis
			/*
			* Octant 2
			*/
				offset = OCTANT_2_OFFSET;
				yy = (pos.y - pos.x);
			}
			else { // Left of and including pos.y-axis
		 /*
		 * Octant 3
		 */
				offset = OCTANT_3_OFFSET;
				yy = -pos.x;
			}
		}
	}
	else { // Below and including pos.y = -pos.x
		if (pos.y > pos.x) { // Above pos.y = pos.x
			xx = -pos.x;
			if (pos.y > 0) { // Above pos.x-axis
			/*
			* Octant 4
			*/
				offset = OCTANT_4_OFFSET;
				yy = -pos.x - pos.y;
			}
			else { // Below and including pos.x-axis
		 /*
		 * Octant 5
		 */
				offset = OCTANT_5_OFFSET;
				yy = -pos.y;
			}
		}
		else { // Below and including pos.y = pos.x
			xx = -pos.y;
			if (pos.x > 0) { // Right of pos.y-axis
			/*
			* Octant 7
			*/
				offset = OCTANT_7_OFFSET;
				yy = pos.x;
			}
			else { // Left of and including pos.y-axis
				if (pos.y != 0) {
					/*
					* Octant 6
					*/
					offset = OCTANT_6_OFFSET;
					yy = pos.x - pos.y;
				}
				else {
					/*
					* Origin
					*/
					glm::vec3 result = glm::vec3(0.0f, 1.0f, 0.0f);
					glm::vec2 invResult = SemisphereToPoint(result);

					if (abs(original.x - invResult.x) > 0.0001f &&
						abs(original.y - invResult.y) > 0.0001f) DebugBreak();
					return result;
				}
			}
		}
	}

	theta = acos(1 - xx * xx);
	phi = offset + (M_PI / 4) * (yy / xx);

	glm::vec3 result(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));

#if DEBUG 
	// If we are in debug we can perform some checks in our
	// reverse map function
	glm::vec2 invResult = SemisphereToPoint(result);

	// Let's use a small epsilon
	assert(abs(original.x - invResult.x) <= 0.000001f);
	assert(abs(original.y - invResult.y) <= 0.000001f);
#else
#endif
	return result;
}

/// <summary>
/// Reverse function of PointToSemisphere
/// </summary>
/// <param name="direction">Sampling direction</param>
/// <returns>Point in the "sempisphere" grid from witch the direction was calculated</returns>
/// <remarks>This function is used mostly in the shader. It's here for debug purposes to check the correctness</remarks>

glm::vec2 SemisphereToPoint(const glm::vec3& direction) {
	// direction.y is the cos of the theta angle
	// theta angle is the acos of (1 - xx ^ 2) so we can easily calculate xx from direction.y
	// (1 - xx ^ 2) is always >= 0 so direction.y >= 0 and theta angle does' t require adjustem to calculare
	// sin(theta)

	float xx = sqrt(1 - direction.y);
	if (xx == 0.0f) {
		return glm::vec2(0.5, 0.5);
	}

	float theta = acos(direction.y);
	float sinTheta = sin(theta);
	float sinPhi = ClampAngle(direction.z / sinTheta);
	float cosPhi = ClampAngle(direction.x / sinTheta);
	float phi = acos(cosPhi);


	// phi must be corrected in ordet to calculate the correct quadrant offset
	// Base angle is phi but if sin is < 0 we have to consider the "negative" angle
	float originalAngle = sinPhi < 0 ? 2 * M_PI - phi : phi;
	glm::vec2 result(0.0f);
	if (originalAngle >= OCTANT_8_OFFSET) {
		/* Octant 8 */
		float yy = (originalAngle - OCTANT_8_OFFSET) * (xx / M_PI_4);
		result.x = xx;
		result.y = yy - result.x;
	}
	else if (originalAngle >= OCTANT_7_OFFSET) {
		/* Octant 7 */
		float yy = (originalAngle - OCTANT_7_OFFSET) * (xx / M_PI_4);
		result.y = -xx;
		result.x = yy;
	}
	else if (originalAngle >= OCTANT_6_OFFSET) {
		/* Octant 6 */
		float yy = (originalAngle - OCTANT_6_OFFSET) * (xx / M_PI_4);
		result.y = -xx;
		result.x = yy + result.y;
	}
	else if (originalAngle >= OCTANT_5_OFFSET) {
		/* Octant 5 */
		float yy = (originalAngle - OCTANT_5_OFFSET) * (xx / M_PI_4);
		result.x = -xx;
		result.y = -yy;
	}
	else if (originalAngle >= OCTANT_4_OFFSET) {
		/* Octant 4 */
		float yy = (originalAngle - OCTANT_4_OFFSET) * (xx / M_PI_4);
		result.x = -xx;
		result.y = -(yy + result.x);
	}
	else if (originalAngle >= OCTANT_3_OFFSET) {
		/* Octant 3 */
		float yy = (originalAngle - OCTANT_3_OFFSET) * (xx / M_PI_4);
		result.y = xx;
		result.x = -yy;
	}
	else if (originalAngle >= OCTANT_2_OFFSET) {
		/* Octant 2 */
		float yy = (originalAngle - OCTANT_2_OFFSET) * (xx / M_PI_4);
		result.y = xx;
		result.x = result.y - yy;
	}
	else {
		/* Octant 1 */
		float yy = (originalAngle - OCTANT_1_OFFSET) * (xx / M_PI_4);
		result.x = xx;
		result.y = yy;
	}

	/* Re - normalization */
	result = (result + 1.0f) / 2.0f;

	assert(result.x >= 0.0 && result.x <= 1.0f);
	assert(result.y >= 0.0 && result.y <= 1.0f);
	return result;
}
