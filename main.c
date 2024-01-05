#include <stdio.h>
#include <stdbool.h>

void my_utf8_encode(char *input, char *output) {
    // Iterate through each character in the input string until the null terminator is reached.
    while (*input != '\0') {
        //Check if the current character and the next one form the start of a Unicode escape sequence.
        if (*input == '\\' && *(input + 1) == 'u') {
            // Get the Unicode code point
            unsigned int codePoint = 0;
            //Extract the Unicode code point from the input string using sscanf.
            //input + 2 skips the "\u" part of the escape sequence
            //%4x specifies that we want to read up to four hexadecimal digits.
            sscanf(input + 2, "%4x", &codePoint);

            // Move the input pointer past the Unicode escape sequence
            input += 6;

            // Encode the UTF-8 sequence based on the code point, using the UTF-8 encoding logic from wikipedia
            if (codePoint <= 0x007F) { // byte format 0xxxxxxx - x unicode code point-
                // numerical value (in hexadecimal) assigned to every character and symbol in the Unicode standard
                //within the Ascii range 0 to 127 - one byte (according to wikipedia table)
                *output++ = (char)codePoint;
            } else if (codePoint <= 0x07FF) { // two bytes - 0x0080 to 0x7FF
                //110xxxxx
                *output++ = (char)(0xC0 | (codePoint >> 6)); // shifting to "xxxxx" part of the first byte
                //0xCO the binary 11000000 leading byte and casts the results to char
                //combined leading byte with unicode point
                // store the value at the memory location pointed to by output and then increment the output pointer
                // to point to the next memory location.
                *output++ = (char)(0x80 | (codePoint & 0x3F)); // 00111111 binary rep od 0x3F, 10000000 0x80 in binary
                //continuation bytes in a multi-byte UTF-8 character.
            } else if (codePoint <= 0xFFFF) { // three bytes 0x0800 to 0xFFFF
                *output++ = (char)(0xE0 | (codePoint >> 12)); //right shifting by 12 because 3 bytes
                //11100000 is binary leading for 3 bytes (0xE0)
                *output++ = (char)(0x80 | ((codePoint >> 6) & 0x3F)); //second byte
                *output++ = (char)(0x80 | (codePoint & 0x3F)); //similar to second byte encoding
            } else { // 4-bytes: 0x10000 to maximum
                *output++ = (char)(0xF0 | (codePoint >> 18)); //shifting 18 times to get to the first of thr four bytes
                *output++ = (char)(0x80 | ((codePoint >> 12) & 0x3F)); //same as the others
                *output++ = (char)(0x80 | ((codePoint >> 6) & 0x3F));
                *output++ = (char)(0x80 | (codePoint & 0x3F));
            }
        } else {
            // Copy regular ASCII character
            //copying a regular ASCII character from the input buffer to the output buffer and advancing
            // the pointers to the next characters in their respective buffers
            *output++ = *input++;
        }
    }

    *output = '\0'; // Null-terminate the output string
}

void appendUnicodeEscape(unsigned int codePoint, char **output) {
    //"helper function"
    //specifies that we want a four-digit hexadecimal representation with leading zeros.
    //format the Unicode escape sequence and write it to the output.
    sprintf(*output, "\\u%04X", codePoint);
    *output += 6; // Move the output pointer to the end of the escape sequence
}

int my_utf8_decode(unsigned char *input, unsigned char *output) {
    // The loop iterates through each character in the input string until the null terminator ('\0') is encountered.
    while (*input != '\0') {
        // retrieve the byte at the memory location pointed to by input and store it as an unsigned char named ch
        unsigned char ch = (unsigned char)*input;

        if (ch < 0x80) {
            // ASCII character
            //for ASCII - Unicode code points ranging from U+0000 to U+007F.
            *output++ = *input++;
        } else if ((ch & 0xE0) == 0xC0) { // 11100000 is binary of 0xE0 - using to mask the bytes and check
            //if its 11000000 - leading 110 meaning : 2 bytes-sequence
            // 2-byte UTF-8 sequence
            unsigned int codePoint = ((ch & 0x1F) << 6) | (input[1] & 0x3F);
            // Extracts the lower 5 bits of ch (since 0x1F is 00011111 in binary) and left-shifts them by 6 positions.
            //input[1] - second byte in the sequence
            //the binary value of 0x3F is 00111111
            //performing input[1] & 0x3F retains only the lower 6 bits of input[1] and sets the higher bits to 0.
            input += 2; //moving to next 2 character in the input (+2 because each is two bytes)
            appendUnicodeEscape(codePoint, &output);
        } else if ((ch & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence
            unsigned int codePoint = ((ch & 0x0F) << 12) | ((input[1] & 0x3F) << 6) | (input[2] & 0x3F);
            input += 3;
            appendUnicodeEscape(codePoint, &output);
        } else if ((ch & 0xF8) == 0xF0) {
            // 4-byte UTF-8 sequence
            unsigned int codePoint = ((ch & 0x07) << 18) | ((input[1] & 0x3F) << 12) | ((input[2] & 0x3F) << 6) | (input[3] & 0x3F);
            input += 4;
            appendUnicodeEscape(codePoint, &output);
        } else {
            // Invalid UTF-8 sequence, treat as ASCII
            *output++ = *input++;
        }
    }

    *output = '\0'; // Null-terminate the output string
    return 0; // Success
}

int my_utf8_strlen(char *string) {
    // Initializes a counter variable (count) to keep track of the number of characters in the string.
    int count = 0;
    //continues until the end of the string is reached (indicated by the null terminator '\0')
    while (*string != '\0') {

        /* Checks if the current byte is the start of a new UTF-8 character. The bitwise AND operation with 0xC0 and
        the inequality != 0x80 checks if the two most significant bits are not '10', which indicates the start of a
        new character */
        if ((*string & 0xC0) != 0x80) { //if the bits are not 10 - new char
            //The condition checks whether the two most significant bits of the byte are not the continuation
            // byte pattern, which would indicate the start of a new character.
            // If it's the start of a new UTF-8 character
            count++;
        }
        //moves the pointer to the next byte in the string
        string++;
    }
    return count;
}

char *my_utf8_charat(unsigned char *string, int index) {
    if (index < 0) {
        // Negative indices are considered invalid.
        return NULL;
    }

    // Iterate through the input string until either the end of the string ('\0') is reached or the specified index is 0
    while (*string != '\0' && index > 0) {
        unsigned char ch = (unsigned char)*string;

        if ((ch & 0xC0) != 0x80) {
            // Check if the current byte is the start of a new UTF-8 character
            index--;
        }

        string++;
    }

    if (*string == '\0' || (index > 0 && *string == '\0')) {
        // Check for out-of-bounds indices
        return NULL;
    }

    // At this point, 'string' points to the start of the character at the specified index
    // Return only the first byte of the character
    return string;
}

// Function to compare two UTF-8 strings
int my_utf8_strcmp( unsigned char *string1, unsigned char *string2) {
    while (*string1 != '\0' && *string2 != '\0') {
        // Decode the next character in each string
        unsigned int code_point1 = 0;
        unsigned int code_point2 = 0;
        int bytes1 = my_utf8_decode(string1, &code_point1); // decoded Unicode code points
        int bytes2 = my_utf8_decode(string2, &code_point2); // should contain the number of bytes used to
        // encode those characters in UTF-8.

        if (bytes1 == -1 || bytes2 == -1) {
            // Invalid UTF-8 sequence in either string
            // represent the number of bytes consumed during the UTF-8 decoding of string1 and string2
            // bytes1 and bytes2 should be greater than or equal to 0, indicating the number of bytes consumed
            return -1;
        }

        // Compare the decoded code points
        if (code_point1 != code_point2) {
            return code_point1 - code_point2;
        }

        // Move to the next character
        string1 += bytes1;
        string2 += bytes2;
    }

    // Check if one string is shorter than the other
    if (*string1 != '\0') {
        return 1; // string2 is shorter
    } else if (*string2 != '\0') {
        return -1; // string1 is shorter
    }

    return 0; // Both strings are equal
}

/* one of the 2 creative functions I decided to add - this function will take as an input
 * from the user the string, start index for substring extraction and length of the substring,
 * and returns a substring */

void my_utf8_substring(char *input, int start, int length, char *output) {
    int i = 0; //iteration purposes
    int inputLength = 0; //length of input string in bytes

    // Find the actual length of the input string in bytes
    while (input[i] != '\0') {
        inputLength++;
        i++;
    }

    // Initialize variables for UTF-8 decoding
    int currentIndex = 0; //Keep track of the current index in the input string

    // Iterate through the input string to find the starting index
    i = 0;
    while (i < inputLength && currentIndex < start) {
        unsigned char ch = (unsigned char)input[i];

        // Move to the next character based on UTF-8 encoding rules
        if (ch < 0x80) {
            i += 1;
        } else if ((ch & 0xE0) == 0xC0) {
            i += 2;
        } else if ((ch & 0xF0) == 0xE0) {
            i += 3;
        } else if ((ch & 0xF8) == 0xF0) {
            i += 4;
        } else {
            // Invalid UTF-8 sequence, move to the next byte
            i += 1;
        }

        currentIndex++;
    }

    // Extract the substring
    int j = 0;
    while (j < length && i < inputLength) {
        output[j] = input[i];
        j++;

        unsigned char ch = (unsigned char)input[i];

        // Move to the next character based on UTF-8 encoding rules
        if (ch < 0x80) {
            i += 1;
        } else if ((ch & 0xE0) == 0xC0) {
            i += 2;
        } else if ((ch & 0xF0) == 0xE0) {
            i += 3;
        } else if ((ch & 0xF8) == 0xF0) {
            i += 4;
        } else {
            // Invalid UTF-8 sequence, move to the next byte
            i += 1;
        }
    }

    // Null-terminate the output string
    output[j] = '\0';
}

// Function to check if a character is a UTF-8 whitespace character
bool isUTF8Whitespace(unsigned char ch) {
    return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
}

// Function to find the longest continuous sequence in a UTF-8 string
void findLongestContinuousSequence(char *input) {
    int i = 0;
    int currentLength = 0; //Length of the current continuous sequence
    int maxLength = 0; //current longest
    char *currentStart = NULL; //pointers to each one, points to NULL while initializes to avoid errors
    char *maxStart = NULL;

    // Iterate through the input string
    while (input[i] != '\0') {
        unsigned char ch = (unsigned char)input[i];

        // Move to the next character based on UTF-8 encoding rules
        if (ch < 0x80) {
            if (isUTF8Whitespace(ch)) {
                //  Reset the current sequence if the character is a whitespace
                currentLength = 0;
                currentStart = NULL;
            } else {
                // Update the current sequence
                if (currentStart == NULL) {
                    currentStart = &input[i];
                }
                currentLength++;
            }
            i++;
        } else if ((ch & 0xE0) == 0xC0) {
            i += 2;
        } else if ((ch & 0xF0) == 0xE0) {
            i += 3;
        } else if ((ch & 0xF8) == 0xF0) {
            i += 4;
        } else {
            // Invalid UTF-8 sequence, move to the next byte
            i++;
        }

        // Update the maximum sequence if needed
        if (currentLength > maxLength) {
            maxLength = currentLength;
            maxStart = currentStart;
        }
    }

    // Output the result
    if (maxStart != NULL) {
        printf("Longest continuous sequence: %.*s (length: %d)\n", maxLength, maxStart, maxLength);
    } else {
        printf("No continuous sequence found.\n");
    }
}


// Function to check if a given byte is a continuation byte
//continuation bytes start with pattern "10xxxxxx"
int is_continuation_byte(unsigned char byte) {
    return (byte & 0xC0) == 0x80; // checks if 10
}

// Function to check if a given byte is a start byte of a UTF-8 character
int is_start_byte(unsigned char byte) {
    return (byte & 0xC0) != 0x80; // ensure does not have '10' pattern
}

// Function to check if a given byte is a valid lead byte (for 2, 3, or 4-byte sequence)
//For example, valid-lead for 2-byte sequence: 110xxxxx	 (case 2)
int is_valid_lead_byte(unsigned char byte, int num_bytes) {
    // Check the high bits based on the number of bytes expected
    switch (num_bytes) {
        case 2:
            return (byte & 0xE0) == 0xC0;
        case 3:
            return (byte & 0xF0) == 0xE0;
        case 4:
            return (byte & 0xF8) == 0xF0;
        default:
            return 0;
    }
}

// Function to check if a given code point is a surrogate
//use in UTF-16 so should not appear in UTF-8
int is_surrogate(unsigned int code_point) {
    return (code_point >= 0xD800 && code_point <= 0xDFFF);
}

// Function to check if a given code point is overlong encoded
int is_overlong_encoding(unsigned char byte, unsigned int code_point) {
    if (code_point <= 0x7F) {
        // One-byte character, should not be overlong - range (0X00 to 0X7F)
        return (byte & 0x80) == 0x80;
    } else if (code_point <= 0x7FF) {
        // Two-byte character (0x80 to 0x7FF)
        return (byte & 0xE0) == 0xC0 && code_point <= 0x7F;
    } else if (code_point <= 0xFFFF) {
        // Three-byte character
        return (byte & 0xF0) == 0xE0 && code_point <= 0x7FF;
    } else {
        // Four-byte character
        return (byte & 0xF8) == 0xF0 && code_point <= 0xFFFF;
    }
}

// Function to check if a given code point is valid
//does not exceed maximum value in Unicode
int is_valid_code_point(unsigned int code_point) {
    return !is_surrogate(code_point) && code_point <= 0x10FFFF;
}

// Function to check if a UTF-8 string is valid
int my_utf8_check(unsigned char *string) {
    while (*string != '\0') {
        unsigned char lead_byte = *string;

        if (is_start_byte(lead_byte)) {
            // Determine the number of bytes for this character
            int num_bytes = 0;
            if ((lead_byte & 0x80) == 0) {
                num_bytes = 1; // 1-byte character
            } else if ((lead_byte & 0xE0) == 0xC0) {
                num_bytes = 2; // 2-byte character
            } else if ((lead_byte & 0xF0) == 0xE0) {
                num_bytes = 3; // 3-byte character
            } else if ((lead_byte & 0xF8) == 0xF0) {
                num_bytes = 4; // 4-byte character
            } else {
                return -1; // Invalid lead byte
            }

            // Check continuation bytes
            for (int i = 1; i < num_bytes; i++) {
                if (!is_continuation_byte(string[i])) {
                    return -2; // Invalid continuation byte
                }
            }

            // Decode the code point
            unsigned int code_point = 0;
            for (int i = 0; i < num_bytes; i++) {
                code_point = (code_point << 6) | (string[i] & 0x3F);
            }

            // Check for overlong encoding
            if (is_overlong_encoding(lead_byte, code_point)) {
                return -3; // Overlong encoding
            }

            // Check if the code point is valid
            if (!is_valid_code_point(code_point)) {
                return -4; // Invalid code point
            }

            // Move to the next character
            string += num_bytes;
        } else {
            return -5; // Unexpected continuation byte
        }
    }

    return 0; // Valid UTF-8 string
}

// Function to test the my_utf8_check function with meaningful names and error codes
void test_utf8_check(char *test_string, int expected_error) {
    int result = my_utf8_check((unsigned char *)test_string);

    printf("Testing: %s\n", test_string);

    if (result == expected_error) {
        printf("Result: Passed\n");
    } else {
        printf("Result: Failed (Expected: %d, Actual: %d)\n", expected_error, result);
    }

    printf("\n");
}

// Function to test all UTF-8 check cases
void test_all_utf8_checks() {
    // Test cases with meaningful names and error codes
    test_utf8_check("Valid string", 0);
    test_utf8_check("Invalid lead byte", -1);
    test_utf8_check("Invalid continuation byte", -2);
    test_utf8_check("Overlong encoding", -3); /* for example, using more bytes than necessary,
    U0041 need two bytes to encode, if you use 3 - overlong */
    //code point outside of valid Unicode range (0xD800 to 0xFFFF)
    test_utf8_check("Invalid code point", -4);
    //if continuation byte encountered by unexpected position
    test_utf8_check("Unexpected continuation byte", -5);
}
//tests for every function
// "helper function" since we cant not use any str library
int compareStrings(const char *str1, const char *str2) {
    while (*str1 != '\0' && *str2 != '\0') {
        if (*str1 != *str2) {
            return 1; // Strings are not equal
        }
        str1++;
        str2++;
    }

    if (*str1 != '\0' || *str2 != '\0') {
        return 1; // One string is shorter than the other
    }

    return 0; // Strings are equal
}
//Tests for deocding
void test_my_utf8_decode() {
    // Test with ASCII characters
    unsigned char input1[] = "Hello";
    unsigned char output1[20];
    int result1 = my_utf8_decode(input1, output1);
    printf("%s: input='%s', expected='Hello', actual='%s'\n",
           (result1 == 0 && compareStrings(output1, "Hello") == 0) ? "PASSED" : "FAILED", input1, output1);

    // Test with non-ASCII characters
    unsigned char input2[] = "你好";
    unsigned char output2[20];
    int result2 = my_utf8_decode(input2, output2);
    printf("%s: input='%s', expected='你好', actual='%s'\n",
           (result2 == 0 && compareStrings(output2, "你好") == 0) ? "PASSED" : "FAILED", input2, output2);

    // Test with invalid UTF-8 sequence
    unsigned char input3[] = "abc\xFFxyz";
    unsigned char output3[20];
    int result3 = my_utf8_decode(input3, output3);
    printf("%s: input='%s', expected=-1, actual=%d\n", (result3 == -1) ? "PASSED" : "FAILED", input3, result3);

    // Test with a mix of ASCII and non-ASCII characters
    unsigned char input4[] = "Hello 你好 😊";
    unsigned char output4[20];
    int result4 = my_utf8_decode(input4, output4);
    printf("%s: input='%s', expected='Hello 你好 😊', actual='%s'\n",
           (result4 == 0 && compareStrings(output4, "Hello 你好 😊") == 0) ? "PASSED" : "FAILED", input4, output4);
}
//Tests for strcmp

void test_my_utf8_strcmp() {
    // Test with equal strings
    unsigned char string1_1[] = "Hello";
    unsigned char string2_1[] = "Hello";
    int result1 = my_utf8_strcmp(string1_1, string2_1);
    printf("%s: string1='%s', string2='%s', expected=0, actual=%d\n", (result1 == 0) ? "PASSED" : "FAILED", string1_1,
           string2_1, result1);

    // Test with string1 shorter than string2
    unsigned char string1_2[] = "Hi";
    unsigned char string2_2[] = "Hello";
    int result2 = my_utf8_strcmp(string1_2, string2_2);
    printf("%s: string1='%s', string2='%s', expected<0, actual=%d\n", (result2 < 0) ? "PASSED" : "FAILED", string1_2,
           string2_2, result2);

    // Test with string1 longer than string2
    unsigned char string1_3[] = "Hello";
    unsigned char string2_3[] = "Hi";
    int result3 = my_utf8_strcmp(string1_3, string2_3);
    printf("%s: string1='%s', string2='%s', expected>0, actual=%d\n", (result3 > 0) ? "PASSED" : "FAILED", string1_3,
           string2_3, result3);

    // Test with non-ASCII characters
    unsigned char string1_4[] = "你好";
    unsigned char string2_4[] = "こんにちは";
    int result4 = my_utf8_strcmp(string1_4, string2_4);
    printf("%s: string1='%s', string2='%s', expected<0, actual=%d\n", (result4 < 0) ? "PASSED" : "FAILED", string1_4,
           string2_4, result4);

    // Test with equal non-ASCII characters
    unsigned char string1_5[] = "你好";
    unsigned char string2_5[] = "你好";
    int result5 = my_utf8_strcmp(string1_5, string2_5);
    printf("%s: string1='%s', string2='%s', expected=0, actual=%d\n", (result5 == 0) ? "PASSED" : "FAILED", string1_5,
           string2_5, result5);

    // Test with an empty string
    unsigned char string1_6[] = "";
    unsigned char string2_6[] = "Hello";
    int result6 = my_utf8_strcmp(string1_6, string2_6);
    printf("%s: string1='%s', string2='%s', expected<0, actual=%d\n", (result6 < 0) ? "PASSED" : "FAILED", string1_6,
           string2_6, result6);

}

//tests for my strlen function
void test_my_utf8_strlen() {
    // Test with ASCII characters
    unsigned char string1[] = "Hello";
    int result1 = my_utf8_strlen(string1);
    printf("%s: string='%s', expected=5, actual=%d\n", (result1 == 5) ? "PASSED" : "FAILED", string1, result1);

    // Test with non-ASCII characters
    unsigned char string2[] = "你好";
    int result2 = my_utf8_strlen(string2);
    printf("%s: string='%s', expected=2, actual=%d\n", (result2 == 2) ? "PASSED" : "FAILED", string2, result2);

    // Test with a mix of ASCII and non-ASCII characters
    unsigned char string3[] = "Hello 你好";
    int result3 = my_utf8_strlen(string3);
    printf("%s: string='%s', expected=9, actual=%d\n", (result3 == 9) ? "PASSED" : "FAILED", string3, result3);

    // Test with an empty string
    unsigned char string4[] = "";
    int result4 = my_utf8_strlen(string4);
    printf("%s: string='%s', expected=0, actual=%d\n", (result4 == 0) ? "PASSED" : "FAILED", string4, result4);
}

//Tests for my chrat function
void test_my_utf8_charat() {
    // Test with valid index within the string
    unsigned char string1[] = "Hello";
    char *result1 = my_utf8_charat(string1, 2);
    printf("%s: string='%s', index=2, expected='l', actual='%s'\n",
           (result1 != NULL && compareStrings(result1, "l") == 0) ? "PASSED" : "FAILED", string1, result1);

    // Test with valid index at the end of the string
    unsigned char string2[] = "你好";
    char *result2 = my_utf8_charat(string2, 2);
    printf("%s: string='%s', index=2, expected='好', actual='%s'\n",
           (result2 != NULL && compareStrings(result2, "好") == 0) ? "PASSED" : "FAILED", string2, result2);

    // Test with valid index at the start of the string
    unsigned char string3[] = "Hello 你好";
    char *result3 = my_utf8_charat(string3, 0);
    printf("%s: string='%s', index=0, expected='H', actual='%s'\n",
           (result3 != NULL && compareStrings(result3, "H") == 0) ? "PASSED" : "FAILED", string3, result3);

    // Test with invalid negative index
    unsigned char string4[] = "Hello";
    char *result4 = my_utf8_charat(string4, -1);
    printf("%s: string='%s', index=-1, expected=NULL, actual='%s'\n", (result4 == NULL) ? "PASSED" : "FAILED", string4,
           result4);

    // Test with invalid index beyond the string length
    unsigned char string5[] = "Hello";
    char *result5 = my_utf8_charat(string5, 10);
    printf("%s: string='%s', index=10, expected=NULL, actual='%s'\n", (result5 == NULL) ? "PASSED" : "FAILED", string5,
           result5);

    // Test with an empty string
    unsigned char string6[] = "";
    char *result6 = my_utf8_charat(string6, 0);
    printf("%s: string='%s', index=0, expected=NULL, actual='%s'\n", (result6 == NULL) ? "PASSED" : "FAILED", string6,
           result6);
}
//Tests for substring
void test_my_utf8_substring() {
    // Test with valid substring within the string
    char input1[] = "Hello World";
    char output1[20];
    my_utf8_substring(input1, 6, 5, output1);
    printf("%s: input='%s', start=6, length=5, expected='World', actual='%s'\n",
           (compareStrings(output1, "World") == 0) ? "PASSED" : "FAILED", input1, output1);

    // Test with valid substring starting from the beginning
    char input2[] = "你好 World";
    char output2[20];
    my_utf8_substring(input2, 0, 6, output2);
    printf("%s: input='%s', start=0, length=6, expected='你好 Wo', actual='%s'\n",
           (compareStrings(output2, "你好 Wo") == 0) ? "PASSED" : "FAILED", input2, output2);

    // Test with valid substring starting from the middle
    char input3[] = "Hello 你好";
    char output3[20];
    my_utf8_substring(input3, 6, 2, output3);
    printf("%s: input='%s', start=6, length=2, expected='你好', actual='%s'\n",
           (compareStrings(output3, "你好") == 0) ? "PASSED" : "FAILED", input3, output3);

    // Test with valid substring until the end of the string
    char input4[] = "Hello 你好";
    char output4[20];
    my_utf8_substring(input4, 6, 10, output4);
    printf("%s: input='%s', start=6, length=10, expected='你好', actual='%s'\n",
           (compareStrings(output4, "你好") == 0) ? "PASSED" : "FAILED", input4, output4);

    // Test with invalid negative start index
    char input5[] = "Hello";
    char output5[20];
    my_utf8_substring(input5, -2, 3, output5);
    printf("%s: input='%s', start=-2, length=3, expected='', actual='%s'\n",
           (compareStrings(output5, "") == 0) ? "PASSED" : "FAILED", input5, output5);

    // Test with invalid length exceeding the string length
    char input6[] = "Hello";
    char output6[20];
    my_utf8_substring(input6, 2, 10, output6);
    printf("%s: input='%s', start=2, length=10, expected='llo', actual='%s'\n",
           (compareStrings(output6, "llo") == 0) ? "PASSED" : "FAILED", input6, output6);

    // Test with an empty string
    char input7[] = "";
    char output7[20];
    my_utf8_substring(input7, 0, 5, output7);
    printf("%s: input='%s', start=0, length=5, expected='', actual='%s'\n",
           (compareStrings(output7, "") == 0) ? "PASSED" : "FAILED", input7, output7);
}
//Tests for my longest sequence
void test_findLongestContinuousSequence() {
    // Test with multiple spaces
    char input1[] = "Hello   World";
    printf("Input: %s\n", input1);
    printf("Expected: Hello\n");
    printf("Actual: ");
    findLongestContinuousSequence(input1);
    printf("\n");

    // Test with a mix of spaces and tabs
    char input2[] = "Hello\t\tWorld";
    printf("Input: %s\n", input2);
    printf("Expected: Hello\n");
    printf("Actual: ");
    findLongestContinuousSequence(input2);
    printf("\n");

    // Test with a mix of spaces, tabs, and newlines
    char input3[] = "Hello  \tWorld\n";
    printf("Input: %s\n", input3);
    printf("Expected: Hello\n");
    printf("Actual: ");
    findLongestContinuousSequence(input3);
    printf("\n");

    // Test with continuous sequence at the end
    char input4[] = "Hello World   ";
    printf("Input: %s\n", input4);
    printf("Expected: World\n");
    printf("Actual: ");
    findLongestContinuousSequence(input4);
    printf("\n");

    // Test with an empty string
    char input5[] = "";
    printf("Input: %s\n", input5);
    printf("Expected: No continuous sequence found.\n");
    printf("Actual: ");
    findLongestContinuousSequence(input5);
    printf("\n");

    // Test with a single character
    char input6[] = "H";
    printf("Input: %s\n", input6);
    printf("Expected: H (length: 1)\n");
    printf("Actual: ");
    findLongestContinuousSequence(input6);
    printf("\n");

    // Test with multiple continuous sequences
    char input7[] = "Hello  World   ";
    printf("Input: %s\n", input7);
    printf("Expected: World\n");
    printf("Actual: ");
    findLongestContinuousSequence(input7);
    printf("\n");
}

//Testing for encoding
void test_my_utf8_encode() {
    // Test with a simple ASCII string
    char input1[] = "Hello";
    char output1[20];
    my_utf8_encode(input1, output1);
    printf("%s: input='%s', expected='%s', actual='%s'\n", (compareStrings(input1, output1) == 0) ? "PASSED" : "FAILED",
           input1, input1, output1);

    // Test with a string containing a single Unicode escape sequence
    char input2[] = "H\\u00E9llo"; // "é" in Unicode
    char output2[20];
    my_utf8_encode(input2, output2);
    printf("%s: input='%s', expected='Héllo', actual='%s'\n",
           (compareStrings("Hé llo", output2) == 0) ? "PASSED" : "FAILED", input2, output2);

    // Test with a string containing multiple Unicode escape sequences
    char input3[] = "He\\u006C\\u006Co";
    char output3[20];
    my_utf8_encode(input3, output3);
    printf("%s: input='%s', expected='Hello', actual='%s'\n",
           (compareStrings("Hello", output3) == 0) ? "PASSED" : "FAILED", input3, output3);

    // Test with an empty string
    char input4[] = "";
    char output4[20];
    my_utf8_encode(input4, output4);
    printf("%s: input='%s', expected='', actual='%s'\n", (compareStrings("", output4) == 0) ? "PASSED" : "FAILED",
           input4, output4);

    // Test with a string containing only Unicode escape sequences
    char input5[] = "\\u0041\\u0042\\u0043";
    char output5[20];
    my_utf8_encode(input5, output5);
    printf("%s: input='%s', expected='ABC', actual='%s'\n", (compareStrings("ABC", output5) == 0) ? "PASSED" : "FAILED",
           input5, output5);
}

int main() {
    char input1[1024], input2[1024];
    char encodedOutput1[2048], encodedOutput2[2048];
    char decodedOutput1[2048], decodedOutput2[2048];

    // Encoding
    printf("Hey, please enter the first input for Encoding: ");
    fgets(input1, sizeof(input1), stdin);
    my_utf8_encode(input1, encodedOutput1);
    printf("Encoded UTF-8 string 1: %s\nLength: %d characters\n", encodedOutput1, my_utf8_strlen(encodedOutput1));

    printf("Hey, please enter the second input for Encoding: ");
    fgets(input2, sizeof(input2), stdin);
    my_utf8_encode(input2, encodedOutput2);
    printf("Encoded UTF-8 string 2: %s\nLength: %d characters\n", encodedOutput2, my_utf8_strlen(encodedOutput2));

    // Decoding
    printf("Hey, please enter an input for Decoding: ");
    fgets(input1, sizeof(input1), stdin);
    my_utf8_decode(input1, decodedOutput1);
    printf("Decoded version 1: %s\nLength: %d characters\n", decodedOutput1, my_utf8_strlen(decodedOutput1));

    printf("Hey, please enter another input for Decoding: ");
    fgets(input2, sizeof(input2), stdin);
    my_utf8_decode(input2, decodedOutput2);
    printf("Decoded version 2: %s\nLength: %d characters\n", decodedOutput2, my_utf8_strlen(decodedOutput2));


    // Index retrieval
    int index;
    printf("Enter index to retrieve character: ");
    scanf("%d", &index);
    char *result = my_utf8_charat(decodedOutput1, index);
    if (result != NULL) {
        printf("Character at index %d in Decoded version 1: %.*s\n", index, (int) (result - decodedOutput1), result);
    } else {
        printf("Error: Invalid index or improperly encoded string.\n");
    }

    // Substring extraction
    int start, length;
    printf("Enter start index for substring extraction: ");
    scanf("%d", &start);
    printf("Enter length for substring extraction: ");
    scanf("%d", &length);
    char substringOutput[2048];
    my_utf8_substring(decodedOutput1, start, length, substringOutput);
    printf("Substring: %s\n", substringOutput);

    // Find longest continuous sequence
    findLongestContinuousSequence(decodedOutput1);


    // Validate the UTF-8 encoded string
    int validationResult = my_utf8_check((unsigned char *)encodedOutput1);

    if (validationResult == 0) {
        printf("The input string is a valid UTF-8 encoded string.\n");
    } else {
        printf("Error: ");
        switch (validationResult) {
            case -1:
                printf("Invalid UTF-8 sequence.\n");
                break;
            case -2:
                printf("Unexpected continuation byte.\n");
                break;
            case -3:
                printf("Non-continuation byte before the end of the character.\n");
                break;
            case -4:
                printf("Invalid code point.\n");
                break;
            case -5:
                printf("Overlong encoding.\n");
                break;
            default:
                printf("Unknown error.\n");
        }
    }

    int comparisonResult = my_utf8_strcmp(encodedOutput1, encodedOutput2);
    if (comparisonResult ==0) {
        printf("Encoded strings are the same.\n");
    } else if(comparisonResult <0) {
        printf("Encoded string 1 is less than Encoded string 2.\n");
    } else {
        printf("Encoded string 2 is greater than Encoded string 1.\n");
    }
    return 0;
}

