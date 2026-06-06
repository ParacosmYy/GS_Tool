#include "t31979/m31979.h"
QVector<double> m31979::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
