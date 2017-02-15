/* OLED DISPLAY */
#ifdef oled
void drawText() {
  // clear the display
  display.clear();
  // create more fonts at http://oleddisplay.squix.ch/
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  String disp_temp = String("Temp: ") + (DHT_temp / AVG_VALUES);
  String disp_hum = String("Umidità: ") + (DHT_hum/AVG_VALUES);
  String ph = String("pH: ") + (ORP_value / AVG_VALUES);
  String ossigeno = String("Ossigeno: ") + (DO_value / AVG_VALUES);
  //String conducibilita = String("Conducibilità: 641 microS");
  //drawString(int16_t x, int16_t y, String text);
  display.drawString(0, 0, disp_temp);
  display.drawString(0, 16, disp_hum);
  display.drawString(0, 32, ph);
  display.drawString(0, 48, ossigeno);
  //display.drawString(0, 36, conducibilita);
  display.display();
}
#endif
