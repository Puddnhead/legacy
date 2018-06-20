.model small
.data
	EXTRN 	p1_name:BYTE,p2_name:BYTE,location
	NL					db 13,10,'$'
	quit_confirmation 	db "Are you sure you want to quit? ",0,'$'
	ttt_border		  	db "***************************",0,'$'
    tic_tac_toe         db "* Sk8er Tic-Tac Toe v2.0  *",0,'$'
	p1_wins				dw 0
	p2_wins				dw 0
	wins				db "Wins",0
	turn				db "Turn",0
	control_turn		db 0			;byte 0=turn byte;
	board				dw 3 dup (3 dup (0)) 		;0-nothing 1-X 2-O
	board_locs			dw 3 dup (3 dup (0))
	player1_prompt 		db "Name of Player 1: ",0
	player2_prompt 		db "Name of Player 2: ",0
    welcome             db "Welcome to Sk8er Tic-Tac-Toe v2.0",0
	play_again_prompt	db "You're playing again right? ",0
	cats_game			db "Cats game.  Play another pointless game? ",0
.code
    PUBLIC GET_MOVE,TOGGLE_TURN,PRINT_WINS,MOVE_CURSOR,MAKE_MOVE,SETUP,PLAY_AGAIN,INIT_ARRAY
    PUBLIC BACKGFUN,SETMODE,MAKE_BOARD,BORDER_ROW,INSIDE_ROW,SET_SCREEN,CHECK_WIN
    EXTRN GETCH:NEAR,GETS:NEAR
    EXTRN SETBACKG:NEAR
	EXTRN PUTCHAR:NEAR, PRINTS:NEAR, COLOR_PRINTS:NEAR, PRINTINT:NEAR
	EXTRN CURSMOV:NEAR,CURSSET:NEAR,GETCURSSETS:NEAR

PROC	INIT_ARRAY
push	di bx cx
	mov		di,offset board
	mov		cx,3
	xor		bx,bx
out_array_loop:
	push	cx
	mov		cx,3
in_array_loop:
	mov		[di+bx],word ptr 0
	add		bx,2 		;next word
	loop	in_array_loop
	
	pop		cx
	loop	out_array_loop

	pop		cx bx di
	ret
ENDP	INIT_ARRAY

PROC	CHECK_WIN NEAR
	push 	di bx cx dx
	mov		di,offset board
	xor		bx,bx			;set up for horizontal check
	mov		cx,3
	cmp		control_turn,1
	jnz		player_two_check_win
	mov		ax,1
	jmp		check_horizontal
player_two_check_win:
	mov		ax,2
check_horizontal:
	push	cx
	mov		cx,3
inner_horizontal:
	cmp		ax,[di+bx]
	jz		horizontal_iloop
	jmp		next_horizontal
horizontal_iloop:
	add		bx,2
	loop	inner_horizontal
	jmp		somebody_won_vh
next_horizontal:
	pop		cx
	cmp		cx,3		;first time through
	jz		first_horizontal
	mov		bx,12		;second time through
	jmp		loop_to_horizontal
first_horizontal:
	mov		bx,6
	
loop_to_horizontal:
	loop	check_horizontal
pre_vertical:
	xor		bx,bx
	mov		cx,3
	mov		di,offset board
check_vertical:
	push	cx
	mov		cx,3
inner_vertical:
	cmp		ax,[di+bx]
	jz		vertical_iloop
	jmp		next_vertical
vertical_iloop:
	add		di,6
	loop	inner_vertical
	jmp		somebody_won_vh
next_vertical:
	pop		cx
	add		bx,2
	mov		di,offset board
	loop	check_vertical
pre_diagonal1:
	xor		bx,bx
	mov		di,offset board
	mov		cx,3
inner_diagonal1:
	cmp		ax,[di+bx]
	jz		diagonal1_iloop
	jmp		pre_diagonal2
diagonal1_iloop:
	add		bx,8
	loop	inner_diagonal1
	jmp		somebody_won
pre_diagonal2:
	mov		di,offset board
	add		di,4
	xor		bx,bx
	mov		cx,3
inner_diagonal2:
	cmp		ax,[di+bx]
	jz		diagonal2_iloop
	jmp		check_cats
diagonal2_iloop:
	add		bx,4
	loop	inner_diagonal2
	jmp		somebody_won
check_cats:
	mov		di,offset board
	xor 	bx,bx
	mov		cx,9
check_cats_loop:
	mov		dx,[di+bx]
	and		dx,dx
	jnz		check_cats_iloop
	jmp		nobody_won
check_cats_iloop:
	add		bx,2
	loop	check_cats_loop
	mov		ax,9
	jmp		finish_check_win	
somebody_won:
	cmp		ax,1
	jnz		player_two_won
	inc		p1_wins
	jmp		cont_somebody_won
player_two_won:
	inc		p2_wins
cont_somebody_won:
	mov		ax,1
	jmp		finish_check_win
nobody_won:
	xor		ax,ax
finish_check_win:
	pop		dx cx bx di
	ret
somebody_won_vh:
	pop		cx
	jmp		somebody_won
ENDP	CHECK_WIN

PROC	PLAY_AGAIN NEAR
	push	cx dx
	
	call	GETCURSSETS
	push	dx
	
	mov		dx,0720h
	call	CURSMOV
	cmp		ax,9
	jnz		not_cats
	mov		di,offset	cats_game
	jmp		continue_play_again_print
not_cats:
	mov		di,offset	play_again_prompt
continue_play_again_print:
	mov		bx,15
	call	COLOR_PRINTS
	
play_again_loop:
	call	GETCH
	cmp		al,59h	;Y
	jnz		skip_y1
	jmp		yes_play_again
skip_y1:
	cmp		al,79h	;y
	jnz		skip_y2
	jmp		yes_play_again
skip_y2:
	cmp		al,4Eh	;N
	jnz		skip_n1
	jmp		no_play_again
skip_n1:
	cmp		al,6Eh	;n
	jnz		skip_n2
	jmp		no_play_again
skip_n2:
	jmp	play_again_loop
yes_play_again:
	mov		ax,0259h
	int		21h
	pop		dx
	call	CURSMOV
	call	CLEAR_PROMPT_LINE
	mov		ax,1
	pop		dx cx
	ret
no_play_again:
	mov		ax,024Eh
	int		21h
	xor		ax,ax
	pop		dx dx cx
	ret
ENDP	PLAY_AGAIN
	
		

PROC	MAKE_MOVE NEAR
	push	di bx cx dx
	mov		di,offset board_locs
	call	GETCURSSETS
	mov		cx,9
	xor		bx,bx
make_move_loop1:
	cmp		dx,[di+bx]
	jz		found_it
	add		bx,2
	loop	make_move_loop1
found_it:
	mov		di,offset board
	mov		ax,[di+bx]
	and		ax,ax
	jz		continue_make_move1
	jmp		abort_move
continue_make_move1:
	mov		cl,control_turn
	and		cl,cl
	jnz		p1_make_move
	jmp		p2_make_move
p1_make_move:
	mov		[di+bx],word ptr 1
	mov		ax,0958h
	mov		cx,1
	mov		bx,13
	int		10h
	mov		ax,1
	jmp		finish_make_move
p2_make_move:
	mov		[di+bx],word ptr 2
	mov		ax,094Fh
	mov		cx,1
	mov		bx,14
	int		10h
	mov		ax,2
	jmp		finish_make_move
abort_move:
	xor		ax,ax
finish_make_move:
	pop		dx cx bx di
	ret
ENDP

PROC	MOVE_CURSOR NEAR
	push	bx cx dx
	cmp		ax,1
	jnz		skip_up_move
	jmp		up_move
skip_up_move:
	cmp		ax,2
	jnz		skip_down_move
	jmp		down_move
skip_down_move:
	cmp		ax,3
	jnz		right_move
	jmp		left_move
right_move:
	call	GETCURSSETS
	cmp		dl,58			;is it off the board?
	jnz		continue_right_move
	jmp		finish_move
continue_right_move:
	add		dl,7
	call	CURSMOV
	jmp		finish_move
up_move:
	call	GETCURSSETS
	cmp		dh,12
	jnz		continue_up_move
	jmp		finish_move
continue_up_move:
	sub		dh,4
	call	CURSMOV
	jmp		finish_move
down_move:
	call	GETCURSSETS
	cmp		dh,20
	jnz		continue_down_move
	jmp		finish_move
continue_down_move:
	add		dh,4
	call	CURSMOV
	jmp		finish_move
left_move:
	call	GETCURSSETS
	cmp		dl,44
	jnz		continue_left_move
	jmp		finish_move
continue_left_move:
	sub		dl,7
	call	CURSMOV
finish_move:
	pop		dx cx bx
	ret
ENDP	MOVE_CURSOR
	

PROC TOGGLE_TURN NEAR
	push	ax bx cx dx
	call	GETCURSSETS
	
	push	dx cx
	mov		cx,1000h
	call	CURSSET
	
	xor		control_turn,1
	mov		cl,control_turn
	and		cl,cl			;get turn only-1 is player one 0 is player two
	jz		playertwo
	
	mov		dx,1220h
	call	CURSMOV
	mov		ax,0220h
	int		21h
	mov		dx,0E20h
	call	CURSMOV
	mov		cx,1
	mov		ax,0905h
	mov		bx,12
	int		10h
	jmp		finish_up
	
playertwo:
	mov		dx,0E20h
	call	CURSMOV
	mov		ax,0220h
	int		21h
	mov		dx,1220h
	call	CURSMOV
	mov		cx,1
	mov		ax,0905h
	mov		bx,12
	int		10h
finish_up:
	pop		cx
	call	CURSSET
	pop		dx
	call	CURSMOV
	pop		dx cx bx ax	
	ret
ENDP TOGGLE_TURN
	
PROC PRINT_WINS
	push	ax bx cx dx
	
	call	GETCURSSETS
	push	cx dx
	
	mov		cx,1000h
	call	CURSSET
	
	xor		bx,bx
	mov		dx,0E19h
	call	CURSMOV
	mov		ax,p1_wins
	call	PRINTINT
	mov		dx,1219h
	call	CURSMOV
	mov		ax,p2_wins
	call	PRINTINT
	
	pop		dx
	call	CURSMOV
	pop		cx
	call	CURSSET
	
	pop		dx cx bx ax
	ret
ENDP PRINT_WINS

PROC SET_SCREEN NEAR
	push 	ax bx cx dx di
	mov		dx,001Ah
	call	CURSMOV
	mov		di,offset ttt_border
	mov		bx,1
	call	COLOR_PRINTS
	mov		dx,011Ah
	call	CURSMOV
	mov		di,offset tic_tac_toe      ;make little thing on top of screen
	call	COLOR_PRINTS
	mov		dx,021Ah
	call	CURSMOV
	mov		di,offset ttt_border
	call	COLOR_PRINTS
	
	mov		dx,0A19h                ;this part makes wins and turns
	call	CURSMOV
	mov		di,offset wins
	mov		bx,5
	call	COLOR_PRINTS
	mov		dx,0A20h
	call	CURSMOV
	mov		di,offset turn
	call	COLOR_PRINTS
	
	mov		dx,0E00h
	call	CURSMOV
	mov		di,offset p1_name        ;prints player names
	mov		bx,2
	call	COLOR_PRINTS
	mov		dx,1200h
	call	CURSMOV
	mov		di,offset p2_name
	mov		bx,2
	call	COLOR_PRINTS

	pop		di dx cx bx ax
	ret
ENDP SET_SCREEN
	
PROC MAKE_BOARD
	push 	ax bx cx dx
	
	mov		dx,0A28h
	mov		cx,4
boarder_row_loop:
	call	CURSMOV
	call 	BORDER_ROW
	add		dh,4
	loop 	boarder_row_loop

	mov		dx,0B28h
	mov		cx,3
outer_inside_row_loop:
	push	cx
	mov		cx,3
inner_inside_row_loop:
	call	CURSMOV
	call	INSIDE_ROW
	inc		dh
	loop	inner_inside_row_loop
	
	pop		cx
	inc		dh
	loop	outer_inside_row_loop
	
	mov		dx,0C2Ch
	call	CURSMOV
	
	pop		dx cx bx ax
	ret
ENDP MAKE_BOARD
	
PROC BORDER_ROW NEAR
	push	ax bx cx dx
	mov		cx,23
	mov		bx,10
	mov		al,2
boarder_loop:
	call	PUTCHAR
	loop	boarder_loop
	
	pop		dx cx bx ax
	ret
ENDP BORDER_ROW

PROC INSIDE_ROW NEAR
	push	ax bx cx dx
	mov		cx,3
	mov		bx,4
	mov		al,1
	call	PUTCHAR
	call	PUTCHAR
outside_loop:
	push	cx
	mov		cx,5
	mov		al,20h
inside_loop:
	call	PUTCHAR
	loop	inside_loop
	
	mov		bx,4
	mov		al,1
	call	PUTCHAR
	call	PUTCHAR
	pop 	cx
	loop	outside_loop
	pop		dx cx bx ax
	ret
ENDP INSIDE_ROW			

PROC GET_MOVE NEAR			;returns ax
	push 	bx cx dx		;0-quit		1-up	2-down	3-left	4-right
	mov 	cx,000Fh		;make cursor big
	call 	CURSSET	
arrow_loop:
	call 	GETCH
	xor 	ah,ah

	cmp 	al,113
	jz 		quit
	jmp 	skip_quit1
quit:           ;both quit comparison's lead here, ask for confirmation
				;first, save cursor position and move
	call	GETCURSSETS
	push	dx
	mov		dx,0728h
	call	CURSMOV
	mov 	dx,offset quit_confirmation     ;quit if necessary, else loop again
	mov 	ax,0900h
	int		21h
	call 	GETCH
	pop		dx
	call	CURSMOV
	cmp 	al,121
	jz 		endgame
	jmp 	skip_endgame1
endgame:
	xor 	ax,ax
	pop 	dx cx bx
	ret
skip_endgame1:
	cmp 	al,89
	jz 		endgame
skip_endgame2:
	call	CLEAR_PROMPT_LINE
	jmp 	arrow_loop
skip_quit1:
	cmp 	al,81
	jnz 	skip_quit2
	jmp 	quit
skip_quit2:
	cmp		al,0
	jnz		skip_zero
	jmp		is_zero
skip_zero:
	cmp		al,13
	jz		is_enter
	jmp		arrow_loop				;this executes if nothing checks out		
is_enter:
	mov		ax,13
	jmp		finish_it
is_zero:
	call	GETCH
	cmp		al,72
	jnz		continue_zero1
	jmp		is_up
continue_zero1:
	cmp		al,80
	jnz		continue_zero2
	jmp		is_down
continue_zero2:
	cmp		al,75
	jnz		continue_zero3
	jmp		is_left
continue_zero3:
	cmp		al,77
	jnz		continue_loop
	jmp		is_right
continue_loop:
	jmp		arrow_loop
is_up:
	mov		ax,1
	jmp		finish_it
is_down:
	mov		ax,2
	jmp		finish_it
is_left:
	mov		ax,3
	jmp		finish_it
is_right:
	mov		ax,4
finish_it:
	pop		dx cx bx
	ret
endp GET_MOVE

PROC BACKGFUN NEAR
	push	bx cx dx
	
	mov 	al,12h
	call 	SETMODE

	xor 	bx,bx
	xor 	dx,dx

tomgreen:
	mov 	cx,15
wkrp:
	call 	SETBACKG
	inc 	bl
	loop 	wkrp
	
	inc 	dx
	cmp 	dx,3
	jnz 	tomgreen
	
	mov 	bl,0
	call 	SETBACKG
	
	mov		al,2
	call	SETMODE
	pop		dx cx bx
	ret
ENDP BACKGFUN

PROC SETMODE NEAR
	xor 	ah,ah
	int 	10h
	ret
ENDP SETMODE

PROC CLEAR_PROMPT_LINE
	push	ax cx dx
	call	GETCURSSETS
	push	dx
	
	mov		dx,0700h
	call	CURSMOV
	
	mov		cx,80
	mov		ax,0220h
clearp_loop:
	int		21h
	loop	clearp_loop

	pop		dx
	call	CURSMOV
	
	pop		dx cx ax
	ret
ENDP CLEAR_PROMPT_LINE

PROC COLOR_PRINTAT NEAR  ;dh=row dl=column bh=page bl=color di=pointer
	push	dx
	mov 	dx,location
	call 	CURSMOV
	pop 	dx
	call 	COLOR_PRINTS
	ret
ENDP COLOR_PRINTAT

PROC SETUP NEAR
	push 	di ax bx dx
	call	SETUP_ARRAY		
	mov	location,0017h
	mov 	di,offset welcome
	mov 	bx,132
	call 	COLOR_PRINTAT
	mov 	location,0400h
	mov 	di,offset player1_prompt
	mov 	bx,6
	call 	COLOR_PRINTAT
	mov 	dx,offset p1_name
	mov 	cx,25
	call 	GETS
	mov 	location,0500h
	mov 	di,offset player2_prompt
	mov 	bx,6
	call 	COLOR_PRINTAT
	mov 	cx,25
	mov 	dx,offset p2_name
	call 	GETS
	pop		dx bx ax di
	ret
endp SETUP

PROC	SETUP_ARRAY NEAR
	push	di bx cx dx
	mov		di,offset board_locs
	mov		dx,0C2Ch 	;- location of upper left square
	mov		cx,3
	xor		bx,bx
o_array_loop:
	push	cx
	mov		cx,3
i_array_loop:
	mov		[di+bx],dx
	add		dl,7		;next column
	add		bx,2 		;next word
	loop	i_array_loop
	
	add		dh,4		;next row
	sub		dl,21		;back to start of column (14 + 7 that was unecessarily added on last pass)
	
	pop		cx
	loop	o_array_loop

	pop		dx cx bx di
	ret
ENDP	SETUP_ARRAY
end
