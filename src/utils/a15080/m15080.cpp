#include "a15080/m15080.h"
QVector<double> m15080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
