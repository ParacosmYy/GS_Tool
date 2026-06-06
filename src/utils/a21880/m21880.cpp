#include "a21880/m21880.h"
QVector<double> m21880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
