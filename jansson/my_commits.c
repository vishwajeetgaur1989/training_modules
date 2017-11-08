#include <stdlib.h>
#include <string.h>
#include<fcntl.h>
#include<unistd.h>
#include <jansson.h>

#define BUFFER_SIZE  (256 * 1024)  /* 256 KB */
void print_json(json_t *element, int indent);
void print_json_indent(int indent);
const char *json_plural(int count);
void print_json_object(json_t *element, int indent);
void print_json_array(json_t *element, int indent);
void print_json_string(json_t *element, int indent);
void print_json_integer(json_t *element, int indent);
void print_json_real(json_t *element, int indent);
void print_json_true(json_t *element, int indent);
void print_json_false(json_t *element, int indent);
void print_json_null(json_t *element, int indent);

void print_json(json_t *element, int indent) 
{
        switch (json_typeof(element)) {
                case JSON_OBJECT:
                        print_json_object(element, indent);
                        break;
                case JSON_ARRAY:
                        print_json_array(element, indent);
                        break;
                case JSON_STRING:
                        print_json_string(element, indent);
                        break;
                case JSON_INTEGER:
                        print_json_integer(element, indent);
                        break;
                case JSON_REAL:
                        print_json_real(element, indent);
                        break;
                case JSON_TRUE:
                        print_json_true(element, indent);
                        break;
                case JSON_FALSE:
                        print_json_false(element, indent);
                        break;
                case JSON_NULL:
                        print_json_null(element, indent);
                        break;
                default:
                        fprintf(stderr, "unrecognized JSON type %d\n", json_typeof(element));
		}
}
void print_json_indent(int indent) {
        int i;
        for (i = 0; i < indent; i++) { putchar(' '); }
}

const char *json_plural(int count) {
        return count == 1 ? "" : "s";
}

void print_json_object(json_t *element, int indent) {
        size_t size;
        const char *key;
        json_t *value;

        print_json_indent(indent);
        size = json_object_size(element);

        printf("JSON Object of %ld pair%s:\n", size, json_plural(size));
        json_object_foreach(element, key, value) {
                print_json_indent(indent + 2);
                printf("JSON Key: \"%s\"\n", key);
                print_json(value, indent + 2);
        }

}

void print_json_array(json_t *element, int indent) {
        size_t i;
        size_t size = json_array_size(element);
        print_json_indent(indent);

        printf("JSON Array of %ld element%s:\n", size, json_plural(size));
        for (i = 0; i < size; i++) {
                print_json(json_array_get(element, i), indent + 2);
        }
}

void print_json_string(json_t *element, int indent) {
        print_json_indent(indent);
        printf("JSON String: \"%s\"\n", json_string_value(element));
}

void print_json_integer(json_t *element, int indent) {
        print_json_indent(indent);
        printf("JSON Integer: \"%" JSON_INTEGER_FORMAT "\"\n", json_integer_value(element));
}

void print_json_real(json_t *element, int indent) {
        print_json_indent(indent);
        printf("JSON Real: %f\n", json_real_value(element));
}
void print_json_true(json_t *element, int indent) {
        (void)element;
        print_json_indent(indent);
        printf("JSON True\n");
}

void print_json_false(json_t *element, int indent) {
        (void)element;
        print_json_indent(indent);
        printf("JSON False\n");
}

void print_json_null(json_t *element, int indent) {
        (void)element;
        print_json_indent(indent);
        printf("JSON Null\n");
}



/* Return the offset of the first newline in text or the length of
   text if there's no newline */
static int newline_offset(const char *text)
{
	const char *newline = strchr(text, '\n');
	if(!newline)
		return strlen(text);
	else
		return (int)(newline - text);
}

struct write_result
{
	char *data;
	int pos;
};

static size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
	struct write_result *result = (struct write_result *)stream;

	if(result->pos + size * nmemb >= BUFFER_SIZE - 1)
	{
		fprintf(stderr, "error: too small buffer\n");
		return 0;
	}

	memcpy(result->data + result->pos, ptr, size * nmemb);
	result->pos += size * nmemb;

	return size * nmemb;
}

int main(int argc, char *argv[])
{
	size_t i;
	char *text=malloc(sizeof(char)*4096);
	char file[10];
	json_t *root;
	json_error_t error;
	
	strcpy(file,argv[1]);
	int fd=open(file,O_RDONLY);
	if(fd==-1)
	{
		perror("Open");
		exit(EXIT_FAILURE);
	}
	read(fd,text,4096);
	if(!text)
		return 1;

	root = json_loads(text, 0, &error);
	free(text);
	print_json(root,0);

	if(!root)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		return 1;
	}

	if(!json_is_array(root))
	{
		fprintf(stderr, "error: root is not an array\n");
		json_decref(root);
		return 1;
	}
	for(i = 0; i < json_array_size(root); i++)
	{
		json_t *data, *sha, *commit, *message;
		const char *message_text;

		data = json_array_get(root, i);
		if(!json_is_object(data))
		{
			fprintf(stderr, "error: commit data %d is not an object\n", (int)(i + 1));
			json_decref(root);
			return 1;
		}

		sha = json_object_get(data,"Inventum");
		if(!json_is_string(sha))
		{
			fprintf(stderr, "error: Inventum %d: Embedded 02 is not a string\n", (int)(i + 1));
			return 1;
		}

		commit = json_object_get(data,"title");
		if(!json_is_object(commit))
		{
			fprintf(stderr, "error: title %d: title is not an object\n", (int)(i + 1));
			json_decref(root);
			return 1;
		}

		message = json_object_get(commit, "Full Name");
		if(!json_is_string(message))
		{
			fprintf(stderr, "error: commit %d: message is not a string\n", (int)(i + 1));
			json_decref(root);
			return 1;
		}

		message_text = json_string_value(message);
		printf("%.8s %.*s\n",
				json_string_value(sha),
				newline_offset(message_text),
				message_text);
	}
	

	json_decref(root);
	json_t *object;
	json_object_set(object,"Name","Vishwajeet");

	return 0;
}
