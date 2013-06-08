#ifndef __EXCEPTIONS__H__
#define __EXCEPTIONS__H__

#include <exception>

/*! \brief Generic exception class
 *
 *  This is a basic exception class. It only adds to the std::exception a constructor taking a string
 *  which describes the exception and is extracted by the virtual what() member function.
 *
 */

class Exception : public std::exception
{
	public:
		/*! \brief Generic Exception constructor
		 *
		 *  \param a_message description of the exception. This message can be catched by the member function what().
		 *  @see what()
		 */
		Exception(const char *a_message);
		virtual ~Exception() throw(){};
		/*! \brief returns the string describing the exception
		 *
		 */
		virtual const char * what();
	private:
		
		const char *m_message;
};

#endif //__EXCEPTIONS__H__
