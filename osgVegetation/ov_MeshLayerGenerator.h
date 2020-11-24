#pragma once
#include "ov_GPUCullData.h"
#include "ov_MeshLayerConfig.h"
#include "ov_Utils.h"
#include <osg/CullFace>
#include <osg/io_utils>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

namespace osgVegetation
{
	//Hack to avoid gl-crash on first update
	static bool _firstTerrainDrawDone = false;

	class MeshLayerGenerator
	{
	private:

		// We must ensure that cull shader finished filling indirect commands and indirect targets, before draw shader
		// starts using them. We use glMemoryBarrier() barrier to achieve that.
		// It is also possible that we should use glMemoryBarrier() after resetting textures, but i implemented that only for
		// dynamic rendering.
		struct TerrainGeometryDrawCB : public osg::Drawable::DrawCallback
		{
			TerrainGeometryDrawCB(GLbitfield barriers)
				: _barriers(barriers)
			{
			}

			virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const
			{
				//std::cout << "Terrain Draw\n";
				renderInfo.getState()->get<osg::GLExtensions>()->glMemoryBarrier(_barriers);
				drawable->drawImplementation(renderInfo);
				_firstTerrainDrawDone = true;
			}
			GLbitfield _barriers;
		};

		struct InstanceGeometryDrawCB : public osg::Drawable::DrawCallback
		{
			InstanceGeometryDrawCB(osg::TextureBuffer* tex, osg::BindImageTexture* binding) : _texture(tex), _binding(binding)
			{

			}

			virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const
			{
				if (_firstTerrainDrawDone)
				{
					renderInfo.getState()->get<osg::GLExtensions>()->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
					drawable->drawImplementation(renderInfo);

					if (osg::BufferData* img = const_cast<osg::BufferData*>(_texture->getBufferData()))
						img->dirty();
				}
			}
			osg::TextureBuffer* _texture;
			osg::BindImageTexture* _binding;
		};

		class DrawCallbackVisitor : public osg::NodeVisitor
		{
		public:
			DrawCallbackVisitor() :
				osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
			}

			void apply(osg::Node& node)
			{
				osg::Geometry* geom = dynamic_cast<osg::Geometry*>(&node);
				if (geom)
				{
					if (!geom->getDrawCallback())
						geom->setDrawCallback(new TerrainGeometryDrawCB(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT));
				}
				else
				{
					traverse(node);
				}
			}
		};


		struct BoundingBoxCB : public osg::Drawable::ComputeBoundingBoxCallback
		{
			BoundingBoxCB() {}
			BoundingBoxCB(const osg::BoundingBox &bbox) : _bbox(bbox) {};
			osg::BoundingBox computeBound(const osg::Drawable&) const { return _bbox; }
		private:
			osg::BoundingBox _bbox;
		};
	public:
		MeshLayerGenerator(const MeshLayerConfig& config) : m_MaxMeshHeight(0)
		{
			GPUCullData* gpuData = _CreateGPUData(config);
			m_TerrainStateSet = _CreateTerrainStateSet(gpuData);

			if (config.TerrainModulatedIntensity)
				m_TerrainStateSet->setDefine("OV_TERRAIN_MODULATED_INTENSITY");
			//apply filters
			config.Filter.Apply(m_TerrainStateSet);

			const double target_instance_area = 1.0 / config.Density;
			const float target_tri_side_lenght = static_cast<float>(GetEquilateralTriangleSideLengthFromArea(target_instance_area));
			osg::Uniform* targetTriangleSide = new osg::Uniform(osg::Uniform::FLOAT, "ov_TargetTriangleSide");
			targetTriangleSide->set(target_tri_side_lenght);
			m_TerrainStateSet->addUniform(targetTriangleSide);

			m_InstanceGroup = _CreateInstanceGroup(gpuData);

			if (config.BackFaceCulling)
			{
				m_InstanceGroup->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));
				m_InstanceGroup->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
			}
			else
				m_InstanceGroup->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

			if(config.OverrideNormals)
				m_InstanceGroup->getOrCreateStateSet()->setDefine("OV_OVERRIDE_NORMALS");

			unsigned int node_mask = m_InstanceGroup->getNodeMask();

			if (config.CastShadow)
				node_mask |= Register.CastsShadowTraversalMask;
			else
				node_mask &= ~Register.CastsShadowTraversalMask;

			//if (config.ReceiveShadow)
			//	node_mask |= Register.ReceivesShadowTraversalMask;
			//else
			//	node_mask &= ~Register.ReceivesShadowTraversalMask;

			m_InstanceGroup->setNodeMask(node_mask);
			m_InstanceGroup->getOrCreateStateSet()->addUniform(new osg::Uniform("osg_ReceiveShadow", config.ReceiveShadow));
			
			delete gpuData;
		}

		class PaddBoundsVisitor : public osg::NodeVisitor
		{
		public:
			PaddBoundsVisitor(double padding = 40) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _padding(padding)
			{}

			void apply(osg::Drawable& drawable)
			{
				osg::BoundingBoxd bb = drawable.getBoundingBox();
				bb.zMax() += _padding;
				drawable.setInitialBound(bb);
			}
		protected:
			double _padding;
		};


		osg::Group* CreateMeshNode(osg::Node* terrain_geometry) const
		{
			osg::Group* group = new osg::Group();
			group->getOrCreateStateSet()->setRenderBinDetails(1, "TraversalOrderBin", osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
			osg::Group* veg_group = new osg::Group();
			veg_group->addChild(terrain_geometry);
			veg_group->setStateSet(m_TerrainStateSet);

			PaddBoundsVisitor padding(m_MaxMeshHeight);
			terrain_geometry->accept(padding);

			//inject draw callbacks
			DrawCallbackVisitor scbv;
			terrain_geometry->accept(scbv);
			group->addChild(veg_group);
			group->addChild(m_InstanceGroup);
			group->setNodeMask(m_InstanceGroup->getNodeMask());
			return group;
		}
	private:

		GPUCullData* _CreateGPUData(const MeshLayerConfig &mesh_data)
		{
			GPUCullData* gpuData = new GPUCullData();
			const bool useMultiDrawArraysIndirect = true;
			gpuData->setUseMultiDrawArraysIndirect(useMultiDrawArraysIndirect);

			osg::ref_ptr < osg::Program> drawProgram = new osg::Program;
			drawProgram->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_mesh_render_vertex.glsl")));
			drawProgram->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_shadow_vertex.glsl")));
			drawProgram->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_terrain_color.glsl")));
			drawProgram->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_mesh_render_fragment.glsl")));
			drawProgram->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_common_fragment.glsl")));
			drawProgram->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_shadow_fragment.glsl")));
			gpuData->registerIndirectTarget(0, new AggregateGeometryVisitor(new ConvertTrianglesOperatorClassic()), drawProgram);

			float acc_probability = 0;
			for (size_t i = 0; i < mesh_data.MeshTypes.size(); i++)
			{
				acc_probability += mesh_data.MeshTypes[i].Probability;
			}

			for (size_t i = 0; i < mesh_data.MeshTypes.size(); i++)
			{
				for (size_t j = 0; j < mesh_data.MeshTypes[i].MeshLODs.size(); j++)
				{
					osg::ref_ptr<osg::Node> mesh = osgDB::readNodeFile(mesh_data.MeshTypes[i].MeshLODs[j].Mesh);

					const float norm_prob = acc_probability > 0 ? mesh_data.MeshTypes[i].Probability / acc_probability : 1.0f / mesh_data.MeshTypes.size();
					const float density = mesh_data.Density * norm_prob;
					const float max_dist = mesh_data.DynamicLODMaxDistanceRatio * mesh_data.MeshTypes[i].MeshLODs.back().Distance.w();
					gpuData->registerType(i, 
						0, 
						mesh.get(), 
						density,
						max_dist,
						norm_prob,
						mesh_data.MeshTypes[i].MeshLODs[j]);
				}
				if (i < gpuData->instanceTypes->getData().size())
				{
					InstanceType& itd = gpuData->instanceTypes->getData().at(i);
					itd.floatParams.x() = mesh_data.MeshTypes[i].IntensityVariation;
					itd.floatParams.y() = mesh_data.MeshTypes[i].ScaleVariation;
					itd.floatParams.z() = mesh_data.MeshTypes[i].DiffuseIntensity;
					itd.floatParams.w() = mesh_data.MeshTypes[i].Scale;
				}
			}

			

			// every target will store 6 rows of data in GL_RGBA32F_ARB format ( the whole StaticInstance structure )
			gpuData->endRegister(6, GL_RGBA, GL_FLOAT, GL_RGBA32F_ARB);

			m_MaxMeshHeight = gpuData->_maxHeight;
			// in the end - we create OSG objects that draw instances using indirect targets and commands.
			std::map<unsigned int, IndirectTarget>::iterator it, eit;
			for (it = gpuData->targets.begin(), eit = gpuData->targets.end(); it != eit; ++it)
			{
				//it->second.geometryAggregator->getAggregatedGeometry()->setDrawCallback(new InvokeMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT));
				it->second._geometryAggregator->getAggregatedGeometry()->setDrawCallback(new InstanceGeometryDrawCB(it->second._indirectCommandTextureBuffer, it->second._indirectCommandImageBinding));
				it->second._geometryAggregator->getAggregatedGeometry()->setComputeBoundingBoxCallback(new BoundingBoxCB());
				it->second._geometryAggregator->getAggregatedGeometry()->setCullingActive(false);
			}
			return gpuData;
		}

		osg::Group* _CreateInstanceGroup(GPUCullData* gpuData)
		{
			osg::Group* group = new osg::Group();
			// in the end - we create OSG objects that draw instances using indirect targets and commands.
			std::map<unsigned int, IndirectTarget>::iterator it, eit;
			for (it = gpuData->targets.begin(), eit = gpuData->targets.end(); it != eit; ++it)
			{
				osg::ref_ptr<osg::Geode> drawGeode = new osg::Geode;
				drawGeode->addDrawable(it->second._geometryAggregator->getAggregatedGeometry());
				it->second.addIndirectTargetData(false, "ov_indirectTarget", it->first, drawGeode->getOrCreateStateSet());
				drawGeode->getOrCreateStateSet()->setAttributeAndModes(gpuData->instanceTypesUBB.get());
				it->second.addDrawProgram("ov_instanceTypesData", drawGeode->getOrCreateStateSet());
				group->addChild(drawGeode);
			}
			return group;
		}

		osg::StateSet* _CreateTerrainStateSet(GPUCullData* gpuData)
		{
			osg::StateSet* terrain_ss = new osg::StateSet();
			// instance OSG tree is connected to cull shader with all necessary data ( all indirect commands, all
			// indirect targets, necessary OpenGl modes etc. )
			{
				osg::ref_ptr < osg::Program> cullProgram = new osg::Program;
				cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_mesh_cull_vertex.glsl")));
				cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_mesh_cull_tess_ctrl.glsl")));
				cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_mesh_cull_tess_eval.glsl")));
				cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_mesh_cull_geometry.glsl")));
				cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_shadow_vertex.glsl")));
				cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_terrain_elevation.glsl")));
				cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_terrain_color.glsl")));
				cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_terrain_pass_filter.glsl")));
				

#if 0
				cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_mesh_cull_fragment.glsl")));
#else
				terrain_ss->setMode(GL_RASTERIZER_DISCARD, osg::StateAttribute::ON);
#endif
				cullProgram->addBindUniformBlock("ov_instanceTypesData", 1);
				terrain_ss->setAttributeAndModes(cullProgram.get(), osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
				terrain_ss->setAttributeAndModes(gpuData->instanceTypesUBB.get());

				std::map<unsigned int, IndirectTarget>::iterator it, eit;
				for (it = gpuData->targets.begin(), eit = gpuData->targets.end(); it != eit; ++it)
				{
					it->second.addIndirectCommandData("ov_indirectCommand", it->first, terrain_ss);
					it->second.addIndirectTargetData(true, "ov_indirectTarget", it->first, terrain_ss);
				}

				osg::Uniform* indirectCommandSize = new osg::Uniform(osg::Uniform::INT, "ov_indirectCommandSize");
				indirectCommandSize->set((int)(sizeof(DrawArraysIndirectCommand) / sizeof(unsigned int)));
				terrain_ss->addUniform(indirectCommandSize);

				osg::Uniform* numInstanceTypesUniform = new osg::Uniform(osg::Uniform::INT, "ov_numInstanceTypes");
				int num_instance_types = static_cast<int>(gpuData->instanceTypes->getData().size());
				numInstanceTypesUniform->set(num_instance_types);
				terrain_ss->addUniform(numInstanceTypesUniform);
			}
			return terrain_ss;
		}
	private:
		osg::ref_ptr<osg::StateSet>  m_TerrainStateSet;
		osg::ref_ptr<osg::Group> m_InstanceGroup;
		double m_MaxMeshHeight;
	};

	class MeshMultiLayerGenerator
	{
	public:
		MeshMultiLayerGenerator(std::vector<MeshLayerConfig> layers)
		{
			for (size_t i = 0; i < layers.size(); i++)
			{
				if(layers[i].Enable)
					m_MeshGenerators.push_back(MeshLayerGenerator(layers[i]));
			}
		}

		osg::ref_ptr<osg::Group> CreateMeshNode(osg::ref_ptr<osg::Node> terrain_geometry)
		{
			osg::ref_ptr<osg::Group> root = new osg::Group();
			for (size_t i = 0; i < m_MeshGenerators.size(); i++)
			{
				osg::Group* mesh_node = m_MeshGenerators[i].CreateMeshNode(terrain_geometry);
				root->addChild(mesh_node);
			}
			return root;
		}
	private:
		std::vector<MeshLayerGenerator> m_MeshGenerators;
		
	};
}