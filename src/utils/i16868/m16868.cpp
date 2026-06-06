#include "i16868/m16868.h"
QVector<double> m16868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
