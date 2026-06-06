#include "l25211/m25211.h"
QVector<double> m25211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
