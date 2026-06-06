#include "t31579/m31579.h"
QVector<double> m31579::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
