#include "l8251/m8251.h"
QVector<double> m8251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
