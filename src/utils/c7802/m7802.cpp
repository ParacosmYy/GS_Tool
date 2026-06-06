#include "c7802/m7802.h"
QVector<double> m7802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
