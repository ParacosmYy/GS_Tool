#include "a8640/m8640.h"
QVector<double> m8640::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
