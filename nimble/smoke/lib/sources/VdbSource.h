/*
 * VdbSource.h
 *
 *  Created on: 30-Jan-2016
 *      Author: user
 */

#ifndef LIB_SOURCES_VDBSOURCE_H_
#define LIB_SOURCES_VDBSOURCE_H_

#include <smoke/lib/core/CommonTypes.h>
namespace smoke
{
namespace sources
{

class VdbSource
{
public:
	VdbSource(smoke::core::FloatGridPtr floatGridPtr) :
			floatGridPtr(floatGridPtr)
	{

	}
	virtual ~VdbSource();

	const smoke::core::FloatGridPtr& getFloatGridPtr() const
	{
		return floatGridPtr;
	}

	void setFloatGridPtr(const smoke::core::FloatGridPtr& floatGridPtr)
	{
		this->floatGridPtr = floatGridPtr;
	}

private:
	smoke::core::FloatGridPtr floatGridPtr;
};

} /* namespace sources */
} /* namespace smoke */

#endif /* LIB_SOURCES_VDBSOURCE_H_ */
