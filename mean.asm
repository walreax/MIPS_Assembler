; SIMPLE Assembly program to calculate the square of a number

; Data section
number:     data 4         ; Number to be squared (change this value as needed)
square:     data 0         ; Result of the square
counter:    data 0         ; Counter to keep track of the additions

; Code section
start:
            ldc 0           ; Load 0 into accumulator A
            stl square      ; Initialize square to 0

            ldl number      ; Load the number to be squared
            stl counter     ; Store it in counter for looping

loop:
            ldl counter     ; Load counter
            brz end         ; If counter is zero, end the loop

            ldl square      ; Load current square value
            ldl number      ; Load the number to add to square
            add             ; Add number to square
            stl square      ; Store updated square

            ldl counter     ; Load counter
            ldc 1           ; Load 1 into A
            sub             ; Decrement counter by 1
            stl counter     ; Store updated counter

            brz loop        ; Repeat the loop if counter is not zero

end:
            ldc 0           ; Stop the program (workaround for halt)
            br end          ; Infinite loop to halt
