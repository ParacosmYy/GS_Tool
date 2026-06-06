#include "e16364/m16364.h"
QVector<double> m16364::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
