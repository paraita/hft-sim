#include "Exceptions.h"

Exception::Exception(const char *a_message):exception()
{
	m_message = a_message;
}

const char * Exception::what()
{
	return m_message;
}
