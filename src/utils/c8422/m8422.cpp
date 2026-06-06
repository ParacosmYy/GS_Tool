#include "c8422/m8422.h"
QVector<double> m8422::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
