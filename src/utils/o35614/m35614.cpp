#include "o35614/m35614.h"
QVector<double> m35614::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
