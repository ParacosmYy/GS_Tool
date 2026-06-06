#include "a18260/m18260.h"
QVector<double> m18260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
