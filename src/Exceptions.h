#ifndef __EXCEPTIONS__H__
#define __EXCEPTIONS__H__

#include <exception>

class Exception : public std::exception {

 public:

  Exception(const char *a_message);
  
  virtual ~Exception() throw(){};

  virtual const char * what();

 private:
		
  const char *m_message;

};

#endif //__EXCEPTIONS__H__
