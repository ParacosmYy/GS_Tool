#include "l15211/m15211.h"
QVector<double> m15211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
