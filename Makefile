CC = gcc
CFLAGS = -g -Wall
LEX = flex
YACC = bison
YFLAGS = -d

TARGET = json2relcsv

OBJS = main.o ast.o csv.o schema.o parser.tab.o lex.yy.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Flex rule - ensures parser.tab.h exists first
lex.yy.c: scanner.l | parser.tab.h
	$(LEX) $<

# Bison rule - generates both .c and .h files
parser.tab.c parser.tab.h: parser.y
	$(YACC) $(YFLAGS) $<

# Compilation rule for all .c files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Additional explicit dependencies
main.o: ast.h csv.h schema.h parser.tab.h
ast.o: ast.h
csv.o: csv.h ast.h schema.h
schema.o: schema.h ast.h
parser.tab.o: ast.h
lex.yy.o: parser.tab.h

clean:
	rm -f $(TARGET) *.o lex.yy.c parser.tab.c parser.tab.h

.PHONY: all clean
