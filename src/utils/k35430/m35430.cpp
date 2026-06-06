#include "k35430/m35430.h"
QVector<double> m35430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
