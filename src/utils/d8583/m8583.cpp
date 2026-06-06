#include "d8583/m8583.h"
QVector<double> m8583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
