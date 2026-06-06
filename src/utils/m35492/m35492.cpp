#include "m35492/m35492.h"
QVector<double> m35492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
