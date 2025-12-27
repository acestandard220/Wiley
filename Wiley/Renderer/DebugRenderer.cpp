#include "DebugRenderer.h"

namespace Renderer3D {
	
	void DebugRenderer::AddLine(Vector3 const& start, Vector3 const& end, Vector3 const& color) {
		debugLines.push_back(DebugLine({ start, color }, { end, color }));
	}
	
	void DebugRenderer::AddTriangle(Vector3 const& a, Vector3 const& b, Vector3 const& c) {
		debugTriangle.push_back(DebugTriangle({ a }, { b }, { c }));
	}
	
	void DebugRenderer::AddAABB(Vector3 min, Vector3 max, Vector3 color, Bool wireFrame) {
		Vector3 c1 = Vector3(min.x, min.y, min.z);
		Vector3 c2 = Vector3(min.x, min.y, max.z);
		Vector3 c3 = Vector3(min.x, max.y, min.z);
		Vector3 c4 = Vector3(min.x, max.y, max.z);

		Vector3 c5 = Vector3(max.x, min.y, min.z);
		Vector3 c6 = Vector3(max.x, min.y, max.z);
		Vector3 c7 = Vector3(max.x, max.y, min.z);
		Vector3 c8 = Vector3(max.x, max.y, max.z);

		if (wireFrame) {
			AddLine(c1, c2);
			AddLine(c2, c3);
			AddLine(c3, c4);
			AddLine(c4, c1);

			AddLine(c5, c6);
			AddLine(c6, c7);
			AddLine(c7, c8);
			AddLine(c8, c5);

			AddLine(c1, c5);
			AddLine(c2, c6);
			AddLine(c3, c7);
			AddLine(c4, c8);
		}
		else {
			AddTriangle(c1, c2, c3);
			AddTriangle(c1, c4, c3);

			AddTriangle(c1, c2, c5);
			AddTriangle(c1, c6, c5);

			AddTriangle(c1, c5, c8);
			AddTriangle(c1, c4, c8);

			AddTriangle(c8, c5, c6);
			AddTriangle(c8, c7, c6);

			AddTriangle(c8, c4, c3);
			AddTriangle(c8, c7, c3);

			AddTriangle(c3, c7, c2);
			AddTriangle(c3, c6, c2);
		}
	}

	void DebugRenderer::DebugRender(RenderPass& pass) {
			


	}

}