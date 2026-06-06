#include "i8228/m8228.h"
QVector<double> m8228::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
