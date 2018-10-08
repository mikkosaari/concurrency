#include "stdlib.h"

struct n{
	struct n* l;
	struct n* r;
	int a;
};

struct n* alloc(){
	struct n* p=(struct n*)malloc(sizeof(struct n));
	return (p);
}

void write(){
	int M=3;
	int c=0;
	struct n* v[15];
	while(c<M){
		if(c=0){		
			struct n* p=alloc();
			(*p).a=0;
			v[0]=(p);	
		}
		else{		
			struct n* l=alloc();
			struct n* r=alloc();
			(*(*p).l).a=0;
			(*(*p).r).a=0;	
		}
	}
	struct n* v1=((*p)).l;
	struct n* v2=((*p)).r;
	(*v1).a=0;
	(*v2).a=0;
}

int main(){
	struct n* p=alloc();
	printf("%u",(p));
	//struct n* v=(p);	
	(*p).a=0;	
	(*p).l=alloc();
	struct n* v1=((*p)).l;
	(*p).r=alloc();
	(*v1).a=0;
	(*(*p).r).a=0;

}
