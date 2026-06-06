#include "m21732/m21732.h"
QVector<double> m21732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
