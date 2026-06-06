#include "h8287/m8287.h"
QVector<double> m8287::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
