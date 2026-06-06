#include "p8615/m8615.h"
QVector<double> m8615::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
