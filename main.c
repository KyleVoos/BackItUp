#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

int debugging = 0;

// struct to hold info about the individual files
typedef struct _files {
    int thread_id;      // ID of the thread that is copying the file
    ssize_t copy_size;  // number of bytes written
    char *source;       // source filepath
    char *destination;  // destination filepath
    char *source_filename;
    char *dest_filename;
} files;

// struct that holds an array of struct files
typedef struct _file_info {
    int count;
    int num_copied;
    int backup;         // flag for whether we are backing up or restoring
    files *f;
    ssize_t total_bytes_copied;
} file_info;

// function to initialize the file_info struct
file_info *initalize(int flag) {

    file_info *finfo = (file_info *) malloc(sizeof(file_info)); // malloc space for the structure
    finfo->f = (files *) malloc(sizeof(files));                 // malloc space for the struct file array
    finfo->backup = flag;
    finfo->count = 0;
    finfo->num_copied = 0;
    finfo->total_bytes_copied = 0;

    return finfo;
}

// function to initialize a files struct element that is held in the array
files *init(char *orig, char *backup, int tid, char *source_filename, char *dest_filename) {

    files *f = (files *) malloc(sizeof(files));                 // malloc space for the individual element
    f->source = strdup(orig);
    f->destination = strdup(backup);
    f->source_filename = strdup(source_filename);
    f->dest_filename = strdup(dest_filename);
    f->thread_id = tid;
    f->copy_size = 0;

    return f;
}

// free the memory held by each element in the array of struct files
void freeElement(files *finfo) {

    free(finfo->source);
    free(finfo->destination);
    free(finfo->source_filename);
    free(finfo->dest_filename);
}

void freeMemory(file_info *f) {

    for (int ii = 0; ii < f->count; ii++) {
        freeElement(&f->f[ii]);
//        free(&f->f[ii]);
    }
}

// function that copies a file from the source filepath to the destination filepath
ssize_t copyFile(char *source, char *dest) {

    ssize_t read_size;
    ssize_t total = 0;
    int sfd, dfd;
    char buffer[100] = {0}; // initialize an empty buffer that can hold 100 characters

    if (debugging) {
        fprintf(stdout, "source = %s\n", source);
        fprintf(stdout, "dest = %s\n", dest);
    }

    if ((sfd = open(source, O_RDONLY)) > 0) {   // open the source file
        // open the destination file, create it if it doesn't exist and also truncate - needed for overwritting
        // backed up file when the new file is smaller than the original backup
        if ((dfd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0777)) > 0) {
            while ((read_size = read(sfd, buffer, 100)) > 0) {  // read up to 100 bytes from source file into buffer
                if (write(dfd, buffer, (size_t) read_size) == read_size) {  // write read_size bytes to dest file
                    total += read_size;                                     // update total bytes written
                }
            }
        }
        else
            fprintf(stdout, "%s\n", strerror(errno));

        if (close(dfd))
            fprintf(stdout, "%s\n", strerror(errno));
    }
    else
        fprintf(stdout, "%s\n", strerror(errno));

    if (close(sfd) < 0)
        fprintf(stdout, "%s\n", strerror(errno));


    return total;   // return the total number of bytes that were copied to the destination file
}

// function to check the last modified time of the files
void *checkTime(void *arg) {

    files *f = arg;
    struct stat source, dest;

    fprintf(stdout, "[thread %d] Backing up %s\n", f->thread_id, f->source_filename);

    if (stat(f->source, &source) == 0) {                            // stat the source file
        if (stat(f->destination, &dest) == 0) {                     // stat the destination file
            if (difftime(dest.st_mtime, source.st_mtime) < 0) {     // check the time difference between the two files
                fprintf(stdout, "[thread %d] WARNING: Overwriting %s\n", f->thread_id, f->source_filename);
                f->copy_size = copyFile(f->source, f->destination); // if the source file is newer copy it
            }
            else {  // if the backup file is already up to date print message for user
                fprintf(stdout, "[thread %d] NOTICE: %s is already the most current version\n", f->thread_id,
                        f->source_filename);
                return NULL;
            }
        }
        else if (errno == ENOENT) {
            f->copy_size = copyFile(f->source, f->destination);
        }
        else {
            fprintf(stdout, "%s\n", strerror(errno));
        }
    }

    // print message to user about the number of bytes that were copied
    if (f->copy_size > 0)
        fprintf(stdout, "[thread %d] Copied %d bytes from %s to %s\n", f->thread_id, (int) f->copy_size,
                f->source_filename,f->dest_filename);

    return NULL;
}

// function that handles creating and joining the necessary number of threads to copy the file(s)
void spawnThreads(file_info *finfo) {

    pthread_t threads[finfo->count];    // an array of pthreads with count elements

    for (int ii = 0; ii < finfo->count;) {  // iterate from 0 to count creating a pthread each iteration
        // for each thread created start executing the checkTime function, passing the struct file element
        pthread_create(&threads[ii], NULL, checkTime, &finfo->f[ii]);
        ii++;   // increment ii after the thread was created
    }

    for (int ii = 0; ii < finfo->count; ii++) { // join all threads so they terminate once every thread has finished
        pthread_join(threads[ii], NULL);
    }

}

// function that creates a directory
void createDir(char *path) {

    struct stat nd;

    if (stat(path, &nd) != 0 && errno == ENOENT) {
        if (debugging) fprintf(stdout, "Directory %s does not exist.\n", path);
        mkdir(path, 0777);
    } else if (debugging)
        fprintf(stdout, "DIRECTORY %s already exists\n", path);
}

// function to recursively traverse a directory
void traverse(file_info *finfo, char *path) {

    DIR *dir;
    struct dirent *d;

    if ((dir = opendir(path)) != NULL) {    // open the directory
        while ((d = readdir(dir)) != NULL) {    // read the directory
            if ((strcmp(d->d_name, "..") != 0) && (strcmp(d->d_name, ".") != 0)
                && (strcmp(d->d_name, ".backup")) != 0) {   // make sure not to recurse into . .. or .backup
                                                            // (unless it was originally passed to the function)
                char filepath[PATH_MAX] = {0};              // holds the filepath being built

                strncpy(filepath, path, PATH_MAX);
                strncat(filepath, "/", PATH_MAX);
                strncat(filepath, d->d_name, PATH_MAX);     // build the filepath

                if (debugging) fprintf(stdout, "filepath = %s\n", filepath);

                size_t filepath_len = strlen(filepath);     // get the length of the filepath

                if (d->d_type != DT_LNK) {                  // make sure its not a sym link
                    char new_path[filepath_len -1];
                    strncpy(new_path, &filepath[2], filepath_len - 1);
                    if (debugging)fprintf(stdout, "new_path = %s\n", new_path);

                    if (d->d_type == DT_DIR) {          // if its a directory
                        if (finfo->backup == 1) {       // if we are backing up (changes the filepath being built)
                            if (debugging) fprintf(stdout, "d->d_name = %s\n", d->d_name);
                            char backup_dir[filepath_len + 9];
                            strncpy(backup_dir, ".backup/", filepath_len + 8);
                            strncat(backup_dir, new_path, filepath_len + 8);
                            if (debugging) fprintf(stdout, "backup_dir = %s\n", backup_dir);
                            createDir(backup_dir);      // create the directory in .backup if it doesn't exist
                            traverse(finfo, filepath);  // pass filepath to traverse to continue recursion
                        }
                        else {                          // if we are restoring
                            char restore_dir[filepath_len - 5];
                            strncpy(restore_dir, "./", filepath_len - 6);
                            strncat(restore_dir, &filepath[8], filepath_len - 6);
                            if (debugging) fprintf(stdout, "restore_dir = %s\n", restore_dir);
                            createDir(restore_dir);     // create the directory in orig directory if it doesn't exist
                            traverse(finfo, filepath);  // pass filepath to traverse to continue recursion
                        }
                    }
                    else if (d->d_type == DT_REG) {     // if its a regular file
                        if (finfo->backup == 1) {       // if we are backing up
                            char backup_path[strlen(filepath) + 15];
                            char new_fname[strlen(d->d_name) + 5];
                            strcpy(new_fname, d->d_name);
                            strncat(new_fname, ".bak", strlen(d->d_name) + 4);
                            if (debugging) fprintf(stdout, "d->d_name = %s\n", d->d_name);
                            if (debugging) fprintf(stdout, "new_fname = %s\n", new_fname);
                            strcpy(backup_path, "./.backup/");
                            strcat(backup_path, new_path);
                            strcat(backup_path, ".bak");    // build the backup filepath and filename strings
                            if (debugging) fprintf(stdout, "destination = %s\n", backup_path);
                            // realloc the array of struct files to hold 1 more element
                            finfo->f = realloc(finfo->f, (sizeof(files) * (finfo->count + 1)));
                            finfo->f[finfo->count] = *init(filepath, backup_path, finfo->count + 1, d->d_name,
                                    new_fname);         // initialize a struct files element to hold the info about file
                            finfo->count += 1;
                        }
                        else {                          // if we are restoring
                            char orig_path[filepath_len + 2]; memset(orig_path, 0, filepath_len);
                            char *orig_name = strdup(d->d_name);
                            orig_name[strlen(orig_name) - 4] = '\0';
                            if (debugging) fprintf(stdout, "orig_name = %s\n", orig_name);
                            strcpy(orig_path, "./");
                            strcat(orig_path, &filepath[8]);
                            orig_path[strlen(orig_path) - 4] = '\0';    // build the orig filepath and filename strings
                            if (debugging) fprintf(stdout, "source = %s\n", orig_path);
                            // realloc the array of struct files to hold 1 more element
                            finfo->f = realloc(finfo->f, (sizeof(files) * (finfo->count + 1)));
                            d->d_name[strlen(d->d_name)-4] = 0;
                            finfo->f[finfo->count] = *init(filepath, orig_path, finfo->count + 1, d->d_name, orig_name);
                            finfo->count += 1;
                            free(orig_name);
                        }
                    }
                }
            }
        }
        closedir(dir);  // close the directory after there are no new files or directories to open in it
    }
}



void backItUp(file_info *finfo) {

    if (finfo->backup == 1) {   // if the backup flag is set to one (we are backing up the CWD)
        if (mkdir(".backup", 0777) == 0 || errno == EEXIST) {   // create the ./.backup directory if it doesn't exist
            traverse(finfo, ".");                               // start recursively going through the CWD
            spawnThreads(finfo);                                // spawn the necessary threads to copy files
        } else {
            fprintf(stdout, "%s\n", strerror(errno));
            exit(1);
        }
    }
    else {                      // if we are restoring
        traverse(finfo, ".backup");                             // start recursively going through the ./.backup dir
        spawnThreads(finfo);                                    // spawn the necessary threads to restore the files
    }

    for (int ii = 0; ii < finfo->count; ii++) {                 // after threads are finished find total bytes copied
        finfo->total_bytes_copied += finfo->f[ii].copy_size;
        if (finfo->f[ii].copy_size > 0)
            finfo->num_copied++;
    }

    fprintf(stdout, "Successfully copied %d files (%d bytes)\n", finfo->num_copied, (int) finfo->total_bytes_copied);

    freeMemory(finfo);
    free(finfo->f);
}

int main(int argc, char *argv[]) {

    for (int ii = 1; ii < argc; ii++) {
        if (strcmp(argv[ii], "-d") == 0)
            debugging = 1;
    }

    if (argc < 4) {
        for (int ii = 1; ii < argc; ii++) {
            if ((strcmp(argv[ii], "-d") == 0) || (strcmp(argv[1], "-r") == 0)) {
                if (strcmp(argv[1], "-r") == 0) {
                    file_info *finfo = initalize(0);    // initialize the file_info struct and set the backup flag to 0
                    backItUp(finfo);                    // pass the file_info struct to backItUp
                    free(finfo);
                    return 0;
                }
            }
            else {
                fprintf(stdout, "Invalid input, use: %s -r\n", argv[0]);
                return 0;
            }
        }
        if (argc == 1 || debugging == 1) {
            file_info *finfo = initalize(1);            // initialize the file_info struct and set the backup flag to 1
            backItUp(finfo);                            // pass the file_info struct to backItUp
            free(finfo);
            return 0;
        }
    } else                                              // print message showing correct usage
        fprintf(stdout, "Usage: %s [-r]\n", argv[0]);

    return 0;
}