#include "g16526/m16526.h"
QVector<double> m16526::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
