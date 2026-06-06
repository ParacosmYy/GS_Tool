#include "k31910/m31910.h"
QVector<double> m31910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
