/*
 * cheesyd
 * 
 * It's a pet project for my very spare time :( that started off based on
 * the educative examples of the great wkhtmltopdf C bindings.
 *
 * Leandro Silva <leandrodoze@gmail.com>
 * https://github.com/leandrosilva/cheesyd
 */

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <sstream>
#include <fstream>
#include <iterator>
#include <vector>

#include <wkhtmltox/pdf.h>
#include <json11/json11.hpp>

#include "job.hpp"

// Exit signal handling
volatile std::sig_atomic_t interrupt_signal;

void signal_handler(int signum) {
    interrupt_signal = signum;
    std::cout << "\nGot interrupt signal " << interrupt_signal << "\n";
}

// Print out percentage of loading progress information
void progress_changed(wkhtmltopdf_converter *c, int p) {
    printf("- %3d%%\r", p);
    fflush(stdout);
}

// Print loading phase information
void phase_changed(wkhtmltopdf_converter *c) {
    int phase = wkhtmltopdf_current_phase(c);
    printf("- %s\n", wkhtmltopdf_phase_description(c, phase));
}

// Print a message to stderr when an error occures
void error(wkhtmltopdf_converter *c, const char *msg) {
    fprintf(stderr, "- Error: %s\n", msg);
}

// Print a message to stderr when a warning is issued
void warning(wkhtmltopdf_converter *c, const char *msg) {
    fprintf(stderr, "- Warning: %s\n", msg);
}

// Main method convert pdf
int main() {
    // Gets a job_manager to work on
    auto job_manager = cheesyd::JobManager::Create();
    if (job_manager == nullptr) {
        std::cerr << "Fail to create JobManager\n";
        exit(123);
    }

    // WK magic stuff
    wkhtmltopdf_global_settings *global_settings;
    wkhtmltopdf_object_settings *object_settings;
    wkhtmltopdf_converter *converter;

    // Init wkhtmltopdf in graphics less mode
    wkhtmltopdf_init(false);
    std::cout << "WK is ready to work\n";

    // Signal handler
    std::signal(SIGINT, signal_handler);

    // Infinite loop until signal ctrl-c (KILL) received
    while (!interrupt_signal) {
        std::string job_id = job_manager->DequeueJob();
        if (job_id.empty()) {
            std::cout << "Nothing to work yet\n";
            std::this_thread::sleep_for(std::chrono::seconds(3));
            continue;
        }
        std::cout << "Got a conversion job: " << job_id << "\n";

        try {
            auto job_data = job_manager->GetJobData(job_id);
            std::cout << "Job data: " << job_data.payload.dump() << "\n";

            /*
             * Create a global settings object used to store options that are not
             * related to input objects, note that control of this object is parsed to
             * the converter later, which is then responsible for freeing it
             */
            global_settings = wkhtmltopdf_create_global_settings();

            // We want the result to be storred in the file called test.pdf
            std::string pdf_name("test-" + job_id + ".pdf");
            std::cout << "[" << pdf_name << "]\n";

            //// wkhtmltopdf_set_global_setting(global_settings, "out", pdf_name.c_str());
            //// wkhtmltopdf_set_global_setting(gs, "load.cookieJar", "myjar.jar");

            /*
             * Create a input object settings object that is used to store settings
             * related to a input object, note again that control of this object is parsed to
             * the converter later, which is then responsible for freeing it
             */
            object_settings = wkhtmltopdf_create_object_settings();

            // We want to convert to convert the qstring documentation page
            //// wkhtmltopdf_set_object_setting(os, "page", "http://doc.trolltech.com/4.6/qstring.html");
            wkhtmltopdf_set_object_setting(object_settings, "page", job_data.payload["page"].string_value().c_str());
            wkhtmltopdf_set_object_setting(object_settings, "load.windowStatus", job_data.payload["windowStatus"].string_value().c_str());

            // Create the actual converter object used to convert the pages
            converter = wkhtmltopdf_create_converter(global_settings);

            // Call the progress_changed function when progress changes
            wkhtmltopdf_set_progress_changed_callback(converter, static_cast<wkhtmltopdf_int_callback>(progress_changed));

            // Call the phase _changed function when the phase changes
            wkhtmltopdf_set_phase_changed_callback(converter, phase_changed);

            // Call the error function when an error occures
            wkhtmltopdf_set_error_callback(converter, error);

            // Call the waring function when a warning is issued
            wkhtmltopdf_set_warning_callback(converter, warning);

            /*
             * Add the the settings object describing the qstring documentation page
             * to the list of pages to convert. Objects are converted in the order in which
             * they are added
             */
            wkhtmltopdf_add_object(converter, object_settings, nullptr);

            // Perform the actual conversion
            if (!wkhtmltopdf_convert(converter)) {
                fprintf(stderr, "Conversion failed!\n");
            }

            // Output possible http error code encountered
            int http_error_code = wkhtmltopdf_http_error_code(converter);
            printf("- HTTP error code: %d\n", http_error_code);

            if (http_error_code == 0) {
                // Store result PDF
                const unsigned char *pdf_content = nullptr;
                auto pdf_content_length = static_cast<unsigned long>(wkhtmltopdf_get_output(converter, &pdf_content));
                std::cout << "- PDF content: " << pdf_content_length << " bytes\n";
                if (pdf_content_length > 0) {
                    job_manager->StoreJobResult(job_id, pdf_content, pdf_content_length);
                    job_manager->FinishJob(job_id);
                }
            } else {
                job_manager->FinishJob(job_id, "PDF conversion has failed");
            }

            // Destroy the converter object since we are done with it
            wkhtmltopdf_destroy_converter(converter);
        } catch (cheesyd::InvalidJobException &e) {
            std::cout << "Ooops! Can't do it: " << e.what() << "\n";
            job_manager->FinishJob(job_id, e.what());
        } catch (std::exception &e) {
            std::cout << "Oh no! Job " << job_id << " got it really bad now: " << e.what() << "\n";
            job_manager->FinishJob(job_id, e.what());
        }
    }

    // We will no longer be needing wkhtmltopdf funcionality
    wkhtmltopdf_deinit();
    std::cout << "WK is down\n";

    return interrupt_signal;
}
