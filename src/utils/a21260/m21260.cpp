#include "a21260/m21260.h"
QVector<double> m21260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
