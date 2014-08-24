#include "VegetationRenderingTech.h"
#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Projection>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/TextureBuffer>
#include <osg/Image>
#include "VegetationCell.h"


namespace osgVegetation
{
	osg::Geometry* VegetationRenderingTech::createOrthogonalQuadsNoColor( const osg::Vec3& pos, float w, float h)
	{
		// set up the coords
		osg::Vec3Array& v = *(new osg::Vec3Array(8));
		osg::Vec2Array& t = *(new osg::Vec2Array(8));

		float rotation = random(0.0f,osg::PI/2.0f);
		float sw = sinf(rotation)*w*0.5f;
		float cw = cosf(rotation)*w*0.5f;

		v[0].set(pos.x()-sw,pos.y()-cw,pos.z()+0.0f);
		v[1].set(pos.x()+sw,pos.y()+cw,pos.z()+0.0f);
		v[2].set(pos.x()+sw,pos.y()+cw,pos.z()+h);
		v[3].set(pos.x()-sw,pos.y()-cw,pos.z()+h);

		v[4].set(pos.x()-cw,pos.y()+sw,pos.z()+0.0f);
		v[5].set(pos.x()+cw,pos.y()-sw,pos.z()+0.0f);
		v[6].set(pos.x()+cw,pos.y()-sw,pos.z()+h);
		v[7].set(pos.x()-cw,pos.y()+sw,pos.z()+h);

		t[0].set(0.0f,0.0f);
		t[1].set(1.0f,0.0f);
		t[2].set(1.0f,1.0f);
		t[3].set(0.0f,1.0f);

		t[4].set(0.0f,0.0f);
		t[5].set(1.0f,0.0f);
		t[6].set(1.0f,1.0f);
		t[7].set(0.0f,1.0f);

		osg::Geometry *geom = new osg::Geometry;

		geom->setVertexArray( &v );

		geom->setTexCoordArray( 0, &t );

		geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,8) );

		return geom;
	}

	osg::Geometry* VegetationRenderingTech::createSprite( float w, float h, osg::Vec4ub color )
	{
		// set up the coords
		osg::Vec3Array& v = *(new osg::Vec3Array(4));
		osg::Vec2Array& t = *(new osg::Vec2Array(4));
		osg::Vec4ubArray& c = *(new osg::Vec4ubArray(1));

		v[0].set(-w*0.5f,0.0f,0.0f);
		v[1].set( w*0.5f,0.0f,0.0f);
		v[2].set( w*0.5f,0.0f,h);
		v[3].set(-w*0.5f,0.0f,h);

		c[0] = color;

		t[0].set(0.0f,0.0f);
		t[1].set(1.0f,0.0f);
		t[2].set(1.0f,1.0f);
		t[3].set(0.0f,1.0f);

		osg::Geometry *geom = new osg::Geometry;

		geom->setVertexArray( &v );

		geom->setTexCoordArray( 0, &t );

		geom->setColorArray( &c, osg::Array::BIND_OVERALL );

		geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4) );

		return geom;
	}

	osg::Geometry* VegetationRenderingTech::createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color )
	{
		// set up the coords
		osg::Vec3Array& v = *(new osg::Vec3Array(8));
		osg::Vec2Array& t = *(new osg::Vec2Array(8));
		osg::Vec4ubArray& c = *(new osg::Vec4ubArray(1));

		float rotation = random(0.0f,osg::PI/2.0f);
		float sw = sinf(rotation)*w*0.5f;
		float cw = cosf(rotation)*w*0.5f;

		v[0].set(pos.x()-sw,pos.y()-cw,pos.z()+0.0f);
		v[1].set(pos.x()+sw,pos.y()+cw,pos.z()+0.0f);
		v[2].set(pos.x()+sw,pos.y()+cw,pos.z()+h);
		v[3].set(pos.x()-sw,pos.y()-cw,pos.z()+h);

		v[4].set(pos.x()-cw,pos.y()+sw,pos.z()+0.0f);
		v[5].set(pos.x()+cw,pos.y()-sw,pos.z()+0.0f);
		v[6].set(pos.x()+cw,pos.y()-sw,pos.z()+h);
		v[7].set(pos.x()-cw,pos.y()+sw,pos.z()+h);

		c[0] = color;

		t[0].set(0.0f,0.0f);
		t[1].set(1.0f,0.0f);
		t[2].set(1.0f,1.0f);
		t[3].set(0.0f,1.0f);

		t[4].set(0.0f,0.0f);
		t[5].set(1.0f,0.0f);
		t[6].set(1.0f,1.0f);
		t[7].set(0.0f,1.0f);

		osg::Geometry *geom = new osg::Geometry;

		geom->setVertexArray( &v );

		geom->setTexCoordArray( 0, &t );

		geom->setColorArray( &c, osg::Array::BIND_OVERALL );

		geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,8) );
		return geom;
	}
}