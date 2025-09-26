#include <DFRobot_HuskyLens.h>
#include <Wire.h>

DFRobot_HuskyLens huskylens;

// Devuelve 1 si ve objeto id1, 2 si ve objeto id2, 0 si nada
int leerHuskyLens() {
    huskylens.request();
    if (huskylens.isAppear(1, HUSKYLENSResultBlock)) return 1;
    if (huskylens.isAppear(2, HUSKYLENSResultBlock)) return 2;
    return 0;
}
