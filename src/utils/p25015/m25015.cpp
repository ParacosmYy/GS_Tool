#include "p25015/m25015.h"
QVector<double> m25015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
