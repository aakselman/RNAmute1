#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GC 3
#define AU 2
#define GU 1
#define DIFF 0




typedef struct point point;
struct point {
	int i;
	int j;
};




typedef struct mutation mutation;
struct mutation {
	point place;
	char iinit;
	char jinit;
	char imut;
	char jmut;
	int distance_from_stem;
	int rank;
	mutation* next;
};




typedef struct mutations mutations;
struct mutations {
	mutation* front;
	mutation* rear;
};





typedef struct one_mutation one_mutation;
struct one_mutation {
	mutation* source; // source of one point mutation.
	char init;
	char mut;
	int index;
};




typedef struct final_mutation final_mutation;
struct final_mutation {
	mutation* source;  // set of n-point mutations
	one_mutation* one_muts;
	char sequence[1000];
	char dot_repr[1000];
	char mutation_name[2000];
	double energy;
	int distance;
	int allDistance[4];//changed: cell number 0 = tree edit , 1= shap, 2=ham, 3=bp
	int new_bonds_num;
	int opt_dis_bonds_num;
	final_mutation* next;
};




typedef struct mut_set mut_set;
struct mut_set {
	int num_muts;
	mutation* npoint_mut;
	final_mutation* fnpoint_muts;
	mut_set* next;
};




typedef struct stem stem;
struct stem {
	point start;
	point end;
	mutations before;
	mutations after;
	stem* next;
};




typedef struct list list;
struct list {
	int distance;
	int allDistance[4];//changed: cell number 0 = tree edit , 1= shap, 2=ham, 3=bp
	double energy;
	char dot_repr[1000];
	int bonds_num;
	stem* stems;
	mutations disruptive;
	mut_set* np_muts;
	list* next;
};



typedef struct stack_node stack_node;
struct stack_node {
	int data;
	stack_node* next;
};


int distance_mode;  /* 1 for Humming distance and 2 for base pair distance */

int teDist, bpDist, shapDist = 0, hamDist, ev = 0;//if dist>0 than user wish to calculate this dist type

final_mutation* fms = NULL;
char temp[1000];

/* functions for linked list of sets of n-point mutations
"npoint_mut" - array of "num_muts" mutations	*/


mut_set* create_npoint_mutation(int num_muts, mutation* npoint_mut) {
	printf("create_npoint_mutation\n");
	int i;
	mut_set* temp = NULL;
	mutation* temp_mut = NULL;
	temp = (mut_set*)malloc(sizeof(mut_set));
	temp_mut = (mutation*)malloc(num_muts * sizeof(mutation));
	if (!temp || !temp_mut) {
		printf("Error in memory allocation!\n");
		exit(1);
	}
	for (i = 0; i<num_muts; i++)
		temp_mut[i] = npoint_mut[i];
	temp->npoint_mut = temp_mut;
	temp->fnpoint_muts = NULL;
	temp->num_muts = num_muts;
	temp->next = NULL;
	return temp;
}



int already_exists(mutation* npoint_mut, mut_set* head, int num_muts) {
	printf("already_exists\n");
	int i, j;
	int flag;
	for (; head; head = head->next) {
		flag = 0;
		for (i = 0; i<num_muts; i++)
			for (j = 0; j<num_muts; j++)
				if (npoint_mut[i].place.i == head->npoint_mut[j].place.i &&
					npoint_mut[i].place.j == head->npoint_mut[j].place.j) {
					flag++;
					break;
				}
		if (flag >= num_muts) return 1;
	}
	return 0;
}



void add_npoint_mutation(mut_set** head, int num_muts, mutation* npoint_mut) {
	printf("add_npoint_mutation\n");
	if (already_exists(npoint_mut, *head, num_muts)) return;
	mut_set* temp = create_npoint_mutation(num_muts, npoint_mut);
	temp->next = *head;
	*head = temp;
}


/* ----------------------------------------------- */






/* functions for linked list of mutations */

char give_base_for(int place, char* sequence) {
	printf("give_base_for\n");
	if (sequence[place] == 'A') return 'U';
	if (sequence[place] == 'U') return 'A';
	if (sequence[place] == 'C') return 'G';
	if (sequence[place] == 'G') return 'C';
	return 'E';
}



void init_mutations(mutations* mut) {
	printf("init_mutattions\n");
	mut->front = mut->rear = NULL;
}



mutation* create_mutation(int i, int j, char* sequence, int dist, int rank) {
	printf("create_mutation\n");
	mutation* temp = NULL;
	char ch1, ch2;
	temp = (mutation*)malloc(sizeof(mutation));
	if (!temp) {
		printf("Error in memory allocation!\n");
		exit(1);
	}
	temp->place.i = i;
	temp->place.j = j;
	temp->distance_from_stem = dist;
	if (j == -1) {
		temp->iinit = sequence[i];
		temp->jinit = 'E';
		temp->imut = 'E';
		temp->jmut = 'E';
		temp->rank = rank;
	}
	else {
		ch1 = give_base_for(j, sequence);
		ch2 = give_base_for(i, sequence);
		temp->iinit = sequence[i];
		temp->jinit = sequence[j];
		if (ch1 == sequence[i] || ch2 == sequence[j]) {
			temp->imut = 'E';
			temp->jmut = 'E';
			temp->rank = 0;
		}
		else {
			temp->imut = ch1;
			temp->jmut = ch2;
			temp->rank = rank;
		}
	}
	temp->next = NULL;
	return temp;
}



void add_mutation(mutations* mut, int i, int j, char* sequence, int dist, int rank) {
	printf("add_mutation\n");
	mutation* temp = create_mutation(i, j, sequence, dist, rank);
	if (mut->rear != NULL) {
		mut->rear->next = temp;
	}
	else
		mut->front = temp;
	mut->rear = temp;
}


/* ----------------------------------------------- */


/* functions for linked list of stems */


stem* create_stem(int start_i, int start_j, int end_i, int end_j) {
	printf("create_stem\n");
	stem* temp = NULL;
	temp = (stem*)malloc(sizeof(stem));
	if (!temp) {
		printf("Error in memory allocation!\n");
		exit(1);
	}
	temp->start.i = start_i;
	temp->start.j = start_j;
	temp->end.i = end_i;
	temp->end.j = end_j;
	temp->next = NULL;
	init_mutations(&temp->before);
	init_mutations(&temp->after);
	return temp;
}


void add_stem(stem** head, int start_i, int start_j, int end_i, int end_j) {
	printf("add_stem\n");
	stem* temp = create_stem(start_i, start_j, end_i, end_j);
	temp->next = *head;
	*head = temp;
}


/* ----------------------------------------------- */


/* functions for linked list of suboptimal solutions */


list* create_link(char* dot_repr, double energy) {
	printf("create_link\n");
	list* temp = NULL;
	temp = (list*)malloc(sizeof(list));
	if (!temp) {
		printf("Error in memory allocation!\n");
		exit(1);
	}
	temp->energy = energy;
	strcpy(temp->dot_repr, dot_repr);
	temp->next = NULL;
	temp->np_muts = NULL;
	temp->stems = NULL;
	init_mutations(&temp->disruptive);
	temp->bonds_num = 0;
	temp->distance = 0;

	return temp;
}


void add(list** head, char* dot_repr, double energy) {
	printf("add\n");
	list* temp = create_link(dot_repr, energy);
	temp->next = *head;
	*head = temp;
}

/* ----------------------------------------------- */



/* functions for the stack */

stack_node* create_stack_node(int data) {
	printf("create_stack_node\n");
	stack_node* temp = NULL;
	temp = (stack_node*)malloc(sizeof(stack_node));
	if (!temp) {
		printf("Error in memory allocation!\n");
		exit(1);
	}
	temp->data = data;
	temp->next = NULL;
	return temp;
}

void add_to_stack(stack_node** stack, int data) {
	printf("data=%d\n", data);
	printf("add_to_stack\n");
	stack_node* temp = create_stack_node(data);
	temp->next = *stack;
	*stack = temp;
}

int remove_from_stack(stack_node** stack) {
	printf("remove_from_stack\n");
	int x;
	stack_node* temp;
	if (!*stack) return 0;
	x = (*stack)->data;
	temp = *stack;
	*stack = (*stack)->next;
	free(temp);
	return x;
}


int is_empty_stack(stack_node* stack) {
	printf("is_empty_stack\n");
	return (stack == NULL) ? 1 : 0;
}


/* ------------------------------------------------- */




/*
void read_parameters(int* dist1,int* dist2,int* range,int* num_mut,char* method){;
printf("Enter the distance between optimal and suboptimal structures:\n");
scanf("%d",dist1);
printf("Enter the distance between suboptimal solutions:\n");
scanf("%d",dist2);
printf("Enter the e range:\n");
scanf("%d",range);
printf("Enter what distance to use (1-Humming, 2-base pair):\n");
scanf("%d",&distance_mode);
printf("Enter number of point mutations:\n");
scanf("%d",num_mut);
printf("Enter the method (f,s,d,e):\n");
scanf("%*c%c",method);

}
*/

//read the parameters from a file that the java-prog create
void readParameters(int* dist1, int* dist2, int* range, int* num_mut, int* readingFrame, char* method)
{
	char temp_method[10];
	char* seqTest;
	FILE *file;
	FILE *f;
	file = fopen("log.txt", "r");
	//file = fopen("seq.fa", "r");
    char ch[4],temp='a';


    
   
	fgets(ch,4, file);
	printf("%s\n",ch);
    *dist1 = atoi(&ch);


	fgets(ch, 4, file);
	printf("%s\n", ch);
	*dist2 = atoi(&ch);

	fgets(ch, 4, file);
	printf("%s\n", ch);
	*range = atoi(&ch);


	fgets(ch, 4, file);
	printf("%s\n", ch);
	hamDist = atoi(&ch);

	fgets(ch, 4, file);
	printf("%s\n", ch);
	teDist = atoi(&ch);

	fgets(ch, 4, file);
	printf("%s\n", ch);
	bpDist = atoi(&ch);

	fgets(ch, 4, file);
	printf("%s\n", ch);
	*num_mut = atoi(&ch);

	fgets(ch, 4, file);
	printf("%s\n", ch);
	*readingFrame = atoi(&ch);
	*readingFrame = -1;

	temp= fgetc(file);
	printf("%c\n", temp);
	if(temp=='h')
	{
		distance_mode = 1;
	}
	else{
		distance_mode = 2;
	}

	*method = fgetc(file);
	*method = fgetc(file);
	printf("%c\n", *method);
	//printf("%d  %d  %d  %d  %d  %d  %d  %d  %d  %c\n\n", *dist1, *dist2, *range, hamDist, teDist, bpDist, *num_mut, *readingFrame, distance_mode, *method);



	ev = 0;
	shapDist = 0;

}


/*
Calculates optimal and all subotimal secondary structures for sequence appearing in "seq"
and returns via parameters their dot-bracket representations.
parameters:
"opt" - dot-bracket representation and energy of optimal secondary structure.
"head" - list of dot-bracket representations of all suboptimal secondary structures.
"sequence" - query sequence
"range" - for RNAsubopt (-e range);
*/


void read_structures(list** suboptimals, list** optimal, char* sequence, int range) {
	printf("in_read_structures\n");
	FILE *opt;
	FILE *subs;
	char command[100];
	char dot_repr[1000];
	double energy, d;
	char tempp[1000];
	int ok = 2;


	sprintf(command, "RNAsubopt --deltaEnergy=%d --noLP < seq.fa > sub_structures", range);
	system(command);
	strcpy(command, "RNAfold --noLP seq.fa > opt_structure ");
	//system("convert -flatten rna.ps -quality 92 rna.jpg");
	system(command);

	opt = fopen("opt_structure", "r");
	subs = fopen("sub_structures", "r");
	if (!opt || !subs) {
		printf("Error in openning file!\n");
		exit(1);
	}

	// reading optimal structure 
	fscanf(opt, "%s%s%*c%*c%s", sequence, dot_repr, tempp);//&energy
	temp[strlen(tempp) - 1] = 0;
	energy = atof(tempp);
	add(optimal, dot_repr, energy);
	//------------------------------------------------------------------
	//reading suboptimal structures 
	fscanf(subs, "%s%lf%lf", sequence, &energy, &d);
	while ((ok = fscanf(subs, "%s%lf", dot_repr, &energy)) != EOF) {
		printf("In subs\n");
		add(suboptimals, dot_repr, energy);
	}

	fclose(opt);
	fclose(subs);

}




/************************************************ test printing functions **********************************************/



void print_final_mutations(final_mutation* fnpoint_muts) {
	printf("print_final_mutations\n");
	if (fnpoint_muts == NULL) return;
	for (; fnpoint_muts; fnpoint_muts = fnpoint_muts->next) {
		printf("%s %d\n", fnpoint_muts->sequence, fnpoint_muts->distance);
		printf("%s %s\n", fnpoint_muts->dot_repr, fnpoint_muts->mutation_name);
	}
	printf("\n");
}





void print_places_and_final_mutations(mut_set* np_muts) {
	printf("print_places_and_final_mutations\n");
	int i;
	if (np_muts == NULL) return;
	printf("The places of n-point mutations are:\n");
	for (; np_muts; np_muts = np_muts->next) {
		for (i = 0; i<np_muts->num_muts; i++)
			printf("(%d,%d,rank-%d)", np_muts->npoint_mut[i].place.i, np_muts->npoint_mut[i].place.j, np_muts->npoint_mut[i].rank);
		printf("\n");
		print_final_mutations(np_muts->fnpoint_muts);

	}
	printf("\n");
}




void print_mutations(mutation* muts) {
	printf("print_mutations\n");
	if (muts == NULL) return;

	for (; muts; muts = muts->next) {
		printf("([%d,%d],dist-%d,rank-%d)", muts->place.i, muts->place.j, muts->distance_from_stem, muts->rank);
	}
	printf("\n");
}




void print_stems(stem* stems) {
	printf("print_stems\n");
	if (stems == NULL) return;
	for (; stems; stems = stems->next) {
		printf("start: %d,%d. end: %d,%d.\n", stems->start.i, stems->start.j, stems->end.i, stems->end.j);
		print_mutations(stems->before.front);
		print_mutations(stems->after.front);
	}
	printf("\n");
}




/* prints dot-bracket representations and energies appearing in the list "head" */

void print_structures(list* head) {
	printf("print_structures\n");
	for (; head; head = head->next) {
		printf("%s %lf bonds=%d dist=%d\n", head->dot_repr, head->energy, head->bonds_num, head->distance);
		print_stems(head->stems);
		print_mutations(head->disruptive.front);
		print_places_and_final_mutations(head->np_muts);
	}
	printf("end of the list\n");
}

//changed
int check_reading_frame(char* rep1, char* rep2) {
	printf("check_reading_frame\n");
	//checks if the AA doesnt change and returns 1, else 0
	//if the amino-acid is serine
	if (((rep1[0] == 'U' && rep1[1] == 'C') || (rep1[0] == 'A' && rep1[1] == 'G' && (rep1[2] == 'U' || rep1[2] == 'C'))) && ((rep2[0] == 'U' && rep2[1] == 'C') || (rep2[0] == 'A' && rep2[1] == 'G' && (rep2[2] == 'U' || rep2[2] == 'C')))) return 1;

	//if the amino-acid is leu
	if (((rep1[0] == 'U' && rep1[1] == 'U' && (rep1[2] == 'A' || rep1[2] == 'G')) || (rep1[0] == 'C' && rep1[1] == 'U')) && ((rep2[0] == 'U' && rep2[1] == 'U' && (rep2[2] == 'A' || rep2[2] == 'G')) || (rep2[0] == 'C' && rep2[1] == 'U'))) return 1;
	//if the amino-acid is arg
	if (((rep1[0] == 'C'&& rep1[1] == 'G') || (rep1[0] == 'A'&& rep1[1] == 'G' && (rep1[2] == 'A' || rep1[2] == 'G'))) && ((rep2[0] == 'C' && rep2[1] == 'G') || (rep2[0] == 'A' && rep2[1] == 'G' && (rep2[2] == 'A' || rep2[2] == 'G')))) return 1;
	//if the amino acid is phe
	if ((rep1[0] == 'U' && rep1[1] == 'U' && (rep1[2] == 'U' || rep1[2] == 'C')) && (rep2[0] == 'U' && rep2[1] == 'U' && (rep2[2] == 'U' || rep2[2] == 'C'))) return 1;
	//if the amino acid is Ile
	if ((rep1[0] == 'A' && rep1[1] == 'U' && rep1[2] != 'G') && (rep2[0] == 'A' && rep2[1] == 'U' && rep2[2] != 'G')) return 1;
	//if the amino acid is valine
	if (rep1[0] == 'G' && rep1[1] == 'U' && rep2[0] == 'G' && rep2[1] == 'U')
		return 1;
	//if the amino acid is proline
	if (rep1[0] == 'C' && rep1[1] == 'C' && rep2[0] == 'C' && rep2[1] == 'C')
		return 1;
	//if the amino acid is Threonine
	if (rep1[0] == 'A' && rep1[1] == 'C' && rep2[0] == 'A' && rep2[1] == 'C')
		return 1;
	//if the amino acid is alanine
	if (rep1[0] == 'G' && rep1[1] == 'C' && rep2[0] == 'G' && rep2[1] == 'C')
		return 1;
	//if the amino acid is tyrosine
	if ((rep1[0] == 'U'&& rep1[1] == 'A' && (rep1[2] == 'U' || rep1[2] == 'C')) && (rep2[0] == 'U'&& rep2[1] == 'A' && (rep2[2] == 'U' || rep2[2] == 'C')))
		return 1;
	//if the amino acid is his
	if ((rep1[0] == 'C' && rep1[1] == 'A' && (rep1[2] == 'U' || rep1[2] == 'C')) && (rep2[0] == 'C' && rep2[1] == 'A' && (rep2[2] == 'U' || rep2[2] == 'C')))
		return 1;
	// if the amino acid is Gln
	if ((rep1[0] == 'C'&& rep1[1] == 'A' && (rep1[2] == 'A' || rep1[2] == 'G')) && (rep2[0] == 'C'&& rep2[1] == 'A' && (rep2[2] == 'A' || rep2[2] == 'G')))
		return 1;
	//if the amino acid is Asn
	if ((rep1[0] == 'A' && rep1[1] == 'A' && (rep1[2] == 'U' || rep1[2] == 'C')) && (rep2[0] == 'A' && rep2[1] == 'A' && (rep2[2] == 'U' || rep2[2] == 'C')))
		return 1;
	//if the amino acid is Lys
	if ((rep1[0] == 'A' && rep1[1] == 'A' && (rep1[2] == 'A' || rep1[2] == 'G')) && (rep2[0] == 'A' && rep2[1] == 'A' && (rep2[2] == 'A' || rep2[2] == 'G')))
		return 1;
	//if the amino acid is Asp
	if ((rep1[0] == 'G' && rep1[1] == 'A' && (rep1[2] == 'U' || rep1[2] == 'C')) && (rep2[0] == 'G' && rep2[1] == 'A' && (rep2[2] == 'U' || rep2[2] == 'C')))
		return 1;
	//if the amino acid is Glu
	if ((rep1[0] == 'G' && rep1[1] == 'A' && (rep1[2] == 'A' || rep1[2] == 'G')) && (rep2[0] == 'G' && rep2[1] == 'A' && (rep2[2] == 'A' || rep2[2] == 'G')))
		return 1;
	// if the amino acid is Cys
	if ((rep1[0] == 'U' && rep1[1] == 'G' && (rep1[2] == 'U' || rep1[2] == 'C')) && (rep2[0] == 'U' && rep2[1] == 'G' && (rep2[2] == 'U' || rep2[2] == 'C')))
		return 1;
	//if the amino acid is Gly
	if (rep1[0] == 'G' && rep1[1] == 'G'&&rep2[0] == 'G' && rep2[1] == 'G')
		return 1;
	//if the amino acid is Stop codon
	if (((rep1[0] == 'U' && rep1[1] == 'A' && (rep1[2] == 'A' || rep1[2] == 'G')) || (rep1[0] == 'U' && rep1[1] == 'G' && rep1[2] == 'A')) && ((rep2[0] == 'U' && rep2[1] == 'A' && (rep2[2] == 'A' || rep2[2] == 'G')) || (rep2[0] == 'U' && rep2[1] == 'G' && rep2[2] == 'A')))
		return 1;
	else return 0;
}



//changed
void print_2(final_mutation* fnpoint_muts, FILE* results, list* optimal, int* readingFrame, int* num_muts) {
	printf("print2\n");
	char rep1[3];
	char rep2[3];
	char mutP[4];
	int i, check, x, mutPlace, mp, k, frame;
	char tmp, mut, wt;

	if (fnpoint_muts == NULL) return;

	fprintf(results, "Mutation,Brackets,Energy,Sequence,Distance,Distance1,Distance2,Distance3,Distance4\n");

	for (; fnpoint_muts; fnpoint_muts = fnpoint_muts->next)
	{
		if (*readingFrame != -1)//checks if AA changed from mut. 
		{

			x = 1;
			for (k = 0; k<4; k++)
			{
				mutP[k] = '\0';
			}
			for (i = 0; i<*num_muts; i++)
			{

				wt = fnpoint_muts->mutation_name[x - 1];
				mutP[0] = fnpoint_muts->mutation_name[x];
				tmp = fnpoint_muts->mutation_name[x + 1];
				mut = tmp;//fnpoint_muts->mutation_name[x+2];
				if (tmp != 'A' & tmp != 'U' & tmp != 'G' & tmp != 'C')
				{
					mutP[1] = tmp;//fnpoint_muts->mutation_name[x+1];
					tmp = fnpoint_muts->mutation_name[x + 2];
					mut = tmp;
					if (tmp != 'A'&tmp != 'U'&tmp != 'G'& tmp != 'C')
					{
						mutP[2] = tmp;//fnpoint_muts->mutation_name[x+2];
						mut = fnpoint_muts->mutation_name[x + 3];
						x++;
					}
					x++;
				}

				sscanf(mutP, "%d", &mutPlace);
				mutPlace = mutPlace - 1;
				if (mutPlace >= *readingFrame)
				{
					mp = mutPlace;//mut place value in Mut name starts with 0;
					frame = (mp - *readingFrame) % 3;

					if (frame == 0)
					{
						rep1[0] = fnpoint_muts->sequence[mp];
						rep2[0] = wt;
						rep1[1] = fnpoint_muts->sequence[mp + 1];
						rep2[1] = rep1[1];
						rep1[2] = fnpoint_muts->sequence[mp + 2];
						rep2[2] = rep1[2];
					}
					else if (frame == 1)
					{
						rep1[1] = fnpoint_muts->sequence[mp];
						rep2[1] = wt;
						rep1[0] = fnpoint_muts->sequence[mp - 1];
						rep2[0] = rep1[0];
						rep1[2] = fnpoint_muts->sequence[mp + 1];
						rep2[2] = rep1[2];
					}
					else if (frame == 2)
					{
						rep1[2] = fnpoint_muts->sequence[mp];
						rep2[2] = wt;
						rep1[0] = fnpoint_muts->sequence[mp - 2];
						rep2[0] = rep1[0];
						rep1[1] = fnpoint_muts->sequence[mp - 1];
						rep2[1] = rep1[1];
					}

					check = check_reading_frame(rep1, rep2);

					x = x + 4;
					if (!check) i = *num_muts;
				}//readingFrame
				else
					x = x + 4;
			}//for i
			if (check)//AA doesn't change although the mute
			{

				fprintf(results, "%s,%s,", fnpoint_muts->mutation_name, fnpoint_muts->dot_repr);
				fprintf(results, "%lf,%s,%d,%dl%d,%d,%d\n", fnpoint_muts->energy, fnpoint_muts->sequence, fnpoint_muts->distance, fnpoint_muts->allDistance[0], fnpoint_muts->allDistance[1], fnpoint_muts->allDistance[2], fnpoint_muts->allDistance[3]);
			}//if check			
		}//if dontChangeAA
		else
		{

			fprintf(results, "%s,%s,", fnpoint_muts->mutation_name, fnpoint_muts->dot_repr);
			fprintf(results, "%lf,%s,%d,%d,%d,%d,%d\n", fnpoint_muts->energy, fnpoint_muts->sequence, fnpoint_muts->distance, fnpoint_muts->allDistance[0], fnpoint_muts->allDistance[1], fnpoint_muts->allDistance[2], fnpoint_muts->allDistance[3]);

		}//else


	}//for fn
}

//////changed
/*void print_1(mut_set* np_muts,FILE* results, list* optimal, int* readingFrame, int* num_muts)
{
int i;
if (np_muts==NULL) return;
for(; np_muts; np_muts=np_muts->next)
{
print_2(np_muts->fnpoint_muts,results, optimal, readingFrame, num_muts);
}
}*/

//////changed
void print_results(list* optimal, int* readingFrame, int* num_muts)
{
	printf("print_results\n");
	FILE* result = fopen("results.txt", "w");
	print_2(fms, result, optimal, readingFrame, num_muts);
	fclose(result);
	printf("\n");
}
/************************************************************************************************************************/





/* calculates the dot-bracket distance between two dot-bracket representations of
the RNA secondary structures */

int calculate_distance_te(char* rep1, char* rep2) {
	printf("calculate_distance_te\n");
	int distance;
	char temp[10000];
	FILE* reps = fopen("reps", "w");
	FILE* dist;
	if (!reps) {
		printf("Error in opening reps file!\n");
		exit(1);
	}
	fprintf(reps, "%s\n%s\n", rep1, rep2);
	fclose(reps);
	system("RNAdistance < reps > dist");
	dist = fopen("dist", "r");
	if (!dist) {
		printf("Error in opening dist file!\n");
		exit(1);
	}
	fscanf(dist, "%s", temp);
	fscanf(dist, "%d", &distance);
	fclose(dist);
	remove("reps");
	remove("dist");
	return distance;
}


/*int calculate_base_pair_distance(char* rep1, char* rep2){
int distance;
char temp[10000];
FILE* reps = fopen("reps","w");
FILE* dist;
if (!reps){
printf("Error in opening reps file!\n");
exit(1);
}
fprintf(reps,"%s\n%s\n",rep1,rep2);
fclose(reps);
system("bin/RNAdistance -DP < reps >dist");
dist = fopen("dist","r");
if (!dist){
printf("Error in opening dist file!\n");
exit(1);
}
fscanf(dist,"%s",temp);
fscanf(dist,"%d",&distance);
fclose(dist);
remove("reps");
remove("dist");
return distance;
} */



int calculate_humming_distance(char* rep1, char* rep2) {
	printf("calculate_hamming_distance\n");
	int sum = 0;
	while (*rep1 && *rep2) {
		if (rep1[0] != rep2[0]) sum++;
		rep1++;
		rep2++;
	}
	return sum;
}



int calculate_base_pair_distance(char* rep1, char* rep2) {
	printf("calculate_base_pair_distance\n");
	int sum = 0;
	int len;
	stack_node* stack = NULL;
	int *arr1, *arr2;
	int place, i;

	len = strlen(rep1);
	arr1 = (int*)malloc(len * sizeof(int));
	arr2 = (int*)malloc(len * sizeof(int));

	for (i = 0; i<len; i++) {
		arr1[i] = -1;
		arr2[i] = -1;
	}

	for (i = 0; rep1[i]; i++) {
		if (rep1[i] == '(') add_to_stack(&stack, i);
		if (rep1[i] == ')') {
			place = remove_from_stack(&stack);
			arr1[place] = i;
		}
	}

	stack = NULL;
	for (i = 0; rep2[i]; i++) {
		if (rep2[i] == '(') add_to_stack(&stack, i);
		if (rep2[i] == ')') {
			place = remove_from_stack(&stack);
			arr2[place] = i;
		}
	}

	for (i = 0; i<len; i++) {
		if ((arr1[i] == -1 && arr2[i] != -1) || (arr1[i] != -1 && arr2[i] == -1)) sum++;
		else if (arr1[i] != arr2[i]) sum += 2;
	}

	return sum;
}




//changed
/* calculates the distance between two dot bracket representations */
int calculate_distance(char* rep1, char* rep2) {
	printf("calculate_distance\n");
	int sum = 0;
	//if (distance_mode == 3) sum = calculate_distance_te(rep1,rep2);
	//if (distance_mode == 4) sum = calculate_humming_distance(rep1,rep2);//ToDo - calculate_distance_shap
	if (distance_mode == 1) sum = calculate_humming_distance(rep1, rep2);
	if (distance_mode == 2) sum = calculate_base_pair_distance(rep1, rep2);
	return sum;
}





/* calculates the number of hydrogen bonds in RNA secondary structure */
int calculate_number_of_bonds(char *sequence, char *repr) {
	printf("calculate_number_of_bonds\n");
	printf("sequence=%s     repr=%s\n", sequence, repr);
	int i;
	char ch1, ch2;
	int count = 0;
	stack_node* stack = NULL;
	for (i = 0; repr[i]; i++) {
		if (repr[i] == '(') add_to_stack(&stack, (int)sequence[i]);
		if (repr[i] == ')') {
			ch1 = (char)remove_from_stack(&stack);
			ch2 = sequence[i];
			if ((ch1 == 'G' && ch2 == 'C') || (ch1 == 'C' && ch2 == 'G')) count += 3;
			if ((ch1 == 'A' && ch2 == 'U') || (ch1 == 'U' && ch2 == 'A')) count += 2;
			if ((ch1 == 'G' && ch2 == 'U') || (ch1 == 'U' && ch2 == 'G')) count += 1;
		}
	}
	return count;
}





void test_calc_number_of_bonds() {
	char seq[] = "CGGCUCGUCGCAUCCUGGGGCUGAAGAAGGUCCCAAGGGUUGGGCUGUUCGCCCAUUAAAGCGGCACGCGAGCUGG";
	char rep[] = "((((..(((((..(((((((((......))))))..))).(((((.....))))).....)))))......)))).";
	printf("test of calc number of bonds: %d  %d\n", calculate_number_of_bonds(seq, rep), 58);
}





/* sorting suboptimal secondary structures according their number energy (stability) or distance from WT */
list* sort(list* suboptimals, int num_of_suboptimals, char ch) {
	printf("sort\n");
	double value1, value2;
	int i, j;
	list* curr = NULL, *prev = NULL, *next = NULL;
	int flag = 1;
	for (i = 0; i<num_of_suboptimals && flag; i++) {
		flag = 0;
		prev = NULL;
		for (curr = suboptimals, j = 0; j<num_of_suboptimals - i - 1 && curr; j++) {
			next = curr->next;
			if (next) {
				if (ch == 'b') {
					value1 = -1 * curr->energy;
					value2 = -1 * next->energy;
				}
				else if (ch == 'd') {
					value1 = curr->distance;
					value2 = next->distance;
				}
				if (value1 < value2) {
					flag = 1;
					curr->next = next->next;
					next->next = curr;
					if (!prev) {
						suboptimals = next;
						prev = next;
					}
					else {
						prev->next = next;
						prev = next;
					}
				}
				else {
					prev = curr;
					curr = curr->next;
				}
			}
			else curr = NULL;
		}
	}
	return suboptimals;
}







/* returns the linked list of stems (starting and ending points) belonging to structure "repr" */
stem* find_stems(char* repr) {
	printf("find_stems\n");
	stem* stems = NULL;
	int i;
	int start_i, start_j, end_i, end_j;
	stack_node* stack = NULL;
	for (i = 0; repr[i];) {
		if (repr[i] == '.') add_to_stack(&stack, -1);
		if (repr[i] == '(') add_to_stack(&stack, i);
		if (repr[i] == ')') {
			while (stack->data == -1) remove_from_stack(&stack);
			end_j = i;
			end_i = stack->data;
			for (; repr[i] == ')' && stack->data != -1; start_j = i++, start_i = remove_from_stack(&stack));
			add_stem(&stems, start_i, start_j, end_i, end_j);

		}
		else i++;
	}
	return stems;
}






void calculate_stems(list* structures) {
	printf("calculate_stems\n");
	list* temp = structures;
	for (; temp; temp = temp->next)
		temp->stems = find_stems(temp->dot_repr);
}





/* checks if mutation at i,j may disrupt one of the given stems */
int is_distruptive_mutation(int i, int j, stem* stems) {
	printf("is_distruptive_mutation\n");
	for (; stems; stems = stems->next) {
		if ((i >= stems->start.i && i <= stems->end.i) || (j <= stems->start.j && j >= stems->end.j) ||
			(i <= stems->start.j && i >= stems->end.j) || (j >= stems->start.i && j <= stems->end.i))
			return 1;
	}
	return 0;
}





/* checks if it is possible to introduce mutations at both ends of all stems, if yes it adds such mutations to the lists
of mutations "before" and "after". before - one end of the stem, after - another */

void calculate_mutations_for_subopt(stem* subopt_stems, stem* opt_stems, int length, char* sequence) {
	printf("calculate_mutations_for_subopt\n");
	int i, j, dist, rank;
	stem* temp = subopt_stems;
	for (; temp; temp = temp->next) {
		for (i = temp->start.i - 1, j = temp->start.j + 1, dist = 1; (i >= 0 && j<length && !is_distruptive_mutation(i, j, subopt_stems)); i--, j++, dist++) {
			if (is_distruptive_mutation(i, j, opt_stems)) rank = 3;
			else rank = 2;
			add_mutation(&temp->before, i, j, sequence, dist, rank);
		}
		for (i = temp->end.i + 1, j = temp->end.j - 1, dist = 1; (j - i >= 4 && !is_distruptive_mutation(i, j, subopt_stems)); i++, j--, dist++) {
			if (is_distruptive_mutation(i, j, opt_stems)) rank = 3;
			else rank = 2;
			add_mutation(&temp->after, i, j, sequence, dist, rank);
		}

	}
}





int aready_found_mutation(int i, list* sub) {
	printf("aready_found_mutation\n");
	stem* temp;
	for (temp = sub->stems; temp; temp = temp->next) {
		if (temp->before.front && (temp->before.front->place.i == i || temp->before.front->place.j == i)) return 1;
		if (temp->after.front && (temp->after.front->place.i == i || temp->after.front->place.j == i)) return 1;
	}
	return 0;
}




/* checks if there are mutations that do not disrupt suboptimal solution but do disrupt optimal
and adds such mutations to the linked list of mutations "disruptive"
this function is optional (invoked if "method" = 'b')*/

void calculate_mutations_that_disrupt_optimal(list* sub, list* opt, char* sequence) {
	printf("calculate_mutations_that_disrupt_optimal\n");
	int i;
	for (i = 0; sub->dot_repr[i]; i++)
		if (sub->dot_repr[i] == '.' &&  is_distruptive_mutation(i, i, opt->stems) && !aready_found_mutation(i, sub))
			add_mutation(&sub->disruptive, i, -1, sequence, 0, 1);

}






void calculate_mutations_for_all_suboptimals(list* subs, list* opt, int length, char*sequence, char method) {
	printf("calculate_mutations_for_all_suboptimals\n");
	for (; subs; subs = subs->next) {
		calculate_mutations_for_subopt(subs->stems, opt->stems, length, sequence);
		if (method == 'd' || method == 'e') calculate_mutations_that_disrupt_optimal(subs, opt, sequence);
	}
}





/* converts the list of stems with lists of mutations to array of lists of mutations */

mutation** get_array_of_mutations(stem* stems, int* length) {
	printf("get_array_of_mutations\n");
	int len = 0;
	stem* temp;
	mutation** array;
	int i = 0;

	for (temp = stems; temp; temp = temp->next) len += 2;
	*length = len;
	array = (mutation**)malloc(len * sizeof(mutation*));

	for (temp = stems; temp; temp = temp->next, i += 2) {
		array[i] = temp->before.front;
		array[i + 1] = temp->after.front;
	}
	return array;
}






/* checks if new found mutation clashes with one of the mutations that already in found set */

int clashing_mutation(mutation* found_set, int length, mutation new_mut) {
	printf("clashing_mutation\n");
	int i;
	for (i = 0; i<length; i++)
		if (found_set[i].place.i == new_mut.place.i || found_set[i].place.j == new_mut.place.j)
			return 1;
	return 0;
}







/* finds all possible sets of n-point mutations from array "muts" and calculates the sets in "found_set" array */
void find_npoint_mutations_rec(mutation** muts, int place1, int length, int num_mut, char* sequence,
	mutation* found_set, int place2, mut_set** npoint_mutations) {

	printf("find_npoint_mutations_rec\n");
	int i, j;
	mutation *temp, *temp1;
	if (muts == NULL) return;


	if (num_mut == 0) {
		add_npoint_mutation(npoint_mutations, place2, found_set);

		/*for (i=0; i<place2; i++)
		printf("(%d,%d) ",found_set[i].place.i,found_set[i].place.j);
		printf("\n");*/
		return;
	}

	if (place1 == length) return;


	find_npoint_mutations_rec(muts, place1 + 1, length, num_mut, sequence, found_set, place2, npoint_mutations);


	for (temp = muts[place1]; temp && !(*temp).rank; temp = temp->next);
	if (temp && !clashing_mutation(found_set, place2, *temp) && (*temp).rank) {
		found_set[place2] = *temp;
		find_npoint_mutations_rec(muts, place1 + 1, length, num_mut - 1, sequence, found_set, place2 + 1, npoint_mutations);
	}

	for (i = 2; i <= num_mut; i++) {
		for (temp = muts[place1]; temp; temp = temp->next) {
			for (j = 0, temp1 = temp; (temp1 && j<i && !clashing_mutation(found_set, place2 + j, *temp1)); temp1 = temp1->next, j++) {
				if ((*temp1).rank) found_set[place2 + j] = *temp1;
				else j--;
			}
			if (j == i)
				find_npoint_mutations_rec(muts, place1 + 1, length, num_mut - i, sequence, found_set, place2 + i, npoint_mutations);
		}
	}

}






void find_npoint_mutations_rec_fast(mutation** muts, int place1, int length, int num_mut, char* sequence,
	mutation* found_set, int place2, mut_set** npoint_mutations) {

	printf("find_npoint_mutations_rec_fast\n");
	int i, j;
	mutation *temp, *temp1;
	if (muts == NULL) return;


	if (num_mut == 0) {
		add_npoint_mutation(npoint_mutations, place2, found_set);

		/*for (i=0; i<place2; i++)
		printf("(%d,%d) ",found_set[i].place.i,found_set[i].place.j);
		printf("\n");*/
		return;
	}

	if (place1 == length) return;

	for (i = 0; i <= num_mut; i++) {
		for (j = 0, temp1 = muts[place1]; (temp1 && j<i && !clashing_mutation(found_set, place2 + j, *temp1)); temp1 = temp1->next, j++) {
			if ((*temp1).rank) found_set[place2 + j] = *temp1;
			else j--;
		}
		if (j == i)
			find_npoint_mutations_rec_fast(muts, place1 + 1, length, num_mut - i, sequence, found_set, place2 + i, npoint_mutations);
	}

}





void find_npoint_mutations_with_disruptive(mutation** muts, int length, int num_mut, char* sequence, mutation* found_set, int place2,
	mut_set** npoint_mutations, mutation* disruptive, char method) {
	printf("find_npoint_mutations_with_disruptive\n");
	if (muts == NULL) return;
	if (disruptive == NULL || num_mut == 0) {
		if (method == 'd')
			find_npoint_mutations_rec_fast(muts, 0, length, num_mut, sequence, found_set, place2, npoint_mutations);
		if (method == 'e')
			find_npoint_mutations_rec(muts, 0, length, num_mut, sequence, found_set, place2, npoint_mutations);
		return;
	}

	find_npoint_mutations_with_disruptive(muts, length, num_mut, sequence, found_set, place2, npoint_mutations, disruptive->next, method);
	found_set[place2] = *disruptive;
	find_npoint_mutations_with_disruptive(muts, length, num_mut - 1, sequence, found_set, place2 + 1, npoint_mutations, disruptive->next, method);
}





mut_set* find_npoint_mutations(stem* stems, mutations disruptive, char* sequence, int num_mut, char method) {
	printf("find_npoint_mutations\n");
	mutation** muts;
	mut_set* npoint_mutations = NULL;
	int length;
	mutation* found_set;

	muts = get_array_of_mutations(stems, &length);
	found_set = (mutation*)malloc((num_mut) * sizeof(mutation));


	if (method == 's') find_npoint_mutations_rec(muts, 0, length, num_mut, sequence, found_set, 0, &npoint_mutations);
	else if (method == 'f') find_npoint_mutations_rec_fast(muts, 0, length, num_mut, sequence, found_set, 0, &npoint_mutations);
	else find_npoint_mutations_with_disruptive(muts, length, num_mut, sequence, found_set, 0, &npoint_mutations, disruptive.front, method);
	return npoint_mutations;
}





void sort_one_point_mutation(one_mutation* one_muts, int num_muts) {
	printf("sort_one_point_mutation\n");
	int i, j, min_pos;
	one_mutation temp;
	for (i = 0; i< num_muts; i++) {
		min_pos = i;
		for (j = i + 1; j< num_muts; j++)
			if (one_muts[j].index < one_muts[min_pos].index)
				min_pos = j;

		temp = one_muts[i];
		one_muts[i] = one_muts[min_pos];
		one_muts[min_pos] = temp;
	}
}





void add_final_mutation(final_mutation** head, mut_set np_mut, one_mutation* one_muts, char* mutated_sequence,
	char* dot_repr, char* mutation_name, double energy, int distance, char* opt_dot_repr) {

	printf("add_final_mutation\n");
	int i;
	final_mutation* temp = (final_mutation*)malloc(sizeof(final_mutation));
	if (temp == NULL) {
		printf("Error in memory allocaton!\n");
		exit(1);
	}

	temp->source = np_mut.npoint_mut;
	/*temp->one_muts = (one_mutation*) malloc(np_mut.num_muts*sizeof(one_mutation));
	for(i=0; i<np_mut.num_muts; i++)
	temp->one_muts[i] = one_muts[i];*/
	temp->one_muts = one_muts;
	strcpy(temp->sequence, mutated_sequence);
	strcpy(temp->dot_repr, dot_repr);
	strcpy(temp->mutation_name, mutation_name);
	temp->energy = energy;
	temp->distance = distance;
	//////////////calculate allDistance: teDist,shapDist,hamDist,bpDist
	int k = 0;
	for (k; k<4; k++)
	{
		temp->allDistance[k] = -1;
	}
	if (teDist > 0)
	{
		temp->allDistance[0] = calculate_distance_te(opt_dot_repr, temp->dot_repr);
	}
	if (shapDist > 0)
	{
		temp->allDistance[1] = -1;//ToDo: calculate_distance_Shap
	}
	if (hamDist > 0)
	{
		temp->allDistance[2] = calculate_humming_distance(opt_dot_repr, temp->dot_repr);
	}
	if (bpDist > 0)
	{
		temp->allDistance[3] = calculate_base_pair_distance(opt_dot_repr, temp->dot_repr);
	}
	//////////////
	temp->new_bonds_num = 0;
	temp->opt_dis_bonds_num = 0;

	temp->next = fms;
	fms = temp;
}





/* checks if n-point mutation "one_muts" has distance >= dist, if yes adds it to list of final n-point mutations */
void check_and_add_final_npoint_mutation(final_mutation** fnp_muts, one_mutation* one_muts, mut_set np_mut, char* sequence, int dist, list* optimal) {
	printf("check_and_add_final_npoint_mutation\n");
	char mutated_sequence[1000];
	char mutation_name[2000];
	int i;
	double energy;
	FILE* mut_seq;
	FILE* mut_stract;
	int distance;
	char dot_repr[1000];
	char temp[1000];
	final_mutation* temp_fms = fms;

	strcpy(mutated_sequence, sequence);
	mutation_name[0] = '\0';
	for (i = 0; i<np_mut.num_muts; i++) {
		mutated_sequence[one_muts[i].index] = one_muts[i].mut;
		sprintf(mutation_name, "%s%c%d%c-", mutation_name, one_muts[i].init, one_muts[i].index + 1, one_muts[i].mut);
	}
	mutation_name[strlen(mutation_name) - 1] = '\0';

	while (temp_fms) {
		if (strcmp(temp_fms->mutation_name, mutation_name) == 0) return;
		temp_fms = temp_fms->next;
	}

	mut_seq = fopen("mutated_sequence.fa", "w");
	if (mut_seq == NULL) {
		printf("Error in openning file <mutated_sequence>!\n");
		exit(1);
	}

	fprintf(mut_seq, "%s\n", mutated_sequence);
	fclose(mut_seq);
	system("RNAfold --noLP mutated_sequence.fa > mutated_structure");

	mut_stract = fopen("mutated_structure", "r");
	if (mut_stract == NULL) {
		printf("Error in openning file <mutated_structure>!\n");
		exit(1);
	}


	fscanf(mut_stract, "%s%s%*c%*c%lf", temp, dot_repr, &energy);
	fclose(mut_stract);
	if ((distance = calculate_distance(dot_repr, optimal->dot_repr)) >= dist) {

		add_final_mutation(fnp_muts, np_mut, one_muts, mutated_sequence, dot_repr, mutation_name, energy, distance, optimal->dot_repr);
	}


}






void calculate_final_npoint_mutations(mut_set np_mut, char *sequence, int place, one_mutation* one_muts, final_mutation** fnp_muts, int dist, list* optimal) {
	printf("calculate_final_npoint_mutations\n");
	one_mutation* one;
	char nucleotides[] = { 'A','U','C','G' };
	int k;
	if (place == np_mut.num_muts) {
		one = (one_mutation*)malloc(np_mut.num_muts * sizeof(one_mutation));
		for (k = 0; k<np_mut.num_muts; k++) one[k] = one_muts[k];
		sort_one_point_mutation(one, np_mut.num_muts);
		check_and_add_final_npoint_mutation(fnp_muts, one, np_mut, sequence, dist, optimal);
		return;
	}
	if (np_mut.npoint_mut[place].place.j == -1) {
		one_muts[place].index = np_mut.npoint_mut[place].place.i;
		one_muts[place].init = np_mut.npoint_mut[place].iinit;
		one_muts[place].source = &np_mut.npoint_mut[place];
		for (k = 0; k<4; k++)
			if (nucleotides[k] != np_mut.npoint_mut[place].iinit) {
				one_muts[place].mut = nucleotides[k];
				calculate_final_npoint_mutations(np_mut, sequence, place + 1, one_muts, fnp_muts, dist, optimal);
			}
	}
	else {
		one_muts[place].index = np_mut.npoint_mut[place].place.i;
		one_muts[place].init = np_mut.npoint_mut[place].iinit;
		one_muts[place].mut = np_mut.npoint_mut[place].imut;
		one_muts[place].source = &np_mut.npoint_mut[place];
		calculate_final_npoint_mutations(np_mut, sequence, place + 1, one_muts, fnp_muts, dist, optimal);

		one_muts[place].index = np_mut.npoint_mut[place].place.j;
		one_muts[place].init = np_mut.npoint_mut[place].jinit;
		one_muts[place].mut = np_mut.npoint_mut[place].jmut;
		one_muts[place].source = &np_mut.npoint_mut[place];
		calculate_final_npoint_mutations(np_mut, sequence, place + 1, one_muts, fnp_muts, dist, optimal);
	}
}





void calculate_final_npoint_mutations_for_suboptimal(mut_set* np_muts, char* sequence, int dist, list* optimal) {
	printf("calculate_final_npoint_mutations_for_suboptimal\n");
	final_mutation* result;
	one_mutation one_muts[100];
	for (; np_muts; np_muts = np_muts->next) {
		result = NULL;
		calculate_final_npoint_mutations(*np_muts, sequence, 0, one_muts, &result, dist, optimal);
		np_muts->fnpoint_muts = result;
	}
}






void calculate_npoint_mutations_for_all_suboptimals(list* suboptimals, char* sequence, int num_mut, char method, int dist, list* optimal) {
	printf("calculate_npoint_mutations_for_all_suboptimals\n");
	list* temp;
	for (temp = suboptimals; temp; temp = temp->next) {
		temp->np_muts = find_npoint_mutations(temp->stems, temp->disruptive, sequence, num_mut, method);
		calculate_final_npoint_mutations_for_suboptimal(temp->np_muts, sequence, dist, optimal);
	}
}








/* filtering subotimal secondary structures */
list* filter_suboptimal_structures(char* sequence, list* suboptimals, list* optimal, int dist1, int dist2, int num_mut) {
	printf("filter_suboptimal_structures\n");
	list* temp1, *temp2, *temp;
	list* prev = NULL;
	int distance;
	int bonds_num_opt;
	int bonds_num_subs;
	int num_of_suboptimals;

	/* filtering suboptimal structures that are close to optimal (WT) */
	temp1 = suboptimals;
	while (temp1) {
		distance = calculate_distance(optimal->dot_repr, temp1->dot_repr);
		if (distance >= dist1) {
			temp1->distance = distance;
			////////////calculate allDistance: teDist,shapDist,hamDist,bpDist
			int i = 0;
			for (i; i<4; i++)
			{
				temp1->allDistance[i] = -1;
			}
			if (teDist > 0)
			{
				temp1->allDistance[0] = calculate_distance_te(optimal->dot_repr, temp1->dot_repr);
			}
			if (shapDist > 0)
			{
				temp1->allDistance[1] = -1;//ToDo: calculate_distance_shap
			}
			if (hamDist > 0)
			{
				temp1->allDistance[2] = calculate_humming_distance(optimal->dot_repr, temp1->dot_repr);
			}
			if (bpDist > 0)
			{
				temp1->allDistance[3] = calculate_base_pair_distance(optimal->dot_repr, temp1->dot_repr);
			}
			////////////
			prev = temp1;
			temp1 = temp1->next;
		}
		else {
			temp = temp1->next;
			free(temp1);
			if (!prev) suboptimals = temp;
			else prev->next = temp;
			temp1 = temp;
		}
	}

	//printf("after first filter\n");
	print_structures(suboptimals);
	/* filtering suboptimal structures that have insufficient number of H-bonds (stability) */
	temp1 = suboptimals;
	prev = NULL;
	num_of_suboptimals = 0;
	bonds_num_opt = calculate_number_of_bonds(sequence, optimal->dot_repr);
	optimal->bonds_num = bonds_num_opt;
	while (temp1) {
		bonds_num_subs = calculate_number_of_bonds(sequence, temp1->dot_repr);
		if (((bonds_num_subs + (2 * num_mut*GC)) - bonds_num_opt) >= DIFF) {
			num_of_suboptimals++;
			temp1->bonds_num = bonds_num_subs;
			prev = temp1;
			temp1 = temp1->next;
		}
		else {
			temp = temp1->next;
			free(temp1);
			if (!prev) suboptimals = temp;
			else prev->next = temp;
			temp1 = temp;
		}
	}


	//printf("after second filter\n");
	print_structures(suboptimals);
	print_structures(optimal);
	/* sorting suboptimal secondary structures according their number of H-bonds (stability) */
	suboptimals = sort(suboptimals, num_of_suboptimals, 'd');
	suboptimals = sort(suboptimals, num_of_suboptimals, 'b');

	//printf("after sorting\n");
	print_structures(suboptimals);
	/* filtering suboptimal structures that are close to each other */
	temp1 = suboptimals;
	while (temp1) {
		temp2 = temp1->next;
		prev = temp1;
		while (temp2) {
			distance = calculate_distance(temp1->dot_repr, temp2->dot_repr);
			if (distance >= dist2) {
				prev = temp2;
				temp2 = temp2->next;
			}
			else {
				temp = temp2->next;
				free(temp2);
				prev->next = temp;
				temp2 = temp;
			}
		}
		temp1 = temp1->next;
	}

	return suboptimals;
}






void test_find_mutations(char* repr, list* optimal, int length, char* sequence, int num_mut) {
	printf("test_find_mutations\n");
	int i, j;
	char tinoco[200][200];
	stem* temp;
	mut_set* np_m;
	stem* stems = find_stems(repr);
	calculate_mutations_for_subopt(stems, optimal->stems, length, sequence);
	printf("\ntest\n%s\n", repr);
	print_stems(stems);
	np_m = find_npoint_mutations(stems, optimal->disruptive, sequence, num_mut, 'f');
	print_places_and_final_mutations(np_m);


	for (i = 0; i<strlen(repr); i++)
		for (j = 0; j<strlen(repr); j++)
			if (i<j) tinoco[i][j] = '.';
			else if (i == j) tinoco[i][j] = '-';
			else tinoco[i][j] = ' ';

			for (temp = stems; temp; temp = temp->next) {
				for (i = temp->start.i, j = temp->start.j; i <= temp->end.i && j >= temp->end.j; i++, j--)
					tinoco[i][j] = '*';
			}

			for (i = 0; i<strlen(repr); i++)
				printf("%2c", sequence[i]);
			printf("\n");

			for (i = 0; i<strlen(repr); i++)
				printf("%2c", repr[i]);
			printf("\n");

			for (i = 0; i<strlen(repr); i++)
				printf("%2d", i);
			printf("\n");

			for (i = 0; i<strlen(repr); i++) {
				for (j = 0; j<strlen(repr); j++)
					printf("%2c", tinoco[i][j]);
				printf("%2d%c%c\n", i, repr[i], sequence[i]);
			}
}





int main() {
	printf("In main\n");
	list* optimal = NULL;
	list* suboptimal = NULL;
	char opt[1000];
	char sequence[1000];
	int dist1, dist2, range, length, num_mut, readingFrame;
	char method = ' ';

	readParameters(&dist1,&dist2,&range,&num_mut,&readingFrame,&method);
	read_structures(&suboptimal, &optimal, sequence, range);
	length = strlen(sequence);
	suboptimal = filter_suboptimal_structures(sequence, suboptimal, optimal, dist1, dist2, num_mut);
	calculate_stems(optimal);
	calculate_stems(suboptimal);
	calculate_mutations_for_all_suboptimals(suboptimal, optimal, length, sequence, method);
	calculate_npoint_mutations_for_all_suboptimals(suboptimal, sequence, num_mut, method, dist1, optimal);
	print_results(optimal, &readingFrame, &num_mut);
	FILE* result = fopen("results.txt", "a");
	fprintf(result, "WT,%s,%f,%s,0,-1,-1,-1,-1 \n", optimal->dot_repr, optimal->energy, sequence);
	fclose(result);
	
	FILE* end = fopen("end.txt", "a");
	fclose(end);
	return 0;
}
