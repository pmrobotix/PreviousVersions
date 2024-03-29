#include "EncoderControl.hpp"

using namespace std;

EncoderControl::EncoderControl(Asserv & asserv) :
		AAsservElement(asserv)
{
	asservdriver = AAsservDriver::create();
}

EncoderControl::~EncoderControl()
{
}

long EncoderControl::getLeftEncoder()
{
	return asservdriver->getLeftInternalEncoder();
}

long EncoderControl::getRightEncoder()
{
	return asservdriver->getRightInternalEncoder();
}

void EncoderControl::reset()
{
	return asservdriver->resetEncoder();
}

/*
//WRAPPER C
extern "C" long call_EncoderControl_getLeftEncoder(EncoderControl* p) // wrapper function
{
	return p->EncoderControl::getLeftEncoder();
}
*/
