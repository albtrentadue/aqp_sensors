/* OLED DISPLAY */
#ifdef oled
void drawText() {
  // clear the display
  display.clear();
  // create more fonts at http://oleddisplay.squix.ch/
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  String disp_temp = String("Temperatura: ") + (DHT_temp/AVG_VALUES);
  String disp_hum = String("Umidità: ") + (DHT_hum/AVG_VALUES);
  String ph = String("PH: 7.6");
  String ossigeno = String("Ossigeno: 6.3 ppm");
  String conducibilita = String("Conducibilità: 641 microS");
  //drawString(int16_t x, int16_t y, String text);
  display.drawString(0, 0, disp_temp);
  display.drawString(0, 12, disp_hum);
  display.drawString(0, 24, ph);
  display.drawString(0, 36, ossigeno);
  display.drawString(0, 48, conducibilita);
  display.display();
}
#endif
