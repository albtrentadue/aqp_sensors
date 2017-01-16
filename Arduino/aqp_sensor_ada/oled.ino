/* OLED DISPLAY */
#ifdef oled
void drawText() {
  // clear the display
  display.clear();
  // create more fonts at http://oleddisplay.squix.ch/
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  String disp_temp = String("Temperatura: ") + temp;
  String disp_hum = String("Umidità: ") + hum;
  //drawString(int16_t x, int16_t y, String text);
  display.drawString(0, 0, disp_temp);
  display.drawString(0, 20, disp_hum);
  display.display();
}
#endif
