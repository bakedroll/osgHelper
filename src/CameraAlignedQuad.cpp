#include <osgHelper/CameraAlignedQuad.h>

#include <osg/Geometry>

namespace osgHelper
{

CameraAlignedQuad::CameraAlignedQuad(int renderBin)
	: Geode()
{
	makeQuad(renderBin);
}

osg::ref_ptr<osg::Geometry> CameraAlignedQuad::getGeometry() const
{
	return m_geometry;
}

osg::ref_ptr<osg::Vec3Array> CameraAlignedQuad::getVertexArray() const
{
	return m_vertexArray;
}

osg::ref_ptr<osg::Vec3Array> CameraAlignedQuad::getNormalArray() const
{
	return m_normalArray;
}

void CameraAlignedQuad::makeQuad(int renderBin)
{
  auto stateSet = new osg::StateSet();

	stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	stateSet->setRenderBinDetails(renderBin, "RenderBin");

	m_geometry = new osg::Geometry();
	m_geometry->setUseVertexBufferObjects(true);

	m_vertexArray = new osg::Vec3Array(4);
	m_vertexArray->setDataVariance(DYNAMIC);
	m_geometry->setVertexArray(m_vertexArray);

	m_normalArray = new osg::Vec3Array(4);
	m_normalArray->setDataVariance(DYNAMIC);
	m_geometry->setNormalArray(m_normalArray);
	m_geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

  auto colors = new osg::Vec4Array();
	colors->push_back(osg::Vec4(-1.0f, -1.0f, 0.0f, 1.0f));
	colors->push_back(osg::Vec4(-1.0f, 1.0f, 0.0f, 1.0f));
	colors->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
	colors->push_back(osg::Vec4(1.0f, -1.0f, 0.0f, 1.0f));
	m_geometry->setColorArray(colors);
	m_geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  auto indices = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);
	indices->push_back(3);
	indices->push_back(2);
	indices->push_back(1);
	indices->push_back(0);
	m_geometry->addPrimitiveSet(indices);

	addDrawable(m_geometry);
	setStateSet(stateSet);
}

}