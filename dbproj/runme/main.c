#include <stdlib.h>
#include <stdio.h>

int main() 
{
	return system("java -cp lib\\mysql-connector-java-5.1.18-bin.jar -cp lib\\swt.jar lib\\ponytrivia.jar ponytrivia.gui.MainScreen");
}

