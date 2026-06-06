#include "a25560/m25560.h"
QVector<double> m25560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
