#include "f21805/m21805.h"
QVector<double> m21805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
