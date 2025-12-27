#pragma once
#include "Buffer.h"

namespace RHI
{
	enum class Semantic
	{
		Position,
		Normal,
		UV,
		Tangent,
		Color
	};

	enum class InputClassification
	{
		Vertex,
		Instance
	};

	struct InputElement
	{
		BufferFormat dataFormat;
		Semantic semantic;
		UINT semanticIndex;
		UINT layoutIndex;
		UINT offset;
		InputClassification inputClassification;
		int stepRate;
	};


	struct InputLayout
	{
		InputElement* inputElements;
		int nElements;
	};

}