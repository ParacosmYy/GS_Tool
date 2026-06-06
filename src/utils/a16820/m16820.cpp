#include "a16820/m16820.h"
QVector<double> m16820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
