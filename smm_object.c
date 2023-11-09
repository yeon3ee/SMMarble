//
//  smm_node.c
//  SMMarble
//
//  Created by Juyeop Kim on 2023/11/05.
//

#include "smm_common.h"
#include "smm_object.h"
#include <string.h>

#define MAX_NODETYPE    7
#define MAX_GRADE       9
#define MAX_NODE		100

static char smmobj_name[MAX_NODE][MAX_CHARNAME];
static int smmobj_type[MAX_NODE];
static int smmobj_credit[MAX_NODE];
static int smmobj_energy[MAX_NODE];
static int smmobj_noNOde=0;




//object generation
void smmobj_genNode(char* name,int type,int credit,int energy) //노드를 만드는 함수  
{
	strcpy(smmobj_name[smmobj_noNode],name);
	smmobj_type[smmobj_noNOde]=type;
    smmobj_credit[smmobj_noNOde]=credit;
    smmobj_energy[smmobj_noNOde]=energy;
    
    smmobj_noNode++;
}

char* smmobj_getnodename[int node_nr];

int smmobj_getnodetype(int node_nr);
{
	return smmobj_type[node_nr];
}

#if 0



//member retrieving



//element to string
char* smmObj_getNodeName(smmNode_e type)
{
    return smmNodeName[type];
}

char* smmObj_getGradeName(smmGrade_e grade)
{
    return smmGradeName[grade];
}

