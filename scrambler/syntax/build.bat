bison --define=.\ha_parser.tab.h -o.\ha_parser.tab.c -d ha_parser.y
flex -o.\ha_parser.c ha_parser.l