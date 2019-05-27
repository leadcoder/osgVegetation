#pragma once

#include "AggregateGeometryVisitor.h"
#include <osg/PrimitiveSetIndirect>
#include <osg/TextureBuffer>
#include <osg/BindImageTexture>
#include <osg/BufferIndexBinding>
#include <osg/BufferTemplate>
#include <osg/ComputeBoundsVisitor>

namespace osgVegetation
{
	// each instance type may have max 8 LODs ( if you change
	// this value, don't forget to change it in vertex shaders accordingly )
	const unsigned int OV_MAXIMUM_LOD_NUMBER = 8;
	// during culling each instance may be sent to max 4 indirect targets
	const unsigned int OV_MAXIMUM_INDIRECT_TARGET_NUMBER = 4;

	// Struct defining information about specific instance type : bounding box, lod ranges, indirect target indices etc
	struct InstanceType
	{
		// Struct defining LOD data for particular instance type
		struct InstanceLOD
		{
			InstanceLOD()
				: bbMin(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f), bbMax(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f)
			{
			}
			InstanceLOD(const InstanceLOD& iLod)
				: bbMin(iLod.bbMin), bbMax(iLod.bbMax), indirectTargetParams(iLod.indirectTargetParams), distances(iLod.distances)
			{
			}
			InstanceLOD& operator=(const InstanceLOD& iLod)
			{
				if (&iLod != this)
				{
					bbMin = iLod.bbMin;
					bbMax = iLod.bbMax;
					indirectTargetParams = iLod.indirectTargetParams;
					distances = iLod.distances;
				}
				return *this;
			}

			inline void setBoundingBox(const osg::BoundingBox& bbox)
			{
				bbMin = osg::Vec4f(bbox.xMin(), bbox.yMin(), bbox.zMin(), 1.0f);
				bbMax = osg::Vec4f(bbox.xMax(), bbox.yMax(), bbox.zMax(), 1.0f);
			}
			inline osg::BoundingBox getBoundingBox()
			{
				return osg::BoundingBox(bbMin.x(), bbMin.y(), bbMin.z(), bbMax.x(), bbMax.y(), bbMax.z());
			}

			osg::Vec4f bbMin;                   // LOD bounding box
			osg::Vec4f bbMax;
			osg::Vec4i indirectTargetParams;    // x=lodIndirectCommand, y=lodIndirectCommandIndex, z=offsetsInTarget, w=lodMaxQuantity
			osg::Vec4f distances;               // x=minDistance, y=minFadeDistance, z=maxFadeDistance, w=maxDistance
		};


		InstanceType()
			: bbMin(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f), bbMax(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f)
		{
			params.x() = 0; // this variable defines the number of LODs
			for (unsigned int i = 0; i < OV_MAXIMUM_LOD_NUMBER; ++i)
				lods[i] = InstanceLOD();
		}
		InstanceType(const InstanceType& iType)
			: bbMin(iType.bbMin), bbMax(iType.bbMax), params(iType.params)
		{
			for (unsigned int i = 0; i < OV_MAXIMUM_LOD_NUMBER; ++i)
				lods[i] = iType.lods[i];
		}
		InstanceType& operator=(const InstanceType& iType)
		{
			if (&iType != this)
			{
				bbMin = iType.bbMin;
				bbMax = iType.bbMax;
				params = iType.params;
				for (unsigned int i = 0; i < OV_MAXIMUM_LOD_NUMBER; ++i)
					lods[i] = iType.lods[i];
			}
			return *this;
		}
		inline void setLodDefinition(unsigned int i, unsigned int targetID, unsigned int indexInTarget, const osg::Vec4& distance, unsigned int offsetInTarget, unsigned int maxQuantity, const osg::BoundingBox& lodBBox)
		{
			if (i >= OV_MAXIMUM_LOD_NUMBER)
				return;
			params.x() = osg::maximum<int>(params.x(), i + 1);
			lods[i].indirectTargetParams = osg::Vec4i(targetID, indexInTarget, offsetInTarget, maxQuantity);
			lods[i].distances = distance;
			lods[i].setBoundingBox(lodBBox);
			expandBy(lodBBox);
		}
		inline void expandBy(const osg::BoundingBox& bbox)
		{
			osg::BoundingBox myBBox = getBoundingBox();
			myBBox.expandBy(bbox);
			setBoundingBox(myBBox);
		}
		inline void setBoundingBox(const osg::BoundingBox& bbox)
		{
			bbMin = osg::Vec4f(bbox.xMin(), bbox.yMin(), bbox.zMin(), 1.0f);
			bbMax = osg::Vec4f(bbox.xMax(), bbox.yMax(), bbox.zMax(), 1.0f);
		}
		inline osg::BoundingBox getBoundingBox()
		{
			return osg::BoundingBox(bbMin.x(), bbMin.y(), bbMin.z(), bbMax.x(), bbMax.y(), bbMax.z());
		}

		osg::Vec4f  bbMin;                              // bounding box that includes all LODs
		osg::Vec4f  bbMax;
		osg::Vec4i  params;                             // x=number of active LODs
		InstanceLOD lods[OV_MAXIMUM_LOD_NUMBER]; // information about LODs
	};

	// CPU side representation of a struct defined in ARB_draw_indirect extension
	struct DrawArraysIndirectCommand
	{
		DrawArraysIndirectCommand()
			: count(0), primCount(0), first(0), baseInstance(0)
		{
		}
		DrawArraysIndirectCommand(unsigned int aFirst, unsigned int aCount)
			: count(aCount), primCount(0), first(aFirst), baseInstance(0)
		{
		}
		unsigned int count;
		unsigned int primCount;
		unsigned int first;
		unsigned int baseInstance;
	};

	// During the first phase of instance rendering cull shader places information about
	// instance LODs in texture buffers called "indirect targets"
	// All data associated with the indirect target is placed in struct defined below
	// ( like for example - draw shader associated with specific indirect target.
	// Draw shader performs second phase of instance rendering - the actual rendering of objects
	// to screen or to frame buffer object ).
	struct IndirectTarget
	{
		IndirectTarget()
			: _maxTargetQuantity(0)
		{
			_indirectCommands = new osg::DefaultIndirectCommandDrawArrays;
			_indirectCommands->getBufferObject()->setUsage(GL_DYNAMIC_DRAW);
		}
		IndirectTarget(AggregateGeometryVisitor* agv, osg::Program* program)
			: _geometryAggregator(agv), _drawProgram(program), _maxTargetQuantity(0)
		{
			_indirectCommands = new osg::DefaultIndirectCommandDrawArrays;
			_indirectCommands->getBufferObject()->setUsage(GL_DYNAMIC_DRAW);
		}
		void endRegister(unsigned int index, unsigned int rowsPerInstance, GLenum pixelFormat, GLenum type, GLint internalFormat, bool useMultiDrawArraysIndirect)
		{
			_indirectCommandTextureBuffer = new osg::TextureBuffer(_indirectCommands.get());
			_indirectCommandTextureBuffer->setInternalFormat(GL_R32I);
			_indirectCommandTextureBuffer->setUnRefImageDataAfterApply(false);

			_indirectCommandImageBinding = new osg::BindImageTexture(index, _indirectCommandTextureBuffer.get(), osg::BindImageTexture::READ_WRITE, GL_R32I);

			// add proper primitivesets to geometryAggregators
			if (!useMultiDrawArraysIndirect) // use glDrawArraysIndirect()
			{
				std::vector<osg::DrawArraysIndirect*> newPrimitiveSets;

				for (unsigned int j = 0; j < _indirectCommands->size(); ++j)
				{
					osg::DrawArraysIndirect *ipr = new osg::DrawArraysIndirect(GL_TRIANGLES, j);
					ipr->setIndirectCommandArray(_indirectCommands.get());
					newPrimitiveSets.push_back(ipr);
				}

				_geometryAggregator->getAggregatedGeometry()->removePrimitiveSet(0, _geometryAggregator->getAggregatedGeometry()->getNumPrimitiveSets());

				for (unsigned int j = 0; j < _indirectCommands->size(); ++j)
					_geometryAggregator->getAggregatedGeometry()->addPrimitiveSet(newPrimitiveSets[j]);


			}
			else // use glMultiDrawArraysIndirect()
			{
				osg::MultiDrawArraysIndirect *ipr = new osg::MultiDrawArraysIndirect(GL_TRIANGLES);
				ipr->setIndirectCommandArray(_indirectCommands.get());
				_geometryAggregator->getAggregatedGeometry()->removePrimitiveSet(0, _geometryAggregator->getAggregatedGeometry()->getNumPrimitiveSets());
				_geometryAggregator->getAggregatedGeometry()->addPrimitiveSet(ipr);
			}

			_geometryAggregator->getAggregatedGeometry()->setUseDisplayList(false);
			_geometryAggregator->getAggregatedGeometry()->setUseVertexBufferObjects(true);


			osg::Image* instanceTargetImage = new osg::Image;
			instanceTargetImage->allocateImage(_maxTargetQuantity*rowsPerInstance, 1, 1, pixelFormat, type);

			osg::VertexBufferObject * instanceTargetImageBuffer = new osg::VertexBufferObject();
			instanceTargetImageBuffer->setUsage(GL_DYNAMIC_DRAW);
			instanceTargetImage->setBufferObject(instanceTargetImageBuffer);

			_instanceTarget = new osg::TextureBuffer(instanceTargetImage);
			_instanceTarget->setInternalFormat(internalFormat);

			_instanceTargetimagebinding = new osg::BindImageTexture(OV_MAXIMUM_INDIRECT_TARGET_NUMBER + index, _instanceTarget.get(), osg::BindImageTexture::READ_WRITE, internalFormat);

			_geometryAggregator->generateTextureArray();
		}

		void addIndirectCommandData(const std::string& uniformNamePrefix, int index, osg::StateSet* stateset)
		{
			std::string uniformName = uniformNamePrefix + char('0' + index);
			osg::Uniform* uniform = new osg::Uniform(uniformName.c_str(), (int)index);
			stateset->addUniform(uniform);
			stateset->setAttribute(_indirectCommandImageBinding);
			stateset->setTextureAttribute(index, _indirectCommandTextureBuffer.get());
		}

		void addIndirectTargetData(bool cullPhase, const std::string& uniformNamePrefix, int index, osg::StateSet* stateset)
		{
			std::string uniformName;
			if (cullPhase)
				uniformName = uniformNamePrefix + char('0' + index);
			else
				uniformName = uniformNamePrefix;

			osg::Uniform* uniform = new osg::Uniform(uniformName.c_str(), (int)(OV_MAXIMUM_INDIRECT_TARGET_NUMBER + index));
			stateset->addUniform(uniform);

			stateset->setAttribute(_instanceTargetimagebinding);
			stateset->setTextureAttribute(OV_MAXIMUM_INDIRECT_TARGET_NUMBER + index, _instanceTarget.get());
		}

		void addDrawProgram(const std::string& uniformBlockName, osg::StateSet* stateset)
		{
			_drawProgram->addBindUniformBlock(uniformBlockName, 1);
			stateset->setAttributeAndModes(_drawProgram.get(), osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
		}

		osg::ref_ptr< osg::DefaultIndirectCommandDrawArrays >        _indirectCommands;
		osg::ref_ptr<osg::TextureBuffer>                                _indirectCommandTextureBuffer;
		osg::ref_ptr<osg::BindImageTexture>                             _indirectCommandImageBinding;
		osg::ref_ptr< AggregateGeometryVisitor >                        _geometryAggregator;
		osg::ref_ptr<osg::Program>                                      _drawProgram;
		osg::ref_ptr< osg::TextureBuffer >                              _instanceTarget;
		osg::ref_ptr<osg::BindImageTexture>                             _instanceTargetimagebinding;
		unsigned int                                                    _maxTargetQuantity;
	};
	// This is the main structure holding all information about particular 2-phase instance rendering
	// ( instance types, indirect targets, etc ).
	struct GPUCullData
	{
		GPUCullData()
		{
			useMultiDrawArraysIndirect = false;
			instanceTypes = new osg::BufferTemplate< std::vector<InstanceType> >;
			// build Uniform BufferObject with instanceTypes data
			instanceTypesUBO = new osg::UniformBufferObject;
			//        instanceTypesUBO->setUsage( GL_STREAM_DRAW );
			instanceTypes->setBufferObject(instanceTypesUBO.get());
			instanceTypesUBB = new osg::UniformBufferBinding(1, instanceTypes.get(), 0, 0);

		}

		void setUseMultiDrawArraysIndirect(bool value)
		{
			useMultiDrawArraysIndirect = value;
		}

		void registerIndirectTarget(unsigned int index, AggregateGeometryVisitor* agv, osg::Program* targetDrawProgram)
		{
			if (index >= OV_MAXIMUM_INDIRECT_TARGET_NUMBER || agv == NULL || targetDrawProgram == NULL)
				return;
			targets[index] = IndirectTarget(agv, targetDrawProgram);
		}

		bool registerType(unsigned int typeID, unsigned int targetID, osg::Node* node, const osg::Vec4& lodDistances, float maxDensityPerSquareKilometer)
		{
			if (typeID >= instanceTypes->getData().size())
				instanceTypes->getData().resize(typeID + 1);
			InstanceType& itd = instanceTypes->getData().at(typeID);
			unsigned int lodNumber = (unsigned int)itd.params.x();
			if (lodNumber >= OV_MAXIMUM_LOD_NUMBER)
				return false;

			std::map<unsigned int, IndirectTarget>::iterator target = targets.find(targetID);
			if (target == targets.end())
				return false;

			// AggregateGeometryVisitor creates single osg::Geometry from all objects used by specific indirect target
			AggregateGeometryVisitor::AddObjectResult aoResult = target->second._geometryAggregator->addObject(node, typeID, lodNumber);
			// Information about first vertex and a number of vertices is stored for later primitiveset creation
			target->second._indirectCommands->push_back(osg::DrawArraysIndirectCommand(aoResult.count, 1, aoResult.first));

			osg::ComputeBoundsVisitor cbv;
			node->accept(cbv);

			// Indirect target texture buffers have finite size, therefore each instance LOD has maximum number that may be rendered in one frame.
			// This maximum number of rendered instances is estimated from the area that LOD covers and maximum density of instances per square kilometer.
			float lodArea = osg::PI * (lodDistances.w() * lodDistances.w() - lodDistances.x() * lodDistances.x()) / 1000000.0f;
			// calculate max quantity of objects in lodArea using maximum density per square kilometer
			unsigned int maxQuantity = (unsigned int)ceil(lodArea * maxDensityPerSquareKilometer);

			itd.setLodDefinition(lodNumber, targetID, aoResult.index, lodDistances, target->second._maxTargetQuantity, maxQuantity, cbv.getBoundingBox());
			target->second._maxTargetQuantity += maxQuantity;
			return true;
		}

		// endRegister() method is called after all indirect targets and instance types are registered.
		// It creates indirect targets with pixel format and data type provided by user ( indirect targets may hold
		// different information about single instance depending on user's needs ( in our example : static rendering
		// sends all vertex data to indirect target during GPU cull phase, while dynamic rendering sends only a "pointer"
		// to texture buffer containing instance data ( look at endRegister() invocations in createStaticRendering() and
		// createDynamicRendering() )
		void endRegister(unsigned int rowsPerInstance, GLenum pixelFormat, GLenum type, GLint internalFormat)
		{
			OSG_INFO << "instance types" << std::endl;
			for (unsigned int i = 0; i < instanceTypes->getData().size(); ++i)
			{
				InstanceType& iType = instanceTypes->getData().at(i);
				int sum = 0;
				OSG_INFO << "Type " << i << " : ( ";
				int lodCount = iType.params.x();
				for (int j = 0; j < lodCount; ++j)
				{
					OSG_INFO << "{" << iType.lods[j].indirectTargetParams.x() << "}" << iType.lods[j].indirectTargetParams.z() << "->" << iType.lods[j].indirectTargetParams.w() << " ";
					sum += iType.lods[j].indirectTargetParams.w();
				}
				OSG_INFO << ") => " << sum << " elements" << std::endl;
			}

			OSG_INFO << "indirect targets" << std::endl;
			std::map<unsigned int, IndirectTarget>::iterator it, eit;
			for (it = targets.begin(), eit = targets.end(); it != eit; ++it)
			{
				for (unsigned j = 0; j < it->second._indirectCommands->size(); ++j)
				{
					osg::DrawArraysIndirectCommand& iComm = it->second._indirectCommands->at(j);
					OSG_INFO << "(" << iComm.first << " " << iComm.instanceCount << " " << iComm.count << ") ";
				}
				unsigned int sizeInBytes = (unsigned int)it->second._maxTargetQuantity * sizeof(osg::Vec4);
				OSG_INFO << " => Maximum elements in target : " << it->second._maxTargetQuantity << " ( " << sizeInBytes << " bytes, " << sizeInBytes / 1024 << " kB )" << std::endl;
			}

			instanceTypesUBB->setSize(instanceTypes->getTotalDataSize());
			for (it = targets.begin(), eit = targets.end(); it != eit; ++it)
				it->second.endRegister(it->first, rowsPerInstance, pixelFormat, type, internalFormat, useMultiDrawArraysIndirect);

		}

		bool                                                useMultiDrawArraysIndirect;
		osg::ref_ptr< osg::BufferTemplate< std::vector<InstanceType> > >   instanceTypes;
		osg::ref_ptr<osg::UniformBufferObject>              instanceTypesUBO;
		osg::ref_ptr<osg::UniformBufferBinding>             instanceTypesUBB;

		std::map<unsigned int, IndirectTarget>               targets;
	};
}
