#include "b16501/m16501.h"
QVector<double> m16501::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
