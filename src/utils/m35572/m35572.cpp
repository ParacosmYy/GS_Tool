#include "m35572/m35572.h"
QVector<double> m35572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
