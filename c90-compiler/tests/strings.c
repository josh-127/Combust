
int main(int argc, char** argv)
{
    char c = 'A';
    c = '\373';
    c = 'dk';
    c = ';

    printf("Hello, world!\n");
    printf("\' \" \? \\ \a \b \f \n \r \t \v \0 \73 \323 \x7F");
    printf("The quick brown fox \x5c\n"
           "jumps over the lazy dog.\n"
           "lorem ipsum dolor");
    printf("foo
    );

    return 0;
}
