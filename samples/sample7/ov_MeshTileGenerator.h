#pragma once
#include "ov_GPUCullData.h"
#include "ov_MeshTileGeneratorConfig.h"
#include <osg/CullFace>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>


class MeshTileGenerator
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
			//DrawIndirectGLExtensions *ext = DrawIndirectGLExtensions::getExtensions( renderInfo.getContextID(), true );
			renderInfo.getState()->get<osg::GLExtensions>()->glMemoryBarrier(_barriers);
			drawable->drawImplementation(renderInfo);
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
			//osg::BufferData* img = const_cast<osg::BufferData*>(_texture->getBufferData());
			//if (img != NULL)
			//{
				//img->dirty();
				//_binding->apply(*renderInfo.getState());
				//_texture->dirtyTextureParameters();
				//apply(*renderInfo.getState());
			//}
			//_texture->dirtyTextureObject();
			//_texture->dirtyTextureParameters();
			//_texture->apply(*renderInfo.getState());
			
			renderInfo.getState()->get<osg::GLExtensions>()->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
			drawable->drawImplementation(renderInfo);

			if(osg::BufferData* img = const_cast<osg::BufferData*>(_texture->getBufferData()))
				img->dirty();
		}
		osg::TextureBuffer* _texture;
		osg::BindImageTexture* _binding;
	};

	struct BoundingBoxCB : public osg::Drawable::ComputeBoundingBoxCallback
	{
		BoundingBoxCB() {}
		BoundingBoxCB(const osg::BoundingBox &bbox) : _bbox(bbox) {};
		osg::BoundingBox computeBound(const osg::Drawable&) const { return _bbox; }
	private:
		osg::BoundingBox _bbox;
	};

	struct BoundingSphereCB : public osg::Drawable::ComputeBoundingSphereCallback
	{
		BoundingSphereCB() {}
		BoundingSphereCB(const osg::BoundingSphere &bs) : _bounds(bs) {};
		osg::BoundingSphere computeBound(const osg::Node&) const { return _bounds; }
	private:
		osg::BoundingSphere _bounds;
	};
public:
	MeshTileGenerator(const MeshTileGeneratorConfig &mesh_data)
	{
		const bool useMultiDrawArraysIndirect = true;
		m_GpuData.setUseMultiDrawArraysIndirect(useMultiDrawArraysIndirect);
		_SetupGPUData(mesh_data);
		_terrainSS = _CreateTerrainState();
	}

	osg::Node* CreateNode(osg::Geometry* terrain_geometry)
	{
		osg::Group* group = new osg::Group();
		group->getOrCreateStateSet()->setRenderBinDetails(1, "TraversalOrderBin", osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
		osg::Geode* terrain_geode = new osg::Geode();
		terrain_geode->addDrawable(terrain_geometry);
		osg::BoundingSphere bs = terrain_geode->getBound();
		terrain_geometry->setStateSet(_terrainSS);
		terrain_geometry->setDrawCallback(new TerrainGeometryDrawCB(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT));
		group->addChild(terrain_geode);
		osg::ref_ptr<osg::Group> instance_group = _CreateInstanceGroup(bs);
		group->addChild(instance_group);
		return group;
	}
private:
	void _SetupGPUData(const MeshTileGeneratorConfig &mesh_data)
	{
		osg::ref_ptr < osg::Program> drawProgram = new osg::Program;
		//stateset->setAttribute(program, osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
		drawProgram->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_mesh_render_vertex.glsl")));
		drawProgram->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_mesh_render_fragment.glsl")));
		m_GpuData.registerIndirectTarget(0, new AggregateGeometryVisitor(new ConvertTrianglesOperatorClassic()), drawProgram);
		//m_GpuData.registerIndirectTarget(1, new AggregateGeometryVisitor(new ConvertTrianglesOperatorClassic()), drawProgram);

		for (size_t i = 0; i < mesh_data.MeshTypes.size(); i++)
		{
			for (size_t j = 0; j < mesh_data.MeshTypes[i].MeshLODs.size(); j++)
			{

#if 1
				osg::ref_ptr<osg::Node> mesh = osgDB::readNodeFile(mesh_data.MeshTypes[i].MeshLODs[j].Mesh);
#else
				osg::ref_ptr<osg::Node> mesh;
				if (mesh_data[i].MeshLODs[j].Mesh == "LOD0") mesh = createConiferTree(0.75f, osg::Vec4(1.0, 1.0, 1.0, 1.0), osg::Vec4(0.0, 1.0, 0.0, 1.0));
				else if (mesh_data[i].MeshLODs[j].Mesh == "LOD1") mesh = createConiferTree(0.45f, osg::Vec4(0.0, 0.0, 1.0, 1.0), osg::Vec4(1.0, 1.0, 0.0, 1.0));
				else if (mesh_data[i].MeshLODs[j].Mesh == "LOD2") mesh = createConiferTree(0.15f, osg::Vec4(1.0, 0.0, 0.0, 1.0), osg::Vec4(0.0, 0.0, 1.0, 1.0));
				else mesh = osgDB::readNodeFile(mesh_data[i].MeshLODs[j].Mesh);
#endif
				m_GpuData.registerType(i, 0, mesh.get(), mesh_data.MeshTypes[i].MeshLODs[j].Distance, mesh_data.Density);
			}
		}
		// every target will store 6 rows of data in GL_RGBA32F_ARB format ( the whole StaticInstance structure )
		m_GpuData.endRegister(6, GL_RGBA, GL_FLOAT, GL_RGBA32F_ARB);

		// in the end - we create OSG objects that draw instances using indirect targets and commands.
		std::map<unsigned int, IndirectTarget>::iterator it, eit;
		for (it = m_GpuData.targets.begin(), eit = m_GpuData.targets.end(); it != eit; ++it)
		{
			//it->second.geometryAggregator->getAggregatedGeometry()->setDrawCallback(new InvokeMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT));
			it->second.geometryAggregator->getAggregatedGeometry()->setDrawCallback(new InstanceGeometryDrawCB(m_GpuData.targets.at(0).indirectCommandTextureBuffer, m_GpuData.targets.at(0).indirectCommandImageBinding));
			it->second.geometryAggregator->getAggregatedGeometry()->setComputeBoundingBoxCallback(new BoundingBoxCB());
			it->second.geometryAggregator->getAggregatedGeometry()->setCullingActive(false);
		}
	}

	osg::Group*  _CreateInstanceGroup(osg::BoundingSphere terrain_bs)
	{
		osg::Group* group = new osg::Group();
		// in the end - we create OSG objects that draw instances using indirect targets and commands.
		std::map<unsigned int, IndirectTarget>::iterator it, eit;
		for (it = m_GpuData.targets.begin(), eit = m_GpuData.targets.end(); it != eit; ++it)
		{
			osg::ref_ptr<osg::Geode> drawGeode = new osg::Geode;
			drawGeode->addDrawable(it->second.geometryAggregator->getAggregatedGeometry());
			//drawGeode->setCullingActive(false);
			drawGeode->setComputeBoundingSphereCallback(new BoundingSphereCB(terrain_bs));

			it->second.addIndirectTargetData(false, "ov_indirectTarget", it->first, drawGeode->getOrCreateStateSet());
			drawGeode->getOrCreateStateSet()->setAttributeAndModes(m_GpuData.instanceTypesUBB.get());
			it->second.addDrawProgram("ov_instanceTypesData", drawGeode->getOrCreateStateSet());
			drawGeode->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK));
			//it->second.geometryAggregator->getAggregatedGeometry()->setComputeBoundingBoxCallback(new StaticBoundingBox());
			group->addChild(drawGeode);
		}
		return group;
	}

	osg::StateSet* _CreateTerrainState()
	{
		osg::StateSet* terrain_ss = new osg::StateSet();
		// instance OSG tree is connected to cull shader with all necessary data ( all indirect commands, all
		// indirect targets, necessary OpenGl modes etc. )
		{
			//osg::ref_ptr<ResetTexturesCallback> resetTexturesCallback = new ResetTexturesCallback;
			osg::ref_ptr < osg::Program> cullProgram = new osg::Program;
			cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("ov_mesh_cull_vertex.glsl")));
			cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("ov_mesh_cull_tess_ctrl.glsl")));
			cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("ov_mesh_cull_tess_eval.glsl")));
			cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("ov_mesh_cull_geometry.glsl")));
#if 0
			cullProgram->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("ov_mesh_cull_fragment.glsl")));
#else
			terrain_ss->setMode(GL_RASTERIZER_DISCARD, osg::StateAttribute::ON);
#endif
			cullProgram->addBindUniformBlock("ov_instanceTypesData", 1);
			terrain_ss->setAttributeAndModes(cullProgram.get(), osg::StateAttribute::ON);
			terrain_ss->setAttributeAndModes(m_GpuData.instanceTypesUBB.get());

			std::map<unsigned int, IndirectTarget>::iterator it, eit;
			for (it = m_GpuData.targets.begin(), eit = m_GpuData.targets.end(); it != eit; ++it)
			{
				it->second.addIndirectCommandData("ov_indirectCommand", it->first, terrain_ss);
				//resetTexturesCallback->addTextureDirty(it->first);
				//resetTexturesCallback->addTextureDirtyParams(it->first);

				it->second.addIndirectTargetData(true, "ov_indirectTarget", it->first, terrain_ss);
				//resetTexturesCallback->addTextureDirtyParams(OSGGPUCULL_MAXIMUM_INDIRECT_TARGET_NUMBER + it->first);
			}

			osg::Uniform* indirectCommandSize = new osg::Uniform(osg::Uniform::INT, "ov_indirectCommandSize");
			indirectCommandSize->set((int)(sizeof(DrawArraysIndirectCommand) / sizeof(unsigned int)));
			terrain_ss->addUniform(indirectCommandSize);

			osg::Uniform* numInstanceTypesUniform = new osg::Uniform(osg::Uniform::INT, "ov_numInstanceTypes");
			int num_instance_types = static_cast<int>(m_GpuData.instanceTypes->getData().size());
			numInstanceTypesUniform->set(num_instance_types);
			terrain_ss->addUniform(numInstanceTypesUniform);
			//terrain_ss->setUpdateCallback(resetTexturesCallback.get());
		}
		return terrain_ss;
	}
private:
	osg::Node* _terrain;
	osg::ref_ptr<osg::StateSet>  _terrainSS;
	GPUCullData m_GpuData;
};