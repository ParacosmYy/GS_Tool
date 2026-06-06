#include "k31030/m31030.h"
QVector<double> m31030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
