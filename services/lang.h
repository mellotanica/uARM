/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * uMPS - A general purpose computer system simulator
 *
 * Copyright (C) 2010 Tomislav Jonjic
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef BASE_LANG_H
#define BASE_LANG_H

// Smart pointers (here we could just as well depend on TR1 and use
// its version of these i guess - the choice is arbitrary).

#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr.hpp>

using boost::shared_ptr;
using boost::scoped_ptr;
using boost::scoped_array;
using boost::enable_shared_from_this;

// A foreach construct for C++, just like in all the cool languages
// out there! This is just too tempting not to use. And when C++0x
// becomes ubiquitous (_if_ it does) we can easily switch to the
// standard version.
#include <boost/foreach.hpp>

#define foreach BOOST_FOREACH

// Make the nature of "reference"-type classes excplicit to readers
// and compilers by using the macro below somewhere in a private
// section of a class definition.
#define DISABLE_COPY_AND_ASSIGNMENT(Class)             \
    Class(const Class&);                               \
    Class& operator=(const Class&)

#define UNUSED_ARG(x) ((void) x)

#endif // BASE_LANG_H
