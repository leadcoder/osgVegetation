#pragma once

#include "XBFInstance.h"
#include "XBFVegetationData.h"
#include <osgTerrain/TerrainTile>

#ifndef GL_TRANSFORM_FEEDBACK_BUFFER
#define GL_TRANSFORM_FEEDBACK_BUFFER      0x8C8E
#endif

#ifndef TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN
#define TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8c88
#endif

#ifndef GL_PRIMITIVES_GENERATED
#define GL_PRIMITIVES_GENERATED           0x8C87
#endif

class XBFInstanceGenerator : public osg::Geometry
{
public:
	XBFInstanceGenerator(const std::vector<XBFInstance*> &instances,
		osgTerrain::TerrainTile& tile, 
		const XBFVegetationData& data)
	{
		setUseVertexBufferObjects(true);
		setUseDisplayList(false);

		const osgTerrain::HeightFieldLayer* layer = dynamic_cast<const osgTerrain::HeightFieldLayer*>(tile.getElevationLayer());
		if (!layer)
			throw std::runtime_error("RenderToInstanceGroup::RenderToInstanceGroup - Tile has no HeightFieldLayer ");
		const osg::HeightField* hf = layer->getHeightField();
		if(!hf)
			throw std::runtime_error("RenderToInstanceGroup::RenderToInstanceGroup - HeightFieldLayer has no HeightField");

		_createPatchGeomtryFromHF(hf, this);

		// Add a custom draw callback that will render the control points 
		// into the the XF buffer. (Must add this callbacks AFTER setting up the geometry)
		getOrCreateStateSet()->setAttribute(makeGeneratorProgram());
	
		getOrCreateStateSet()->setRenderBinDetails(-1,"RenderBin");
		for (size_t l = 0; l < instances.size(); l++)
		{
			LODData data;
			osg::Array* xfb = instances[l]->getControlPoints();
			data._vbo = xfb->getVertexBufferObject();
			int numBufferData = data._vbo->getNumBufferData();
			data._offset = 0;
			data._size = 0;
			for (int i = 0; i < numBufferData; ++i)
			{
				data._size = data._vbo->getBufferData(i)->getTotalDataSize();
				if (data._vbo->getBufferData(i)->getDataPointer() == xfb->getDataPointer())
					break;
				else
					data._offset += data._size;
			}

			data._instance = instances[l];
			data._queries.resize(64);
			for (size_t i = 0; i < 64; i++)
			{
				data._queries[i] = INT_MAX;
			}
			_LODLevels.push_back(data);
		}

		//Add color texture
		osgTerrain::Layer* colorLayer = tile.getColorLayer(0);
		if (colorLayer)
		{
			osg::Image* image = colorLayer->getImage();
			osg::Texture2D* texture = new osg::Texture2D(image);
			getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
			getOrCreateStateSet()->addUniform(new osg::Uniform("baseTexture", 0));
		}

		osgTerrain::Layer* colorLayer1 = tile.getColorLayer(1);
		if (colorLayer1)
		{
			osg::Image* image = colorLayer1->getImage();
			osg::Texture2D* texture = new osg::Texture2D(image);
			getOrCreateStateSet()->setTextureAttributeAndModes(1, texture, osg::StateAttribute::ON);
			getOrCreateStateSet()->addUniform(new osg::Uniform("landCoverTexture", 1));
		}

		getOrCreateStateSet()->addUniform(new osg::Uniform("vegMaxDistance", data.MaxDistance));
		getOrCreateStateSet()->addUniform(new osg::Uniform("vegDensity", data.Density));
		getOrCreateStateSet()->addUniform(new osg::Uniform("vegFadeDistance", data.FadeDistance));
		
		for (size_t i = 0; i < data.MeshLODs.size() - 1; i++)
		{
			std::stringstream ss;
			ss << "vegDistanceLOD" << i;
			std::string name = ss.str();
			getOrCreateStateSet()->addUniform(new osg::Uniform(name.c_str(), data.MeshLODs[i].MaxDist));
		}
	}
private:
	void drawImplementation(osg::RenderInfo& renderInfo) const
	{
		bool const sync_this_frame = true;
		osg::GLExtensions* ext = renderInfo.getState()->get<osg::GLExtensions>();
		unsigned contextID = renderInfo.getState()->getContextID();
		for (size_t l = 0; l < _LODLevels.size(); l++)
		{
			const LODData& ld = _LODLevels[l];
			osg::GLBufferObject* bo = ld._vbo->getOrCreateGLBufferObject(contextID);
			if (bo->isDirty())
				bo->compileBuffer();

			GLuint objID = bo->getGLObjectID();

			GLuint numPrims = 0;
			GLuint* query = &ld._queries[contextID];

			if (*query == INT_MAX)
			{
				ext->glGenQueries(1, query);
			}
			else if(!sync_this_frame)
			{
				ext->glGetQueryObjectuiv(*query, GL_QUERY_RESULT, &numPrims);
				ld._instance->setNumInstancesToDraw(numPrims);
			}
			ext->glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, l, objID, ld._offset, ld._size);
		}
		
		glEnable(GL_RASTERIZER_DISCARD);
		//	  if ( renderInfo.getState()->checkGLErrors("PRE:glEnable(GL_RASTERIZER_DISCARD)") ) exit(0);

		for (size_t l = 0; l < _LODLevels.size(); l++)
		{
			GLuint* query = &_LODLevels[l]._queries[contextID];
			ext->glBeginQueryIndexed(GL_PRIMITIVES_GENERATED, l, *query);
		}
		ext->glBeginTransformFeedback(GL_POINTS); // get from input geom?
		//if ( renderInfo.getState()->checkGLErrors("PRE:glBeginTransformFeedback(GL_POINTS)") ) 
		//	exit(0);

		osg::Geometry::drawImplementation(renderInfo);

		ext->glEndTransformFeedback();
		//if ( renderInfo.getState()->checkGLErrors("POST:glEndTransformFeedback") ) 
		//   exit(0);

		glDisable(GL_RASTERIZER_DISCARD);
		//if ( renderInfo.getState()->checkGLErrors("POST:glDisable(GL_RASTERIZER_DISCARD)") ) exit(0);

		for (size_t l = 0; l < _LODLevels.size(); l++)
		{
			ext->glEndQueryIndexed(GL_PRIMITIVES_GENERATED, l);
			ext->glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, l, 0);
		}

		for (size_t l = 0; l < _LODLevels.size(); l++)
		{
			const LODData& ld = _LODLevels[l];
			GLuint numPrims = 0;
			GLuint* query = &ld._queries[contextID];

			if(sync_this_frame)
			{ 
				ext->glGetQueryObjectuiv(*query, GL_QUERY_RESULT, &numPrims);
				ld._instance->setNumInstancesToDraw(numPrims);
			}
		}
		//if ( renderInfo.getState()->checkGLErrors("POST:glBindBufferBase") ) exit(0);
	}

	osg::Program* makeGeneratorProgram()
	{
		osg::Program* program = new osg::Program();

		program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("xbf_points_vertex.glsl")));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSCONTROL, osgDB::findDataFile("xbf_points_tess_ctrl.glsl")));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::TESSEVALUATION, osgDB::findDataFile("xbf_points_tess_eval.glsl")));
		program->addShader(osg::Shader::readShaderFile(osg::Shader::GEOMETRY, osgDB::findDataFile("xbf_points_geometry.glsl")));

		program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 1);
		program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_POINTS);

		program->addTransformFeedBackVarying("xfb_output_lod0");
		program->addTransformFeedBackVarying("gl_NextBuffer");
		program->addTransformFeedBackVarying("xfb_output_lod1");
		program->setTransformFeedBackMode(GL_INTERLEAVED_ATTRIBS);
		//program->setTransformFeedBackMode(GL_SEPARATE_ATTRIBS);
		return program;
	}
	
	static void _createPatchGeomtryFromHF(const osg::HeightField* hf, osg::Geometry* geometry)
	{
		unsigned int numColumns = hf->getNumColumns();
		unsigned int numRows = hf->getNumRows();
		float columnCoordDelta = hf->getXInterval();
		float rowCoordDelta = hf->getYInterval();

		osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
		osg::Vec2Array& t = *(new osg::Vec2Array(numColumns*numRows));
		//osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));
		//color[0].set(255, 255, 255, 255);
		float rowTexDelta = 1.0f / (float)(numRows - 1);
		float columnTexDelta = 1.0f / (float)(numColumns - 1);
		osg::Vec3 local_origin(0, 0, 0);

		osg::Vec3 pos(local_origin.x(), local_origin.y(), local_origin.z());
		osg::Vec2 tex(0.0f, 0.0f);
		int vi = 0;
		for (unsigned int r = 0; r < numRows; ++r)
		{
			pos.x() = local_origin.x();
			tex.x() = 0.0f;
			for (unsigned int c = 0; c < numColumns; ++c)
			{
				float h = hf->getHeight(c, r);
				v[vi].set(pos.x(), pos.y(), h);
				t[vi].set(tex.x(), tex.y());
				pos.x() += columnCoordDelta;
				tex.x() += columnTexDelta;
				++vi;
			}
			pos.y() += rowCoordDelta;
			tex.y() += rowTexDelta;
		}

		geometry->setVertexArray(&v);
		geometry->setTexCoordArray(0, &t);
		//geometry->setColorArray(&color, osg::Array::BIND_OVERALL);

		osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_PATCHES, 2 * 3 * (numColumns*numRows)));
		geometry->addPrimitiveSet(&drawElements);
		int ei = 0;
		for (unsigned int r = 0; r < numRows - 1; ++r)
		{
			for (unsigned int c = 0; c < numColumns - 1; ++c)
			{
				// Try to imitate how GeometryTechnique::generateGeometry optimize 
				// which way to put the diagonal by choosing to
				// place it between the two corners that have the least curvature
				// relative to each other.
				// Due to how normals are calculated we don't get exact match...fix this by using same normal calulations

				osg::Vec3 n00 = hf->getNormal(c, r);
				osg::Vec3 n01 = hf->getNormal(c, r + 1);
				osg::Vec3 n10 = hf->getNormal(c + 1, r);
				osg::Vec3 n11 = hf->getNormal(c + 1, r + 1);
				float dot_00_11 = n00 * n11;
				float dot_01_10 = n01 * n10;
				if (dot_00_11 > dot_01_10)
				{
					drawElements[ei++] = (r)*numColumns + c;
					drawElements[ei++] = (r)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c + 1;

					drawElements[ei++] = (r + 1)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c;
					drawElements[ei++] = (r)*numColumns + c;
				}
				else
				{
					drawElements[ei++] = (r)*numColumns + c;
					drawElements[ei++] = (r)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c;

					drawElements[ei++] = (r)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c + 1;
					drawElements[ei++] = (r + 1)*numColumns + c;
				}
			}
		}
	}

	struct LODData
	{
		XBFInstance *_instance;
		int _offset, _size;
		osg::ref_ptr<osg::BufferObject> _vbo;
		mutable osg::buffered_object<GLuint> _queries;
	};
	std::vector<LODData> _LODLevels;
};