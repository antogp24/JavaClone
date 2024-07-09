void function() {
    soutln("Function that does nothing.");
    return;
}

void print_matrix(int rows, int cols) {
    for (int y = 0; y < rows; y++) {
        sout("[ ");
        for (int x = 0; x < cols; x++) {
            if (x < cols - 1)
                sout(x == y ? "1, " : "0, ");
            else 
                sout(x == y ? "1 " : "0 ");
        }
        soutln("] ");
    }
}

int gcd(int a, int b) {
    if (b > a) {
        int temp = a; a = b; b = temp;
    }
    if (b == 0) return a;
    if (b == 1) return 1;
    return gcd(b, a % b);
}

print_matrix(10, 10);
soutln(gcd(105, 30));
function();
soutln(function);
