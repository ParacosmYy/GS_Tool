#include "f34025/m34025.h"
QVector<double> m34025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
