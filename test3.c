int a;
enum e1{
	A=11||15,
	B=!1.0,
	C=sizeof(a),
	D=A==B,
	E=3<=D,
	F=3!=2,
	G=1.1>1.3	
}ee;
enum e2
{
	Z=1,
	Y=3
}ee2;
int main()
{
	int i,*p;
	p=A;
	ee=a=i=1.1;
	scanf("%d",&i);
	for(;i<10;i++)
	{
		a=a+i;
		printf("i=%d, a=%d\n",i,a);
	}
	printf("i=%d, a=%d\n",i,a);
	if(ee>0)
		if(a>41)
			printf("wow\n");
		else
			printf("how %d\n",a);
	return 0;
}

