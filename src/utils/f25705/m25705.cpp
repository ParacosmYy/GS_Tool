#include "f25705/m25705.h"
QVector<double> m25705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
