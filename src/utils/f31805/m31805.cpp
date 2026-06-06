#include "f31805/m31805.h"
QVector<double> m31805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
