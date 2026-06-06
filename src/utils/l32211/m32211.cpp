#include "l32211/m32211.h"
QVector<double> m32211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
