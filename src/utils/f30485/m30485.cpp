#include "f30485/m30485.h"
QVector<double> m30485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
