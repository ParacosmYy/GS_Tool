#include "i25228/m25228.h"
QVector<double> m25228::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
