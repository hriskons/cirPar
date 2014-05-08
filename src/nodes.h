#ifndef NODES_H
#define NODES_H

#define MAX_NAME_LENGTH 10

enum{
	//NODE_RCL_TYPE = 0,
	NODE_RESISTANCE_TYPE = 0 ,
	NODE_CAPACITY_TYPE,
	NODE_INDUCTANCE_TYPE,
	NODE_SOURCE_V_TYPE,
	NODE_SOURCE_I_TYPE,
	NODE_MOSFET_TYPE,
	NODE_BJT_TYPE
};

enum  {  
	PULSE_EXP=0,
	PULSE_SIN,
	PULSE_PWL,
	PULSE_PULSE
};

typedef struct pwl_pair{
	double ti;
	double ii;

	struct pwl_pair* next;
}PWL_PAIR_T;

typedef struct pair_list_t{
	
	PWL_PAIR_T* head;
	int n;
}PAIR_LIST;
/*
 * R: resistor
 * C: capacity
 * L: inductance
 */
typedef struct resistance{

	char name[MAX_NAME_LENGTH]; // these should be arrays
	long node1;
	long node2;

	double value; 
}RESISTANCE_T;


typedef struct capacity{

	char name[MAX_NAME_LENGTH]; // these should be arrays
	long node1;
	long node2;

	double value; 
}CAPACITY_T;

typedef struct inductance{

	char name[MAX_NAME_LENGTH]; // these should be arrays
	long node1;
	long node2;

	double value; 
}INDUCTANCE_T;

/* Voltage source.DC for now */
typedef struct source_v {
	char name[MAX_NAME_LENGTH];
	long node1;
	long node2;
	int mna_row;

	double value;

	char pulse_type;
	char is_ac;

	/*values for transient spec, might need to add specific nodes for each type (EXP,SIN etc) to save memory*/
	double i1;
	double i2;
	double td1;
	double td2;
	double tc1;
	double tc2; 

	double ia;
	double fr;
	double td;
	double df;
	double ph;

	double tr;
	double tf;
	double pw;
	double per;

	/*need to implement a dynamic way to add (ti ii) pairs for PWL transient spec*/
	PAIR_LIST* pair_list;

}SOURCE_V_T;

typedef struct source_i{
	char name[MAX_NAME_LENGTH];
	long node1;
	long node2;

	double value;

	char pulse_type;
	char is_ac;

	/*values for transient spec, might need to add specific nodes for each type (EXP,SIN etc) to save memory*/
	double i1;
	double i2;
	double td1;
	double td2;
	double tc1;
	double tc2; 

	double ia;
	double fr;
	double td;
	double df;
	double ph;

	double tr;
	double tf;
	double pw;
	double per;

	/*need to implement a dynamic way to add (ti ii) pairs for PWL transient spec*/
	PAIR_LIST* pair_list;
}SOURCE_I_T;

/* MOSFET transistor*/
typedef struct mos{
	char name[MAX_NAME_LENGTH];
	int drain;
	int gate;
	int source;
	int body;

	/* model name */

	double l;
	double w;

}MOSFET_T;

/* bipolar junction transistor*/
typedef struct bjt{

	char name[MAX_NAME_LENGTH];
	int collector;
	int base;
	int emitter;

	/* model name */

	/* area*/
}BJT_T;

/* diode */
typedef struct diode{
	char name[MAX_NAME_LENGTH];
	long node1;
	long node2;

	/* area */

	/* model name */

}DIODE_T;

/*
 * Higher level base node */
typedef union node{
	BJT_T bjt;
	MOSFET_T mosfet;
	SOURCE_V_T source_v;
	SOURCE_I_T source_i;
	RESISTANCE_T resistance;
	CAPACITY_T capacity;
	INDUCTANCE_T inductance;
	DIODE_T diode;
}NODE;
#endif
