#include<stdio.h>
#include<jansson.h>
#include<fcntl.h>
#include<unistd.h>
int main(int argc,char *argv[])
{
	int i;
	char text[4096];

	json_t *object=json_object();
	json_t *array = json_array();
	
	json_array_append_new(array, json_string("Vishwajeet"));
	json_array_append_new(array, json_string("Gaur"));
	for(i=0;i<10;i++)
	json_array_append_new(array, json_integer(i));
	
	json_object_set_new(object, "hello", json_string("world"));
	json_object_set_new(object, "number", json_integer(2));
	json_object_set_new(object, "By", array);

	//json_object_set_new(object, "object", json_pack("{ss}", "hello", "world"));
	//ret=json_object_set(object,"By",array);
	
	size_t size = json_dumpb(object, NULL,0, 0);
	char *buf = alloca(size);

//	json_dumps(object, JSON_INDENT(20)); 

	json_dumpb(object, text,size, 0);
        json_dump_file(object, "hello.json", 0);
	printf("Size is %d \nJson Format is :: %s\n",(int)size,text);
	json_decref(object);
	/* Freeing up all the new references is must while using json_object_set however if we use json_object_set_new you can only free json object and rest will be done automatically..!! */
	return 0;

}
