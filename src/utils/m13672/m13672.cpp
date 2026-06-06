#include "m13672/m13672.h"
QVector<double> m13672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
