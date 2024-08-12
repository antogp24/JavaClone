void function() {
    soutln("Function that does nothing.");
}

void print_identity_matrix(int rows, int cols) {
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

long fibonacci(long n) {
    if (n < 2) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

long fib(long n) {
    if (n < 2) return n;
    double p = pow(1 + sqrt(5), n) - pow(1 - sqrt(5), n);
    double q = pow(2, n) * sqrt(5);
    return (long)(p / q);
}

boolean is_prime(int n) {
    boolean result = true;
    for (int d = 2; d <= (int)sqrt(n); d++) {
        if (n % d == 0) {
            result = false;
            break;
        }
    }
    return result;
}

long gcd(long a, long b) {
    if (b > a) {
        long temp = a; a = b; b = temp;
    }
    if (b == 0) return a;
    if (b == 1) return 1;
    return gcd(b, a % b);
}

long lcm(long a, long b) {
    return (a * b) / gcd(a, b);
}

class Math {
    static final double PI = 3.14159;

    static int gcd(int a, int b) {
        if (b > a) {
            int temp = a; a = b; b = temp;
        }
        if (b == 0) return a;
        if (b == 1) return 1;
        return gcd(b, a % b);
    }

    static int lcm(int a, int b) {
        return (a * b) / gcd(a, b);
    }
}


// Printing an identity matrix.
print_identity_matrix(10, 10);

// Printing greatest common divisor, and lowest common multiple of 105 and 30.
sout("gcd(105, 30): ");
soutln(gcd(105, 30));
sout("lcm(105, 30): ");
soutln(lcm(105, 30));


// Printing out the first n fibonacci numbers and timing it.
long start = clock(), n = 50;
for (int i = 1; i <= n; i++) {
    sout(fib(i));
    if (i < n) sout(", ");
    else soutln("");
}
long end = clock();
sout("It took ");
sout(end - start);
sout(" milliseconds to print the first ");
sout(n);
soutln(" fibonacci numbers.");

// Checking if a number is prime.
int number = 12;
sout("Is ");
sout(number);
sout(" prime? ");
soutln(is_prime(number));

// Calling a function and printing its value.
function();
soutln(function);
soutln(pow);

// Testing classes and instances.
soutln(Math);
soutln(Math.PI);
soutln(Math.gcd(30, 105));