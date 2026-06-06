#include "s35718/m35718.h"
QVector<double> m35718::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
