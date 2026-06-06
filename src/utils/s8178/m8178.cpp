#include "s8178/m8178.h"
QVector<double> m8178::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
