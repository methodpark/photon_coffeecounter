/* This sketch is designed to work with Jura coffeemakers (e.g. X9) on a Particle Photon.
Special thanks to Oliver Krohn who published many source code to get the communication running: https://github.com/psct/sharespresso
*/

#ifndef bitRead
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#endif

byte z0, z1, z2, z3;
byte x0, x1, x2, x3, x4;
byte intra = 1, inter = 7;
String hexval;
String outputString;
bool init = false;

int espresso_new = 0, ristretto_new = 0, coffee_new = 0, cappuccino_new = 0, latte_macchiato_new = 0, milkcoffee_new = 0;
int espresso_old = 0, ristretto_old = 0, coffee_old = 0, cappuccino_old = 0, latte_macchiato_old = 0, milkcoffee_old = 0;

int totalnumberofcups_new = 0;
int totalnumberofcups_old = 0;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  //init particle cloud variable
  Particle.variable("cupcounter", totalnumberofcups_new);

}

int getCounterFromCoffeemaker(int offset)
{
  if(offset > 0x800) return -1;

  hexval = String(offset, HEX);
  while(hexval.length() < 3) hexval = "0" + hexval;

  toCoffeemaker('R'); delay(inter);
  toCoffeemaker('E'); delay(inter);
  toCoffeemaker(':'); delay(inter);
  toCoffeemaker('0'); delay(inter);
  toCoffeemaker(hexval[0]); delay(inter);
  toCoffeemaker(hexval[1]); delay(inter);
  toCoffeemaker(hexval[2]); delay(inter);
  toCoffeemaker(0x0D); delay(inter);
  toCoffeemaker(0x0A); delay(100);

  String answer = "";
  byte d0, d1, d2, d3;

  while(Serial1.available()) {
    delay(intra); d0 = Serial1.read();
    delay(intra); d1 = Serial1.read();
    delay(intra); d2 = Serial1.read();
    delay(intra); d3 = Serial1.read();
    delay(inter);
    answer += char(fromCoffeemaker(d0,d1,d2,d3));
  }

  if (answer.length() == 9) {
    String hex = answer.substring(3,7);
    int number = (int)strtol(hex.c_str(), NULL, 16);
    return number;
  } else {
    return -1;
  }
}

void setNewValues() {
    espresso_old = espresso_new;
    ristretto_old = ristretto_new;
    coffee_old = coffee_new;
    cappuccino_old = cappuccino_new;
    latte_macchiato_old = latte_macchiato_new;
    milkcoffee_old = milkcoffee_new;
    totalnumberofcups_old = totalnumberofcups_new;
}

void readCoffeeCounters(){
  //read all relevant counters from the coffeemaker EEPROM
  //EEPROM adresses may be different according to coffeemaker configuration
  espresso_new = getCounterFromCoffeemaker(0x280);
  ristretto_new = getCounterFromCoffeemaker(0x281);
  coffee_new = getCounterFromCoffeemaker(0x282);
  cappuccino_new = getCounterFromCoffeemaker(0x284);
  latte_macchiato_new = getCounterFromCoffeemaker(0x285);
  milkcoffee_new = getCounterFromCoffeemaker(0x288);

}

void initCounters() {

  readCoffeeCounters();

  totalnumberofcups_new = espresso_new + ristretto_new + coffee_new + cappuccino_new + latte_macchiato_new + milkcoffee_new;

  setNewValues();

  //publish to particle console
  Particle.publish("coffeecups", String(totalnumberofcups_new), 60, PRIVATE);
  //publish as particle cloud variable
  Particle.variable("cupcounter", totalnumberofcups_new);

}

//decode message from coffeemaker
byte fromCoffeemaker(byte x0, byte x1, byte x2, byte x3) {
  bitWrite(x4, 0, bitRead(x0,2));
  bitWrite(x4, 1, bitRead(x0,5));
  bitWrite(x4, 2, bitRead(x1,2));
  bitWrite(x4, 3, bitRead(x1,5));
  bitWrite(x4, 4, bitRead(x2,2));
  bitWrite(x4, 5, bitRead(x2,5));
  bitWrite(x4, 6, bitRead(x3,2));
  bitWrite(x4, 7, bitRead(x3,5));
  return x4;
}

//encode messages to coffeemaker
byte toCoffeemaker(byte z) {
  z0 = 255;
  z1 = 255;
  z2 = 255;
  z3 = 255;

  bitWrite(z0, 2, bitRead(z,0));
  bitWrite(z0, 5, bitRead(z,1));
  bitWrite(z1, 2, bitRead(z,2));
  bitWrite(z1, 5, bitRead(z,3));
  bitWrite(z2, 2, bitRead(z,4));
  bitWrite(z2, 5, bitRead(z,5));
  bitWrite(z3, 2, bitRead(z,6));
  bitWrite(z3, 5, bitRead(z,7));

  delay(intra); Serial1.write(z0);
  delay(intra); Serial1.write(z1);
  delay(intra); Serial1.write(z2);
  delay(intra); Serial1.write(z3);
  delay(inter);
}

void loop() {

if (!init) {
  initCounters();
  init = true;
}

  readCoffeeCounters();

  totalnumberofcups_new = espresso_new + ristretto_new + coffee_new + cappuccino_new + latte_macchiato_new + milkcoffee_new;

  if(totalnumberofcups_new > totalnumberofcups_old) {
    //publish to particle console
    Particle.publish("coffeecups", String(totalnumberofcups_new), 600, PRIVATE);
    //publish as particle cloud variable
    Particle.variable("cupcounter", totalnumberofcups_new);
  }

  setNewValues();
  //check for new values every 10 seconds
  delay(10 * 1000);
}
