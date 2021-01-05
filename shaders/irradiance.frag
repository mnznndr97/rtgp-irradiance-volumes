#version 430 core

const float PI = 3.14159265359f;

// Forward declaration
vec2 SemisphereToPoint(vec3 direction);


layout (std430, binding = 1) buffer GridInfoUniform
{
	vec3 GridMin;
	vec3 GridMax;
	ivec3 NumCellsPerDimension;
	mat4 GridTransform;
	int SamplesResolution;
	int _aligment_;
	int CellsSampleIndex[];
};

layout (std430, binding = 2) buffer IrradianceData
{
	vec4 IrradianceBuffer[];
};
layout (std430, binding = 3) buffer SubGridsDataBuffer
{
	int SubGridsData[];
};

// debug buffer used in some points to see the output of some computations
layout (std430, binding = 4) buffer DebugB
{
	int Debug[50];
	vec3 Debug2[10];
};

vec3 radianceSimple(vec3 direction, int samples, int sampleOffset){
    direction = normalize(direction);
	float resolutionF = sqrt(samples / 2.0f);
	int resolution = int(resolutionF);

	int hemisphereOffset = 0;
	if (direction.y < 0) {
		direction.y = -direction.y;
		hemisphereOffset = resolution * resolution;
	}

    vec2 point = SemisphereToPoint(direction);

	int ptX = int(floor(point.x * resolutionF));
	int ptY = int(floor(point.y * resolutionF));

	int storageIndex = hemisphereOffset + ptX * resolution + ptY;


	if(storageIndex >= 0 && storageIndex <= samples){
		storageIndex = (sampleOffset * samples) + storageIndex;
	    return IrradianceBuffer[storageIndex].rgb;
	}
	else
	{
		return vec3(1.0f);	
	}
}


/**
	Calculate a grid cell position from a given point and grid bounds

	OUT ->
		cellFinalIndex: Global cell index in the grid
		offset : point offset in the grid
		cellPos : Cell position in the subgrid
*/
void GetCellIndex(vec3 pos, int subGridIndex, vec3 gridMin, vec3 gridMax,
					out int cellFinalIndex, out vec3 offset, out ivec3 cellPos){

	// We first calculate the grid step
	vec3 gridStep = (gridMax - gridMin) / NumCellsPerDimension;

	Debug2[0] = gridMin;
	Debug2[1] = gridMax;
	// We get the transformed coordinates 
	vec3 gridMinTransformed = vec3(GridTransform * vec4(gridMin, 1.0f));
	vec3 gridMaxTransformed = vec3(GridTransform * vec4(gridMax, 1.0f));

	/*
	Debug2[2] = gridMinTransformed;
	Debug2[3] = gridMaxTransformed;
	*/

	// We normalize the position based on the grid dimension and cells
	vec3 qd = (pos - gridMinTransformed) / (gridMaxTransformed - gridMinTransformed) * NumCellsPerDimension;

	// We calculate the cell index (float and int)
	vec3 fcell = floor(qd);
	ivec3 cell = ivec3(fcell);
	
	// We set the 
	cellFinalIndex = (subGridIndex * NumCellsPerDimension.x * NumCellsPerDimension.y * NumCellsPerDimension.z) +
					(cell.x * NumCellsPerDimension.y * NumCellsPerDimension.z) +
					(cell.y * NumCellsPerDimension.z) +
					(cell.z);
	// Offset in the cell [0 - 1]
	offset = qd - fcell;
	// Cell position in the grid
	cellPos = cell;
}


void GetFinalCellIndex(vec3 pos, out int cellFinalIndex, out vec3 offset){

	// We calculate the cell index and offset for the main subgrid (0)
	vec3 currentGridStep = (GridMax - GridMin) / NumCellsPerDimension;
	ivec3 cellPos;
	GetCellIndex(pos, 0, GridMin, GridMax, cellFinalIndex, offset, cellPos);

	if(cellPos.x < 0 || cellPos.y < 0 || cellPos.z < 0
		|| cellPos.x >= NumCellsPerDimension.x
		|| cellPos.y >= NumCellsPerDimension.y
		|| cellPos.z >= NumCellsPerDimension.z
		)
	{
		// We are outside the grid bounds
		cellFinalIndex = -1;
		offset = vec3(0.0f);
	}

	// We have now to look if there are data in any subgrid
	int subGridIndex = 0;

	/*
	Debug[0] = -1;
	Debug[1] = cellFinalIndex;
	Debug[2] = -2;
	Debug[3] = -2;
	Debug[4] = -2;
	*/
	
	vec3 subgridMin = GridMin;
	

	// We loop until we reach the end of the array (-1)
	while(SubGridsData[subGridIndex] >= 0){
		// In the array, if at cell Y there is the subgrid (X+1), Array[X] = Y
		// So we need only to do a simple linear search
		if (SubGridsData[subGridIndex] == cellFinalIndex) {
			// Here we have found that the cell at {cellIndex} contains the subgrid {subGridIndex + 1}

			// We have to calculate the subgrid "area" and then we do another index step search
			// We move the min based on the step and then we set the max
			subgridMin += (currentGridStep * cellPos);
			vec3 subgridMax = subgridMin + currentGridStep;

			GetCellIndex(pos, subGridIndex + 1, subgridMin, subgridMax, 
						 cellFinalIndex, offset, cellPos);
			// Debug[2 + subGridIndex] = cellFinalIndex;
			// We reduce the dimension of the grid step since we are moving to the next grid level
			currentGridStep /= NumCellsPerDimension;
			
			// We have to start back due to "random" index ordering
			subGridIndex = -1;
		}
		++subGridIndex;
	}
	// Debug[4] = -42;

}

vec3 GetIrradiance(vec3 pos, vec3 normal, float reflectance)
{
	int finalCellIndex;
	vec3 offset;
	GetFinalCellIndex(pos, finalCellIndex, offset);
	
	if (finalCellIndex < 0) {
		// We are outside the grid. Let's return the a zero vector
		return vec3(0.0f);
	}

	// We add in our finalCellIndex the 8-samples space
	finalCellIndex *= 8;


	vec3 eightColors[8];
	for(int i = 0; i < 8; i++){
		vec3 color = radianceSimple(normal, SamplesResolution, CellsSampleIndex[finalCellIndex + i]);
		eightColors[i] = color;
	}	
	
	// Trilinear interpolation - x dimension
	vec3 c00 = eightColors[0] * (1.0f - offset.x) + eightColors[4] * (offset.x);
	vec3 c01 = eightColors[1] * (1.0f - offset.x) + eightColors[5] * (offset.x);
	vec3 c10 = eightColors[2] * (1.0f - offset.x) + eightColors[6] * (offset.x);
	vec3 c11 = eightColors[3] * (1.0f - offset.x) + eightColors[7] * (offset.x);
	// Trilinear interpolation - y dimension
	vec3 c0 = c00 * (1.0f - offset.y) + c10 * (offset.y);
	vec3 c1 = c01 * (1.0f - offset.y) + c11 * (offset.y);
	// Trilinear interpolation - < dimension
	vec3 finalIrradiance = c0 * (1.0f - offset.z) + c1 * (offset.z);

	finalIrradiance = (reflectance * finalIrradiance / PI);
	return finalIrradiance;
}