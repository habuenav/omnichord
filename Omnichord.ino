const int SOFT_POT_PIN = A0;
const int GRAPH_LENGTH = 60;

void setup() 
{
  Serial.begin(9600);
  pinMode(SOFT_POT_PIN, INPUT);
}

void loop() 
{
  int softPotADC = analogRead(SOFT_POT_PIN);
  int softPotPosition = map(softPotADC, 0, 1023, 0, GRAPH_LENGTH);

  Serial.print("<"); // Starting end
  for (int i=0; i<GRAPH_LENGTH; i++) {
    if (i == softPotPosition) Serial.print("|");
    else Serial.print("-");
  }
  Serial.println("> (" + String(softPotADC) + ")");

  delay(25);
}
