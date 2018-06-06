#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 32954

#define SHIP_CELLS_NUM 30

#define NO_ERROR "\n"
#define INVALID_DIRECTION "Invalid direction (use h or v)\n"
#define INVALID_COLUMN "Column letter must be between 'a' and 'j'\n"
#define INVALID_LINE "Line must be between 1 and 9, or 0 (for 10)\n"
#define WRONG_PLACEMENT "Ship can't be placed here,as it goes out of bounds or overrides another ship\n"
#define ALREADY_SHOT "This cell has already been shot\n"

#define h_addr h_addr_list[0] /* for backward compatibility */

// SOCKETS PART

void error(const char *msg){
	perror(msg);
	exit(0);
}

int prepareClientSocket(int sock){
	struct sockaddr_in servername;
	struct hostent *server;
	struct in_addr addr;
	char buffer[256];
	bzero(buffer, 256);
	int option = 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		error("ERROR opening socket");
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	bzero(buffer, 256);
	printf("Please type in opponent IP address: ");
	fgets(buffer, 255, stdin);

	inet_aton(buffer, &addr);
	server = gethostbyaddr(&addr, sizeof(addr), AF_INET);
	if (server == NULL){
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

	bzero((char *) &servername, sizeof(servername));
	servername.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		 (char *)&servername.sin_addr.s_addr,
		 server->h_length);
	servername.sin_port = htons(PORT);

	if (connect(sock,(struct sockaddr *) &servername, sizeof(servername)) < 0)
		error("ERROR connecting");

	return sock;
}

int prepareServerSocket(int sock){
	struct sockaddr_in servername, clientname;
	socklen_t clientlen;
	char buffer[256];
	int option = 1;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
		error("ERROR opening socket");
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = INADDR_ANY;
	servername.sin_port = htons(PORT);

	if(bind(sock, (struct sockaddr *) &servername, sizeof(servername)) < 0)
		error("ERROR on binding");

	listen(sock, 1);

	printf("Waiting your opponent...\n\n");

	clientlen = sizeof(clientname);
	sock = accept(sock, (struct sockaddr *) &clientname, &clientlen);

	return sock;
}

// END OF SOCKETS PART

// GAME PART

void displayGridsHeader()
{
    printf("\n  Main board\t\t\tMark board\n");
}

void displayLineNumber(int line)
{
    line++;
    if(line<10){
        printf(" %i ", line);
    }else{
        printf("%i ", line);
    }
}

void displayGridLine(int line, char grid[10][10])
{
    for(int j=0; j<10; j++)
    {
        putchar(grid[line][j]);
        putchar(' ');
    }
}

void displayGridsLine(int line, char mainBoard[10][10], char markBoard[10][10])
{
    displayLineNumber(line);
    displayGridLine(line, mainBoard);
    printf("\t\t");
    displayGridLine(line, markBoard);
    printf("\n");
}

void displayGridsFooter()
{
    printf("   A B C D E F G H I J\t\tA B C D E F G H I J\n\n");
}

void displayGrids(char mainBoard[10][10], char markBoard[10][10])
{
    displayGridsHeader();
    for(int i=0; i<10; i++){
        displayGridsLine(i, mainBoard, markBoard);
    }
    displayGridsFooter();
}

// Returns 0 if okay, 1 if not
int tryToPlace(int placeShipHorizontaly, int line, int column, int shipSize, char shipSymbol, char mainBoard[10][10])
{
    line -= 1;
    line = (line == -1) ? 9 : line;

    //Check if ship goes out of bounds
    int maxColumnOrLinePlace = 10 - shipSize;

    if(placeShipHorizontaly){

        if(column > maxColumnOrLinePlace) return 1;

        // Horizontal check

        // Check if cells are free
        for(int i=column; i<column+shipSize; i++){
            if(mainBoard[line][i] != '~') return 1;
        }

        for(int i=column; i<column+shipSize; i++)
        {
            mainBoard[line][i] = shipSymbol;
        }
    }
    else
    {
        if(line > maxColumnOrLinePlace) return 1;

        // Vertical check

	// Check cells are free

        for(int i=line; i<line+shipSize; i++)
        {
            if(mainBoard[i][column] != '~') return 1;
        }

        for(int i=line; i<line+shipSize; i++)
        {
            mainBoard[i][column] = shipSymbol;
        }
    }

    return 0;
}

void placeShip(char* shipName, int shipSize, char shipSymbol, char mainBoard[10][10], char markBoard[10][10])
{
    int shipIsNotPlaced = 1;
    char* error = NO_ERROR;

    do
    {
        displayGrids(mainBoard, markBoard);

        printf("%s", error);
        printf("Place the %s (%i cells): ", shipName, shipSize);

        char input[256];
        for(int i=0; i<256;i++) input[i] = '\0';

        fgets(input, sizeof(input), stdin);

        char direction = input[0];
        char column = input[1];
        char line = input[2];

        if(direction == 'h' || direction == 'H' || direction == 'v' || direction == 'V')
        {
            int placeShipHorizontaly = (direction == 'h' || direction == 'H') ? 1 : 0;
            if(line >= '0' && line <= '9')
            {
                int l = line - '0';

                if((column >= 'a' && column <= 'j') || (column >= 'A' && column <= 'J'))
                {
                    int c = (column >= 'a' && column <= 'j') ? column - 'a' : column - 'A';
                    shipIsNotPlaced = tryToPlace(placeShipHorizontaly, l, c, shipSize, shipSymbol, mainBoard);
                    if(shipIsNotPlaced) error = WRONG_PLACEMENT;
                }
                else
                {
                    error = INVALID_COLUMN;
                }
            }
            else
            {
                error = INVALID_LINE;
            }
        }
        else
        {
            error = INVALID_DIRECTION;
        }
    }while(shipIsNotPlaced);
}

void placeShips(char mainBoard[10][10], char markBoard[10][10]){

	//placeShip("carrier", 5, 'C', mainBoard, markBoard);

	//placeShip("battleship #1", 4, 'B', mainBoard, markBoard);
	//placeShip("battleship #2", 4, 'B', mainBoard, markBoard);

	placeShip("submarine #1", 3, 'S', mainBoard, markBoard);
	placeShip("submarine #2", 3, 'S', mainBoard, markBoard);
	placeShip("submarine #3", 3, 'S', mainBoard, markBoard);

	//placeShip("destroyer #1", 2, 'D', mainBoard, markBoard);
	//placeShip("destroyer #2", 2, 'D', mainBoard, markBoard);
	//placeShip("destroyer #3", 2, 'D', mainBoard, markBoard);
	//placeShip("destroyer #4", 2, 'D', mainBoard, markBoard);
}

int gameIsRunning(int *ownShipsTouched, int *opponentShipsTouched){
	if(*ownShipsTouched == SHIP_CELLS_NUM) return 0;
	if(*opponentShipsTouched == SHIP_CELLS_NUM) return 0;
	return 1;
}

int isDesignatedPlaceAlreadyShot(int line, int column, char markBoard[10][10])
{
	line -= 1;
    line = (line == -1) ? 9 : line;

	return (markBoard[line][column] == 'x' || markBoard[line][column] == 'X');
}

void fireOpponent(char mainBoard[10][10], char markBoard[10][10], int sock, int *opponentShipsTouched){
	int invalidCoordinatesToFire = 1;
	char* error = NO_ERROR;

	char column = ' ', line = ' ';

	do
    {
        displayGrids(mainBoard, markBoard);

        printf("%s", error);
        printf("Enter shooting coordinates: ");

        char input[256];
        for(int i=0; i<256;i++) input[i] = '\0';

        fgets(input, sizeof(input), stdin);

        column = input[0];
        line = input[1];


        if(line >= '0' && line <= '9')
        {
            int l = line - '0';

            if((column >= 'a' && column <= 'j') || (column >= 'A' && column <= 'J'))
            {
		
                int c = (column >= 'a' && column <= 'j') ? column - 'a' : column - 'A';

				// Check if coordinates designate an already shot place
				invalidCoordinatesToFire = isDesignatedPlaceAlreadyShot(l, c, markBoard);
				if(invalidCoordinatesToFire) error = ALREADY_SHOT;
				else{
					// Send coordinates of fire to opponent
					char coordinates[3];
					coordinates[0] = l + '0'; coordinates[1] = c + '0'; coordinates[2] = '\0';
					write(sock, coordinates, 2);

					// Wait for response and mark markBoard
					char fireResponse[2];
					bzero(coordinates, 2);
					read(sock, fireResponse, 1);
					char result = fireResponse[0];

					// Result process
					l-=1; if(l<0) l=9;
					markBoard[l][c] = result;
					if(result=='x') (*opponentShipsTouched)++;
					displayGrids(mainBoard, markBoard);
					if(result=='X'){
						printf("Missed shot.\n\n");
					}else{
						printf("THAT WAS A WIN SHOT!\n\n");
					}
				}
            }
            else
            {
                error = INVALID_COLUMN;
            }
        }
        else
        {
            error = INVALID_LINE;
        }

    }while(invalidCoordinatesToFire);

}

void dealWithOpponentFire(int sock, char mainBoard[10][10], int *ownShipsTouched){
	char coordinates[3];
	bzero(coordinates, 3);
	read(sock, coordinates, 2);
	char line = coordinates[0], column = coordinates[1];

	int l = line - '0';
	l -= 1;
	if(l<0) l=9;
	int c = column - '0';

	char shotCell = mainBoard[l][c];

	char fireResponse[2];
	fireResponse[1] = '\0';

	if(shotCell == '~'){
		fireResponse[0] = 'X';
		printf("Opponent missed is shot in %c%i.\n\n", c + 'A', (line=='0')? 10 : line-'0');
	}else{
		mainBoard[l][c] = shotCell + 'a' - 'A';
		fireResponse[0] = 'x';
		(*ownShipsTouched)++;
		printf("CAPTAIN! THE ENNEMY SHOT ONE OF OUR WARSHIP IN %c%i!\n\n", c + 'A', (line=='0')? 10 : line-'0');
	}

	write(sock, fireResponse, 1);
}

// END OF GAME PART

int main()
{
	char choice;
	int sock;
	char buffer[256];
	bzero(buffer, 256);

	int *ownShipsTouched, *opponentShipsTouched;
	int a = 0, b = 0;
	ownShipsTouched = &a; opponentShipsTouched = &b;

	char mainBoard[10][10] = {
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
		{'~','~','~','~','~','~','~','~','~','~'}
 	};

 	char markBoard[10][10] = {
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
        {'~','~','~','~','~','~','~','~','~','~'},
		{'~','~','~','~','~','~','~','~','~','~'},
		{'~','~','~','~','~','~','~','~','~','~'},
		{'~','~','~','~','~','~','~','~','~','~'}
 	};

	do
	{
		printf("Attack (a) or defend (d)?: ");
		choice = getchar(); getchar();
	}while(choice != 'a' && choice != 'A' && choice != 'D' && choice != 'd');

	if(choice == 'd' || choice == 'D'){
		sock = prepareServerSocket(sock);
		placeShips(mainBoard, markBoard);
		displayGrids(mainBoard, markBoard);
		printf("All ships placed, Sir! Waiting for the ennemy to engage...\n");
		dealWithOpponentFire(sock, mainBoard, ownShipsTouched);
	}else{
		sock = prepareClientSocket(sock);
		placeShips(mainBoard, markBoard);
	}

	while(gameIsRunning(ownShipsTouched, opponentShipsTouched))
	{
		fireOpponent(mainBoard, markBoard, sock, opponentShipsTouched);
		if(!gameIsRunning(ownShipsTouched, opponentShipsTouched)) break;	//Check if game is still running
		dealWithOpponentFire(sock, mainBoard, ownShipsTouched); //Read opponent fire, compute, return result
	}

	if(*ownShipsTouched == SHIP_CELLS_NUM){
		printf("\n\nOur fleet has been entirely destroyed. That is a defeat...\n");
	}else{
		printf("\n\nWELL DONE SIR! VICTORY!\n");
	}

	close(sock);

	return 0;
}
