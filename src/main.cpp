
#include "Backpack.hpp"
#include "ActiveFrame.hpp"

#include <bb/cascades/Application>

#include <Flurry.h>

using namespace bb::cascades;

Q_DECL_EXPORT int main(int argc, char **argv)
{
    Application app(argc, argv);

    Flurry::Analytics::StartSession("PG37BZ5VV7GP2ZC2VRZC");

    app.setObjectName("Backpack");
    app.setCover(new ActiveFrame(&app));

    new Backpack(&app);

    return Application::exec();
}
