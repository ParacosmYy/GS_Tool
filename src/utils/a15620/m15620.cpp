#include "a15620/m15620.h"
QVector<double> m15620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
