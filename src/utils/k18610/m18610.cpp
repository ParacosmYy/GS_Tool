#include "k18610/m18610.h"
QVector<double> m18610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
