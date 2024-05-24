#include "LoRa.hpp"

namespace Lora2 {

HardwareSerial LoRaWioE5(1);

void init() {
  // ZROBIĆ DO SETUPU ODPOWIEDNIĄ OBSŁUGĘ W RAZIE ŹLE PODŁĄCZONEJ LoRy
  // .isavailable() (taka metoda z chata)

  delay(1000);
  LoRaWioE5.begin(9600, SERIAL_8N1, 15, 19);
  // Komunikacja z modułem LoRa Wio E5, piny RX i TX

  Serial.println(
      "\n[debug]Wstepna konfiguracja modulu");  // drukowanie do debugu, mozna
                                                // usunac
  // Ustawiamy moduł w tryb testowy
  LoRaWioE5.print("AT+MODE=TEST\r\n");  // Ustawia tryb nadajnika na TEST

  String response = LoRaWioE5.readString();

  if (response != "+MODE: TEST") {
    Serial.println("\n[debug] Lora is not available");
    Serial.println("\n[debug] Failed message: " + response);
  }

  Serial.println("\n[debug]Odpowiedz: " + response);
  // Drukuje na serial monitorze odpowiedz na wyslana komende

  Serial.println("\n[debug]Przesylanie danych");
  delay(1000);  // Chwilowe opoznienie, prawdopodobnie niezbedne do prawidlowego
                // dzialania

  // Konfigurujemy moduł
  LoRaWioE5.print(
      "AT+TEST=RFCFG, 868, SF12, 125, 12, 15, 14, ON, OFF, OFF\r\n");
  // nastepne
  // ustawienie
  // nadajnika
  response = LoRaWioE5.readString();
  Serial.println("\n[debug]Odpowiedz: " + response);
  // Drukuje odpowiedz na komende na serial monitorze
  delay(1000);
}

void send(String wiadomosc) {
  // Na transmiterze

  LoRaWioE5.print("AT+TEST=TXLRSTR,\"" + wiadomosc + "\"\r\n");
  // przenieisienie karteki zeby prawidlowo dzialalo
  // Przesyla do LoRy komende, ktora wysyla wiadomosc, sformatowana w
  // odpowiedniej funkcji, ktora jest przesylana jako argument

  String response = LoRaWioE5.readString();
  Serial.println("[debug]Odpowiedz: " + response);
  // Drukuje odpowiedz na komende
}

String give_mess(int i) {
  String message;

  message = String(i) + ". Wiadomosc";
  Serial.print("\n[debug]Wprowadzono: ");
  Serial.println(message);

  return message;
}

String give_bme() {
  String message;

  message = " 53, ";    // temperatura
  message += " 567, ";  // wilgotnosc
  message += " 242, ";  // cisnienie
  message += " 534";    // wysokosc

  Serial.print("\n[debug]Wprowadzono: ");
  Serial.println(message);

  return message;
}

String give_gps() {
  String message = " Bledne dane ";

  message = " 523, ";   // dl geogr
  message += " 523,";   // szer geogr
  message += " 234, ";  // czas
  message += " 654, ";  // wysoksoc
  message += " 4234";   // dokladnosc

  Serial.print("\n[debug]Wprowadzono: ");
  Serial.println(message);

  return message;
}

}  // namespace Lora2