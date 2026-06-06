#include "k11550/m11550.h"
QVector<double> m11550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
