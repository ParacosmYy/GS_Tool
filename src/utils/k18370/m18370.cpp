#include "k18370/m18370.h"
QVector<double> m18370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
