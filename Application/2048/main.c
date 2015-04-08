/*++

    GPLv3.0 FreeAsFreedom @2015 Kurt Qiao

    Abstract:
       2048 char game in UEFI shell, 
    shameless copy from https://github.com/clinew/2048
    
    Revision History
    v0.1    2015/4/8  kurt  1. optimzied fix show and color number
                            2. implement leave game key
                            3. show movement counter 
--*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "board.h"

int main(
  int   argc, 
  char* argv[]
) 
{
	struct		board board;
	int		status; // Game status.
	int		valid;
	EFI_STATUS	EStatus;
	EFI_INPUT_KEY	key;
	UINTN		Index;
	UINTN		move;

	EStatus = EFI_SUCCESS;
	move	= 0;

	// Set up board.
	board_init(&board);
	
	// Play the game.
	status = board_done(&board);
	while (!(status)) {
		
		
		gST->ConOut->SetCursorPosition(gST->ConOut, 0, 17);
		// Print the board.
		board_print(&board);

		// Get the player's move.
		valid = 0;
		
		//wait for key press
		EStatus = gBS->WaitForEvent(1, &(gST->ConIn->WaitForKey), &Index);
		EStatus = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);
		//clear status message
		EStatus = gST->ConOut->SetCursorPosition(gST->ConOut, 25, 17);
		Print(L"                       ");   
		

		if (key.UnicodeChar == L'w'|| key.ScanCode == 0x01){
			valid = board_move_up(&board);
		} else if (key.UnicodeChar == L's'|| key.ScanCode == 0x02){
			valid = board_move_down(&board);
		} else if (key.UnicodeChar == L'a'|| key.ScanCode == 0x04){
			valid = board_move_left(&board);
		} else if (key.UnicodeChar == L'd'|| key.ScanCode == 0x03){
			valid = board_move_right(&board);
		} else if (key.UnicodeChar == L'l'){
			EStatus = gST->ConOut->SetCursorPosition(gST->ConOut, 25, 17);
			Print(L"Leave game!");
			gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGRAY|EFI_BACKGROUND_BLACK);
			return EFI_SUCCESS;
		} else {
			//do nothing when press other keys
			continue;
		}

		// Prepare for user's next move.
		if (valid) {
			board_plop(&board);
		} else {
			gST->ConOut->SetCursorPosition(gST->ConOut, 25, 17);
			Print(L"Invalid move.");
		}
	        status = board_done(&board);

		//show movement counter
		EStatus = gST->ConOut->SetCursorPosition(gST->ConOut, 30, 20);
		Print(L"  [%d]  ", ++move);
	}
	
	// Print the final board.
	gST->ConOut->SetCursorPosition(gST->ConOut, 25, 17);
	gST->ConOut->SetAttribute(gST->ConOut, EFI_RED|EFI_BACKGROUND_BLACK);
	Print(L"Game over, you %s!", (status < 0) ? L"LOSE" : L"WIN");

	gST->ConOut->SetCursorPosition(gST->ConOut, 0, 17);
	board_print(&board);

	gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGRAY|EFI_BACKGROUND_BLACK);
	// Return success.
	return EStatus;
 }
