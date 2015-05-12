// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#ifndef BALL_VIEW_RENDERING_RENDERERS_OSPRAYRENDERER_H
#define BALL_VIEW_RENDERING_RENDERERS_OSPRAYRENDERER_H

#include <BALL/VIEW/RENDERING/RENDERERS/raytracingRenderer.h>

#include <BALL/VIEW/WIDGETS/scene.h>
#include <BALL/VIEW/KERNEL/stage.h>
#include <BALL/VIEW/KERNEL/representationManager.h>
#include <BALL/VIEW/PRIMITIVES/mesh.h>
#include <BALL/VIEW/KERNEL/mainControl.h>
#include <BALL/MATHS/surface.h>
#include <BALL/MATHS/vector3.h>
#include <BALL/DATATYPE/hashMap.h>

#include <ospray/ospray.h>

namespace BALL
{
  namespace VIEW
	{
		/** OSPRayRenderer
			Provides ray tracing through OSPRay
			\ingroup ViewRendering
			*/
		class BALL_VIEW_EXPORT OSPRayRenderer
			: public RaytracingRenderer
		{
			public:
				/** This class encapsulates OSPRay's data structures per Representation.
				 */
				class OSPRayData
				{
					public:

						/// The geometries
						std::vector<OSPGeometry> geometries;

						/// The materials
						std::vector<OSPMaterial> materials;

						/// TODO: cutting planes

						/// Mark previously disabled representations
						bool has_been_disabled;
				};

				/// Default Constructor.
				OSPRayRenderer()
					: RaytracingRenderer(),
					  ospray_needs_update_(false),
						osp_renderer_(0),
						osp_framebuffer_(0),
						osp_camera_(0),
						osp_world_(0)
				{
				}

				/// Destructor
				virtual ~OSPRayRenderer();

				virtual bool hasFPScounter();

				virtual double getFPS();

				/************************************************************************/
				/* RaytracingRenderer methods					   */
				/************************************************************************/
				virtual bool init(Scene& scene);

				virtual String getRenderer()
				{
					return "OSPRay Ray Tracer";
				}

				virtual void formatUpdated()
				{
				}

				virtual GeometricObject* pickObject(Position x, Position y);
				virtual void pickObjects(Position x1, Position y1, Position x2, Position y2,
				                         std::list<GeometricObject*>& objects);



				virtual void setSize(float width, float height);

				virtual void setupStereo(float eye_separation, float focal_length);

				virtual void getFrustum(float& near_f, float& far_f, float& left_f, float& right_f, float& top_f, float& bottom_f);
				virtual void setFrustum(float near_f, float far_f, float left_f, float right_f, float top_f, float bottom_f);

				virtual void prepareBufferedRendering(const Stage& stage);
				virtual void renderToBufferImpl(FrameBufferPtr buffer);

				virtual void useContinuousLoop(bool use_loop);

				void bufferRepresentation(const Representation& rep);
				void bufferRepresentationDynamic(const Representation& rep);
				void removeRepresentation(const Representation& rep);

				void setLights(bool reset_all = false);

				void updateCamera(const Camera* camera = 0);

				void updateBackgroundColor();

				void setupEnvironmentMap(const QImage& image);

				void updateMaterialForRepresentation(Representation const* rep);
				void updateMaterialFromStage(OSPMaterial& osp_material);

				void convertMaterial(Stage::Material const& rt_material, OSPMaterial& osp_material);

				void transformTube(const TwoColoredTube& tube, float *trafo);
				void transformLine(const TwoColoredLine& line, float *trafo);

				/** Raytracing-related functionality **/
				//@{
				/** Intersect a set of rays with the geometry buffered by this renderer.
				 *
				 *  This function will intersect the rays
				 *
				 *     origins[i] + l * directions[i]
				 *
				 *  with the geometry that has been buffered by this renderer previously.
				 */
         virtual std::vector<float> intersectRaysWithGeometry(const std::vector<Vector3>& origins, const std::vector<Vector3>& directions);

         //@}

				 static bool initialized;

       private:
				 static const float vectorDifferenceTolerance_;

				 bool doVectorsDiffer(const Vector3& vecA, const Vector3& vecB);

				 HashMap<Representation const*, OSPRayData> objects_;

				 std::vector<OSPLight>                      lights_;

				 OSPRenderer    osp_renderer_;
				 OSPFrameBuffer osp_framebuffer_;
				 OSPCamera      osp_camera_;

				 OSPModel       osp_world_;

				 bool ospray_needs_update_;

				 Vector3                                                 last_camera_position;
				 Vector3                                                 last_camera_view_vec;
				 Vector3                                                 last_camera_lookup;
		};

    } // namespace VIEW
} // namespace BALL

#endif // BALL_VIEW_RENDERING_OSPRAYRENDERER_H
