#include <LiquidCrystal.h>

// Inicializa a biblioteca com os pinos conectados ao LCD: (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Define os pinos do LDR, LM35, LEDs, buzzer e botões
const int ldrPin = A0;
const int lm35Pin = A1;
const int ledGreenPin = 6;
const int ledYellowPin = 7;
const int ledRedPin = 8;
const int buzzerPin = 9;
const int buttonPins[] = {A2, A3, A4, A5};

// Código de segurança correto
const int correctCode[] = {0, 1, 2, 3};
int enteredCode[4]; // Armazena a entrada do usuário
int codeIndex = 0;
int incorrectAttempts = 0;

// Limite de luminosidade para detectar invasão (valores altos indicam mais luz)
const int ldrThreshold = 900;

bool alarmActive = false;
unsigned long alarmStartTime;
const unsigned long alarmDuration = 60000; // 1 minuto

void setup()
{
  // Configura o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  // Configura os pinos dos LEDs e do buzzer como saída
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledYellowPin, OUTPUT);
  pinMode(ledRedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Configura os pinos dos botões como entrada com pull-up interno
  for (int i = 0; i < 4; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Inicializa os LEDs e o buzzer como desligados
  digitalWrite(ledGreenPin, LOW);
  digitalWrite(ledYellowPin, LOW);
  digitalWrite(ledRedPin, LOW);
  noTone(buzzerPin);
}

void loop()
{
  // Lê o valor do LDR
  int ldrValue = analogRead(ldrPin);

  // Lê o valor do LM35
  int lm35Value = analogRead(lm35Pin);
  float temperature = lm35Value * (5.0 / 1023.0) * 100.0;

  // Exibe a luminosidade e a temperatura na primeira linha do LCD
  lcd.setCursor(0, 0);
  lcd.print("L:");
  lcd.print(ldrValue);
  lcd.print(" T:");
  lcd.print(temperature);
  lcd.print("C  ");

  if (ldrValue > ldrThreshold && !alarmActive)
  {
    // Ativa o alarme
    alarmActive = true;
    alarmStartTime = millis();
  }

  if (alarmActive)
  {
    // Mantém o alarme ativo por um período específico
    if (millis() - alarmStartTime < alarmDuration)
    {
      digitalWrite(ledRedPin, HIGH);
      digitalWrite(ledGreenPin, LOW);
      digitalWrite(ledYellowPin, LOW);
      tone(buzzerPin, 1000); // Liga o buzzer

      // Exibir o prompt para digitar o código na segunda linha do LCD
      lcd.setCursor(0, 1);
      lcd.print("CODE: ");

      // Lê a entrada dos botões para o código de segurança
      for (int i = 0; i < 4; i++)
      {
        if (digitalRead(buttonPins[i]) == LOW)
        {
          enteredCode[codeIndex] = i;
          codeIndex++;
          digitalWrite(ledYellowPin, HIGH); // Indicação visual da entrada do código
          lcd.setCursor(6 + codeIndex - 1, 1);
          lcd.print("*"); // Exibe um * para cada dígito do código
          delay(300);     // Debounce
          digitalWrite(ledYellowPin, LOW);
          while (digitalRead(buttonPins[i]) == LOW)
            ; // Espera até o botão ser liberado

          if (codeIndex >= 4)
          {
            if (isCodeCorrect())
            {
              // Desativa o alarme
              alarmActive = false;
              digitalWrite(ledRedPin, LOW);
              digitalWrite(ledYellowPin, HIGH);
              noTone(buzzerPin); // Desliga o buzzer
              codeIndex = 0;
              lcd.setCursor(6, 1);
              lcd.print("    "); // Limpa os asteriscos
            }
            else
            {
              // Reinicializar a exibição do código se estiver incorreto
              incorrectAttempts++;
              codeIndex = 0;
              lcd.setCursor(6, 1);
              lcd.print("    ");
            }
          }
        }
      }
    }
    else
    {
      // Desativa o alarme após o tempo especificado
      alarmActive = false;
      digitalWrite(ledRedPin, LOW);
      noTone(buzzerPin); // Desliga o buzzer
    }
  }
  else
  {
    // Se o alarme não estiver ativo, mantém o LED verde aceso
    digitalWrite(ledRedPin, LOW);
    digitalWrite(ledGreenPin, HIGH);
    digitalWrite(ledYellowPin, LOW);
    noTone(buzzerPin); // Desliga o buzzer
    lcd.setCursor(0, 1);
    lcd.print("                "); // Limpa a segunda linha
  }

  // Aguarda 500 milissegundos antes de atualizar novamente
  delay(500);
}

bool isCodeCorrect()
{
  for (int i = 0; i < 4; i++)
  {
    if (enteredCode[i] != correctCode[i])
    {
      return false;
    }
  }
  return true;
}
