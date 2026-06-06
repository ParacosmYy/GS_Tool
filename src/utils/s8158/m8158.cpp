#include "s8158/m8158.h"
QVector<double> m8158::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
