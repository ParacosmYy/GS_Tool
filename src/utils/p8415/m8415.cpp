#include "p8415/m8415.h"
QVector<double> m8415::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
