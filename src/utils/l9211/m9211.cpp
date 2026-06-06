#include "l9211/m9211.h"
QVector<double> m9211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
