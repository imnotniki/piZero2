#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <jansson.h>  // JSON library for C

// Function declarations
void print_menu();
json_t* load_config(const char* config_path);
void run_install_scripts(json_t* config);
void modify_configurations(json_t* config);
void run_command(const char* cmd, const char* output_type);

int main(int argc, char* argv[]) {
    const char* config_path = "cmd.cfg";
    
    // Parse command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                config_path = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-c config_file]\n", argv[0]);
                return 1;
        }
    }
    
    // Load commands from config file
    json_t* config = load_config(config_path);
    if (!config) {
        return 1;
    }
    
    char choice[10];
    while (1) {
        // Print menu
        print_menu();
        
        // Get user input
        printf("\nEnter your choice: ");
        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            break;
        }
        
        // Remove newline character
        choice[strcspn(choice, "\n")] = 0;
        
        if (strcmp(choice, "1") == 0) {
            run_install_scripts(config);
        } else if (strcmp(choice, "2") == 0) {
            modify_configurations(config);
        } else if (strcmp(choice, "3") == 0) {
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid choice, please try again.\n");
        }
    }
    
    // Cleanup
    json_decref(config);
    
    return 0;
}

void print_menu() {
    printf("\nRaspberry Pi Setup Tool\n");
    printf("-----------------------\n");
    printf("1. Run install scripts\n");
    printf("2. Modify configurations\n");
    printf("3. Exit\n");
}

json_t* load_config(const char* config_path) {
    printf("Loading config from: %s\n", config_path);
    
    // Load JSON from file
    json_error_t error;
    json_t* config = json_load_file(config_path, 0, &error);
    
    if (!config) {
        fprintf(stderr, "Failed to parse config file %s: %s at line %d, column %d\n", 
                config_path, error.text, error.line, error.column);
        return NULL;
    }
    
    return config;
}

void run_install_scripts(json_t* config) {
    printf("\nRunning install scripts...\n");
    
    bool found = false;
    const char* key;
    json_t* value;
    
    // Iterate over all key-value pairs in the JSON object
    json_object_foreach(config, key, value) {
        const char* output_type = json_string_value(value);
        if (output_type && strcmp(output_type, "out") == 0) {
            found = true;
            run_command(key, output_type);
        }
    }
    
    if (!found) {
        printf("No install scripts found in config.\n");
    }
}

void modify_configurations(json_t* config) {
    printf("\nModifying configurations...\n");
    
    bool found = false;
    const char* key;
    json_t* value;
    
    // Iterate over all key-value pairs in the JSON object
    json_object_foreach(config, key, value) {
        const char* output_type = json_string_value(value);
        if (output_type && strcmp(output_type, "null") == 0) {
            found = true;
            run_command(key, output_type);
        }
    }
    
    if (!found) {
        printf("No configuration commands found in config.\n");
    }
}

void run_command(const char* cmd, const char* output_type) {
    printf("Executing: %s\n", cmd);
    
    if (strcmp(output_type, "out") == 0) {
        // Execute command and show output
        FILE* fp = popen(cmd, "r");
        if (fp == NULL) {
            printf("Failed to execute command.\n");
            return;
        }
        
        printf("Output:\n");
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }
        
        int status = pclose(fp);
        if (status != 0) {
            printf("Command failed with exit status %d\n", status);
        }
    } else if (strcmp(output_type, "null") == 0) {
        // Execute command but discard output
        char redirect_cmd[512];
        snprintf(redirect_cmd, sizeof(redirect_cmd), "%s > /dev/null 2>&1", cmd);
        
        int status = system(redirect_cmd);
        if (status == 0) {
            printf("Command executed successfully (output routed to null).\n");
        } else {
            printf("Command failed (output routed to null).\n");
        }
    } else {
        printf("Unknown output type: %s\n", output_type);
    }
}
