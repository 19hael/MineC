int x = 10;
int y = 20;

void main() {
    int result = x + y;
    print(result);
    
    asm {
        mov rax 42
        add rax 8
        push rax
    }
    
    int z = x * y + 5;
    print(z);
}