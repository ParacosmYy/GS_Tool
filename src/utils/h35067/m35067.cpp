#include "h35067/m35067.h"
QVector<double> m35067::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
