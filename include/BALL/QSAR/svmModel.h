/* svmModel.h
 * 
 * Copyright (C) 2009 Marcel Schumann
 * 
 * This file is part of QuEasy -- A Toolbox for Automated QSAR Model
 * Construction and Validation.
 * QuEasy is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * QuEasy is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
//

#ifndef SVMMODEL
#define SVMMODEL

#ifndef CLASSIFICATION
#include <BALL/QSAR/classificationModel.h>
#endif

#ifndef KMODEL
#include <BALL/QSAR/kernelModel.h>
#endif

#ifndef NLMODEL
#include <BALL/QSAR/nonlinearModel.h>
#endif

namespace BALL{
	namespace QSAR {

		/** class for support vector classification */
		class SVMModel : public ClassificationModel
		{ 
			public:
				/** @name Constructors and Destructors
				 */
				//@{
				SVMModel(const QSARData& q);

				~SVMModel();
				//@}
				
				
				
		};
	}
}

#endif //SVMMODEL