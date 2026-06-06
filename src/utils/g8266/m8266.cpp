#include "g8266/m8266.h"
QVector<double> m8266::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
