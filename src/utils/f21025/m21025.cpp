#include "f21025/m21025.h"
QVector<double> m21025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
