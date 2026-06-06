#include "a25920/m25920.h"
QVector<double> m25920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
