# BackItUp

This is a multithreaded program to backup and restore a directory and all of its contents.
It will recursively go through the current working directory as well as each sub directory to find all of the regular
files that need backed up or restored. After finding all of the files that may need to be copied a new thread is
spawned for each file. From there the program will check to make sure the file being copied is the most recent version.
It will recreate the directory structure of the directory being backed up in the .backup directory and when restoring a
directory will recreate the directory structure as well.

# Sample Output

kyle@LAPTOP-T16SRG68:/mnt/c/Users/Tormentt/Documents/GitHub/BackItUp/TEST$ ls -alR  
.:  
total 1280  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:42 .  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:50 ..  
-rwxrwxrwx 1 kyle kyle   18184 May  3 16:40 BackItUp  
drwxrwxrwx 1 kyle kyle    4096 Apr 11 14:42 TEST1  
drwxrwxrwx 1 kyle kyle    4096 Apr 11 13:15 TEST2  
-rwxrwxrwx 1 kyle kyle   18032 May  3 16:40 backup  
-rwxrwxrwx 1 kyle kyle    5093 May  3 16:40 hashTable.c  
-rwxrwxrwx 1 kyle kyle 1257300 May  3 16:40 mobydick.txt  
-rwxrwxrwx 1 kyle kyle    1849 May  3 16:40 wordCount.c  

./TEST1:  
total 8  
drwxrwxrwx 1 kyle kyle 4096 Apr 11 14:42 .  
drwxrwxrwx 1 kyle kyle 4096 May 13 11:42 ..  
drwxrwxrwx 1 kyle kyle 4096 Apr 11 14:42 TEST3  
-rwxrwxrwx 1 kyle kyle 5093 May  3 16:42 hashTable.c  

./TEST1/TEST3:  
total 1228  
drwxrwxrwx 1 kyle kyle    4096 Apr 11 14:42 .  
drwxrwxrwx 1 kyle kyle    4096 Apr 11 14:42 ..  
-rwxrwxrwx 1 kyle kyle 1257300 May  3 16:43 mobydick.txt  

./TEST2:  
total 4  
drwxrwxrwx 1 kyle kyle 4096 Apr 11 13:15 .  
drwxrwxrwx 1 kyle kyle 4096 May 13 11:42 ..  
-rwxrwxrwx 1 kyle kyle 1849 May  3 16:40 wordCount.c  

kyle@LAPTOP-T16SRG68:/mnt/c/Users/Tormentt/Documents/GitHub/BackItUp/TEST$ ./BackItUp  
[thread 1] Backing up BackItUp  
[thread 2] Backing up backup  
[thread 3] Backing up hashTable.c  
[thread 4] Backing up mobydick.txt  
[thread 5] Backing up hashTable.c  
[thread 6] Backing up mobydick.txt  
[thread 7] Backing up wordCount.c  
[thread 8] Backing up wordCount.c  
[thread 1] Copied 18184 bytes from BackItUp to BackItUp.bak  
[thread 3] Copied 5093 bytes from hashTable.c to hashTable.c.bak  
[thread 7] Copied 1849 bytes from wordCount.c to wordCount.c.bak  
[thread 8] Copied 1849 bytes from wordCount.c to wordCount.c.bak  
[thread 5] Copied 5093 bytes from hashTable.c to hashTable.c.bak  
[thread 2] Copied 18032 bytes from backup to backup.bak  
[thread 6] Copied 1257300 bytes from mobydick.txt to mobydick.txt.bak  
[thread 4] Copied 1257300 bytes from mobydick.txt to mobydick.txt.bak  
Successfully copied 8 files (2564700 bytes)  

kyle@LAPTOP-T16SRG68:/mnt/c/Users/Tormentt/Documents/GitHub/BackItUp/TEST$ ls -alR  
.:  
total 1280  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:51 .  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:52 ..  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:51 .backup  
-rwxrwxrwx 1 kyle kyle   18184 May  3 16:40 BackItUp  
drwxrwxrwx 1 kyle kyle    4096 Apr 11 14:42 TEST1  
drwxrwxrwx 1 kyle kyle    4096 Apr 11 13:15 TEST2  
-rwxrwxrwx 1 kyle kyle   18032 May  3 16:40 backup  
-rwxrwxrwx 1 kyle kyle    5093 May  3 16:40 hashTable.c  
-rwxrwxrwx 1 kyle kyle 1257300 May  3 16:40 mobydick.txt  
-rwxrwxrwx 1 kyle kyle    1849 May  3 16:40 wordCount.c  

./.backup:  
total 1280  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:51 .  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:51 ..  
-rwxrwxrwx 1 kyle kyle   18184 May 13 11:51 BackItUp.bak  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:51 TEST1  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:51 TEST2  
-rwxrwxrwx 1 kyle kyle   18032 May 13 11:51 backup.bak  
-rwxrwxrwx 1 kyle kyle    5093 May 13 11:51 hashTable.c.bak  
-rwxrwxrwx 1 kyle kyle 1257300 May 13 11:51 mobydick.txt.bak  
-rwxrwxrwx 1 kyle kyle    1849 May 13 11:51 wordCount.c.bak  

./.backup/TEST1:  
total 8  
drwxrwxrwx 1 kyle kyle 4096 May 13 11:51 .  
drwxrwxrwx 1 kyle kyle 4096 May 13 11:51 ..  
drwxrwxrwx 1 kyle kyle 4096 May 13 11:51 TEST3  
-rwxrwxrwx 1 kyle kyle 5093 May 13 11:51 hashTable.c.bak  

./.backup/TEST1/TEST3:  
total 1228  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:51 .  
drwxrwxrwx 1 kyle kyle    4096 May 13 11:51 ..  
-rwxrwxrwx 1 kyle kyle 1257300 May 13 11:51 mobydick.txt.bak  

./.backup/TEST2:  
total 4  
drwxrwxrwx 1 kyle kyle 4096 May 13 11:51 .  
drwxrwxrwx 1 kyle kyle 4096 May 13 11:51 ..  
-rwxrwxrwx 1 kyle kyle 1849 May 13 11:51 wordCount.c.bak  

./TEST1:  
total 8  
drwxrwxrwx 1 kyle kyle 4096 Apr 11 14:42 .  
drwxrwxrwx 1 kyle kyle 4096 May 13 11:51 ..  
drwxrwxrwx 1 kyle kyle 4096 Apr 11 14:42 TEST3  
-rwxrwxrwx 1 kyle kyle 5093 May  3 16:42 hashTable.c  

./TEST1/TEST3:  
total 1228  
drwxrwxrwx 1 kyle kyle    4096 Apr 11 14:42 .  
drwxrwxrwx 1 kyle kyle    4096 Apr 11 14:42 ..  
-rwxrwxrwx 1 kyle kyle 1257300 May  3 16:43 mobydick.txt  

./TEST2:  
total 4  
drwxrwxrwx 1 kyle kyle 4096 Apr 11 13:15 .  
drwxrwxrwx 1 kyle kyle 4096 May 13 11:51 ..  
-rwxrwxrwx 1 kyle kyle 1849 May  3 16:40 wordCount.c  
 
