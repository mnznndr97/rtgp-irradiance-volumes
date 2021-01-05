
#version 430 core

const float PI = 3.14159265359f;
const float M_PI = 3.14159265359f;
const float M_PI_2 = PI / 2;
const float M_PI_4 = PI / 4;
const float OCTANT_8_OFFSET = (7 * M_PI) / 4;
const float OCTANT_7_OFFSET = (6 * M_PI) / 4;
const float OCTANT_6_OFFSET = (5 * M_PI) / 4;
const float OCTANT_5_OFFSET = (4 * M_PI) / 4;
const float OCTANT_4_OFFSET = (3 * M_PI) / 4;
const float OCTANT_3_OFFSET = (2 * M_PI) / 4;
const float OCTANT_2_OFFSET = (1 * M_PI) / 4;
const float OCTANT_1_OFFSET = 0;

vec2 SemisphereToPoint(vec3 direction) {
	// direction.y is the cos of the theta angle
	// theta angle is the acos of (1 - xx ^ 2) so we can easily calculate xx from direction.y
	// (1 - xx ^ 2) is always >= 0 so direction.y >= 0 and theta angle does' t require adjustem to calculare
	// sin(theta)
	float xx = sqrt(1 - direction.y);
	if (xx == 0.0f) {
		return vec2(0.5f, 0.5f);
	}

	float theta = acos(direction.y);

	float sinPhi = direction.z / sin(theta);
	float cosPhi = direction.x / sin(theta);
	float phi = acos(cosPhi);

	

	// phi must be corrected in ordet to calculate the correct quadrant offset
	// Base angle is phi but if sin is < 0 we have to consider the "negative" angle
	float originalAngle = sinPhi < 0 ? 2 * M_PI - phi : phi;
	vec2 result = vec2(0.0f);
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
	return (result + 1.0f) / 2.0f;
}