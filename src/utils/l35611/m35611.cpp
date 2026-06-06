#include "l35611/m35611.h"
QVector<double> m35611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
