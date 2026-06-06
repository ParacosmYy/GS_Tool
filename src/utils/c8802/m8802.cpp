#include "c8802/m8802.h"
QVector<double> m8802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
