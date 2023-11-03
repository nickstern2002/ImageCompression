#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Bitmap {
    int rows;
    int cols;
    short *pixels;
};

struct Node {
    short intensity;
    struct Node *children[4];
};

struct Tree {
    struct Node *root;
};

struct Bitmap Bitmap_new(char *fname) {
    char line[4];
    int linecount = 0;
    FILE *input = fopen(fname, "r");
    struct Bitmap map;
    int i = 1;

    while(fgets(line, sizeof(line), input))
        linecount++;
 
    while((i*i) != linecount)
        i++;

    map.rows = i;
    map.cols = i;
    
    rewind(input);
    
    map.pixels = (short *) malloc(sizeof(short) * linecount);
    
    i = 0;
    while(fgets(line, sizeof(line), input)){
        *(map.pixels + i) = (short) atoi(line);
        i++;
    }

    return map;
}

void Bitmap_clear(struct Bitmap *b) {
    free(b->pixels);
    b->pixels=NULL;
    b->rows=0;
    b->cols=0;
}

void Bitmap_rotate_left(struct Bitmap *b) {
    short *tempPixelList = (short *) malloc(sizeof(short) * (b->rows * b-> cols));
    int i, r, c;
    
    for(c = 1, i = 0; c <= b->cols; c++)
        for(r = b->rows; r <= (b->rows * b->cols); r += b->cols, i++)
            *(tempPixelList + i) = *(b->pixels + (r-c));
        
    for(i = 0; i < (b->rows * b->cols); i++)
        *(b->pixels + i) = *(tempPixelList + i);

    free(tempPixelList);
}

void Bitmap_rotate_right(struct Bitmap *b) {
    short *tempPixelList = (short *) malloc(sizeof(short) * (b->rows * b-> cols));
    int i, r, c;
    
    for(c = b->cols, i = 0; c >= 1; c--)
        for(r = (b->rows * b->cols); r >= b->rows; r -= b->cols, i++)
            *(tempPixelList + i) = *(b->pixels + (r-c));
             
    for(i = 0; i < (b->rows * b->cols); i++)
        *(b->pixels + i) = *(tempPixelList + i);

    free(tempPixelList);
}

void Bitmap_flip_vertical(struct Bitmap *b) {
    short *tempPixelList = (short *) malloc(sizeof(short) * (b->rows * b-> cols));
    int i, r, c;
    
    for(r = b->rows, i = 0; r > 0; r--)
        for(c = b->cols; c > 0; c--, i++)
            *(tempPixelList + i) = *(b->pixels + ((r * b->rows) - c));
    
    for(i=0; i < (b->rows * b->cols); i++)
        *(b->pixels + i) = *(tempPixelList + i);       

    free(tempPixelList);
}


void Bitmap_flip_horizontal(struct Bitmap *b) { 
    short *tempPixelList = (short *) malloc(sizeof(short) * (b->rows * b-> cols));
    int i, r, c;   

    for(r = 1, i = 0; r <= b->rows; r++)
        for(c = 1; c <= b->cols; c++, i++)
            *(tempPixelList + i) = *(b->pixels + ((r * b->rows) - c));    
        
    for(i=0; i < (b->rows * b->cols); i++)
        *(b->pixels + i) = *(tempPixelList + i);       

    free(tempPixelList);
}

struct Node *partition(struct Bitmap *b, int topleft, int rows){ /*takes a bitmap, the position of the partition's topleft corner and the amount of rows in the partion as parameters */
    int val, i, j;
    val = *(b->pixels + topleft);
    for(i = 0; i < (b->rows * rows); i += b->rows)
        for(j = 0; j < rows; j++)
            if(val != *(b->pixels + (topleft+i+j)))
                val = -1;
            
    if(val == -1){
        struct Node *node = (struct Node *) malloc(sizeof(struct Node));
        node->intensity = -1;
        node->children[0] = partition(b, topleft, rows/2);
        node->children[1] = partition(b, topleft + (rows/2), rows/2);
        node->children[2] = partition(b, topleft + (b->rows * (rows/2)), rows/2);
        node->children[3] = partition(b, topleft + (b->rows * (rows/2)) + (rows/2), rows/2);
        return node;
    }
    else{
        struct Node *node = (struct Node *) malloc(sizeof(struct Node));
        node->intensity = val;
        for(i = 0; i < 4; i++)
            node->children[i] = NULL;
        return node;
    }
}

struct Tree Tree_new(struct Bitmap *b) {
    struct Tree tree;
    struct Node *root = (struct Node *) malloc(sizeof(struct Node));
    if(b->pixels == NULL){
        tree.root = NULL;
        return tree;
    }
    root = partition(b, 0, b->rows);
    tree.root = root;
    return tree;
}

void recursiveClear(struct Node *n){
    if(n->intensity == -1){
        recursiveClear(n->children[0]);    
        recursiveClear(n->children[1]);    
        recursiveClear(n->children[2]);    
        recursiveClear(n->children[3]);    
    }
    free(n);
    n = NULL;
}

void Tree_clear(struct Tree *t) {
    recursiveClear(t->root);
    t->root = NULL;
}


void recursiveSave(struct Node *n, FILE *output){
    if(n == NULL)
        return;    
    fprintf(output, "%d\n", n->intensity);
    if(n->intensity == -1){
        recursiveSave(n->children[0], output);    
        recursiveSave(n->children[1], output);    
        recursiveSave(n->children[2], output);    
        recursiveSave(n->children[3], output);    
    }
}

void Tree_save(struct Tree *t, char *fname) {
    char *saveFile;
    FILE *output;
    saveFile = (char *) malloc(strlen(fname) + strlen(".compressed"));

    strcpy(saveFile, fname);
    strcat(saveFile, ".compressed");

    output = fopen(saveFile, "w");

    recursiveSave(t->root, output);
    free(saveFile);
}


void printBitmap(struct Bitmap *b){
    int i;
    for(i = 0; i < (b->rows * b->cols); i++){
        if(i % b->rows == 0 && i != 0)
            printf("\n");
        printf("%d ", *(b->pixels + i));
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    struct Bitmap map;
    struct Tree t;

    if(argc < 2){
        printf("No file name given\n");
        exit(1);
    } 

    map = Bitmap_new(argv[1]); 
    
    t = Tree_new(&map);
    
    while(1){
        int choice;
        printf("Options Menu:\n");
        printf("1. Rotate Left (counterclockwise)\n");
        printf("2. Rotate Right (clockwise)\n");
        printf("3. Flip Vertical\n");
        printf("4. Flip Horizontal\n");
        printf("5. Save and Quit\n");
        printf("\n");
        printf("Your Choice? ");
        
        scanf("%d", &choice);
        switch(choice){
            case 1:
                Bitmap_rotate_left(&map);
                break;
            case 2:
                Bitmap_rotate_right(&map);
                break;
            case 3:
                Bitmap_flip_vertical(&map);
                break;
            case 4:
                Bitmap_flip_horizontal(&map);
                break;
            case 5:
                Tree_save(&t, argv[1]);
                exit(0);
                break;
            default:
                printf("Invalid Input\n");    
        }
    }
    
    Bitmap_clear(&map);
    Tree_clear(&t);

    return 0;
    
}
