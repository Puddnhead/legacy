DOSSEG
.model small
.stack 100h
.data
	PUBLIC p1_name,p2_name,location
	p1_name     db 25 dup (?)
	p2_name     db 25 dup (?)
	location	dw ?
.code

	jmp 	start
	EXTRN CLS:NEAR
	EXTRN PUTCHAR:NEAR,COLOR_PRINTS:NEAR,PRINTS:NEAR
	EXTRN GETCH:NEAR,GETS:NEAR
	EXTRN CURSMOV:NEAR
	EXTRN GET_MOVE:NEAR,TOGGLE_TURN:NEAR,PRINT_WINS:NEAR,MOVE_CURSOR:NEAR,CHECK_WIN:NEAR,INIT_ARRAY:NEAR
	EXTRN BACKGFUN:NEAR,MAKE_BOARD:NEAR,SET_SCREEN:NEAR,MAKE_MOVE:NEAR,SETUP:NEAR,PLAY_AGAIN:NEAR

start:
	mov 	ax,@data
	mov 	ds,ax

	call 	CLS
	call 	SETUP
outside_gameloop:
	call	INIT_ARRAY
	call 	CLS
	call	SET_SCREEN
	call	MAKE_BOARD
	call	PRINT_WINS
inside_gameloop:
	call	TOGGLE_TURN
gameloop:
	call	GET_MOVE
	cmp		ax,0
	jnz		continue_gameloop    ;this stuff checks to see if they want to quit
	jmp		quit_game
continue_gameloop:
	cmp		ax,13
	jnz		continue_gameloop2
	jmp		makemove
continue_gameloop2:
	call	MOVE_CURSOR
	jmp		gameloop
makemove:
	call	MAKE_MOVE
	and		ax,ax
	jnz		jump_outer
	jmp		gameloop
jump_outer:
	call	CHECK_WIN
	and		ax,ax
	jnz		somebody_won
	jmp		inside_gameloop
somebody_won:
	call	PLAY_AGAIN
	and		ax,ax
	jz		quit_game
	jmp		outside_gameloop
quit_game:
	call	CLS
	mov ax,4c00h
	int 21h
end start

