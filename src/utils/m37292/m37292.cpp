#include "m37292/m37292.h"
QVector<double> m37292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
