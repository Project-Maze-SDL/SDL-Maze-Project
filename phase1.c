#include <stdio.h>
#include <stdlib.h>

int main(){
      char con='c';
      char temp;
      int x=0,y=0,flag;

      char maze[5][5]={
        {'P', '0', '1', '1', '1'},
        {'1', '0', '1', '0', 'E'},
        {'1', '0', '0', '0', '1'},
        {'1', '1', '1', '0', '1'},
        {'1', '1', '1', '1', '1'}
      };

      while (1){

          //display maze:
          printf("\n");
          for(int i=0;i<5;i++){
           for(int j=0;j<5;j++){
                printf("%c ",maze[i][j]);
                }
            printf("\n");
            }

          printf("player instructions:\n");

          //get nav key:
          char key;
          printf("enter navigation key(w/a/s/d):");
          scanf("%c",&key);

          //updation:
            //exchange elements
            if(key=='w'){
               if(x==0 || maze[x-1][y]=='1') flag=1;
               else{
                 temp=maze[x][y];
                 maze[x][y]=maze[x-1][y];
                 maze[x-1][y]=temp;

                 //update players x,y coordinates
                 x-=1;
                    }
            }

            else if(key=='s'){
               if(x==5 || maze[x+1][y]=='1') flag=1;
               else{
                 temp=maze[x][y];
                 maze[x][y]=maze[x+1][y];
                 maze[x+1][y]=temp;

                 //update players x,y coordinates
                 x+=1;
                    }
            }

            else if(key=='a'){
               if(y==0 || maze[x][y-1]=='1') flag=1;
               else{
                 temp=maze[x][y];
                 maze[x][y]=maze[x][y-1];
                 maze[x][y-1]=temp;

                 //update players x,y coordinates
                 y-=1;
                    }
            }

            else if(key=='d'){
               if(y==5 || maze[x][y+1]=='1') flag=1;
               else{
                 temp=maze[x][y];
                 maze[x][y]=maze[x][y+1];
                 maze[x][y+1]=temp;

                 //update players x,y coordinates
                 y+=1;
                    }
            }

        if(flag==1){
            printf("OOPS, YOU'RE HITTING THE WALL");
        }
        #ifdef _WIN32
           system("cls");
        #else
           system("clear");
       #endif



      
       //reset error condition:
       flag=0;

      }


    return 0;
}