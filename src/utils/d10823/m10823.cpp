#include "d10823/m10823.h"
QVector<double> m10823::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
