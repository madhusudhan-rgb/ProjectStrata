#include <QApplication>
#include <QPushButton>
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QPushButton button("hello");

    button.resize(400,200);
    button.show();
    return app.exec();
}
