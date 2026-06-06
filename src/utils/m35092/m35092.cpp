#include "m35092/m35092.h"
QVector<double> m35092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
