#include "m17432/m17432.h"
QVector<double> m17432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
