#include "a21000/m21000.h"
QVector<double> m21000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
