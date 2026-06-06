#include "f10385/m10385.h"
QVector<double> m10385::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
