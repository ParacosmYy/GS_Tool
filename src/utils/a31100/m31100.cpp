#include "a31100/m31100.h"
QVector<double> m31100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
