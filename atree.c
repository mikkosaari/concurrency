#include "time.h"
#include "stdlib.h"

#define M 8
#define N 3

struct node{
	int a[M];
	struct node *left;
	struct node *right;
};


/*
struct v* alloc(){
	struct v* vp=(struct v*)malloc(sizeof(struct v));
	return vp;
}*/
float init(){
	srand(time(NULL));
	float r=random();
	return r;
}
float ch(){
	int t=init();
	int t0=t%2;	
	return t0;
}
/*
struct v* part(struct v* vp){
	
	int j=0;
	int N=3;
	while(j<N){	
		if(j==0){
			for(int i=0;i<8;i++){
				(*vp).a[i]=i;
			}
		}
			
		if(j==1){			
			for(int i=0;i<(8/(j+1));i++){	
				(*(*vp).p[0]).a[i]=(*vp).a[i];
			}
			for(int i=(8/(j+1));i<8;i++){	
				(*(*vp).p[1]).a[i]=(*vp).a[i];
			}
		}
		if(j==2){			
			for(int i=0;i<(8/(j+1));i++){	
				
			}	
			}			
		}
		for(int i=0;i<(8/j);i++){	
			if(j==1){			
				(*(*vp).p[0]).a[i]=(*vp).a[i];
			}
			if(j==2){			
				vp->vp->vp->a[i]=vp->vp->a[i];
			}			
		}
	}
	return (vp);

}*/
void comp();

int main(){
	int counter=0;

	struct node* ptr=(struct node*)malloc(sizeof(struct node));
	
	(*ptr).right=(struct node*)malloc(sizeof(struct node));
	(*ptr).left=(struct node*)malloc(sizeof(struct node));
	
/*	for(int i=0;i<2;i++){	
		(*(*vp).p[0]).p[i]=(struct v*)malloc(sizeof(struct v));
	}
	for(int i=0;i<2;i++){	
		(*(*vp).p[1]).p[i]=(struct v*)malloc(sizeof(struct v));
	}

	for(int i=0;i<2;i++){	
		(*(*(*vp).p[0]).p[0]).p[i]=(struct v*)malloc(sizeof(struct v));
	}
	for(int i=0;i<2;i++){	
		(*(*(*vp).p[0]).p[1]).p[i]=(struct v*)malloc(sizeof(struct v));
	}
	for(int i=0;i<2;i++){	
		(*(*(*vp).p[1]).p[0]).p[i]=(struct v*)malloc(sizeof(struct v));
	}
		for(int i=0;i<2;i++){	
		(*(*(*vp).p[1]).p[1]).p[i]=(struct v*)malloc(sizeof(struct v));
	}



/*	for(int i=0;i<2;i++){	
		vp->p[1]->p[i]=(struct v*)malloc(sizeof(struct v));
	}*/*/

	//int t2=init();

/*	while(t2==0){	
		printf("%u",t2);
		i1();
	}*/
//	(void*)(vp)=(vp);
//	(*vp).FP=&f;
	
}
