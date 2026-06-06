#include "h16947/m16947.h"
QVector<double> m16947::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
