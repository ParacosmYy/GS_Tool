#include "b35001/m35001.h"
QVector<double> m35001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
