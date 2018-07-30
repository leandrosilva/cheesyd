/*
 * cheesyd
 * 
 * It's a stub (obviously based on the educative examples of the great
 * wkhtmltopdf C bindings) of a pet project that I'm working on my spare time.
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wkhtmltox/pdf.h>
#include <hiredis.h>

/* Test redis connection */
void test_redis()
{
    redisContext *c;
    redisReply *reply;

    const char *hostname = "127.0.0.1";
    int port = 6379;
    struct timeval timeout = {1, 500000}; // 1.5 seconds
    c = redisConnectWithTimeout(hostname, port, timeout);

    if (c == NULL || c->err)
    {
        if (c)
        {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        }
        else
        {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    /* PING server */
    reply = redisCommand(c, "PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

    /* Disconnects and frees the context */
    redisFree(c);
}

/* Print out loading progress information */
void progress_changed(wkhtmltopdf_converter *c, int p)
{
    printf("%3d%%\r", p);
    fflush(stdout);
}

/* Print loading phase information */
void phase_changed(wkhtmltopdf_converter *c)
{
    int phase = wkhtmltopdf_current_phase(c);
    printf("%s\n", wkhtmltopdf_phase_description(c, phase));
}

/* Print a message to stderr when an error occures */
void error(wkhtmltopdf_converter *c, const char *msg)
{
    fprintf(stderr, "Error: %s\n", msg);
}

/* Print a message to stderr when a warning is issued */
void warning(wkhtmltopdf_converter *c, const char *msg)
{
    fprintf(stderr, "Warning: %s\n", msg);
}

/* Main method convert pdf */
int main()
{
    test_redis();

    wkhtmltopdf_global_settings *gs;
    wkhtmltopdf_object_settings *os;
    wkhtmltopdf_converter *c;

    /* Init wkhtmltopdf in graphics less mode */
    wkhtmltopdf_init(false);

    for (int i = 0; i < 5; i++)
    {
        /*
         * Create a global settings object used to store options that are not
         * related to input objects, note that control of this object is parsed to
         * the converter later, which is then responsible for freeing it
         */
        gs = wkhtmltopdf_create_global_settings();

        /* We want the result to be storred in the file called test.pdf */
        //// wkhtmltopdf_set_global_setting(gs, "out", "test.pdf");
        char pdf_name[12];
        snprintf(pdf_name, sizeof pdf_name, "test-p%i.pdf", (i + 1));
        printf("pdf [%s]", pdf_name);

        wkhtmltopdf_set_global_setting(gs, "out", pdf_name);
        //// wkhtmltopdf_set_global_setting(gs, "load.cookieJar", "myjar.jar");

        /*
         * Create a input object settings object that is used to store settings
         * related to a input object, note again that control of this object is parsed to
         * the converter later, which is then responsible for freeing it
         */
        os = wkhtmltopdf_create_object_settings();

        /* We want to convert to convert the qstring documentation page */
        //// wkhtmltopdf_set_object_setting(os, "page", "http://doc.trolltech.com/4.6/qstring.html");
        wkhtmltopdf_set_object_setting(os, "page", "sample1.html");
        wkhtmltopdf_set_object_setting(os, "load.windowStatus", "ready");

        /* Create the actual converter object used to convert the pages */
        c = wkhtmltopdf_create_converter(gs);

        /* Call the progress_changed function when progress changes */
        wkhtmltopdf_set_progress_changed_callback(c, progress_changed);

        /* Call the phase _changed function when the phase changes */
        wkhtmltopdf_set_phase_changed_callback(c, phase_changed);

        /* Call the error function when an error occures */
        wkhtmltopdf_set_error_callback(c, error);

        /* Call the waring function when a warning is issued */
        wkhtmltopdf_set_warning_callback(c, warning);

        /*
         * Add the the settings object describing the qstring documentation page
         * to the list of pages to convert. Objects are converted in the order in which
         * they are added
         */
        wkhtmltopdf_add_object(c, os, NULL);

        /* Perform the actual convertion */
        if (!wkhtmltopdf_convert(c))
            fprintf(stderr, "Convertion failed!");

        /* Output possible http error code encountered */
        printf("httpErrorCode: %d\n", wkhtmltopdf_http_error_code(c));

        /* Destroy the converter object since we are done with it */
        wkhtmltopdf_destroy_converter(c);
    }

    /* We will no longer be needing wkhtmltopdf funcionality */
    wkhtmltopdf_deinit();

    return 0;
}
