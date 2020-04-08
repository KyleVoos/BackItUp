#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>

typedef struct _files {
    int thread_id;
    ssize_t copy_size;
    char *orig_path;
    char *backup_path;
    char *filename;
} files;

typedef struct _file_info {
    int count;
    int backup;
    files *f;
} file_info;

file_info *initalize(int flag) {

    file_info *finfo = (file_info *) malloc(sizeof(file_info));
    finfo->f = (files *) malloc(sizeof(files));
    finfo->backup = flag;
    finfo->count = 0;

    return finfo;
}

files *init(char *orig, char *backup, int tid, char *filename) {

    files *f = (files *) malloc(sizeof(files));
    f->orig_path = strdup(orig);
    f->backup_path = strdup(backup);
    f->filename = strdup(filename);
    f->thread_id = tid;

    return f;
}

ssize_t copyFile(char *source, char *dest) {

    ssize_t write_size, read_size;
    ssize_t total = 0;
    int sfd, dfd;
    char buffer[100] = {0};

//    fprintf(stdout, "source = %s\n", source);
//    fprintf(stdout, "dest = %s\n", dest);

    if ((sfd = open(source, O_RDONLY)) > 0) {
        if ((dfd = open(dest, O_WRONLY | O_CREAT)) > 0) {
            while ((read_size = read(sfd, buffer, 100)) > 0) {
                if ((write_size = write(dfd, buffer, read_size)) == read_size) {
                    total += read_size;
                }
                else
                    fprintf(stdout, "%s\n", strerror(errno));
            }
        }
    }

    return total;
}

void *checkTime(void *arg) {

    files *f = arg;
    struct stat orig_file, backup_file;

    fprintf(stdout, "[thread %d] Backing up %s\n", f->thread_id, f->filename);

    if (stat(f->orig_path, &orig_file) == 0) {
        if (stat(f->backup_path, &backup_file) == 0) {
            if (difftime(backup_file.st_mtime, orig_file.st_mtime) < 0) {
                fprintf(stdout, "[thread %d] WARNING: Overwriting %s\n", f->thread_id, f->filename);
                f->copy_size = copyFile(f->orig_path, f->backup_path);
            }
            else {
                fprintf(stdout, "[thread %d] NOTICE: %s is already the most current version\n", f->thread_id,
                        f->filename);
                return NULL;
            }
        }
        else if (errno == ENOENT) {
            fprintf(stdout, "ERROR: %s\n", strerror(errno));
            f->copy_size = copyFile(f->orig_path, f->backup_path);
        }
        else {
            fprintf(stdout, "%s\n", strerror(errno));
        }
    }

    if (f->copy_size > 0)
        fprintf(stdout, "[thread %d] Copied %d bytes from %s to %s\n", f->thread_id, (int) f->copy_size, f->orig_path,f->backup_path);

    return NULL;
}

void spawnThreads(file_info *finfo) {

    pthread_t threads[finfo->count];

//    fprintf(stdout, "spawning threads ...\n");

    for (int ii = 0; ii < finfo->count;) {
//        fprintf(stdout, "Creating thread %d\n", ii + 1);
        pthread_create(&threads[ii], NULL, checkTime, &finfo->f[ii]);
        ii++;
    }

    for (int ii = 0; ii < finfo->count; ii++) {
        pthread_join(threads[ii], NULL);
    }

}

void traverse(file_info *finfo, char *path) {

    DIR *dir;
    struct dirent *d;

    if ((dir = opendir(path)) != NULL) {
        while ((d = readdir(dir)) != NULL) {
            if ((strcmp(d->d_name, "..") != 0) && (strcmp(d->d_name, ".") != 0)
                && (strcmp(d->d_name, ".backup")) != 0) {
                char filepath[PATH_MAX] = {0};

                strcpy(filepath, path);
                strcat(filepath, "/");
                strcat(filepath, d->d_name);

                fprintf(stdout, "filepath = %s\n", filepath);

                if (d->d_type != DT_LNK) {
                    char new_path[strlen(filepath) - 1];
                    strcpy(new_path, &filepath[2]);
                    fprintf(stdout, "new_path = %s\n", new_path);

                    if (d->d_type == DT_DIR) {
                        if (finfo->backup == 1) {
                            fprintf(stdout, "d->d_name = %s\n", d->d_name);
                            struct stat nd;
                            char backup_dir[strlen(filepath) + 9];
                            strcpy(backup_dir, "./.backup/");
                            strlcat(backup_dir, new_path, sizeof(backup_dir));
                            fprintf(stdout, "backup_dir = %s\n", backup_dir);
                            if (stat(backup_dir, &nd) != 0 && errno == ENOENT) {
                                fprintf(stdout, "Directory %s does not exist.\n", backup_dir);
                                mkdir(backup_dir, 0777);
                            } else
                                fprintf(stdout, "DIRECTORY %s already exists\n", backup_dir);

                            traverse(finfo, filepath);
                        }
                    }
                    else if (d->d_type == DT_REG) {
                        char backup_path[9 + strlen(filepath) + 6];
                        strcpy(backup_path, "./.backup/");
                        strcat(backup_path, new_path);
                        strcat(backup_path, ".bak");
                        fprintf(stdout, "backup_path = %s\n", backup_path);
                        finfo->f = realloc(finfo->f, (sizeof(files) * (finfo->count + 1)));
                        finfo->f[finfo->count] = *init(filepath, backup_path, finfo->count + 1, d->d_name);
                        finfo->count += 1;
                    }
                }
            }
        }
    }
}

void backItUp(file_info *finfo) {

    if (mkdir("./.backup", 0777) == 0 || errno == EEXIST) {
        traverse(finfo, ".");
        spawnThreads(finfo);
    }
    else {
        fprintf(stdout, "%s\n", strerror(errno));
        exit(1);
    }

}

int main(int argc, char *argv[]) {

    if (argc == 1) {
        file_info *finfo = initalize(1);
        backItUp(finfo);
    }
    else if (argc == 2) {
        if (strcmp(argv[1], "-r") == 0) {

        }
        else
            fprintf(stdout, "Invalid input, use: %s -r\n", argv[0]);
    }
    else
        fprintf(stdout, "Usage: %s [-r]\n", argv[0]);

    return 0;
}