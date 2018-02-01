/* THIS IS A SINGLE-FILE DISTRIBUTION CONCATENATED FROM THE OPEN62541 SOURCES 
 * visit http://open62541.org/ for information about this software
 * Git-Revision: v0.1.0-RC4-1054-gf1a9cca-dirty
 */
 
 /*
 * Copyright (C) 2015 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#ifndef LEPTON_H_
#define LEPTON_H_

#ifdef __cplusplus
extern "C" {
#endif
#define BASEID      10000
#define LEPTONID 	BASEID + 0
#define SNAPSHOTSID BASEID + 1000
#define BOXESID 	BASEID + 2000
#define SPOTSID 	BASEID + 3000

#define NODEID_BOX1 (BOXESID + 100)
#define NODEID_BOX1_MIN (NODEID_BOX1 + 1)
#define NODEID_BOX1_MAX (NODEID_BOX1 + 2)
#define NODEID_BOX1_AVG (NODEID_BOX1 + 3)
#define NODEID_BOX1_VAL (NODEID_BOX1 + 4)

#define NODEID_BOX2 (BOXESID + 200)
#define NODEID_BOX2_MIN (NODEID_BOX2 + 1)
#define NODEID_BOX2_MAX (NODEID_BOX2 + 2)
#define NODEID_BOX2_AVG (NODEID_BOX2 + 3)
#define NODEID_BOX2_VAL (NODEID_BOX2 + 4)

#define NODEID_BOX3 (BOXESID + 300)
#define NODEID_BOX3_MIN (NODEID_BOX3 + 1)
#define NODEID_BOX3_MAX (NODEID_BOX3 + 2)
#define NODEID_BOX3_AVG (NODEID_BOX3 + 3)
#define NODEID_BOX3_VAL (NODEID_BOX3 + 4)

#define NODEID_BOX4 (BOXESID + 400)
#define NODEID_BOX4_MIN (NODEID_BOX4 + 1)
#define NODEID_BOX4_MAX (NODEID_BOX4 + 2)
#define NODEID_BOX4_AVG (NODEID_BOX4 + 3)
#define NODEID_BOX4_VAL (NODEID_BOX4 + 4)

#define NODEID_SPOT1 (SPOTSID + 100)
#define NODEID_SPOT1_MIN (NODEID_SPOT1 + 1)
#define NODEID_SPOT1_MAX (NODEID_SPOT1 + 2)
#define NODEID_SPOT1_AVG (NODEID_SPOT1 + 3)
#define NODEID_SPOT1_VAL (NODEID_SPOT1 + 4)

#define NODEID_SPOT2 (SPOTSID + 200)
#define NODEID_SPOT2_MIN (NODEID_SPOT2 + 1)
#define NODEID_SPOT2_MAX (NODEID_SPOT2 + 2)
#define NODEID_SPOT2_AVG (NODEID_SPOT2 + 3)
#define NODEID_SPOT2_VAL (NODEID_SPOT2 + 4)

#define NODEID_SPOT3 (SPOTSID + 300)
#define NODEID_SPOT3_MIN (NODEID_SPOT3 + 1)
#define NODEID_SPOT3_MAX (NODEID_SPOT3 + 2)
#define NODEID_SPOT3_AVG (NODEID_SPOT3 + 3)
#define NODEID_SPOT3_VAL (NODEID_SPOT3 + 4)

#define NODEID_SPOT4 (SPOTSID + 400)
#define NODEID_SPOT4_MIN (NODEID_SPOT4 + 1)
#define NODEID_SPOT4_MAX (NODEID_SPOT4 + 2)
#define NODEID_SPOT4_AVG (NODEID_SPOT4 + 3)
#define NODEID_SPOT4_VAL (NODEID_SPOT4 + 4)

typedef struct {
    float min;
	float max;
	float avg;
	bool  val;
} UA_LeptonItem;

UA_StatusCode readLepton(void *handle, const UA_NodeId nodeId, UA_Boolean sourceTimeStamp, const UA_NumericRange *range, UA_DataValue *value);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LEPTON_H_ */