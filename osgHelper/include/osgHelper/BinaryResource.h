#pragma once

#include <osg/Object>

namespace osgHelper
{
	class BinaryResource : public osg::Object
	{
	public:
		BinaryResource();
		~BinaryResource();

		osg::Object* cloneType() const override;
		osg::Object* clone(const osg::CopyOp& copyOp) const override;
		const char* libraryName() const override;
		const char* className() const override;

		char* getBytes() const;
    void  setBytes(char* bytes);
		
	private:
		char* m_bytes;
	};
}