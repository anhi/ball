#include <BALL/VIEW/RENDERING/RENDERERS/osprayRenderer.h>

#include <BALL/STRUCTURE/triangulatedSurface.h>
#include <BALL/VIEW/KERNEL/clippingPlane.h>
#include <BALL/VIEW/PRIMITIVES/sphere.h>
#include <BALL/VIEW/PRIMITIVES/twoColoredTube.h>
#include <BALL/VIEW/PRIMITIVES/twoColoredLine.h>

#include <QtGui/QImage>

#include <list>

static const float LINE_RADIUS = 0.02;

namespace BALL
{
	namespace VIEW
	{
		const float OSPRayRenderer::vectorDifferenceTolerance_= 0.0f;
		bool OSPRayRenderer::initialized = false;

		// todo: implement
		bool OSPRayRenderer::hasFPScounter()
		{
			return false;
		}

		// todo: implement
		double OSPRayRenderer::getFPS()
		{
			return -1.;
		}

		bool OSPRayRenderer::init(Scene& scene)
		{
			Renderer::init(scene);

			if (!initialized)
			{	
				int ac=0;
				ospInit(&ac, NULL);
				initialized = true;
			}

			osp_renderer_ = ospNewRenderer("obj");
			ospCommit(osp_renderer_);

			osp_camera_ = ospNewCamera("perspective");
			ospCommit(osp_camera_);

			osp_world_ = ospNewModel();
			ospCommit(osp_world_);

			ospSetObject(osp_renderer_, "world",  osp_world_);
			ospSetObject(osp_renderer_, "model",  osp_world_);
			ospSetObject(osp_renderer_, "camera", osp_camera_);
			ospCommit(osp_camera_);
			ospCommit(osp_renderer_);

			// todo: use the format used by the BALL framebuffer
      osp_framebuffer_ = ospNewFrameBuffer(osp::vec2i(width_, height_),OSP_RGBA_F32,OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
			ospFrameBufferClear(osp_framebuffer_, OSP_FB_ACCUM);

			std::cout << "init" << std::endl;

			return true;
		}

		OSPRayRenderer::~OSPRayRenderer()
		{
			std::cout << "destruct" << std::endl;
			if (osp_framebuffer_)
				ospFreeFrameBuffer(osp_framebuffer_);
			if (osp_world_)
				ospRelease(osp_world_);
			if (osp_camera_)
				ospRelease(osp_camera_);
			if (osp_renderer_)
				ospRelease(osp_renderer_);
		}

		// todo: implement
		GeometricObject* OSPRayRenderer::pickObject(Position x, Position y)
		{
			Log.info() << "pickObject called" << std::endl;
			return NULL;
		}

		// TODO: implement
		void OSPRayRenderer::pickObjects(Position x1, Position y1, Position x2, Position y2,
		                                 std::list<GeometricObject*>& objects)
		{
			Log.info() << "pickObjects called" << std::endl;
		}

		// todo: implement
		void OSPRayRenderer::setSize(float width, float height)
		{
			std::cout << "setting size" << std::endl;
			Renderer::setSize(width, height);

			if (osp_framebuffer_)
			{
				ospFreeFrameBuffer(osp_framebuffer_);
			}

			// todo: use the format used by the BALL framebuffer
      osp_framebuffer_ = ospNewFrameBuffer(osp::vec2i(width_, height_),OSP_RGBA_F32,OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
			ospFrameBufferClear(osp_framebuffer_, OSP_FB_ACCUM);

			ospSetf(osp_camera_, "aspect", width_/(float)height_);
			ospCommit(osp_camera_);
		}

		// todo: implement
		void OSPRayRenderer::setupStereo(float eye_separation, float focal_length)
		{
			Log.info() << "setupStereo called" << std::endl;
		}

		// todo: implement
		void OSPRayRenderer::getFrustum(float& near_f, float& far_f, float& left_f, float& right_f, float& top_f, float& bottom_f)
		{
			Log.info() << "getFrustum called" << std::endl;
		}

		// todo: implement
		void OSPRayRenderer::setFrustum(float near_f, float far_f, float left_f, float right_f, float top_f, float bottom_f)
		{
			Log.info() << "setFrustum called" << std::endl;
		}

		// todo: implement
		void OSPRayRenderer::setLights(bool reset_all)
		{
			std::cout << "reset all? " << reset_all << std::endl;

			// todo: currently, we always remove and re-add all light sources.
			//       we might want to change that.
			if (reset_all)
			{
				for (Position i=0; i<lights_.size(); ++i)
				{
				//	ospRelease(lights_[i]);
				}
			}

			Vector3 direction, light_position, attenuation;

			Size current_light=0;
			Size num_lights=lights_.size();

			lights_.clear();

			std::list<LightSource>::const_iterator it = stage_->getLightSources().begin();
			for (; it != stage_->getLightSources().end(); ++it, ++current_light)
			{
				OSPLight osp_light;
				switch (it->getType())
				{
					case LightSource::DIRECTIONAL:
						osp_light = ospNewLight(osp_renderer_, "DirectionalLight");
						break;
					case LightSource::POSITIONAL:
						osp_light = ospNewLight(osp_renderer_, "PointLight");
						break;
					default:
						std::cerr << "Light source type not supported!" << std::endl;
						break;
				}
				lights_.push_back(osp_light);

				switch (it->getType())
				{
					case LightSource::DIRECTIONAL:
						direction = it->getDirection();
						if (it->isRelativeToCamera())
							direction = stage_->calculateAbsoluteCoordinates(direction);

						ospSet3f(lights_[current_light], "direction", direction.x, direction.y, direction.z);
						break;
					case LightSource::POSITIONAL:
						light_position = it->getPosition();
						if (it->isRelativeToCamera())
						{
							light_position = stage_->calculateAbsoluteCoordinates(it->getPosition())+stage_->getCamera().getViewPoint();
						}
						std::cout << "position: " << light_position << std::endl;

						ospSet3f(lights_[current_light], "position", light_position.x, light_position.y, light_position.z);

						attenuation = it->getAttenuation();
						ospSet1f(lights_[current_light], "attenuation.constant",  attenuation.x);
						ospSet1f(lights_[current_light], "attenuation.linear",    attenuation.y);
						ospSet1f(lights_[current_light], "attenuation.quadratic", attenuation.z);
						break;
					default:
						std::cerr << "Light source type not supported!" << std::endl;
						break;
				}

				float intensity = it->getIntensity() * 100;
				ColorRGBA const& color = it->getColor();

				ospSet3f(lights_[current_light], "color", (float)color.getRed(), (float)color.getGreen(), (float)color.getBlue());
				ospSet1f(lights_[current_light], "intensity", intensity);
				std::cout << "intensity: "<< intensity << std::endl;

				ospCommit(lights_[current_light]);

				OSPData lights_array = ospNewData(lights_.size(), OSP_OBJECT, &lights_[0], 0);
				ospSetData(osp_renderer_, "lights", lights_array);

				ospCommit(osp_renderer_);
			}


			Log.info() << "setLights called" << std::endl;
		}

		void OSPRayRenderer::updateCamera(const Camera* camera)
		{
			// the renderer should be paused whenever the camera has been updated
			if (camera == 0) camera = &(stage_->getCamera());

			Vector3 const& position = camera->getViewPoint();

			// OSPRay relies on a normalized view vector, so we have to normalize it prior to handing it to RTfact
			// TODO: Store a normalized view vector in our Camera, mind project files
			Vector3  view_vector = camera->getViewVector();
			view_vector.normalize();
			Vector3 const& look_up = camera->getLookUpVector();

			if (doVectorsDiffer( last_camera_position, position ))
			{
				ospSetVec3f(osp_camera_, "pos", osp::vec3f(position.x,    position.y,    position.z));
				last_camera_position  = position;

				ospray_needs_update_ = true;
			}

			if (doVectorsDiffer( last_camera_view_vec, view_vector ))
			{
				ospSetVec3f(osp_camera_, "dir", osp::vec3f(view_vector.x, view_vector.y, view_vector.z));
				last_camera_view_vec  = view_vector;

				ospray_needs_update_ = true;
			}

			if (doVectorsDiffer( last_camera_lookup, look_up ))
			{
				ospSetVec3f(osp_camera_, "up",  osp::vec3f(look_up.x,     look_up.y,     look_up.z));
				last_camera_lookup    = look_up;

				ospray_needs_update_ = true;
			}

			ospSetf(osp_camera_, "aspect", width_/(float)height_);
			ospCommit(osp_camera_);

			if (lights_.size() != stage_->getLightSources().size() || lights_.empty()) return;

			// lights that are relative to the camera need to have their position updated
			std::list<LightSource>::const_iterator it = stage_->getLightSources().begin();
			Size current_light=0;
			Vector3 light_position, direction;
			bool changed_light = false;

			for (; it != stage_->getLightSources().end(); ++it, ++current_light)
			{
				switch (it->getType())
				{
					case LightSource::DIRECTIONAL:
						direction = it->getDirection();
						if (it->isRelativeToCamera()) {
							direction = stage_->calculateAbsoluteCoordinates(direction);
							ospSet3f(lights_[current_light], "direction", direction.x, direction.y, direction.z);

							ospCommit(lights_[current_light]);

							changed_light = true;
						}
						break;

					case LightSource::POSITIONAL:
						light_position = it->getPosition();
						if (it->isRelativeToCamera())
						{
							light_position = stage_->calculateAbsoluteCoordinates(it->getPosition())+stage_->getCamera().getViewPoint();
							ospSet3f(lights_[current_light], "position", light_position.x, light_position.y, light_position.z);

							ospCommit(lights_[current_light]);

							changed_light = true;
						}
						break;

					default:
						break;
				}
			}

			if (changed_light)
				ospCommit(osp_renderer_);
		}

		void OSPRayRenderer::updateBackgroundColor()
		{
			ColorRGBA const& bgColor = stage_->getBackgroundColor();

			ospSetVec3f(osp_renderer_, "bgColor", osp::vec3f(bgColor.getRed(), bgColor.getGreen(), bgColor.getBlue()));
			ospCommit(osp_renderer_);
		}

		// todo: implement
		void OSPRayRenderer::setupEnvironmentMap(const QImage& image)
		{
			Log.info() << "setupEnvironmentMap called" << std::endl;
		}

		void OSPRayRenderer::prepareBufferedRendering(const Stage& stage)
		{
		}

		// todo: implement
		void OSPRayRenderer::bufferRepresentationDynamic(const Representation& rep)
		{
			Log.info() << "bufferRepresentationDynamic called" << std::endl;
		}

		void OSPRayRenderer::bufferRepresentation(const Representation& rep)
		{
			if (rep.getGeometricObjects().empty())
				return;

			if (rep.isHidden())
			{
				if (rep.needsUpdate())
				{
					// if the representation has been changed while it was hidden, we need
					// to recreate it from scratch the next time it is enabled
					//
					// NOTE: it is safe to call removeRepresentation even if the representation
					//       has not yet been added
					removeRepresentation(rep);
				}
				return;
			}

			if (objects_.find(&rep) != objects_.end())
			{
				// was the representation previously disabled and now just needs enabling?
				if (objects_[&rep].has_been_disabled)
				{
					OSPRayData& rt_data = objects_[&rep];

					// iterate over all geometries and add them to OSPRay again to make them visible
					for (Position i=0; i<rt_data.geometries.size(); ++i)
					{
						ospAddGeometry(osp_world_, rt_data.geometries[i]);
					}

					ospCommit(osp_world_);

					objects_[&rep].has_been_disabled = false;
					ospray_needs_update_ = true;

					return;
				}
			}
			else
			{
				// TODO: handle the update more gracefully!
				removeRepresentation(rep);
			}

			OSPRayData osp_data;
			osp_data.has_been_disabled = false;

			Stage::Material rt_material = scene_->getStage()->getMaterial();
			if (rep.hasProperty("Rendering::Material"))
			{
				NamedProperty rt_mat_property = rep.getProperty("Rendering::Material");
				boost::shared_ptr<PersistentObject> mat_ptr = rt_mat_property.getSmartObject();
				rt_material = *dynamic_cast<Stage::Material*>(mat_ptr.get());
			}

			std::vector<float>       osp_sphere_data;
			std::vector<OSPMaterial> osp_sphere_materiallist;
			Position sphere_index = 0;

			std::vector<float>       osp_cylinder_data;
			std::vector<OSPMaterial> osp_cylinder_materiallist;
			Position cylinder_index = 0;

			// TODO: add handles to OSPRayData
			//       take care of all possible GeometricObjects
			std::list<GeometricObject*>::const_iterator it;
			for (it =  rep.getGeometricObjects().begin();
					 it != rep.getGeometricObjects().end();
					 it++)
			{
				if (RTTI::isKindOf<Mesh>(**it))
				{
					Mesh const& mesh = *(const Mesh*)*it;

					float const* vertices = reinterpret_cast<float const*>(&(mesh.vertex[0]));
					float const* normals  = reinterpret_cast<float const*>(&(mesh.normal[0]));
					Index const* indices  = reinterpret_cast<Index const*>(&(mesh.triangle[0]));

					// todo: put into OSPRayData

					OSPGeometry osp_mesh = ospNewTriangleMesh();
					osp_data.geometries.push_back(osp_mesh);

					OSPData osp_coords = ospNewData(mesh.vertex.size(), OSP_FLOAT3, vertices, OSP_DATA_SHARED_BUFFER);
					ospCommit(osp_coords);
					ospSetData(osp_mesh, "position", osp_coords);

					OSPData osp_indices = ospNewData(mesh.triangle.size(), OSP_INT3, indices, OSP_DATA_SHARED_BUFFER);
					ospCommit(osp_indices);
					ospSetData(osp_mesh, "index", osp_indices);

					OSPData osp_normals = ospNewData(mesh.normal.size(), OSP_FLOAT3, normals, OSP_DATA_SHARED_BUFFER);
					ospCommit(osp_normals);
					ospSetData(osp_mesh, "vertex.normal", osp_normals);

					//
					// TODO: is this conversion necessary? and how do we handle transparency if there is no alpha channel?
					std::vector<osp::vec3fa> new_colors(mesh.vertex.size());

					OSPData osp_colors;

					if (mesh.colors.size() > 1)
					{
						for (Position i=0; i<mesh.vertex.size(); ++i)
						{
							osp::vec3fa& new_color = new_colors[i];
							ColorRGBA const& ball_color = mesh.colors[i];

							new_color.x = ball_color.getRed();
							new_color.y = ball_color.getGreen();
							new_color.z = ball_color.getBlue();
						}
					}
					else
					{
						// todo: how to handle this correctly?
						ColorRGBA const &ball_color = (mesh.colors.size() == 1) ? mesh.colors[0] : ColorRGBA(1., 1., 1., 1.);

						for (Position i=0; i<mesh.vertex.size(); ++i)
						{
							osp::vec3fa& new_color = new_colors[i];

							new_color.x = ball_color.getRed();
							new_color.y = ball_color.getGreen();
							new_color.z = ball_color.getBlue();
						}
					}

					osp_colors = ospNewData(new_colors.size(), OSP_FLOAT3A, &(new_colors[0]));

					ospCommit(osp_colors);
					ospSetData(osp_mesh, "vertex.color", osp_colors);

					OSPMaterial mesh_material = ospNewMaterial(osp_renderer_, "OBJMaterial");
					// OBJ Kd defaults to blue in OSPRay...
					ospSet3f(mesh_material, "Kd", 1., 1., 1.);
					
					convertMaterial(rt_material, mesh_material);
					ospSetMaterial(osp_mesh, mesh_material);

					osp_data.materials.push_back(mesh_material);

//					geometric_objects_[rt_data.mesh_handles.back()] = *it;

					ospCommit(osp_mesh);

					if (!rep.isHidden())
					{
						ospAddGeometry(osp_world_, osp_mesh);
					}

					ospray_needs_update_ = true;
				} else if (RTTI::isKindOf<Sphere>(**it)) {
					Sphere const& sphere = *(const Sphere*)*it;

					Vector3 const& sphere_pos = sphere.getPosition();
					float radius = sphere.getRadius();

					ColorRGBA const& color = sphere.getColor();

					osp_sphere_data.push_back(sphere_pos.x);
					osp_sphere_data.push_back(sphere_pos.y);
					osp_sphere_data.push_back(sphere_pos.z);
					osp_sphere_data.push_back(radius);
					osp_sphere_data.push_back(0);
					*reinterpret_cast<uint32*>(&(osp_sphere_data[osp_sphere_data.size()-1])) = sphere_index;

					OSPMaterial sphere_material = ospNewMaterial(osp_renderer_, "OBJMaterial");
					convertMaterial(rt_material, sphere_material);
					ospSet3f(sphere_material, "Kd", color.getRed(), color.getGreen(), color.getBlue());
					ospCommit(sphere_material);
					osp_sphere_materiallist.push_back(sphere_material);

					osp_data.materials.push_back(sphere_material);

					sphere_index++;
				} else if (RTTI::isKindOf<TwoColoredTube>(**it) || RTTI::isKindOf<TwoColoredLine>(**it)) {
					Vector3 v1, v2, v3;
					ColorRGBA color1, color2;
					float radius;

					if (RTTI::isKindOf<TwoColoredTube>(**it))
					{
						TwoColoredTube const& tube = *(const TwoColoredTube*)*it;

						v1 = tube.getVertex1();
						v2 = tube.getMiddleVertex();
						v3 = tube.getVertex2();

						radius = tube.getRadius();

						color1 = tube.getColor();
						color2 = tube.getColor2();
					}
					else
					{
						TwoColoredLine const& line = *(const TwoColoredLine*)*it;

						v1 = line.getVertex1();
						v2 = line.getMiddleVertex();
						v3 = line.getVertex2();

						radius = LINE_RADIUS;

						color1 = line.getColor();
						color2 = line.getColor2();
					}

					// we always need the first tube, but the end is either the middle vertex, or the end,
					// depending on the colors
					osp_cylinder_data.push_back(v1.x);
					osp_cylinder_data.push_back(v1.y);
					osp_cylinder_data.push_back(v1.z);

					if (color1 == color2)
					{
						osp_cylinder_data.push_back(v3.x);
						osp_cylinder_data.push_back(v3.y);
						osp_cylinder_data.push_back(v3.z);
					}
					else
					{
						osp_cylinder_data.push_back(v2.x);
						osp_cylinder_data.push_back(v2.y);
						osp_cylinder_data.push_back(v2.z);
					}

					osp_cylinder_data.push_back(radius);

					osp_cylinder_data.push_back(0);
					*reinterpret_cast<uint32*>(&(osp_cylinder_data[osp_cylinder_data.size()-1])) = cylinder_index;

					OSPMaterial cylinder_material = ospNewMaterial(osp_renderer_, "OBJMaterial");
					convertMaterial(rt_material, cylinder_material);
					ospSet3f(cylinder_material, "Kd", color1.getRed(), color1.getGreen(), color1.getBlue());
					ospCommit(cylinder_material);
					osp_cylinder_materiallist.push_back(cylinder_material);

					osp_data.materials.push_back(cylinder_material);

					cylinder_index++;

					if (color1 != color2) // we need a second tube
					{
						osp_cylinder_data.push_back(v2.x);
						osp_cylinder_data.push_back(v2.y);
						osp_cylinder_data.push_back(v2.z);

						osp_cylinder_data.push_back(v3.x);
						osp_cylinder_data.push_back(v3.y);
						osp_cylinder_data.push_back(v3.z);

						osp_cylinder_data.push_back(radius);

						osp_cylinder_data.push_back(0);
						*reinterpret_cast<uint32*>(&(osp_cylinder_data[osp_cylinder_data.size()-1])) = cylinder_index;

						OSPMaterial cylinder_material = ospNewMaterial(osp_renderer_, "OBJMaterial");
						convertMaterial(rt_material, cylinder_material);
						ospSet3f(cylinder_material, "Kd", color2.getRed(), color2.getGreen(), color2.getBlue());
						ospCommit(cylinder_material);
						osp_cylinder_materiallist.push_back(cylinder_material);

						osp_data.materials.push_back(cylinder_material);

						cylinder_index++;
					} 
				}
			}

			if (sphere_index > 0)
			{
				OSPGeometry osp_sphere = ospNewGeometry("spheres");

				// encode position and radius of the spheres
				ospSet1i(osp_sphere, "bytes_per_sphere", 5*sizeof(float));
				ospSet1i(osp_sphere, "offset_radius", 3*sizeof(float));
				ospSet1i(osp_sphere, "offset_materialID", 4*sizeof(float));

				OSPData osp_new_sphere_data = ospNewData(5*sphere_index, OSP_FLOAT, &(osp_sphere_data[0]));
				ospCommit(osp_new_sphere_data);
				ospSetData(osp_sphere, "spheres", osp_new_sphere_data);

				OSPData osp_sphere_material_data = ospNewData(sphere_index, OSP_OBJECT, &(osp_sphere_materiallist[0]), 0);
				ospCommit(osp_sphere_material_data);
				ospSetData(osp_sphere, "materialList", osp_sphere_material_data);

				ospCommit(osp_sphere);

				if (!rep.isHidden())
				{
					ospAddGeometry(osp_world_, osp_sphere);
				}

				osp_data.geometries.push_back(osp_sphere);

				ospray_needs_update_ = true;
			}

			if (cylinder_index > 0)
			{
				OSPGeometry osp_cylinder = ospNewGeometry("cylinders");

				// encode position and radius of the cylinders
				ospSet1i(osp_cylinder, "bytes_per_cylinder", 8*sizeof(float));
				ospSet1i(osp_cylinder, "offset_radius", 6*sizeof(float));
				ospSet1i(osp_cylinder, "offset_materialID", 7*sizeof(float));

				OSPData osp_new_cylinder_data = ospNewData(8*cylinder_index, OSP_FLOAT, &(osp_cylinder_data[0]));
				ospCommit(osp_new_cylinder_data);
				ospSetData(osp_cylinder, "cylinders", osp_new_cylinder_data);

				OSPData osp_cylinder_material_data = ospNewData(cylinder_index, OSP_OBJECT, &(osp_cylinder_materiallist[0]), 0);
				ospCommit(osp_cylinder_material_data);
				ospSetData(osp_cylinder, "materialList", osp_cylinder_material_data);

				ospCommit(osp_cylinder);

				if (!rep.isHidden())
				{
					ospAddGeometry(osp_world_, osp_cylinder);
				}

				osp_data.geometries.push_back(osp_cylinder);

				ospray_needs_update_ = true;
			}

			ospCommit(osp_world_);

			// todo: handle hidden instances
			objects_[&rep] = osp_data;
			
			std::cout << "bufferRepresentation called" << std::endl;
		}

		// todo: implement
		void OSPRayRenderer::removeRepresentation(Representation const& rep)
		{
			if (objects_.find(&rep) != objects_.end())
			{
				// TODO: find out if this also deletes the geometries and materials
				OSPRayData& osp_data = objects_[&rep];

				for (Position i=0; i<osp_data.geometries.size(); ++i)
				{
					ospRemoveGeometry(osp_world_, osp_data.geometries[i]);
				}

				ospCommit(osp_world_);

				osp_data.geometries.clear();
				osp_data.materials.clear();

				ospray_needs_update_ = true;

				objects_.erase(&rep);
			}
			Log.info() << "removeRepresentation called" << std::endl;
		}

		// todo: implement. What do we need to do in OSPRay for this
		void OSPRayRenderer::useContinuousLoop(bool use_loop)
		{
    	Renderer::useContinuousLoop(use_loop);
			Log.info() << "useContinuousLoop called" << std::endl;
		}

		// todo: implement
		void OSPRayRenderer::renderToBufferImpl(FrameBufferPtr buffer)
		{
			// deactivate hidden representations (we need no reactivation code,
			// since reactivated representations will simply be buffered again)
			for (HashMap<Representation const*, OSPRayData>::iterator it = objects_.begin(); it != objects_.end(); ++it)
			{
				if (it->first->isHidden() && !it->second.has_been_disabled)
				{
					OSPRayData& rt_data = it->second;

					for (Position i=0; i<rt_data.geometries.size(); ++i)
					{
						ospRemoveGeometry(osp_world_, rt_data.geometries[i]);
					}

					ospCommit(osp_world_);

					it->second.has_been_disabled = true;

					ospray_needs_update_ = true;
				}
			}
			
			// TODO: only clear if viewport was modified
			if (ospray_needs_update_)
			{
				ospFrameBufferClear(osp_framebuffer_, OSP_FB_ACCUM);
			}

			ospRenderFrame(osp_framebuffer_, osp_renderer_, OSP_FB_COLOR|OSP_FB_ACCUM);

			// todo: handle different formats
			buffer->setData((float*)ospMapFrameBuffer(osp_framebuffer_, OSP_FB_COLOR));

			// todo: unmap!
			ospray_needs_update_ = false;
		}

		void OSPRayRenderer::updateMaterialForRepresentation(Representation const* rep)
		{
			Log.info() << "updateMaterialForRepresentation called" << std::endl;

			if (objects_.find(rep) != objects_.end())
			{
				OSPRayData& rt_data = objects_[rep];

				for (Position i=0; i<rt_data.materials.size(); ++i)
				{
					if (rep->hasProperty("Rendering::Material"))
					{
						NamedProperty rt_mat_property = rep->getProperty("Rendering::Material");
						boost::shared_ptr<PersistentObject> mat_ptr = rt_mat_property.getSmartObject();
						convertMaterial(*dynamic_cast<Stage::Material*>(mat_ptr.get()), rt_data.materials[i]);
					}
					else
						updateMaterialFromStage(rt_data.materials[i]);
				}
			}
		}

		void OSPRayRenderer::updateMaterialFromStage(OSPMaterial& material)
		{
			Stage::Material const& rt_material = scene_->getStage()->getMaterial();
			convertMaterial(rt_material, material);
		}

		void OSPRayRenderer::convertMaterial(Stage::Material const& rt_material, OSPMaterial& osp_material)
		{
			// diffuse
			// ospSet3f(osp_material, "Kd", 1., 1., 1.);

			// ambience
			float red   = (float)rt_material.ambient_color.getRed()   * rt_material.ambient_intensity;
			float green = (float)rt_material.ambient_color.getGreen() * rt_material.ambient_intensity;
			float blue  = (float)rt_material.ambient_color.getBlue()  * rt_material.ambient_intensity;

			ospSet3f(osp_material, "Ka", red, green, blue);	

			// specularity
			red   = (float)rt_material.specular_color.getRed()   * rt_material.specular_intensity;
			green = (float)rt_material.specular_color.getGreen() * rt_material.specular_intensity;
			blue  = (float)rt_material.specular_color.getBlue()  * rt_material.specular_intensity;

			ospSet3f(osp_material, "Ks", red, green, blue);	

			// reflectiveness
			red   = (float)rt_material.reflective_color.getRed()   * rt_material.reflective_intensity;
			green = (float)rt_material.reflective_color.getGreen() * rt_material.reflective_intensity;
			blue  = (float)rt_material.reflective_color.getBlue()  * rt_material.reflective_intensity;

			//ospSet3f(osp_material, 

			// shininess
			ospSetf(osp_material, "Ns", rt_material.shininess);

			// transparency            
			ospSetf(osp_material, "d", 1.-rt_material.transparency/100.);

			ospCommit(osp_material);	
		}

		// todo: implement
		std::vector<float> OSPRayRenderer::intersectRaysWithGeometry(const std::vector<Vector3>& origins,
		                                                             const std::vector<Vector3>& directions)
		{
			Log.info() << "intersectRaysWithGeometry called" << std::endl;
		}

		bool OSPRayRenderer::doVectorsDiffer(const Vector3 &vecA, const Vector3 &vecB)
		{
			return ((vecA - vecB).getSquareLength() > vectorDifferenceTolerance_);
		}
	}

}
