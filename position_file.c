#include "position_file.h"



PositionFile* loadPositionFile(char* path){
    int row_len = (DEVICE_NUM*4); 
    PositionFile* pos = malloc(sizeof(PositionFile));
    for(int i=0;i<DEVICE_NUM;i++){
      pos->tail[i] = &pos->head[i];
    }
    sprintf(pos->path,"%s",path);
    

    int pos_file_fd = open(path, O_RDONLY);
    pos->count = 0; 
		int finished = 0;
		ssize_t read_bytes;
    char *file_row;
    do{
			file_row = (char*)malloc( sizeof(char) * (row_len+1) );      
      read_bytes = read(pos_file_fd, file_row, row_len);
      file_row[row_len] = '\0';
		  if(read_bytes != row_len){
				free(file_row);
        file_row = NULL;
				finished = 1;
			}else{
        //printf("row: %s", file_row);
        
        for (int i=0; i < DEVICE_NUM ; i++){
          PositionFileRow* pos_row = malloc(sizeof(PositionFileRow) * DEVICE_NUM );

          int ix = i*4;
          int iy = ix+2;
          
          pos_row->coords[0] = atoi(&file_row[ix]);
          pos_row->coords[1] = atoi(&file_row[iy]);
          pos_row->next = NULL;
          //printf("(%d, %d)\n", pos_row->coords[0], pos_row->coords[1]);
          
          pos->tail[i]->next = pos_row;
          pos->tail[i] = pos_row;
          
        }
      }
			pos->count++;
    }while(!finished);
    
		return pos;
}