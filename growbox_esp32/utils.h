#ifndef growbox_utils_h
#define growbox_utils_h

String floatToStr(float val)
{
  char buf[50];
  dtostrf(val, -1, 1, buf);
  return buf;
}

#endif