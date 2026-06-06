#include "l18431/m18431.h"
QVector<double> m18431::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
