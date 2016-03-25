/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */
#include "JSON.h"
#include <stddef.h>
#include <string.h>

/******************************************************
 *                      Macros
 ******************************************************/

#define MAX_BACKUP_SIZE 500
#define MAX_PARENTS 4

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

wiced_json_callback_t internal_json_callback;

/******************************************************
 *               Variable Definitions
 ******************************************************/

static char* previous_token   = NULL;
static wiced_json_object_t json_object =
{
    .object_string        = NULL,
    .object_string_length = 0,
    .value_type           = UNKNOWN_JSON_TYPE,
    .value                = NULL,
    .value_length         = 0,
    .parent_object        = NULL
};

static int32_t               parent_counter = 0;
static wiced_json_object_t   parent_json_object[MAX_PARENTS];

static uint8_t       incomplete_response            = 0;

static int32_t            object_counter            = 0;

static char*              string_start              = NULL;
static char*              string_end                = NULL;

static char*              value_start               = NULL;
static char*              value_end                 = NULL;

static wiced_JSON_types_t type                      = UNKNOWN_JSON_TYPE;

static char*              current_input_token;
static char*              end_of_input;

static char*              most_recent_object_marker = NULL;

char                      packet_backup[MAX_BACKUP_SIZE];

uint32_t                  number_of_bytes_backed_up;

static uint8_t            escape_token              = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

/* Register callbacks parser will use to populate fields*/
wiced_result_t wiced_JSON_parser_register_callback( wiced_json_callback_t json_callback )
{

    internal_json_callback = json_callback;

    return WICED_SUCCESS;
}

wiced_result_t wiced_JSON_parser( const char* json_input, uint32_t input_length )
{

    if ( incomplete_response )
    {

        // If there is enough room on backup buffer to hold new data and old backed up
        // data, parse the backup buffer
        if ( ( input_length + number_of_bytes_backed_up ) < MAX_BACKUP_SIZE )
        {
            memcpy( packet_backup + number_of_bytes_backed_up, json_input, input_length );
            current_input_token = packet_backup;
            end_of_input        = current_input_token + number_of_bytes_backed_up + input_length;
        }
        else
        {
            // We must drop entire json object from last buffer. Find start of new json object
            // in new packet, and being parsing from there
            while( *json_input != '{' )
            {
                json_input++;
                if(input_length--);
            }
            current_input_token = (char*)json_input;
            end_of_input        = current_input_token + input_length;
        }

        incomplete_response = 0;
    }
    else
    {
        current_input_token = (char*)json_input;
        end_of_input        = current_input_token + input_length;
        previous_token = current_input_token;
    }

    /* Parse through entire input */
    while ( current_input_token < end_of_input )
    {
        switch( *current_input_token )
        {
            /* This is a start of object token */
            case OBJECT_START_TOKEN:

                if ( escape_token )
                {
                    escape_token = 1;
                    break;
                }

                // In case the json is split across packets, record the most recent object market
                // and copy from this point forward
                most_recent_object_marker = current_input_token;

                /* Keep track of the number of objects open */
                object_counter++;

                type = JSON_OBJECT_TYPE;
                /* If we have already captured some string value, then this string must represent the name of this object */
                if( string_end )
                {

                    /* prepare JSON object. The object string was already piced up by the start of value token */
                    json_object.value_type           = type;
                    json_object.value                = NULL;
                    json_object.value_length         = 0;


                    /* Reset the string and value pointers */
                    string_start = NULL;
                    string_end   = NULL;
                    value_start  = NULL;
                    value_end    = NULL;
                }

                if( ( (json_object.value_type == JSON_ARRAY_TYPE) || ( json_object.value_type == JSON_OBJECT_TYPE ) ) &&
                    ( parent_counter < MAX_PARENTS ) )
                {
                    parent_json_object[parent_counter] = json_object;
                    json_object.parent_object           = &parent_json_object[parent_counter];
                    parent_counter++;
                }
                else if ( *previous_token == COMMA_SEPARATOR )
                {
                        json_object.parent_object = &parent_json_object[parent_counter];
                        parent_counter++;
                }
                previous_token = current_input_token;

                break;

            /* This is an end of object token */
            case OBJECT_END_TOKEN:

                if ( escape_token )
                {
                    escape_token = 0;
                    break;
                }

                object_counter--;

                /* Extract final value in object list. If we have already marked the beginning of a value, than this must be final value in object list */
                if ( value_start )
                {
                    value_end = current_input_token;
                    /* If previous was a string token, then this must be a string value */
                    if ( *previous_token == STRING_TOKEN )
                    {
                        type = JSON_STRING_TYPE;

                         /* Move value token to point prior to string token and to last character of string value*/
                         value_end   = previous_token -1 ;
                         value_start = string_start + 1;

                    }
                    else if( *previous_token == TRUE_TOKEN )
                    {
                        type = JSON_BOOLEAN_TYPE;

                        value_end   = previous_token + sizeof("true")-2;
                        value_start = previous_token;
                    }
                    else if( *previous_token == FALSE_TOKEN )
                    {
                        type = JSON_BOOLEAN_TYPE;

                        value_end   = previous_token + sizeof("false")-2;
                        value_start = previous_token;
                    }
                    else if( *previous_token == NULL_TOKEN )
                    {
                        type = JSON_NULL_TYPE;
                        value_end   = previous_token + sizeof("null")-2; ;
                        value_start = previous_token;
                    }
                    else
                    {
                        /* This must be a number value if not string. Arrays would have been picked up already by the end of array token */
                        type = JSON_NUMBER_TYPE;

                        /* Keep moving the value end token back till you encounter a digit */
                        while ( ( *value_end < '0' ) || (  *value_end > '9') )
                        {
                            value_end--;
                        }

                        /* Initialise the value_start token with value_end */
                        value_start = value_end;

                        /* Move value_start token until we encounter a non-digit value */
                        while ( ( ( *value_start >= '0' ) && (  *value_start <= '9') ) || ( *value_start == '.' ) || ( *value_start == '-' ) )
                        {
                            value_start--;
                        }

                        /*Point to first number */
                        value_start++;
                    }

                    /* Prepare JSON object */
                    json_object.value_type           = type;
                    json_object.value                = value_start;
                    json_object.value_length         = value_end - value_start + 1;

                    if ( internal_json_callback != NULL )
                    {
                        internal_json_callback( &json_object );
                    }

                    /* Reset the value pointers */
                    value_start  = NULL;
                    value_end    = NULL;
                    string_start = NULL;
                    string_end   = NULL;
                    type = UNKNOWN_JSON_TYPE;
                }
                if ( parent_counter )
                {
                    parent_counter--;

                    if ( parent_counter )
                    {
                        json_object.parent_object = &parent_json_object[parent_counter-1];
                    }
                    else
                    {
                        json_object.parent_object = &parent_json_object[parent_counter];
                    }
                }
                previous_token = current_input_token;

                break;

            case STRING_TOKEN:

                if ( escape_token )
                {
                    escape_token = 0;
                    break;
                }
                /* This indicates this must be closing token for object name */
                if( *previous_token == STRING_TOKEN )
                {
                        /* Get the last character of the string name */
                        string_end = current_input_token;

                }
                else
                {
                    /* Find start and end of of object name */
                    string_start = current_input_token;
                    string_end   = NULL;
                }
                previous_token = current_input_token;

                break;
            case TRUE_TOKEN:
                if ( escape_token )
                {
                    escape_token = 0;
                    break;
                }
                if( ( *previous_token == START_OF_VALUE ) &&
                    ( string_end ) )
                {
                        previous_token = current_input_token;
                        current_input_token = current_input_token + sizeof("true")-2;
                }

                break;
            case FALSE_TOKEN:
                if ( escape_token )
                {
                    escape_token = 0;
                    break;
                }
                if( ( *previous_token == START_OF_VALUE ) &&
                    ( string_end ) )
                {
                        /* Skip ahead as this must be boolean false */
                        previous_token = current_input_token;
                        current_input_token = current_input_token + sizeof("false")-2;
                }
                break;
            case NULL_TOKEN:
                if ( escape_token )
                {
                    escape_token = 0;
                    break;
                }
                if( ( *previous_token == START_OF_VALUE ) &&
                    ( string_end ) )
                {
                        previous_token = current_input_token;
                        current_input_token = current_input_token + sizeof("null")-2;
                }
                break;
            case ARRAY_START_TOKEN:

                if ( escape_token )
                {
                    escape_token = 0;
                    break;
                }
                /*This means the last object name must have an array value type*/
                type = JSON_ARRAY_TYPE;

                json_object.value_type           = type;
                json_object.value                = NULL;
                json_object.value_length         = 0;

                if ( internal_json_callback != NULL )
                {
                    internal_json_callback( &json_object );
                }

                /* Reset object string start/end tokens */
                string_start = NULL;
                string_end   = NULL;

                previous_token = current_input_token;

                break;

            case ARRAY_END_TOKEN:

                if ( escape_token )
                {
                    escape_token = 0;
                    break;
                }
                /* Check if there is a value we have not consumed yet */
                if ( value_start )
                {
                    /* If the token prior to the end of array is a string, then the last element in the array must be a string value */
                    if ( *previous_token == STRING_TOKEN )
                    {
                        type        = JSON_STRING_TYPE;
                        value_start = string_start+1;
                        value_end   = string_end-1;
                    }
                    else if( *previous_token == TRUE_TOKEN )
                    {
                        type = JSON_BOOLEAN_TYPE;

                        value_end   = previous_token + sizeof("true")-2;
                        value_start = previous_token;
                    }
                    else if( *previous_token == FALSE_TOKEN )
                    {
                        type = JSON_BOOLEAN_TYPE;

                        value_end   = previous_token + sizeof("false")-2; ;
                        value_start = previous_token;
                    }
                    else if( *previous_token == NULL_TOKEN )
                    {
                        type = JSON_NULL_TYPE;
                        value_end   = previous_token + sizeof("null")-2; ;
                        value_start = previous_token;
                    }
                    else
                    {
                        /* If the last element is not a string value it must be a number value */
                        type = JSON_NUMBER_TYPE;

                        value_end = current_input_token-1;

                        /* Keep moving the value end token back till you encounter a digit */
                        while ( ( *value_end < '0' ) || (  *value_end > '9') )
                        {
                            value_end--;
                        }

                        /* Initialise value_start with value_end token */
                        value_start = value_end;

                        /* Move value_start token until we encounter a non-digit value */
                        while ( ( ( *value_start >= '0' ) && (  *value_start <= '9') ) || ( *value_start == '.' ) || ( *value_start == '-' ) )
                        {
                            value_start--;
                        }
                        value_start++;

                    }

                    // Update the JSON object. The object string is set to null because we are just passing back the value of the array element.
                    // A call to indicate an array object and name would have been made from ARRAY START token
                    json_object.object_string        = NULL;
                    json_object.object_string_length = 0;

                    json_object.value_type           = type;
                    json_object.value                = value_start;
                    json_object.value_length         = value_end - value_start + 1;

                    /* Call post value callback giving, parent object, object and its value */
                    if ( internal_json_callback != NULL )
                    {
                        internal_json_callback( &json_object );
                    }

                    /* Reset the value pointers */
                    value_start  = NULL;
                    value_end    = NULL;
                    string_start = NULL;
                    string_end   = NULL;
                    type         = UNKNOWN_JSON_TYPE;

                }
                type = UNKNOWN_JSON_TYPE;
                previous_token = current_input_token;

                break;

            case START_OF_VALUE:

                if ( escape_token )
                {
                    escape_token = 0;
                    break;
                }

                if ( string_end )
                {
                    /* prepare JSON object */
                    json_object.object_string        = string_start + 1;
                    json_object.object_string_length = string_end - string_start - 1;
                    type = UNKNOWN_JSON_TYPE;
                    previous_token = current_input_token;
                }
                if ( value_start == NULL )
                {
                    value_start    = current_input_token;
                }

                break;

            case COMMA_SEPARATOR:

                if ( escape_token )
                {
                    escape_token = 0;
                    break;
                }

                /* Ignore comma separators in values */
                if ( ( string_start ) && ( string_end == NULL ) )
                {
                    break;
                }
                /* If this comma is within an array, it must be delimiting values, so extract the comma delimited value */
                else if ( type == JSON_ARRAY_TYPE )
                {
                    /* If the token prior to the comma was a string token, then the delimited value must be a string */
                    if ( *previous_token == STRING_TOKEN )
                    {
                        type = JSON_STRING_TYPE;

                         /* Move token to point prior to string token and to last character of string value*/
                         value_end   = previous_token -1 ;
                         value_start = string_start + 1;
                    }
                    else if( *previous_token == TRUE_TOKEN )
                    {
                        type = JSON_BOOLEAN_TYPE;

                        value_end   = previous_token + sizeof("true")-2;
                        value_start = previous_token;
                    }
                    else if( *previous_token == FALSE_TOKEN )
                    {
                        type = JSON_BOOLEAN_TYPE;

                        value_end   = previous_token + sizeof("false")-2;
                        value_start = previous_token;
                    }
                    else if( *previous_token == NULL_TOKEN )
                    {
                        type = JSON_NULL_TYPE;
                        value_end   = previous_token + sizeof("null")-2; ;
                        value_start = previous_token;
                    }
                    else
                    {
                        /* Delimited values must be a NUMBER if they are not a string */
                        type = JSON_NUMBER_TYPE;

                        /* Set value_end to point to current location */
                        value_end = current_input_token;

                        /* Point to last number. Keep moving the value end token back till you encounter a digit */
                        while ( ( *value_end < '0' ) || (  *value_end > '9') )
                        {
                            value_end--;
                        }

                        /* Initialise the value_start pointer to point to last digit */
                        value_start = value_end;

                        /* Increment value_start until you reach first number */
                        while ( ( ( *value_start >= '0' ) && (  *value_start <= '9') ) || ( *value_start == '.' ) || ( *value_start == '-' ) )
                        {
                            value_start--;
                        }

                        /*Point to first number */
                        value_start++;
                    }

                    /* prepare JSON object */
                    json_object.object_string        = NULL;
                    json_object.object_string_length = 0;
                    json_object.value_type           = type;
                    json_object.value                = value_start;
                    json_object.value_length         = value_end - value_start + 1;

                    if ( internal_json_callback != NULL )
                    {
                        internal_json_callback( &json_object );
                    }

                    string_start = NULL;
                    string_end   = NULL;
                    type = JSON_ARRAY_TYPE;

                }
                else if ( value_start )
                {
                    value_end = current_input_token;

                    /* Commas are only used to seperate values so this must indicate an end of value, which means last object information is for us */
                    if ( *previous_token == STRING_TOKEN )
                    {
                        type = JSON_STRING_TYPE;

                         value_end   = previous_token -1 ;
                         value_start = string_start + 1;
                    }
                    else if( *previous_token == ARRAY_END_TOKEN )
                    {
                        if ( string_start )
                        {
                            type        = JSON_STRING_TYPE;
                            value_start = string_start+1;
                            value_end   = string_end-1;
                        }
                        else
                        {
                            type = JSON_NUMBER_TYPE;

                            /* Keep moving the value end token back till you encounter a digit */
                            while ( ( *value_end < '0' ) || (  *value_end > '9') )
                            {
                                value_end--;
                            }

                            value_start = value_end;

                            while ( ( *value_start >= '0' ) && (  *value_start <= '9') )
                            {
                                value_start--;
                            }

                            value_start++;
                        }
                    }
                    else if( *previous_token == TRUE_TOKEN )
                    {
                        type = JSON_BOOLEAN_TYPE;

                        value_end   = previous_token + sizeof("true")-2;
                        value_start = previous_token;
                    }
                    else if( *previous_token == FALSE_TOKEN )
                    {
                        type = JSON_BOOLEAN_TYPE;

                        value_end   = previous_token + sizeof("false")-2;
                        value_start = previous_token;
                    }
                    else if( *previous_token == NULL_TOKEN )
                    {
                        type = JSON_NULL_TYPE;
                        value_end   = previous_token + sizeof("null")-2; ;
                        value_start = previous_token;
                    }
                    else
                    {

                        type = JSON_NUMBER_TYPE;

                        /* Keep moving the value end token back till you encounter a digit */
                        while ( ( *value_end < '0' ) || (  *value_end > '9') )
                        {
                            value_end--;
                        }

                        value_start = value_end;

                        while ( ( ( *value_start >= '0' ) && (  *value_start <= '9') ) || ( *value_start == '.' ) || ( *value_start == '-' ) )
                        {
                            value_start--;
                        }

                        value_start++;
                    }

                    json_object.value_type           = type;
                    json_object.value                = value_start;
                    json_object.value_length         = value_end - value_start + 1;

                    if ( internal_json_callback != NULL )
                    {
                        internal_json_callback( &json_object );
                    }

                    string_start = NULL;
                    string_end   = NULL;
                    value_start  = NULL;
                    value_end    = NULL;
                    type = UNKNOWN_JSON_TYPE;
                }

                previous_token = current_input_token;

                break;

            case ESCAPE_TOKEN:

                escape_token = 1;

                break;

            default:

                break;

        }//switch


        current_input_token++;
    }//while

        /* This means that a closing brace has not been found for an object. This data is split across packets */
    if ( object_counter )
    {
        memset( packet_backup,0x0,sizeof( packet_backup ) );

        // Copy everything from the most recent unfinished object onwards

        number_of_bytes_backed_up = end_of_input - most_recent_object_marker;

        memcpy( packet_backup, most_recent_object_marker, number_of_bytes_backed_up );


        incomplete_response = 1;

        return WICED_PARTIAL_RESULTS;
    }

    memset( &parent_json_object, 0x0, sizeof(parent_json_object) );

    incomplete_response = 0;

    object_counter      = 0;

    string_start        = NULL;
    string_end          = NULL;

    value_start         = NULL;
    value_end           = NULL;

    type                = UNKNOWN_JSON_TYPE;

    previous_token = NULL;

    return WICED_SUCCESS;
}

