#include "a27000/m27000.h"
QVector<double> m27000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
