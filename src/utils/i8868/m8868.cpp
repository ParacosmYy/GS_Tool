#include "i8868/m8868.h"
QVector<double> m8868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
