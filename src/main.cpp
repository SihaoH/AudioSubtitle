#include "Application.h"

int main(int argc, char *argv[]) 
{
    Application app(argc, argv);
    app.init();
    return app.exec();
}
