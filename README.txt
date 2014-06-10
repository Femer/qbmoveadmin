These are the C utiliy files to manage the qbmoves

Compile the software:
Fisrt of all you need to download the qbAPI repository and be sure to have a
file tree like that:

your_workingcopy
    |
    > qbAPI
    > qbmoveadmin

Then you will need to compile the libraries. Go to qbAPI/src and type "make".
Then go to qbmoveadmin/src folder and type "make".

If everything is good, you should have a folder tree like that:

qbmoveadmin
    |
    > bin
    > conf_files
    > objs
    > src

From the qbmoveadmin folder, execute your binary files by typing:

./bin/name_of_the_bin