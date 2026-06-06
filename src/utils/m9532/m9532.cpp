#include "m9532/m9532.h"
QVector<double> m9532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
