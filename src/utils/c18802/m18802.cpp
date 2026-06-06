#include "c18802/m18802.h"
QVector<double> m18802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
