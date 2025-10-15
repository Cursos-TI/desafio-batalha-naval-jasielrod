#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <iostream>
using namespace std;


#define N 10

typedef enum { EMPTY=0, SHIP, HIT, MISS, SUNK } CellState;

typedef struct {
    int size;
    int hits;
    int coords_count;
    int coords[10][2]; // max 10, but our ships are <=4
    int sunk;
} Ship;

typedef struct {
    CellState cell[N][N];
    Ship ships[4];
    int ships_count;
} Board;

void clear_screen(){

    printf("\033[2J\033[H");
}

void pause_enter(){
    printf("\nPressione ENTER para continuar...");
    while (getchar()!='\n');
}

void init_board(Board *b){
    int i,j;
    for(i=0;i<N;i++) for(j=0;j<N;j++) b->cell[i][j]=EMPTY;
    b->ships_count = 0;
}

void print_header_and_legend(int player_points, int player_shots, int round){
    printf("Legenda: o = Navio   v = Navio Atingido   ~ = Navio Afundado   x = Tiro Perdido\n");
    printf("Pontos: %d\tTiros Restantes nesta rodada: %d\tRodada: %d\n", player_points, player_shots, round);
}

void print_coords_top(){
    int j;
    printf("   ");
    for(j=0;j<N;j++){
        printf(" %c ", 'A'+j);
    }
    printf("   ");
}

void print_two_boards(Board *b1, Board *b2, int reveal_b2){
    // imprime lado a lado com letras no topo e números na lateral
    int i,j;
    // Top headers
    printf("\n");
    // esquerda
    printf("Seu Tabuleiro");
    for (int k=0;k<((N*3)+4 - 12); k++) putchar(' ');
    printf("Computador\n");

    print_coords_top();
  
    printf("     ");
    print_coords_top();
    printf("\n");

    for(i=0;i<N;i++){

        printf("%2d ", i);
        for(j=0;j<N;j++){
            char c=' ';
            if(b1->cell[i][j]==EMPTY) c=' ';
            else if(b1->cell[i][j]==SHIP) c='o';
            else if(b1->cell[i][j]==HIT) c='v';
            else if(b1->cell[i][j]==MISS) c='x';
            else if(b1->cell[i][j]==SUNK) c='~';
            printf("|%c", c);
            if(j==N-1) printf("|");
        }

        printf("     ");


        printf("%2d ", i);
        for(j=0;j<N;j++){
            char c=' ';
            if(b2->cell[i][j]==EMPTY) c=' ';
            else if(b2->cell[i][j]==SHIP) c = (reveal_b2 ? 'o' : ' ');
            else if(b2->cell[i][j]==HIT) c='v';
            else if(b2->cell[i][j]==MISS) c='x';
            else if(b2->cell[i][j]==SUNK) c='~';
            printf("|%c", c);
            if(j==N-1) printf("|");
        }
        printf("\n");
    }
    printf("\n");
}

int in_bounds(int r, int c){ return r>=0 && r<N && c>=0 && c<N; }


int can_place(Board *b, int r1, int c1, int r2, int c2, int size){

    if(!in_bounds(r1,c1) || !in_bounds(r2,c2)) return 0;

    if(r1!=r2 && c1!=c2) return 0;

    int dr = (r2>r1) ? 1 : (r2<r1 ? -1 : 0);
    int dc = (c2>c1) ? 1 : (c2<c1 ? -1 : 0);

    int len = 1;
    int rr=r1, cc=c1;
    while(rr!=r2 || cc!=c2){
        rr+=dr; cc+=dc; len++;
    }
    if(len!=size) return 0;

    rr = r1; cc = c1;
    for(int k=0;k<len;k++){

        for(int ar=rr-1; ar<=rr+1; ar++){
            for(int ac=cc-1; ac<=cc+1; ac++){
                if(in_bounds(ar,ac) && b->cell[ar][ac]==SHIP) return 0;
            }
        }
        rr+=dr; cc+=dc;
    }
    return 1;
}

void place_ship(Board *b, int r1, int c1, int r2, int c2, int size){
    int dr = (r2>r1) ? 1 : (r2<r1 ? -1 : 0);
    int dc = (c2>c1) ? 1 : (c2<c1 ? -1 : 0);
    int rr=r1, cc=c1;
    int id = b->ships_count;
    Ship *s = &b->ships[id];
    s->size = size; s->hits=0; s->coords_count=0; s->sunk=0;
    while(1){
        b->cell[rr][cc] = SHIP;
        s->coords[s->coords_count][0] = rr;
        s->coords[s->coords_count][1] = cc;
        s->coords_count++;
        if(rr==r2 && cc==c2) break;
        rr+=dr; cc+=dc;
    }
    b->ships_count++;
}


void random_place_one(Board *b, int size){
    int tries=0;
    while(1){
        int r = rand()%N;
        int c = rand()%N;
        int horiz = rand()%2; 
        int dr = horiz ? 0 : (rand()%2?1:-1);
        int dc = horiz ? (rand()%2?1:-1) : 0;
        int r2 = r + dr*(size-1);
        int c2 = c + dc*(size-1);
        if(!in_bounds(r2,c2)) { tries++; if(tries>5000) return; continue; }
        if(can_place(b,r,c,r2,c2,size)){ place_ship(b,r,c,r2,c2,size); break;}
        tries++;
        if(tries>5000) return;
    }
}

void random_place_all(Board *b){

    int sizes[4] = {4,3,2,2};
    for(int i=0;i<4;i++) random_place_one(b,sizes[i]);
}


int parse_coord_token(const char *token, int *r, int *c){

    if(token==NULL) return 0;
    if(strcmp(token,"-1")==0) return -1;

    int len = strlen(token);
    if(len>=2 && ((token[0]>='A' && token[0]<='Z') || (token[0]>='a' && token[0]<='z'))){
        char letter = token[0];
        int col = (letter>='a') ? letter - 'a' : letter - 'A';
        int row = atoi(token+1);
        if(row>=0 && row<N && col>=0 && col<N){ *r = row; *c = col; return 1; }
        else return 0;
    }

    int a,b;
    if(sscanf(token, "%d,%d",&a,&b)==2){ if(in_bounds(a,b)){ *r=a; *c=b; return 1;} else return 0; }
    if(sscanf(token,"%d %d",&a,&b)==2){ if(in_bounds(a,b)){ *r=a; *c=b; return 1;} else return 0; }

    return 0;
}


void player_place_ships(Board *b){
    init_board(b);
    int sizes[4] = {4,3,2,2};
    char buf[128];

    clear_screen();
    printf("Você vai posicionar seus navios. Digite -1 para posicionamento aleatório.\n");
    for(int i=0;i<4;i++){
        int size = sizes[i];
        printf("\n%d° Navio - Tamanho %d\n", i+1, size);
        printf("Informe as coordenadas das extremidades do navio (ex: A0 A3 ou 0 0 0 3). Letras A-J nas colunas e 0-9 nas linhas.\n");
        printf("Ou digite -1 para posicionamento aleatório deste navio.\n");

        while(1){
            printf("Entrada: ");

            if(!fgets(buf,sizeof(buf),stdin)) return;

            char *p = strchr(buf,'\n'); if(p) *p=0;
            if(strcmp(buf,"-1")==0){

                random_place_one(b,size);
                break;
            }

            int r1=-1,c1=-1,r2=-1,c2=-1;


            char t1[32], t2[32], *tok;
            tok = strtok(buf," ");
            if(tok==NULL){ printf("Entrada inválida\n"); continue; }
            strcpy(t1,tok);
            tok = strtok(NULL," ");
            if(tok==NULL){ printf("Entrada inválida, informe duas extremidades\n"); continue;}
            strcpy(t2,tok);
            // parse t1
            int v1 = parse_coord_token(t1,&r1,&c1);
            int v2 = parse_coord_token(t2,&r2,&c2);
            if(v1==1 && v2==1){
                if(can_place(b,r1,c1,r2,c2,size)){
                    place_ship(b,r1,c1,r2,c2,size);
                    break;
                } else {
                    printf("Não é possível posicionar aí (sobreposição/adjacente/inválido). Tente novamente.\n");
                    continue;
                }
            } else {
                printf("Formato inválido. Use A0 A3 ou 0 0 0 3. Tente de novo.\n");
                continue;
            }
        }

        clear_screen();
        printf("Posições atuais (seu tabuleiro):\n");
        print_two_boards(b, b, 1); // show reveal both as user
    }
    printf("Posicionamento concluído.\n");
    pause_enter();
}


void computer_place_ships(Board *b){
    init_board(b);
    random_place_all(b);
}


int find_ship_at(Board *b, int r, int c){
    for(int i=0;i<b->ships_count;i++){
        for(int k=0;k<b->ships[i].coords_count;k++){
            if(b->ships[i].coords[k][0]==r && b->ships[i].coords[k][1]==c) return i;
        }
    }
    return -1;
}

int check_and_mark_hit(Board *b, int r, int c){
    if(!in_bounds(r,c)) return 0;
    if(b->cell[r][c]==SHIP){
        b->cell[r][c]=HIT;
        int sid = find_ship_at(b,r,c);
        if(sid>=0){
            b->ships[sid].hits++;
            if(b->ships[sid].hits == b->ships[sid].size){
                // sink it: mark all coords as SUNK
                b->ships[sid].sunk = 1;
                for(int k=0;k<b->ships[sid].coords_count;k++){
                    int rr = b->ships[sid].coords[k][0], cc = b->ships[sid].coords[k][1];
                    b->cell[rr][cc] = SUNK;
                }
                return 2; 
            }
            return 1;
        }
        return 1;
    } else if(b->cell[r][c]==EMPTY){
        b->cell[r][c]=MISS;
        return 0; 
    } else {
        return -1; 
    }
}

int ships_remaining(Board *b){
    for(int i=0;i<b->ships_count;i++) if(!b->ships[i].sunk) return 1;
    return 0;
}


typedef struct {
    int queue[100][2];
    int head, tail;
} ShotQueue;

void init_queue(ShotQueue *q){ q->head=q->tail=0; }
void push_queue(ShotQueue *q,int r,int c){ if(q->tail<100){ q->queue[q->tail][0]=r; q->queue[q->tail][1]=c; q->tail++; } }
int pop_queue(ShotQueue *q, int *r,int *c){ if(q->head<q->tail){ *r=q->queue[q->head][0]; *c=q->queue[q->head][1]; q->head++; return 1;} return 0; }

void add_adjacent_targets(ShotQueue *q, int r, int c, Board *b){
    int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for(int i=0;i<4;i++){
        int nr=r+dirs[i][0], nc=c+dirs[i][1];
        if(in_bounds(nr,nc) && (b->cell[nr][nc]==EMPTY || b->cell[nr][nc]==SHIP))
            push_queue(q,nr,nc);
    }
}

void computer_take_shots(Board *pc, Board *player, int shots, int *pc_points){
    ShotQueue q; init_queue(&q);

    int tries=0;
    while(shots>0){
        int r,c;
        if(pop_queue(&q,&r,&c)==0){

            r = rand()%N; c = rand()%N;
            if(player->cell[r][c]==HIT || player->cell[r][c]==MISS || player->cell[r][c]==SUNK) { tries++; if(tries>5000) break; continue; }
        }
        int res = check_and_mark_hit(player,r,c);
        if(res==-1) { tries++; if(tries>5000) break; continue; }
        if(res==0){


            shots--;
        } else if(res==1){
            // hit
            (*pc_points) += 1;
            add_adjacent_targets(&q,r,c,player);
            shots--;
        } else if(res==2){
            (*pc_points) += 1 + 2; 

            shots--;
        }
    }
}


void player_take_shots(Board *enemy, int shots, int *player_points){
    char buf[128];
    while(shots>0){
        clear_screen();
        print_header_and_legend(*player_points, shots, -1);
        print_two_boards(NULL, NULL, 0); 
        print_two_boards(NULL, NULL, 0); 

        printf("Sua vez! Restam %d tiros nesta rodada.\n", shots);
        printf("Informe as coordenadas do tiro (ex: A5). Use letra-coluna e número-linha.\n");
        printf("Entrada: ");
        if(!fgets(buf,sizeof(buf),stdin)) return;
        char *p = strchr(buf,'\n'); if(p) *p=0;
        if(strlen(buf)<1) { printf("Entrada vazia\n"); continue; }
        int r=-1,c=-1;

        if(strlen(buf)>=2 && ((buf[0]>='A'&&buf[0]<='J') || (buf[0]>='a'&&buf[0]<='j'))){
            c = (buf[0]>='a') ? buf[0]-'a' : buf[0]-'A';
            r = atoi(buf+1);
            if(!in_bounds(r,c)){ printf("Coordenada fora do tabuleiro\n"); pause_enter(); continue; }
        } else {

            int a,b;
            if(sscanf(buf,"%d %d",&a,&b)==2){ r=a; c=b; if(!in_bounds(r,c)){ printf("Fora\n"); pause_enter(); continue; } }
            else { printf("Formato inválido\n"); pause_enter(); continue; }
        }
        int res = check_and_mark_hit(enemy,r,c);
        if(res==-1){ printf("Você já atirou nessa posição. Tente outra.\n"); pause_enter(); continue; }
        if(res==0){ printf("Você errou em %c%d.\n", 'A'+c, r); pause_enter(); shots--; }
        else if(res==1){ printf("Você acertou em %c%d!\n", 'A'+c, r); (*player_points) += 1; pause_enter(); shots--; }
        else if(res==2){ printf("Você afundou um navio em %c%d! +bônus\n", 'A'+c, r); (*player_points) += 1 + 2; pause_enter(); shots--; }
    }
}



void show_final_result(Board *player, Board *computer, int player_points, int pc_points){
    clear_screen();
    printf("=== Resultado Final ===\n\n");
    printf("PONTOS: Jogador = %d   Computador = %d\n\n", player_points, pc_points);
    printf("Tabuleiros (ambos revelados):\n");
    print_two_boards(player, computer, 1);
    if(player_points > pc_points) printf("\nPARABÉNS! Você venceu!\n");
    else if(player_points < pc_points) printf("\nO computador venceu. Mais sorte na próxima!\n");
    else printf("\nEmpate!\n");
}

void game_loop(){
    Board player, computer;
    int player_points=0, pc_points=0;
    int round_shots[4] = {2,2,3,4};


    init_board(&player); init_board(&computer);


    int opt;
    while(1){
        clear_screen();
        printf("=== BATALHA NAVAL - Posicionamento ===\n");
        printf("1 - Posicionar manualmente\n2 - Aleatório\nOpção: ");
        if(scanf("%d%*c",&opt)!=1) { printf("Entrada inválida\n"); return; }
        if(opt==1){
            player_place_ships(&player);
            break;
        } else if(opt==2){
            random_place_all(&player);
            break;
        } else {
            printf("Opção inválida\n"); pause_enter();
        }
    }


    computer_place_ships(&computer);


    for(int r=0;r<4;r++){
        int shots = round_shots[r];

        while(shots>0){
            clear_screen();
            print_header_and_legend(player_points, shots, r+1);
            print_two_boards(&player, &computer, 0);
            printf("Sua vez. Restam %d tiros nesta rodada.\n", shots);
            printf("[Digite -1 para ver o tabuleiro do computador temporariamente]\n");
            printf("Informe as coordenadas: ");
            char buf[64];
            if(!fgets(buf,sizeof(buf),stdin)) return;
            char *p = strchr(buf,'\n'); if(p) *p=0;
            if(strcmp(buf,"-1")==0){

                clear_screen();
                printf("Tabuleiro computador (revelado temporariamente):\n");
                print_two_boards(&player, &computer, 1);
                pause_enter();
                continue;
            }
            if(strlen(buf)==0){ printf("Entrada vazia\n"); pause_enter(); continue;}
            int rr=-1,cc=-1;
            if(sscanf(buf, " %c %d", &buf[0], &rr)==2){

            }

            if(strlen(buf)>=2 && ((buf[0]>='A'&&buf[0]<='J') || (buf[0]>='a'&&buf[0]<='j'))){
                cc = (buf[0]>='a') ? buf[0]-'a' : buf[0]-'A';
                rr = atoi(buf+1);
                if(!in_bounds(rr,cc)){ printf("Coordenada fora do tabuleiro\n"); pause_enter(); continue; }
            } else {

                int a,b;
                if(sscanf(buf,"%d %d",&a,&b)==2){ rr=a; cc=b; if(!in_bounds(rr,cc)){ printf("Fora\n"); pause_enter(); continue; } }
                else { printf("Formato inválido. Use A5 ou 'linha coluna'\n"); pause_enter(); continue; }
            }
            int res = check_and_mark_hit(&computer, rr, cc);
            if(res==-1){ printf("Você já atirou nessa posição.\n"); pause_enter(); continue; }
            if(res==0){ printf("Você errou em %c%d.\n", 'A'+cc, rr); shots--; pause_enter(); }
            else if(res==1){ printf("Você acertou em %c%d!\n", 'A'+cc, rr); player_points += 1; shots--; pause_enter(); }
            else if(res==2){ printf("Você afundou um navio em %c%d! Bônus de 2 pontos.\n", 'A'+cc, rr); player_points += 1 + 2; shots--; pause_enter(); }

            int anyComputerLeft = ships_remaining(&computer);
            if(!anyComputerLeft) { r=4; break; }
        }

        if(!ships_remaining(&computer)) break;


        clear_screen();
        print_header_and_legend(player_points, 0, r+1);
        print_two_boards(&player, &computer, 0);
        printf("\nVEZ DO COMP. AGUARDE A SUA VEZ!\n");
        fflush(stdout);


        computer_take_shots(&computer, &player, round_shots[r], &pc_points);

        if(!ships_remaining(&player)) { break; }
    }

    show_final_result(&player, &computer, player_points, pc_points);
    printf("\nJogar de novo? 1-SIM  0-NÃO: ");
    int again; if(scanf("%d%*c",&again)!=1) return;
    if(again==1) game_loop();
}

void show_main_menu(){
    clear_screen();

    printf("/n/n██████╗  █████╗ ████████╗ █████╗ ██╗      ██╗  ██╗ █████╗n");     
    printf("██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗██║      ██║  ██║██╔══██╗n");    
    printf("██████╔╝███████║   ██║   ███████║██║█████╗███████║███████║n");    
    printf("██╔═══╝ ██╔══██║   ██║   ██╔══██║██║╚════╝██╔══██║██╔══██║n");    
    printf("██║     ██║  ██║   ██║   ██║  ██║███████╗ ██║  ██║██║  ██║n");    
    printf("╚═╝     ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚══════╝ ╚═╝  ╚═╝╚═╝  ╚═╝n");    
    printf("                                      n");  
    printf("███╗   ██╗ █████╗ ██╗   ██╗ █████╗ ██╗n");     
    printf("████╗  ██║██╔══██╗██║   ██║██╔══██╗██║n");     
    printf("██╔██╗ ██║███████║██║   ██║███████║██║n");     
    printf("██║╚██╗██║██╔══██║╚██╗ ██╔╝██╔══██║██║n");     
    printf("██║ ╚████║██║  ██║ ╚████╔╝ ██║  ██║███████╗n");
    printf("╚═╝  ╚═══╝╚═╝  ╚═╝  ╚═══╝  ╚═╝  ╚═╝╚══════╝\n\n\n");
 
    printf("=========================================\n\n");
    printf(" 1 - COMECAR\n");
    printf(" 2 - TUTORIAL (COMO JOGAR)\n");
    printf(" 3 - SAIR\n\n");
    printf("OPCAO: ");
    printf("=========================================\n\n");
}

void show_records(){
    clear_screen();
    printf("=== Registros / Como posicionar ===\n\n");
    printf("Os navios são: 1 Porta-Avião (4), 1 Encouraçado (3), 2 Cruzadores (2).\n");
    printf("Ao posicionar, informe as extremidades, ex: A0 A3 (coluna letra A-J + linha 0-9)\n");
    printf("Navios não podem ficar adjacentes (incluindo diagonais).\n");
    pause_enter();
}

int main(){
    srand((unsigned int)time(NULL));
    while(1){
        show_main_menu();
        int op;
        if(scanf("%d%*c",&op)!=1) break;
        if(op==1){
            game_loop();
        } else if(op==2){
            show_records();
        } else if(op==3){
            printf("Encerrando... até a próxima!\n");
            break;
        } else {
            printf("Opção inválida\n");
        }
    }
    return 0;
}
