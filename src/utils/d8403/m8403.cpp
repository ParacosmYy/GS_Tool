#include "d8403/m8403.h"
QVector<double> m8403::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
