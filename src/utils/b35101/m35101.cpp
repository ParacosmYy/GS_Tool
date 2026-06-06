#include "b35101/m35101.h"
QVector<double> m35101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
