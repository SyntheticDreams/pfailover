// $Id$

/*
 * Copyright (C) 2009 Toni Westbrook <twestbrook@synthdreams.com>
 *
 * This file is part of pfailover.
 *
 * pfailover is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * pfailover is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pfailover.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * pfailover.h
 *
 * Program wide general functions
 *
 */

#ifndef PFAILOVER_H_
#define PFAILOVER_H_

namespace pfailover {
   // Used to reports errors to stderr, return 1 to be returned to calling program
   int reportError(std::string passError, std::string passFile = "", int passLine = 0);

   // Show standard usage in shell
   int reportUsage();
}

#endif /* PFAILOVER_H_ */
