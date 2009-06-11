/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * Wrapper.hpp - Redland C Object Pointer wrapper
 *
 * Copyright (C) 2008, David Beckett http://www.dajobe.org/
 * 
 * This package is Free Software and part of Redland http://librdf.org/
 * 
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 * 
 * 
 */

#ifndef REDLANDPP_WRAPPER_HPP
#define REDLANDPP_WRAPPER_HPP

#include <ostream>

namespace Redland {

  typedef void (redland_object_free)(void*);

  template <typename T>
    class Wrapper {
    public:
      // constructor
      Wrapper(redland_object_free* free_fn, T* obj = NULL) :
         obj_(obj) ,
         free_fn_(free_fn) {}

      // destructor
      ~Wrapper() {
        if(obj_ != NULL && free_fn_ != NULL) 
          free_fn_((void*)obj_);
      }

      typedef T cobject;

      inline T*       cobj()       { return obj_; }
      inline const T* cobj() const { return obj_; }
      
    protected:
      // Redland C object pointer
      T* obj_;

    private:
      // Redland C function to free Redland C object pointer
      redland_object_free* free_fn_;

      // disallowed constructors
      // default constructor
      Wrapper();
      // copy constructor
      Wrapper(const Wrapper&);
      // assignment constructor
      void operator=(const Wrapper&);

      friend std::ostream& operator<< (std::ostream& os, const Wrapper<T>& o)
      {
        return os << "<Redland Object 0x"
                  << std::hex
                  << ((Wrapper<T>&)o).cobj()
                  << ">";
      }
    };

} // namespace Redland

#endif
