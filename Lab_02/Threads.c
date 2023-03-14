#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define M 100
#define LOG_FILE "/home/osama/Documents/OS_Labs/Lab_02/logging.log"

int ex;
int **Mat_1, **Mat_2;
int **result_1, **result_2, **result_3;
typedef struct{
    int r1, c1, r2, c2;
} matrices_info;

typedef struct {
        matrices_info info;
        int row;
} thread_per_row_info;

typedef struct {
        matrices_info info;
        int row, col;
} thread_per_element_info;

void * matrix_Multiplication(void *info){ 
    int r1 = (*((matrices_info*) info)).r1,
        c1 = (*((matrices_info*) info)).c1,
        r2 = (*((matrices_info*) info)).r2,
        c2 = (*((matrices_info*) info)).c2;

    if ( c1 != r2 ){ 
        printf("Matrices cannot be multiplied"); 
        return 0; 
    } 

    // Multiply the matrices 
    for (int i = 0; i < r1; i++){ 
        for (int j = 0; j < c2; j++){ 
            for (int k = 0; k < c1; k++){ 
                result_1[i][j] += Mat_1[i][k] * Mat_2[k][j]; 
            } 
        } 
    } 
    pthread_exit(NULL);
}

void * row_Matrix_Multiplication(void *info){
    int row = (*((thread_per_row_info*) info)).row,
        c1  = (*((thread_per_row_info*) info)).info.c1,
        r2  = (*((thread_per_row_info*) info)).info.r2,
        c2  = (*((thread_per_row_info*) info)).info.c2;

    if (c1 != r2){ 
        printf("Row and Matrix cannot be multiplied"); 
        return 0; 
    } 

    // Multiply the row with all the columns of the matrix 
    for (int i = 0; i < c2; i++){ 
        result_2[row][i] = 0; 
        for (int j = 0; j < c1; j++){ 
            result_2[row][i] += Mat_1[row][j] * Mat_2[j][i]; 
        } 
    } 
    pthread_exit(NULL);
}

void * row_Column_Multiplication(void *info){ 
    int row = (*((thread_per_element_info*) info)).row,
        col = (*((thread_per_element_info*) info)).col,
        c1 = (*((thread_per_element_info*) info)).info.c1,
        r2 = (*((thread_per_element_info*) info)).info.r2;

    if (c1 != r2){ 
        printf("Row and Matrix cannot be multiplied"); 
        return 0; 
    }

    result_3[row][col] = 0; 
    // Multiply the corresponding elements of the row and column and add them up 
    for (int i = 0; i < c1; i++){ 
        result_3[row][col] += Mat_1[row][i] * Mat_2[i][col]; 
    } 
    pthread_exit(NULL);
}

void first_tech_thread(matrices_info *info, char *out_file){
    char *out_file_1;
    // strcpy(out_file_1, out_file);
    // strcat(out_file_1, "_per_matrix");

    // allocating result_1 matrix
    result_1 = (int **) calloc((*((matrices_info*) info)).r1 , sizeof(int *));
    for(int i = 0;i < (*((matrices_info*) info)).r1;i++){
        result_1[i] = (int *) calloc((*((matrices_info*) info)).c2 , sizeof(int));
    }

    // start checking time ------------------
    struct timeval stop, start;
    gettimeofday(&start, NULL);

    // create the only thread
    pthread_t id;
    pthread_create(&id, NULL, matrix_Multiplication, info);
    pthread_join(id, NULL);

    gettimeofday(&stop, NULL); 
    //end checking time ---------------------

    FILE *out = fopen(out_file, "w");
    fprintf(out, "Method: A thread per matrix\nrow=%d col=%d\n", (*info).r1, (*info).c2);
    for(int i = 0;i < (*info).r1;i++){
        for (int j = 0;j < (*info).c2;j++)
            fprintf(out, "%d ", result_1[i][j]);
        fprintf(out, "\n");
    }
    fprintf(out, "Time taken = %lu second, %lu microseconds", stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);
    fclose(out);
    free(result_1);
}

void second_tech_thread(matrices_info *info, char *out_file){
    char *out_file_2;
    // strcpy(out_file_2, out_file);
    // strcat(out_file_2, "_per_row");

    // allocating result_2 matrix
    result_2 = calloc((*info).r1, sizeof(int*));
    for(int i = 0;i < (*info).r1;i++){
        result_2[i] = (int *) calloc((*info).c2 , sizeof(int));
    }

    // creating threads    
    pthread_t *ids;
    ids = (pthread_t *) malloc((*info).r1 * sizeof(pthread_t));

    thread_per_row_info *rows;
    rows = (thread_per_row_info *) calloc((*info).r1, sizeof(thread_per_row_info));

    // start checking time ------------------
    struct timeval stop, start;
    gettimeofday(&start, NULL);

    for(int i = 0;i < (*info).r1;i++){
        rows[i].info = (*info), 
        rows[i].row = i;
        pthread_create(&ids[i], NULL, row_Matrix_Multiplication, &rows[i]);
    }

    // waiting threads
    for(int i = 0;i < (*info).r1;i++){
        pthread_join(ids[i], NULL);
    }

    gettimeofday(&stop, NULL); 
    //end checking time ---------------------

    // print the results
    FILE *out = fopen(out_file, "w");
    fprintf(out, "Method: A thread per row\nrow=%d col=%d\n", (*info).r1, (*info).c2);
    for(int i = 0;i < (*info).r1;i++){
        for (int j = 0;j < (*info).c2;j++)
            fprintf(out, "%d ", result_2[i][j]);
        fprintf(out, "\n");
    }
    fprintf(out, "Time taken = %lu second, %lu microseconds", stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);
    fclose(out);

    free(result_2);
    free(ids);
    free(rows);
}

void third_tech_thread(matrices_info *info, char *out_file){
    char *out_file_3;
    // strcpy(out_file_3, out_file);
    // strcat(out_file_3, "_per_element");

    result_3 = (int **) calloc((*info).r1, sizeof(int *));
    for(int i  = 0;i < (*info).r1;i++)
        result_3[i] = (int *) calloc((*info).c2, sizeof(int));

    pthread_t **ids;
    ids = (pthread_t **) malloc((*info).r1 * sizeof(pthread_t *));
    for(int i = 0;i < (*info).r1;i++)
        ids[i] = (pthread_t *) malloc((*info).c2 * sizeof(pthread_t));

    thread_per_element_info **row_col_info;
    row_col_info = (thread_per_element_info **) calloc((*info).r1, sizeof(thread_per_element_info *));
    for(int i  = 0;i < (*info).r1;i++)
        row_col_info[i] = (thread_per_element_info *) calloc((*info).c2, sizeof(thread_per_element_info));

    // start checking time ------------------
    struct timeval stop, start;
    gettimeofday(&start, NULL);

    for(int i = 0;i < (*info).r1;i++){
        for(int j = 0;j < (*info).c2;j++){
            row_col_info[i][j].info = (*info),
            row_col_info[i][j].row = i,
            row_col_info[i][j].col = j;
            pthread_create(&ids[i][j], NULL, row_Column_Multiplication, &row_col_info[i][j]);
        }
    }

    for(int i = 0;i < (*info).r1;i++)
        for(int j = 0;j < (*info).c2;j++)
            pthread_join(ids[i][j], NULL);

    gettimeofday(&stop, NULL); 
    //end checking time ---------------------

    // print the results
    FILE *out = fopen(out_file, "w");
    fprintf(out, "Method: A thread per element\nrow=%d col=%d\n", (*info).r1, (*info).c2);
    for(int i = 0;i < (*info).r1;i++){
        for (int j = 0;j < (*info).c2;j++)
            fprintf(out, "%d ", result_3[i][j]);
        fprintf(out, "\n");
    }
    fprintf(out, "Time taken = %lu second, %lu microseconds", stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);
    fclose(out);

    free(result_3);
    free(ids);
    free(row_col_info);
}

void take_matrix_input(char *file_1, char *file_2, matrices_info *info){
    int r1, c1;
    FILE *file = fopen(file_1, "r");
    fscanf(file, "row=%d col=%d", &r1, &c1);
    Mat_1 = (int **) calloc(r1 , sizeof(int*));
    
    for(int i = 0;i < r1;i++){
        Mat_1[i] = (int *) calloc(c1, sizeof(int));
        for(int j  = 0;j < c1;j++)
            fscanf(file, "%d", &Mat_1[i][j]);
    }
    fclose(file);
    

    int r2, c2;
    file = fopen(file_2, "r");
    fscanf(file, "row=%d col=%d", &r2, &c2);
    Mat_2 = (int **) calloc(r2 , sizeof(int*));
    
    for(int i = 0;i < r2;i++){
        Mat_2[i] = (int *) calloc(c2, sizeof(int));
        for(int j  = 0;j < c2;j++)
            fscanf(file, "%d", &Mat_2[i][j]);
    }
    fclose(file);

    (*info).r1 = r1, (*info).c1 = c1; 
    (*info).r2 = r2, (*info).c2 = c2;
}


int main(int argc, char** argv){
    int ar = 0;
    char *file_1,
         *file_2,
         *out_1,
         *out_2,
         *out_3;

    if(argc < 4) {
        file_1 = "a"; 
        file_2 = "b"; 
        out_1 = "c_per_matrix";
        out_2 = "c_per_row";
        out_3 = "c_per_element";
    } 
    
    else {
        ar = 1;
        file_1 = argv[1];
        file_2 = argv[2];    
        
        out_1 = malloc(strlen(argv[3]) + 12);
        strcpy(out_1, argv[3]);
        strcat(out_1, "_per_matrix");
        
        out_2 = malloc(strlen(argv[3]) + 9);
        strcpy(out_2, argv[3]);
        strcat(out_2, "_per_row");
        
        out_3 = malloc(strlen(argv[3]) + 15);
        strcpy(out_3, argv[3]);
        strcat(out_3, "_per_element");
    }

    matrices_info info = {0, 0, 0, 0};
    take_matrix_input(file_1, file_2, &info);
    
    first_tech_thread(&info, out_1);
    second_tech_thread(&info, out_2);
    third_tech_thread(&info, out_3);
    
    printf("Done :)\n");

    if(ar){
        free(out_1);
        free(out_2);
        free(out_3);
    }
    pthread_exit(NULL);
}

