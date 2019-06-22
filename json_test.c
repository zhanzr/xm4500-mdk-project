#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <jansson.h>
#include <cJSON.h>
//#include <parson.h>
#include "FreeRTOS.h"
#include "task.h"

#define	TEST_LOOP_N	1000

//Benchmark test string
const char g_benchmark_json_str[] = " {\n" \
				"    \"timestamp\": 1556283673.1523004,\n" \
				"    \"task_uuid\": \"0ed1a1c3-050c-4fb9-9426-a7e72d0acfc7\",\n" \
				"    \"task_level\": [1, 2, 1],\n" \
				"    \"action_status\": \"started\",\n" \
				"    \"action_type\": \"main\",\n" \
				"    \"key\": \"value\",\n" \
				"    \"another_key\": 123,\n" \
				"    \"and_another\": [\"a\", \"b\"],\n" \
				"}\n";

//For Jasson test
/* Used by some code below as an example datatype. */
typedef struct str_poi_record {
    const char *precision;
    double lat;
    double lon;
    const char *address;
    const char *city;
    const char *state;
    const char *zip;
    const char *country;
}poi_record;

/* Our "days of the week" array: */
const char *str_weekdays[7] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
};

/* Our matrix: */
const int numbers[3][3] = {
        {0, -1, 0},
        {1, 0, 0},
        {0 ,0, 1}
};

/* Our "gallery" item: */
const int ids[4] = { 116, 943, 234, 38793 };
/* Our array of "records": */
const poi_record fields[2] = {
				{
                "zip",
                37.7668,
                -1.223959e+2,
								"Hot address",
                "SAN FRANCISCO",
                "CA",
                "94107",
                "US"
        },
        {
                "zip",
                37.371991,
                -1.22026e+2,
								"Cold address",
                "SUNNYVALE",
                "CA",
                "94085",
                "US"
        }
};

void test_jansson(void) {
    printf("Jansson Version:%s\n", JANSSON_VERSION); 
	TickType_t t[4];
	
	t[0] = xTaskGetTickCount();
	
	json_t* root;
	json_error_t jasson_err;  
	char* str_dump;

//    //Object Video Creating Test
//    {
//        json_t* fmt;
//        root = json_object();
//        json_object_set_new(root, "name", json_string("Jack (\"Bee\") Nimble"));
//        json_object_set_new(root, "format", fmt=json_object());
//        json_object_set_new(fmt,  "type", json_string("rect"));
//        json_object_set_new(fmt, "width", json_integer(1920));
//        json_object_set_new(fmt, "height", json_integer(1080));
//        json_object_set_new(fmt, "interlace", json_boolean(false));
//        json_object_set_new(fmt, "frame rate", json_integer(24));

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }

//    //1-Dim Array of String Test
//    {
//        root = json_array();

//        for(size_t i=0; i<7; ++i )
//        {
//            json_array_append_new( root, json_string(str_weekdays[i]) );
//        }

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }

//    //2-Dim Array of Integer Test
//    {
//        root = json_array();

//        for(size_t i=0; i<3; ++i )
//        {
//            json_t* jarr2 = json_array();
//            for(size_t j=0; j<3; ++j )
//            {
//                json_array_append_new( jarr2, json_integer((json_int_t)numbers[i][j]));
//            }
//            json_array_append_new( root, jarr2);
//        }

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }

//    //Object Gallery Creating Test
//    {
//        json_t* img;
//        json_t* thm;

//        root = json_object();
//        json_object_set_new(root, "Image", img=json_object());
//        json_object_set_new(img, "Width", json_integer(800));
//        json_object_set_new(img, "Height", json_integer(600));
//        json_object_set_new(img,  "Title", json_string("View from 15th Floor"));
//        json_object_set_new(img, "Thumbnail", thm=json_object());
//        json_object_set_new(thm, "Url", json_string("http:/*www.example.com/image/481989943"));
//        json_object_set_new(thm, "Height", json_integer(125));
//        json_object_set_new(thm, "Width", json_string("100"));

//        json_t* jarr = json_array();
//        for(size_t i=0; i<sizeof(ids)/sizeof(ids[0]); ++i)
//        {
//            json_array_append_new( jarr, json_integer((json_int_t)ids[i]));
//        }
//        json_object_set_new(img, "IDs", jarr);

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }

    //Array of "records" Test
    {
			  //C Struct Array to string			
        root = json_array();
			for(uint32_t loop_cnt=0; loop_cnt<TEST_LOOP_N; ++loop_cnt) {
        for (size_t i = 0; i < 2; i++)
        {
            json_t* item = json_object();

            json_object_set_new(item, "precision", json_string(fields[i].precision));
            json_object_set_new(item, "Latitude", json_real(fields[i].lat));
            json_object_set_new(item, "Longitude", json_real(fields[i].lon));
            json_object_set_new(item, "Address", json_string(fields[i].address));
            json_object_set_new(item, "City", json_string(fields[i].city));
            json_object_set_new(item, "State", json_string(fields[i].state));
            json_object_set_new(item, "Zip", json_string(fields[i].zip));
            json_object_set_new(item, "Country", json_string(fields[i].country));

            json_array_append_new( root, item);
        }
			}
			
				t[1] = xTaskGetTickCount();

        str_dump = json_dumps( root, 0 );
        printf( str_dump );
        printf("\n");

//				typedef struct str_poi_record {
//    const char *precision;
//    double lat;
//    double lon;
//    const char *address;
//    const char *city;
//    const char *state;
//    const char *zip;
//    const char *country;
//}poi_record;
//				
				//string to C Struct Array					
				t[2] = xTaskGetTickCount();

			json_t* item = json_object();

			for(uint32_t loop_cnt=0; loop_cnt<TEST_LOOP_N; ++loop_cnt) {
					root = json_loads(str_dump, 0, &jasson_err);
					size_t arr_len = json_array_size(root);				
					for(size_t i=0; i<arr_len; ++i) {								
						json_t* p_json_struct_obj = json_array_get(root, i);
																						
						item = json_object_get(p_json_struct_obj, "precision");
						printf("precision:%s\n", json_string_value(item)); 
																
						item = json_object_get(p_json_struct_obj, "Latitude");
						printf("Latitude:%f\n", json_real_value(item)); 
						
						item = json_object_get(p_json_struct_obj, "Longitude");
						printf("Longitude:%f\n", json_real_value(item)); 					
						
						item = json_object_get(p_json_struct_obj, "Address");
						printf("Address:%s\n", json_string_value(item)); 
						
						item = json_object_get(p_json_struct_obj, "City");
						printf("City:%s\n", json_string_value(item)); 
						
						item = json_object_get(p_json_struct_obj, "State");
						printf("State:%s\n", json_string_value(item)); 
						
						item = json_object_get(p_json_struct_obj, "Zip");
						printf("Zip:%s\n", json_string_value(item)); 
						
						item = json_object_get(p_json_struct_obj, "Country");
						printf("Country:%s\n", json_string_value(item)); 
				 
						printf("\n");
					}
				}
				json_decref( item );
				
        free( str_dump );
        json_decref( root );
				
				t[3] = xTaskGetTickCount();
    }

//    //null Test
//    {
//        root = json_object();
//        volatile double zero = 0.0;

//        json_object_set_new(root, "number", json_null());

//        str_dump = json_dumps( root, 0 );
//        printf( str_dump );
//        printf("\n");
//        ////flush_stdout();
//        free( str_dump );
//        json_decref( root );
//    }
		
		printf("%u %u[%u], %u %u[%u]\n", t[0], t[1], t[1]-t[0], t[2], t[3], t[3]-t[2]);
}

//For cJSON test
/* Create a bunch of objects as demonstration. */
static int print_preallocated(cJSON *root)
{
    /* declarations */
    char *out = NULL;
    char *buf = NULL;
    char *buf_fail = NULL;
    size_t len = 0;
    size_t len_fail = 0;

    /* formatted print */
    out = cJSON_Print(root);

    /* create buffer to succeed */
    /* the extra 5 bytes are because of inaccuracies when reserving memory */
    len = strlen(out) + 5;
    buf = (char*)malloc(len);
    if (buf == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* create buffer to fail */
    len_fail = strlen(out);
    buf_fail = (char*)malloc(len_fail);
    if (buf_fail == NULL)
    {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    /* Print to buffer */
    if (!cJSON_PrintPreallocated(root, buf, (int)len, 1)) {
        printf("cJSON_PrintPreallocated failed!\n");
        if (strcmp(out, buf) != 0) {
            printf("cJSON_PrintPreallocated not the same as cJSON_Print!\n");
            printf("cJSON_Print result:\n%s\n", out);
            printf("cJSON_PrintPreallocated result:\n%s\n", buf);
        }
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }

    /* success */
    printf("%s\n", buf);

    /* force it to fail */
    if (cJSON_PrintPreallocated(root, buf_fail, (int)len_fail, 1)) {
        printf("cJSON_PrintPreallocated failed to show error with insufficient memory!\n");
        printf("cJSON_Print result:\n%s\n", out);
        printf("cJSON_PrintPreallocated result:\n%s\n", buf_fail);
        free(out);
        free(buf_fail);
        free(buf);
        return -1;
    }
    //flush_stdout();

    free(out);
    free(buf_fail);
    free(buf);
    return 0;
}

/* Create a bunch of objects as demonstration. */
static void create_objects(void)
{
		TickType_t t[4];
	
	t[0] = xTaskGetTickCount();
	
    /* declare a few. */
    cJSON *root = NULL;
    cJSON *fmt = NULL;
    cJSON *img = NULL;
    cJSON *thm = NULL;
    cJSON *fld = NULL;
    int i = 0;

//    volatile double zero = 0.0;

    /* Here we construct some JSON standards, from the JSON site. */

    /* Our "Video" datatype: */
    root = cJSON_CreateObject();
//    cJSON_AddItemToObject(root, "name", cJSON_CreateString("Jack (\"Bee\") Nimble"));
//    cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
//    cJSON_AddStringToObject(fmt, "type", "rect");
//    cJSON_AddNumberToObject(fmt, "width", 1920);
//    cJSON_AddNumberToObject(fmt, "height", 1080);
//    cJSON_AddFalseToObject (fmt, "interlace");
//    cJSON_AddNumberToObject(fmt, "frame rate", 24);

//    /* Print to text */
//    if (print_preallocated(root) != 0) {
//        cJSON_Delete(root);
//        exit(EXIT_FAILURE);
//    }
//    cJSON_Delete(root);

//    /* Our "days of the week" array: */
//    root = cJSON_CreateStringArray(str_weekdays, 7);

//    if (print_preallocated(root) != 0) {
//        cJSON_Delete(root);
//        exit(EXIT_FAILURE);
//    }
//    cJSON_Delete(root);

//    /* Our matrix: */
//    root = cJSON_CreateArray();
//    for (i = 0; i < 3; i++)
//    {
//        cJSON_AddItemToArray(root, cJSON_CreateIntArray(numbers[i], 3));
//    }

//    /* cJSON_ReplaceItemInArray(root, 1, cJSON_CreateString("Replacement")); */

//    if (print_preallocated(root) != 0) {
//        cJSON_Delete(root);
//        exit(EXIT_FAILURE);
//    }
//    cJSON_Delete(root);

//    /* Our "gallery" item: */
//    root = cJSON_CreateObject();
//    cJSON_AddItemToObject(root, "Image", img = cJSON_CreateObject());
//    cJSON_AddNumberToObject(img, "Width", 800);
//    cJSON_AddNumberToObject(img, "Height", 600);
//    cJSON_AddStringToObject(img, "Title", "View from 15th Floor");
//    cJSON_AddItemToObject(img, "Thumbnail", thm = cJSON_CreateObject());
//    cJSON_AddStringToObject(thm, "Url", "http:/*www.example.com/image/481989943");
//    cJSON_AddNumberToObject(thm, "Height", 125);
//    cJSON_AddStringToObject(thm, "Width", "100");
//    cJSON_AddItemToObject(img, "IDs", cJSON_CreateIntArray(ids, 4));

//    if (print_preallocated(root) != 0) {
//        cJSON_Delete(root);
//        exit(EXIT_FAILURE);
//    }
//    cJSON_Delete(root);

    /* Our array of "records": */

			for(uint32_t loop_cnt=0; loop_cnt<TEST_LOOP_N; ++loop_cnt) {
				root = cJSON_CreateArray();
				for (i = 0; i < 2; i++)
				{
						cJSON_AddItemToArray(root, fld = cJSON_CreateObject());
						cJSON_AddStringToObject(fld, "precision", fields[i].precision);
						cJSON_AddNumberToObject(fld, "Latitude", fields[i].lat);
						cJSON_AddNumberToObject(fld, "Longitude", fields[i].lon);
						cJSON_AddStringToObject(fld, "Address", fields[i].address);
						cJSON_AddStringToObject(fld, "City", fields[i].city);
						cJSON_AddStringToObject(fld, "State", fields[i].state);
						cJSON_AddStringToObject(fld, "Zip", fields[i].zip);
						cJSON_AddStringToObject(fld, "Country", fields[i].country);
				}
							/* Print to text */
						char *str_dump = cJSON_Print(root);
						printf(str_dump);
						free(str_dump);
				
				    cJSON_Delete(root);
			}
						t[1] = xTaskGetTickCount();


			//    if (print_preallocated(root) != 0) {
//        cJSON_Delete(root);
//        exit(EXIT_FAILURE);
//    }
		
//    /* cJSON_ReplaceItemInObject(cJSON_GetArrayItem(root, 1), "City", cJSON_CreateIntArray(ids, 4)); */

//    if (print_preallocated(root) != 0) {
//        cJSON_Delete(root);
//        exit(EXIT_FAILURE);
//    }
//    cJSON_Delete(root);

//    root = cJSON_CreateObject();
//    cJSON_AddNumberToObject(root, "number", 1.0 / zero);

		
		printf("%u %u[%u], %u %u[%u]\n", t[0], t[1], t[1]-t[0], t[2], t[3], t[3]-t[2]);		
}

void test_cJSON(void){
    printf("Version: %s\n", cJSON_Version());

    /* Now some samplecode for building objects concisely: */
    create_objects();
}


////For parsson test
//void serialization_example(void) {
//    JSON_Value *root_value = json_value_init_object();
//    JSON_Object *root_object = json_value_get_object(root_value);
//    char *serialized_string = NULL;
//    json_object_set_string(root_object, "name", "John Smith");
//    json_object_set_number(root_object, "age", 25);
//    json_object_dotset_string(root_object, "address.city", "Cupertino");
//    json_object_dotset_value(root_object, "contact.emails", json_parse_string("[\"email@example.com\",\"email2@example.com\"]"));
//    serialized_string = json_serialize_to_string_pretty(root_value);
//    puts(serialized_string);
//    json_free_serialized_string(serialized_string);
//    json_value_free(root_value);
//}

//void test_parsson(void) {
//   serialization_example();
//}	

void test_json(void) {
			//Jasson test
//		test_jansson();
		
		//cJSON test
		test_cJSON();
//		
//		//parson test
//		test_parsson();
}
